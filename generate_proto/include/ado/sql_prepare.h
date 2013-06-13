#ifndef SQL_PREPARE_H_
#define SQL_PREPARE_H_

#include <string>
#include <windows.h>
#include "sql_helper.h"

namespace sql {
class PrepareStatement {
public:
    explicit PrepareStatement(const std::basic_string<TCHAR>& sql_statement)
        : sql_statement_(sql_statement) {

    }

    ~PrepareStatement() {

    }

    void set(const std::basic_string<TCHAR>& value_name, const std::basic_string<TCHAR>& value) {
        replace(sql_statement_, value_name, value);
    }

private:
    friend class Database;
    const std::basic_string<TCHAR>& get_sql_statement() const {
        return sql_statement_;
    }

    std::basic_string<TCHAR> sql_statement_;
};

}  // namespace sql

#endif  // SQL_PREPARE_H_