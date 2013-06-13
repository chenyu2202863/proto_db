#ifndef __EXTEND_STL_UNICODE_HPP
#define __EXTEND_STL_UNICODE_HPP

#include <string>
#include <fstream>
#include <sstream>
#include <strstream>
#include <iostream>
#include <regex>


namespace stdex
{
#if defined UNICODE || defined _UNICODE
 
	typedef wchar_t				 tChar;
	typedef std::wstring         tString;
 
    typedef std::wstringbuf      tStringbuf;
    typedef std::wstringstream   tStringstream;
    typedef std::wostringstream  tOstringstream;
    typedef std::wistringstream  tIstringstream;
 
    typedef std::wstreambuf      tStreambuf;
 
    typedef std::wistream        tIstream;
    typedef std::wiostream       tIostream;
 
    typedef std::wostream        tOstream;
 
    typedef std::wfilebuf        tFilebuf;
    typedef std::wfstream        tFstream;
    typedef std::wifstream       tIfstream;
    typedef std::wofstream       tOfstream;
 
    typedef std::wios            tIos;
	typedef std::tr1::wregex	 tRegex;	
 
#   define tCerr            wcerr
#   define tCin             wcin
#   define tClog            wclog
#   define tCout            wcout
 
#else // defined UNICODE || defined _UNICODE
 
	typedef char				 tChar;
    typedef std::string          tString;
 
    typedef std::stringbuf       tStringbuf;
    typedef std::stringstream    tStringstream;
    typedef std::ostringstream   tOstringstream;
    typedef std::istringstream   tIstringstream;
 
    typedef std::streambuf       tStreambuf;
 
    typedef std::istream         tIstream;
    typedef std::iostream        tIostream;
 
    typedef std::ostream         tOstream;
 
    typedef std::filebuf         tFilebuf;
    typedef std::fstream         tFstream;
    typedef std::ifstream        tIfstream;
    typedef std::ofstream        tOfstream;
 
    typedef std::ios             tIos;

	typedef std::tr1::regex		 tRegex;
 
#   define tCerr            cerr
#   define tCin             cin
#   define tClog            clog
#   define tCout            cout
 
#endif // defined UNICODE || defined _UNICODE
} // namespace stdex


#endif