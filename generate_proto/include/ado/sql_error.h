#ifndef SQL_ERROR_H_
#define SQL_ERROR_H_


#include <Windows.h>
#include <tchar.h>
#include <comdef.h>
#include <string>
#include <functional>
#import "./msado15.dll" no_namespace rename("EOF","adoEOF")

namespace sql {

typedef std::function<void(HRESULT hr, const std::basic_string<TCHAR>&)> ErrorHandlerCallback;

//  非线程安全函数
void set_error_handler(const ErrorHandlerCallback& handler);

std::basic_string<TCHAR> get_error_info(_ConnectionPtr& connection_ptr);

//
//  private:
//
void notify_error(const _com_error& e);

void notify_error(_ConnectionPtr& connection_ptr);

}  // namespace sql

#endif  // SQL_ERROR_H_