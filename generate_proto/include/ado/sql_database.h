#ifndef SQL_DATABASE_H_
#define SQL_DATABASE_H_

#include <Windows.h>
#include <tchar.h>
#include <string>
#include <vector>
#include "sql_recordset.h"
#include "sql_prepare.h"


namespace sql {

const std::basic_string<TCHAR> kCharacterSetUtf8(_T("utf8"));
const std::basic_string<TCHAR> kCharacterSetGBK(_T("gbk"));

class Database
{
public:
    Database();
    ~Database();

    bool open(const std::basic_string<TCHAR>& host,
              unsigned short port,
              const std::basic_string<TCHAR>& database_name,
              const std::basic_string<TCHAR>& user_name, 
              const std::basic_string<TCHAR>& password,
              long timeout, // 单位为秒
              const std::basic_string<TCHAR>& characterSet = kCharacterSetGBK);

    void close();

    //  检查数据库是否有效
    bool valid();

    HRESULT get_last_error() const;

    record_set_ptr execute_sql(const PrepareStatement& statement);

    //  执行SQL语句
    record_set_ptr execute_sql(const std::basic_string<TCHAR>& statement);

    //  创建存储过程的参数
    _ParameterPtr create_parameter(const std::basic_string<TCHAR>& name,
                                   DataTypeEnum data_type,
                                   ParameterDirectionEnum direction_type,
                                   int size,
                                   const _variant_t& value);
    //  执行存储过程
    record_set_ptr execute_procedure(const std::basic_string<TCHAR>& procedure_name,
                                     const std::vector<_ParameterPtr>& parameters);

    bool begin_transcation();

    bool commit_transcation();
    
    bool rollback_transcation();

    _ConnectionPtr native_connection();

private:
    void set_last_error(HRESULT hr);
    void set_last_error(const _com_error& e);

private:
	bool in_transcation_;
    DWORD last_error_tls_;
    _ConnectionPtr	connect_ptr_;
};

typedef Database SqlDatabase;

}  // namespace sql

#endif  // SQL_DATABASE_H_