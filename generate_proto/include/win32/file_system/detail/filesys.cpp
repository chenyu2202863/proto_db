// filesys.cpp -- filesystem support functions
#include <yvals.h>
#include <Windows.h>
#include <crtdbg.h>
#include <string>
#include <direct.h>
#include <io.h>

 #if _HAS_CPP0X
#include "../filesystem.hpp"

 #if __EDG__
#pragma diag_suppress=161	/* unknown pragma */
#pragma diag_suppress=174	/* expression has no effect */
#pragma diag_suppress=177	/* variable declared but never references */
#pragma diag_suppress=748	/* calling convention specified more than once */
#pragma diag_suppress=940	/* missing return statement */
 #endif /* __EDG__ */

//#include <wrapwin.h>

namespace win32 {
	namespace file {
static file_type _Map_mode(int _Mode)
	{	// map Windows file attributes to file_status
	if ((_Mode & FILE_ATTRIBUTE_DIRECTORY) != 0)
		return (directory_file);
	else if ((_Mode & FILE_ATTRIBUTE_NORMAL) != 0)
		return (regular_file);
	else
		return (type_unknown);
	}

void __CLRCALL_PURE_OR_CDECL _Close_dir(void *_Handle)
	{	// close a directory
	FindClose((HANDLE)_Handle);
	}

inline bool __crtIsTailoredApp()
{
	return false;
}


 #include <direct.h>
 #include <io.h>
 
 
 #include "filesys.h"
 #define FILESYS_WIDE	1
 #include "filesys.h"

	}	//namespace 
}	// namespace 

 #endif /* _HAS_CPP0X */

/*
 * Copyright (c) 1992-2011 by P.J. Plauger.  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
V5.40:0009 */
