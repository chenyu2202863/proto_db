#include "sql_error.h"
#include <vector>

namespace sql {

std::basic_string<TCHAR> format_string(const TCHAR* format, ...)
{
	va_list vars;
	va_start(vars, format);
	int size = _vsctprintf(format, vars);
	std::vector<TCHAR> format_buffer(size + 1);
    _vstprintf_s(&format_buffer[0], format_buffer.size(), format, vars);
	va_end(vars);

	return std::basic_string<TCHAR>(&format_buffer[0], size);
}

ErrorHandlerCallback& error_handler_instance() {
    static ErrorHandlerCallback handler_;
    return handler_;
}

void set_error_handler(const ErrorHandlerCallback& handler) {
    ErrorHandlerCallback& handler_instance = error_handler_instance();
    handler_instance = handler;
}

std::basic_string<TCHAR> get_error_info(_ConnectionPtr& connection_ptr) {
    std::basic_string<TCHAR> output;
    if(connection_ptr->Errors->Count > 0) {
        long count = connection_ptr->Errors->Count;
        for(long i = 0; i < count; i++) {
            ErrorPtr error_ptr = connection_ptr->Errors->GetItem(i);
            output.append(format_string(_T("\tError number: %x\t%s\n"), 
                            error_ptr->Number, (const TCHAR*)error_ptr->Description));
        }
    }

    return output;
}

void notify_error(const _com_error& e) {
    HRESULT hr = e.Error();
    _bstr_t info = e.Description();
    if (!info) {
        return;
    }
    
    std::basic_string<TCHAR> str((TCHAR *)info);
    ErrorHandlerCallback& handler = error_handler_instance();
    if (handler != nullptr) {
        handler(hr, str);
    }
}

void notify_error(_ConnectionPtr& connection_ptr) {
    std::basic_string<TCHAR> info = get_error_info(connection_ptr);
    if (!info.empty()) {
        ErrorHandlerCallback& handler = error_handler_instance();
        if (handler != nullptr) {
            handler(0, info);
        }
    }
}

} // namespace sql