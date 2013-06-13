#ifndef __WIN32_FILESYSTEM_FIND_FILE_HPP
#define __WIN32_FILESYSTEM_FIND_FILE_HPP



#include <cassert>
#include <tchar.h>
#include "../../extend_stl/unicode.hpp"



/*
文件目录遍历
	find_file

*/
namespace win32
{


	namespace filesystem
	{
		// --------------------------------------------
		// class 

		// 检索文件

		class find_file
		{
		public:
			// Data members
			WIN32_FIND_DATA fd_;
			TCHAR root_[MAX_PATH];
			TCHAR dir_separator_;
			HANDLE find_;
			BOOL founded_;

			// Constructor/destructor
			find_file() 
				: find_(NULL)
				, dir_separator_(_T('\\'))
				, founded_(FALSE)
			{ }

			~find_file()
			{
				close();
			}

			// Attributes
			ULONGLONG get_file_size() const
			{
				assert(find_ != NULL);

				ULARGE_INTEGER nFileSize = { 0 };

				if(founded_)
				{
					nFileSize.LowPart = fd_.nFileSizeLow;
					nFileSize.HighPart = fd_.nFileSizeHigh;
				}
				else
				{
					nFileSize.QuadPart = 0;
				}

				return nFileSize.QuadPart;
			}

			bool get_file_name(LPTSTR lpstrFileName, int cchLength) const
			{
				assert(find_ != NULL);
				if(lstrlen(fd_.cFileName) >= cchLength)
					return false;

				if(founded_)
					_tcscpy_s(lpstrFileName, cchLength, fd_.cFileName);

				return founded_ == TRUE;
			}

			bool get_file_path(LPTSTR lpstrFilePath, int cchLength) const
			{
				assert(find_ != NULL);

				int nLen = lstrlen(root_);

				assert(nLen > 0);
				if(nLen == 0)
					return false;

				bool bAddSep = (root_[nLen - 1] != _T('\\') && root_[nLen - 1] !=_T('/'));

				if((lstrlen(root_) + (bAddSep ?  1 : 0)) >= cchLength)
					return false;

				_tcscpy_s(lpstrFilePath, cchLength, root_);

				if(bAddSep)
				{
					TCHAR szSeparator[2] = { dir_separator_, 0 };
					_tcscat_s(lpstrFilePath, cchLength, szSeparator);
				}

				_tcscat_s(lpstrFilePath, cchLength, fd_.cFileName);

				return true;
			}


			bool get_file_title(LPTSTR lpstrFileTitle, int cchLength) const
			{
				assert(find_ != NULL);

				TCHAR szBuff[MAX_PATH] = { 0 };
				if(!get_file_name(szBuff, MAX_PATH))
					return false;

				if(lstrlen(szBuff) >= cchLength || cchLength < 1)
					return false;

				// find the last dot
				LPTSTR pstrDot  = (LPTSTR)_cstrrchr(szBuff, _T('.'));
				if(pstrDot != NULL)
					*pstrDot = 0;

				_tcscpy_s(lpstrFileTitle, cchLength, szBuff);

				return true;
			}

			bool get_file_url(LPTSTR lpstrFileURL, int cchLength) const
			{
				assert(find_ != NULL);

				TCHAR szBuff[MAX_PATH] = { 0 };
				if(!get_file_path(szBuff, MAX_PATH))
					return false;

				LPCTSTR lpstrFileURLPrefix = _T("file://");
				if(lstrlen(szBuff) + lstrlen(lpstrFileURLPrefix) >= cchLength)
					return false;
				_tcscpy_s(lpstrFileURL, cchLength, lpstrFileURLPrefix);
				_tcscat_s(lpstrFileURL, cchLength, szBuff);

				return true;
			}

			bool get_root(LPTSTR lpstrRoot, int cchLength) const
			{
				assert(find_ != NULL);
				if(lstrlen(root_) >= cchLength)
					return false;

				_tcscpy_s(lpstrRoot, cchLength, root_);

				return true;
			}


			stdex::tString get_file_name() const
			{
				assert(find_ != NULL);

				stdex::tString ret;

				if(founded_)
					ret = fd_.cFileName;
				return ret;
			}

			stdex::tString get_file_path() const
			{
				assert(find_ != NULL);

				stdex::tString strResult = root_;
				int nLen = strResult.length();

				assert(nLen > 0);
				if(nLen == 0)
					return strResult;

				if((strResult[nLen - 1] != _T('\\')) && (strResult[nLen - 1] != _T('/')))

					strResult += dir_separator_;
				strResult += get_file_name();
				return strResult;
			}


			stdex::tString get_file_title() const
			{
				assert(find_ != NULL);

				TCHAR fileTitle[MAX_PATH] = {0};
				get_file_title(fileTitle, MAX_PATH);

				return fileTitle;
			}


			stdex::tString get_file_url() const
			{
				assert(find_ != NULL);

				stdex::tString strResult(_T("file://"));
				strResult += get_file_path();
				return strResult;
			}

			stdex::tString get_root() const
			{
				assert(find_ != NULL);

				stdex::tString str = root_;
				return str;
			}

			bool get_last_write_time(FILETIME* pTimeStamp) const
			{
				assert(find_ != NULL);
				assert(pTimeStamp != NULL);

				if(founded_ && pTimeStamp != NULL)
				{
					*pTimeStamp = fd_.ftLastWriteTime;
					return true;
				}

				return false;
			}

			bool get_last_access_time(FILETIME* pTimeStamp) const
			{
				assert(find_ != NULL);
				assert(pTimeStamp != NULL);

				if(founded_ && pTimeStamp != NULL)
				{
					*pTimeStamp = fd_.ftLastAccessTime;
					return true;
				}

				return false;
			}

			bool get_creation_time(FILETIME* pTimeStamp) const
			{
				assert(find_ != NULL);

				if(founded_ && pTimeStamp != NULL)
				{
					*pTimeStamp = fd_.ftCreationTime;
					return true;
				}

				return false;
			}

			bool matches_mask(DWORD dwMask) const
			{
				assert(find_ != NULL);

				if(founded_)
					return ((fd_.dwFileAttributes & dwMask) != 0);

				return false;
			}

			bool is_dots() const
			{
				assert(find_ != NULL);

				// return TRUE if the file name is "." or ".." and
				// the file is a directory

				bool bResult = false;
				if(founded_ && is_directory())
				{
					if(fd_.cFileName[0] == _T('.') && (fd_.cFileName[1] == _T('\0') || (fd_.cFileName[1] == _T('.') && fd_.cFileName[2] == _T('\0'))))
						bResult = true;
				}

				return bResult;
			}

			bool is_readonly() const
			{
				return matches_mask(FILE_ATTRIBUTE_READONLY);
			}

			bool is_directory() const
			{
				return matches_mask(FILE_ATTRIBUTE_DIRECTORY);
			}

			bool is_compressed() const
			{
				return matches_mask(FILE_ATTRIBUTE_COMPRESSED);
			}

			bool is_system() const
			{
				return matches_mask(FILE_ATTRIBUTE_SYSTEM);
			}

			bool is_hidden() const
			{
				return matches_mask(FILE_ATTRIBUTE_HIDDEN);
			}

			bool is_temporay() const
			{
				return matches_mask(FILE_ATTRIBUTE_TEMPORARY);
			}

			bool is_normal() const
			{
				return matches_mask(FILE_ATTRIBUTE_NORMAL);
			}

			bool is_archived() const
			{
				return matches_mask(FILE_ATTRIBUTE_ARCHIVE);
			}

			// Operations
			bool find(LPCTSTR pstrName = NULL)
			{
				close();

				if(pstrName == NULL)
				{
					pstrName = _T("*.*");
				}
				else if(lstrlen(pstrName) >= MAX_PATH)
				{
					assert(false);
					return false;
				}

				_tcscpy_s(fd_.cFileName, _countof(fd_.cFileName), pstrName);

				find_ = ::FindFirstFile(pstrName, &fd_);

				if(find_ == INVALID_HANDLE_VALUE)
					return false;

				bool bFullPath = (::GetFullPathName(pstrName, MAX_PATH, root_, NULL) != 0);


				// passed name isn't a valid path but was found by the API
				assert(bFullPath);
				if(!bFullPath)
				{
					close();
					::SetLastError(ERROR_INVALID_NAME);
					return false;
				}
				else
				{
					// find the last forward or backward whack
					LPTSTR pstrBack  = (LPTSTR)_cstrrchr(root_, _T('\\'));
					LPTSTR pstrFront = (LPTSTR)_cstrrchr(root_, _T('/'));

					if(pstrFront != NULL || pstrBack != NULL)
					{
						if(pstrFront == NULL)
							pstrFront = root_;
						if(pstrBack == NULL)
							pstrBack = root_;

						// from the start to the last whack is the root

						if(pstrFront >= pstrBack)
							*pstrFront = _T('\0');
						else
							*pstrBack = _T('\0');
					}
				}

				founded_ = TRUE;

				return true;
			}

			bool find_next()
			{
				assert(find_ != NULL);

				if(find_ == NULL)
					return false;

				if(!founded_)
					return false;

				founded_ = ::FindNextFile(find_, &fd_);

				return founded_ == TRUE;
			}

			void close()
			{
				founded_ = FALSE;

				if(find_ != NULL && find_ != INVALID_HANDLE_VALUE)
				{
					::FindClose(find_);
					find_ = NULL;
				}
			}

			// Helper
			static const TCHAR* _cstrrchr(const TCHAR* p, TCHAR ch)
			{
				return _tcsrchr(p, ch);
			}

		};
	}

}





#endif