#include "sql_database.h"
#include <assert.h>
#include "sql_error.h"
#include "sql_helper.h"


namespace sql {

Database::Database() : in_transcation_(false) {
    last_error_tls_ = TlsAlloc();
}

Database::~Database() {
    TlsFree(last_error_tls_);
}

void Database::set_last_error(const _com_error& e) {
    set_last_error(e.Error());
    notify_error(e);
}

void Database::set_last_error(HRESULT hr) {
    TlsSetValue(last_error_tls_, reinterpret_cast<LPVOID>(hr));
}

HRESULT Database::get_last_error() const {
    HRESULT hr = reinterpret_cast<HRESULT>(TlsGetValue(last_error_tls_));
    DWORD win32_last_error = GetLastError();
    if (hr == 0 && win32_last_error != ERROR_SUCCESS) {
        hr = HRESULT_FROM_WIN32(win32_last_error);
    }

    return hr;
}

bool Database::open(
    const std::basic_string<TCHAR>& host,
    unsigned short port,
    const std::basic_string<TCHAR>& database_name,
    const std::basic_string<TCHAR>& user_name, 
    const std::basic_string<TCHAR>& password,
    long timeout,
    const std::basic_string<TCHAR>& CharacterSet) 
{


    static const std::basic_string<TCHAR> kConnectStr(_T("Provider=MSDASQL;DRIVER={MySQL ODBC 5.1 Driver};")
                                                      _T("UID=%s;Password=%s;")
                                                      _T("DATABASE=%s;")
													  _T("SERVER=%s;")
													  _T("PORT=%d"));
	//Driver=MySQL ODBC 5.2 Unicode Driver;
	//TCHAR connect_string[] = {_T("Provider=MSDASQL;DRIVER={MySQL ODBC 5.1 Driver};SERVER=192.168.1.2;UID=chengyu;Password=123;DATABASE=gamedb;PORT=3306;Option=3;")};
	 TCHAR connect_string[512] = {0};
	_stprintf_s<_countof(connect_string)>(connect_string, kConnectStr.c_str(),
		user_name.c_str(), password.c_str(),
		database_name.c_str(), host.c_str(), port);
    HRESULT hr = S_OK;
    try {
        set_last_error(S_OK);
        hr = connect_ptr_.CreateInstance("ADODB.Connection");
        set_last_error(hr);
        if (SUCCEEDED(hr)) {
            connect_ptr_->ConnectionTimeout = timeout;
            hr = connect_ptr_->Open(connect_string, "", "",adModeUnknown);
            set_last_error(hr);
            return SUCCEEDED(hr);
        } else {
            _com_issue_error(hr);
        }

    } catch(_com_error &e) {
        set_last_error(e);
    }

    return false; 
}

void Database::close() {
    try {
        set_last_error(S_OK);
        if (connect_ptr_!=NULL) {
            connect_ptr_->Close();
            connect_ptr_.Release();
            connect_ptr_ = NULL;
        }
    } catch (_com_error &e) {
        set_last_error(e);
    }
}

//  检查数据库是否有效
bool Database::valid() {
	assert(is_open(connect_ptr_));
    if (!is_open(connect_ptr_)) 
        return false;
    record_set_ptr set_ptr = execute_sql(_T("select 1;"));
    return set_ptr != NULL;
}

record_set_ptr Database::execute_sql(const std::basic_string<TCHAR>& statement) {
	assert(is_open(connect_ptr_));
    if (!is_open(connect_ptr_)) {
        set_last_error(S_FALSE);
        return record_set_ptr();
    }

    set_last_error(S_OK);
    try {
        _CommandPtr command;
        HRESULT hr = command.CreateInstance(__uuidof(Command));
        if (FAILED(hr)) {
            _com_issue_error(hr);
            return record_set_ptr();
        }

        command->PutCommandText(_bstr_t(statement.c_str()));
        command->PutCommandType(adCmdText);
        command->ActiveConnection = connect_ptr_; 

        _RecordsetPtr recordset;
        hr = recordset.CreateInstance(__uuidof(Recordset));
        if (FAILED(hr)) {
            _com_issue_error(hr);
            return record_set_ptr();
        }

        recordset->CursorLocation = adUseClient;
        hr = recordset->Open(static_cast<IDispatch *>(command), vtMissing, adOpenStatic, adLockBatchOptimistic, adOptionUnspecified);
        set_last_error(hr);
        if (SUCCEEDED(hr)) {
            return record_set_ptr(new RecordSet(recordset, command));
        }
    } catch (_com_error& e) {
        set_last_error(e);
    }

    return record_set_ptr();
}

record_set_ptr Database::execute_sql(const PrepareStatement& statement) {
    const std::basic_string<TCHAR> sql_statement = statement.get_sql_statement();
    return execute_sql(sql_statement);
}

_ParameterPtr Database::create_parameter(const std::basic_string<TCHAR>& name, 
                                            DataTypeEnum data_type, 
                                            ParameterDirectionEnum direction_type, 
                                            int size, 
                                            const _variant_t& value) {
    try {
        set_last_error(S_OK);
        _ParameterPtr parameter;
        HRESULT hr = parameter.CreateInstance(__uuidof(Parameter));
        set_last_error(hr);
        if (SUCCEEDED(hr)) {
            parameter->PutName(_bstr_t(name.c_str()));
            parameter->PutType(data_type);
            parameter->PutDirection(direction_type);
            parameter->PutSize(size);
            parameter->PutValue(value);
            return parameter;
        } else {
            _com_issue_error(hr);
        }
    }
    catch (_com_error& e) {
        set_last_error(e);
    }

    return _ParameterPtr();
}

record_set_ptr Database::execute_procedure(const std::basic_string<TCHAR>& procedure_name, 
                                           const std::vector<_ParameterPtr>& parameters)
{
    assert(is_open(connect_ptr_));
    if (!is_open(connect_ptr_)) {
        set_last_error(S_FALSE);
        return record_set_ptr();
    }

    try {
        set_last_error(S_OK);
        _CommandPtr command;
        HRESULT hr = command.CreateInstance(__uuidof(Command));
        set_last_error(hr);
        if (SUCCEEDED(hr)) {
            command->PutCommandText(_bstr_t(procedure_name.c_str()));
            command->PutCommandType(adCmdStoredProc);
            command->ActiveConnection = connect_ptr_;
            std::vector<_ParameterPtr>::const_iterator iter = parameters.begin();
            for (; iter != parameters.end(); iter++) {
                const _ParameterPtr& param_ptr = *iter;
                _ParameterPtr new_param_ptr= command->CreateParameter(
                                                param_ptr->GetName(), param_ptr->GetType(), 
                                                param_ptr->GetDirection(), param_ptr->GetSize(),
                                                param_ptr->GetValue());
                command->Parameters->Append(new_param_ptr);
            }

            _RecordsetPtr recordset;
            hr = recordset.CreateInstance(__uuidof(Recordset));
            if (FAILED(hr)) {
                set_last_error(hr);
                return record_set_ptr();
            }

            recordset->CursorLocation = adUseClient;
            hr = recordset->Open(static_cast<IDispatch *>(command), vtMissing, 
                adOpenStatic, adLockBatchOptimistic, adOptionUnspecified);
            set_last_error(hr);
            if (SUCCEEDED(hr)) {
                return record_set_ptr(new RecordSet(recordset, command)); 
            } else {
                _com_issue_error(hr);
            }
        } else {
            _com_issue_error(hr);
            return record_set_ptr();
        }

    } catch (_com_error& e) {
        set_last_error(e);
    }

    return record_set_ptr();
}

bool Database::begin_transcation() {
	assert(is_open(connect_ptr_));
	assert(!in_transcation_);
    if (!is_open(connect_ptr_) || in_transcation_) {
        set_last_error(S_FALSE);
        return false;
    }

    try
    {
        set_last_error(S_OK);
        HRESULT hr = connect_ptr_->BeginTrans();
		set_last_error(hr);
		if (SUCCEEDED(hr)) {
			in_transcation_ = true;
			return true;
		}
    }
    catch (_com_error& e) {
        set_last_error(e);
    }
    
    return false;
}

bool Database::commit_transcation() {
	assert(is_open(connect_ptr_));
	assert(in_transcation_);
    if (!is_open(connect_ptr_) || !in_transcation_) {
        set_last_error(S_FALSE);
        return false;
    }

    try
    {
        set_last_error(S_OK);
        HRESULT hr = connect_ptr_->CommitTrans();
        set_last_error(hr);
        if (SUCCEEDED(hr)) {
			in_transcation_ = false;
			return true;
		}
    }
    catch (_com_error& e) {
        set_last_error(e);
    }

    return false;
}

bool Database::rollback_transcation() {
	assert(is_open(connect_ptr_));
	assert(in_transcation_);
    if (!is_open(connect_ptr_) || !in_transcation_) {
        set_last_error(S_FALSE);
        return false;
    }

    try
    {
        set_last_error(S_OK);
        HRESULT hr = connect_ptr_->RollbackTrans();
        set_last_error(hr);
        if (SUCCEEDED(hr)) {
			in_transcation_ = false;
			return true;
		}
    }
    catch (_com_error& e) {
        set_last_error(e);
    }

    return false;
}

_ConnectionPtr Database::native_connection() {
    return connect_ptr_;
}

}  // namespace sql