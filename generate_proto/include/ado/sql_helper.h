#ifndef SQL_HELPER_H_
#define SQL_HELPER_H_

#include <tchar.h>
#include <string>

namespace sql {
template < typename AdoT >
bool is_open(const AdoT &type) {
    return type && type->GetState() == adStateOpen;
}

template<typename T>
void replace(std::basic_string<T>& source, 
             const std::basic_string<T>& pszToReplace, 
             const std::basic_string<T>& pszReplace) {
    std::basic_string<T>::size_type nIndex = 0;

    while ( (nIndex = source.find(pszToReplace, nIndex)) != std::basic_string<T>::npos) {
        source.replace(nIndex, pszToReplace.length(), pszReplace);
        nIndex += pszReplace.length();
    }
}

}  // namespace sql

#endif  // SQL_HELPER_H_