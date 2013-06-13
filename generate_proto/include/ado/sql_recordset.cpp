#include "sql_recordset.h"
#include "sql_error.h"
#include "sql_helper.h"

namespace sql {

RecordSet::RecordSet(_RecordsetPtr& record_set, _CommandPtr& command_ptr)
    : record_set_ptr_(record_set)
    , command_ptr_(command_ptr)
    , hr_(S_OK) {
}

RecordSet::~RecordSet() {
    if (record_set_ptr_ != NULL) {
        if (is_open(record_set_ptr_)) { 
            record_set_ptr_->Close();
        }
    }
}

HRESULT RecordSet::get_last_error() const {
    return hr_;
}

void RecordSet::set_last_error(HRESULT hr) {
    hr_ = hr;
}

void RecordSet::set_last_error(const _com_error& e) {
    set_last_error(e.Error());
    notify_error(e);
}

bool RecordSet::is_eof() {
    return (record_set_ptr_->adoEOF == VARIANT_TRUE);
}

bool RecordSet::is_empty() {
    ADO_LONGPTR count = 0;
    try {
        set_last_error(S_OK);
        count = record_set_ptr_->GetRecordCount();
    } catch(_com_error& e) {
        set_last_error(e);
    }

    return count == 0;
}

bool RecordSet::move_next() {

    HRESULT hr = S_OK;
    try
    {
        set_last_error(S_OK);
        hr = record_set_ptr_->MoveNext();
        if (SUCCEEDED(hr)) {
            set_last_error(hr);
        } else {
            _com_issue_error(hr);
        }
    } catch (_com_error& e) {
        set_last_error(e);
    }
    
    return SUCCEEDED(hr);
}

bool RecordSet::get_field_value(const std::basic_string<TCHAR>& name, _variant_t& value) {
    try {
        set_last_error(S_OK);
        value = record_set_ptr_->GetCollect(name.c_str());
    } catch (_com_error& e) {
        set_last_error(e);
        return false;
    }

    return true;
}

bool RecordSet::get_param_value(const std::basic_string<TCHAR>& param_name, _variant_t& value) {
    try {
        set_last_error(S_OK);
        value = command_ptr_->Parameters->GetItem(_variant_t(_bstr_t(param_name.c_str())))->Value;
        
    } catch (_com_error& e) {
        set_last_error(e);
        return false;
    }

    return true;
}

_variant_t RecordSet::get_field_value(const std::basic_string<TCHAR>& name) {
    _variant_t value;
    try {
        set_last_error(S_OK);
        value = record_set_ptr_->GetCollect(name.c_str());
    } catch (_com_error& e) {
        set_last_error(e);
    }

    return _variant_t(value);
}

_variant_t RecordSet::get_param_value(const std::basic_string<TCHAR>& param_name) {
    _variant_t value;
    try {
        set_last_error(S_OK);
        value = command_ptr_->Parameters->GetItem(_variant_t(_bstr_t(param_name.c_str())))->Value;

    } catch (_com_error& e) {
        set_last_error(e);
    }

    return _variant_t(value);
}

_RecordsetPtr RecordSet::native_record_set() {
    return record_set_ptr_;
}

long RecordSet::get_field_count() {
    try {
        set_last_error(S_OK);
        FieldsPtr fields_ptr = record_set_ptr_->GetFields();
        if (fields_ptr != nullptr) 
            return fields_ptr->GetCount();
    } catch (_com_error& e) {
    	set_last_error(e);
    }

    return 0;
}

std::basic_string<TCHAR> RecordSet::get_field_name(long index) {
    try {
        set_last_error(S_OK);
        FieldsPtr fields_ptr = record_set_ptr_->GetFields();
        if (fields_ptr != nullptr) {
            FieldPtr field_ptr = fields_ptr->GetItem(index);
            if (field_ptr != nullptr) {
                _bstr_t str = field_ptr->GetName();
                return std::basic_string<TCHAR>((const TCHAR*)str);
            }
        }
    } catch (_com_error& e) {
        set_last_error(e);
    }

    return std::basic_string<TCHAR>(_T(""));
}

}  // namespace sql