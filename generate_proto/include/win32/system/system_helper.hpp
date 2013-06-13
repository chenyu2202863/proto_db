#ifndef __WIN32_SYSTEM_HELPER_HPP
#define __WIN32_SYSTEM_HELPER_HPP

/** @system_helper.hpp
*
* @author <陈煜>
* [@author <chenyu2202863@yahoo.com.cn>]
* @date <2012/10/12>
* @version <0.1>
*
* win32系统帮助函数
*/

#include <vector>

#include <Psapi.h>
#include <Iphlpapi.h>

#include <Tlhelp32.h>
#include <Userenv.h>
#include <Sddl.h>


#include "../../utility/smart_handle.hpp"
#include "../../utility/shared_handle.hpp"
#include "../../extend_stl/unicode.hpp"
#include "../../extend_stl/string/algorithm.hpp"
#include "../../unicode/string.hpp"
#include "../../utility/select.hpp"


#pragma comment(lib, "Psapi")
#pragma comment(lib, "iphlpapi")
#pragma comment(lib, "Userenv.lib")


/*
everyone_sd	信令
get_os_name 得到系统名称
kill_process 杀死进程
start_process 启动进程
get_token_from_pcrocess_name 从进程得到相关权限
get_current_user_sid 得到当前用户SID

系统时间，本地时间，文件时间互相转换
	time_to_system_time
	system_time_to_time
	file_time_to_time
	time_to_file_time
	
format_size 格式化数据

*/

namespace win32
{
	namespace system
	{
		class everyone_sd 
		{
		private:
			PVOID  ptr;
			SECURITY_ATTRIBUTES sa;
			SECURITY_DESCRIPTOR sd;

		public:
			everyone_sd()
			{
				ptr = 0;
				sa.nLength = sizeof(sa);
				sa.lpSecurityDescriptor = &sd;
				sa.bInheritHandle = FALSE;
				ptr = built(&sd);
			}
			~everyone_sd()
			{
				if(ptr)
				{
					::HeapFree(::GetProcessHeap(), 0, ptr);
				}
			}
			
			operator SECURITY_ATTRIBUTES*()
			{
				return (ptr != NULL) ? &sa : NULL;
			}
		
			operator const SECURITY_ATTRIBUTES *() const
			{
				return (ptr != NULL) ? &sa : NULL;
			}

		public:
			PVOID built(PSECURITY_DESCRIPTOR pSD) 
			{
				PSID   psidEveryone = NULL;
				PACL   pDACL   = NULL;
				BOOL   bResult = FALSE;

				__try 
				{
					SID_IDENTIFIER_AUTHORITY siaWorld = SECURITY_WORLD_SID_AUTHORITY;
					//SECURITY_INFORMATION si = DACL_SECURITY_INFORMATION;

					if (!::InitializeSecurityDescriptor(pSD, SECURITY_DESCRIPTOR_REVISION))
						__leave;

					if (!::AllocateAndInitializeSid(&siaWorld, 1, SECURITY_WORLD_RID, 0, 0, 0, 0, 0, 0, 0, &psidEveryone)) 
						__leave;

					DWORD dwAclLength = sizeof(ACL) + sizeof(ACCESS_ALLOWED_ACE) - sizeof(DWORD) + GetLengthSid(psidEveryone);

					pDACL = (PACL)::HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, dwAclLength);
					if (!pDACL) 
						__leave;

					if (!::InitializeAcl(pDACL, dwAclLength, ACL_REVISION)) 
						__leave;

					if (!::AddAccessAllowedAce(pDACL, ACL_REVISION, GENERIC_ALL, psidEveryone)) 
						__leave;

					if (!::SetSecurityDescriptorDacl(pSD, TRUE, pDACL, FALSE)) 
						__leave;
					bResult = TRUE;
				} 
				__finally 
				{
					if (psidEveryone) 
						::FreeSid(psidEveryone);
				}

				if (bResult == FALSE) 
				{
					if (pDACL) ::HeapFree(::GetProcessHeap(), 0, pDACL);
					pDACL = NULL;
				}

				return (PVOID) pDACL;
			}
			
		};


		/**
		* @brief 获取当前系统名称
		* @param <无>
		* @exception <无任何异常抛出>
		* @return <返回系统名称>
		* @note <无>
		* @remarks <无>
		*/
		template< typename CharT >
		std::basic_string<CharT> get_os_name(const std::basic_string<CharT> &version)
		{
			assert(version.empty());
			if( version.empty() )
				return std::basic_string<CharT>();

			int iMajorVersion, iMinorVersion, iBuildNumber;
			std::basic_istringstream<CharT> is(version);

			CharT ch = 0;
			is >> iMajorVersion >> ch >> iMinorVersion >> ch >> iBuildNumber;
			
			switch(iMajorVersion)
			{
			case 4:
				switch (iMinorVersion)
				{
				case 0:
					return utility::select<CharT>("Windows95", L"Windows95");
				case 1:
					return utility::select<CharT>("Windows98", L"Windows98");
				default:
					return utility::select<CharT>("", L"");
				}
				break;
			case 5:
				switch (iMinorVersion)
				{
				case 0:
					return utility::select<CharT>("Windows2000", L"Windows2000");
				case 1:
					return utility::select<CharT>("WindowsXP", L"WindowsXP");
				case 2:
					return utility::select<CharT>("Windows2003", L"Windows2003");
				default:
					return utility::select<CharT>("", L"");
				}
				break;
			case 6:
				switch (iMinorVersion)
				{
				case 0:
					return utility::select<CharT>("WindowsVista", L"WindowsVista");
				case 1:
					return utility::select<CharT>("Windows7", L"Windows7");
				default:
					return utility::select<CharT>("", L"");
				}
				break;
			default:
				return utility::select<CharT, char, wchar_t>("", L"");
			}
		}


		/**
		* @brief 获取所有硬盘信息
		* @param <无>
		* @exception <无任何异常抛出>
		* @return <返回硬盘信息集合>
		* @note <无>
		* @remarks <无>
		*/
		struct DiskInfo
		{
			TCHAR partion;							// 分区
			UINT type;								// 类型
			ULONGLONG capacity;						// 总大小
			ULONGLONG usedSize;						// 使用大小
			ULONGLONG freeSize;						// 空闲大小
		};
		inline std::vector<DiskInfo> get_disk_info()
		{
			TCHAR driver[MAX_PATH] = {0};
			DWORD len = ::GetLogicalDriveStrings(MAX_PATH, driver);
	
			TCHAR *p = driver;
			std::vector<DiskInfo> disksInfo;

			do 
			{
				DiskInfo disk;
				disk.partion = p[0];
				disk.type = ::GetDriveType(p);

				ULARGE_INTEGER total = {0};
				ULARGE_INTEGER free = {0};
	
				BOOL suc = ::GetDiskFreeSpaceEx(p, &free, &total, 0);
				assert(suc);

				disk.capacity = total.QuadPart;
				disk.freeSize = free.QuadPart;
				disk.usedSize = total.QuadPart - free.QuadPart;
				
				disksInfo.push_back(disk);

				while(*p++);	// Next String
			}while(*p);
			
			return std::move(disksInfo);
		}
		
		/**
		* @brief 杀掉指定名称的进程
		* @param <name> <进程名称>
		* @exception <无任何异常抛出>
		* @return <如果成功则返回true，否则返回false>
		* @note <无>
		* @remarks <无>
		*/
		template < typename CharT >
		bool kill_process(const CharT *name)
		{
			DWORD aProcesses[1024] = {0}, cbNeeded = 0, cProcesses = 0;
			CharT path[MAX_PATH] = {0};
			if ( !::EnumProcesses(aProcesses, sizeof(aProcesses), &cbNeeded ) )
				return	false;

			cProcesses = cbNeeded / sizeof(DWORD);
			for(DWORD i = 0; i < cProcesses; i++ )
			{
				utility::auto_handle hProcess = ::OpenProcess(PROCESS_QUERY_INFORMATION|PROCESS_VM_READ|PROCESS_TERMINATE, FALSE, aProcesses[i]);
				if( hProcess )
				{
					if( utility::select<CharT>(::GetModuleBaseNameA, ::GetModuleBaseNameW)(hProcess, NULL, path, MAX_PATH) == 0)
					{
						continue;
					}

					if( utility::select<CharT>(_strcmpi, _wcsicmp)(path, name) == 0 )
					{
						::TerminateProcess(hProcess, 0);
						return true;
					}
				}
			}
			return false;
		}

		template < typename CharT >
		bool kill_process(const std::basic_string<CharT> &name)
		{
			return kill_process(name.c_str());
		}


		/**
		* @brief 根据进程名获取该进程的token属性
		* @param <name> <进程名称>
		* @exception <无任何异常抛出>
		* @return <返回token的智能句柄>
		* @note <无>
		* @remarks <无>
		*/
		inline utility::handle_ptr get_token_from_process_name(const TCHAR *name)
		{
			PROCESSENTRY32 pe32 = {0};
			utility::auto_handle hProcessSnap = ::CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0); 
			if ( !hProcessSnap )
				return utility::handle_ptr();

			pe32.dwSize = sizeof(pe32);
			if( ::Process32First(hProcessSnap, &pe32) )
			{ 
				do
				{
					if( _tcsicmp(pe32.szExeFile, name) == 0 )
					{
						utility::auto_handle hProcess = ::OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, pe32.th32ProcessID);
						HANDLE hToken = 0;
						BOOL suc = ::OpenProcessToken(hProcess, TOKEN_ALL_ACCESS, &hToken);
						assert(suc);

						return utility::handle_ptr(hToken);
					}
				} 
				while(::Process32Next(hProcessSnap, &pe32)); 
			}

			return utility::handle_ptr();
		}

		inline utility::handle_ptr get_token_from_process_name(const std::basic_string<TCHAR> &name)
		{
			return get_token_from_process_name(name.c_str());
		}

		/**
		* @brief 启动进程
		* @param <name> <进程名称>
		* @exception <无任何异常抛出>
		* @return <返回进程的智能句柄>
		* @note <该进程使用explorer的token，在winsta0\\default分组权限下运行>
		* @remarks <无>
		*/
		template < typename CharT >
		utility::handle_ptr start_process(const CharT *name)
		{
			utility::handle_ptr hToken = get_token_from_process_name(_T("explorer.exe"));
			LPVOID lpEnv = 0;
			if( !hToken )
				::CreateEnvironmentBlock(&lpEnv, hToken, FALSE);

			utility::selector_t<CharT, STARTUPINFOA, STARTUPINFOW>::type si = {0};
			PROCESS_INFORMATION pi = {0};
			si.cb = sizeof(STARTUPINFO);
			char privA[] = { "winsta0\\default" };
			wchar_t privW[] = { L"winsta0\\default" };
			si.lpDesktop = utility::select<CharT>(privA, privW);	

			CharT path[MAX_PATH] = {0};
			utility::select<CharT>(strcpy_s, wcscpy_s)(path, MAX_PATH, name);

			BOOL ret = utility::select<CharT>(::CreateProcessA, ::CreateProcessW)(NULL, path, NULL, NULL, FALSE, CREATE_UNICODE_ENVIRONMENT, lpEnv, NULL, &si, &pi);
			if( ret )
			{
				::CloseHandle(pi.hThread);
				return pi.hProcess;
			}

			return utility::handle_ptr();
		}

		template < typename CharT >
		utility::handle_ptr start_process(const std::basic_string<CharT> &name)
		{
			return start_process(name.c_str());
		}


		/**
		* @brief 获取当前sid的用户名
		* @param <无>
		* @exception <无任何异常抛出>
		* @return <如果成功则返回用户名，否则返回空>
		* @note <无>
		* @remarks <无>
		*/
		inline stdex::tString get_current_user_for_sid() 
		{
			HANDLE hToken = INVALID_HANDLE_VALUE;
			if( !::OpenProcessToken(::GetCurrentProcess(), TOKEN_QUERY, &hToken ) ) 
				return _T("");

			DWORD dwSize = 0;
			if( !::GetTokenInformation(hToken, TokenUser, NULL, dwSize, &dwSize) ) 
			{
				DWORD dwResult = GetLastError();
				if( dwResult != ERROR_INSUFFICIENT_BUFFER )
					return _T("");
			}

			std::vector<char> buf(dwSize + 1);
			PTOKEN_USER pUserInfo = reinterpret_cast<PTOKEN_USER>(&buf[0]);

			if( !::GetTokenInformation(hToken, TokenUser, pUserInfo, dwSize, &dwSize ) ) 
				return _T("");

			LPTSTR StringSid = 0;
			BOOL suc = ::ConvertSidToStringSid(pUserInfo->User.Sid, &StringSid);
			assert(suc);

			stdex::tString SID = StringSid;
			return std::move(SID);
		}

		/**
		* @brief time_t转SYSTEMTIME
		* @param <time> <time_t类型时间>
		* @exception <无任何异常抛出>
		* @return <返回系统格式时间>
		* @note <无>
		* @remarks <无>
		*/
        inline SYSTEMTIME time_to_system_time(time_t time)
        { 
			tm timeInfo = {0};
            if (localtime_s(&timeInfo, &time) != 0)
            {
                assert(0 && "时间转化失败");
            }
            SYSTEMTIME sytemTime = { 1900 + timeInfo.tm_year, 1 + timeInfo.tm_mon, timeInfo.tm_wday, timeInfo.tm_mday, timeInfo.tm_hour, timeInfo.tm_min, timeInfo.tm_sec, 0 };
            return sytemTime;
        }

		/**
		* @brief SYSTEMTIME转time_t
		* @param <sytemTime> <SYSTEMTIME类型时间>
		* @exception <无任何异常抛出>
		* @return <返回time_t格式时间>
		* @note <无>
		* @remarks <无>
		*/
        inline time_t system_time_to_time(const SYSTEMTIME& sytemTime)
        {
            tm timeInfo = { sytemTime.wSecond, sytemTime.wMinute, sytemTime.wHour, sytemTime.wDay, sytemTime.wMonth - 1, sytemTime.wYear - 1900, sytemTime.wDayOfWeek, 0, 0 };
            return mktime(&timeInfo);
        }

		/**
		* @brief FILETIME转time_t
		* @param <fileTime> <FILETIME类型时间>
		* @exception <无任何异常抛出>
		* @return <返回time_t格式时间>
		* @note <无>
		* @remarks <无>
		*/
        inline time_t file_time_to_time(const FILETIME& fileTime)
        {
            SYSTEMTIME systemTime = {0};
            BOOL result = ::FileTimeToSystemTime(&fileTime, &systemTime);
            assert(result && "FileTimeToTime失败");
            return system_time_to_time(systemTime);
        }

		/**
		* @brief time_t转FILETIME
		* @param <time> <time_t类型时间>
		* @exception <无任何异常抛出>
		* @return <返回FILETIME格式时间>
		* @note <无>
		* @remarks <无>
		*/
        inline FILETIME time_to_file_time(time_t time)
        {
			SYSTEMTIME systemTime = time_to_system_time(time);
            FILETIME fileTime = {0};
            BOOL result = ::SystemTimeToFileTime(&systemTime, &fileTime);
            assert(result && "FileTimeToTime失败");
            return fileTime;
        }

		/**
		* @brief 合并两个time_t类型时间
		* @param <date> <日期>
		* @param <time> <时间>
		* @exception <无任何异常抛出>
		* @return <返回合并后的time_t格式时间>
		* @note <无>
		* @remarks <无>
		*/
        inline time_t get_fixed_time(time_t date, time_t time)
        {
			tm dateInfo = {0};
			if( ::localtime_s(&dateInfo, &date) != 0 )
                return 0;
        
			tm timeInfo = {0};
			if( ::localtime_s(&timeInfo, &time) != 0)
				return 0;

            tm dateTimeInfo = dateInfo;
            dateTimeInfo.tm_hour = timeInfo.tm_hour;
            dateTimeInfo.tm_min = timeInfo.tm_min;
            dateTimeInfo.tm_sec = timeInfo.tm_sec;
			
			return ::mktime(&dateTimeInfo);
        }


		/**
		* @brief 格式化大小
		* @param <date> <日期>
		* @param <time> <时间>
		* @exception <无任何异常抛出>
		* @return <返回合并后的time_t格式时间>
		* @note <无>
		* @remarks <无>
		*/
		template < typename CharT >
		inline std::basic_string<CharT> format_size(unsigned long long size)
		{
            // 按Byte\KB\MB\GB来划分
            static struct
            {
                unsigned long long size;
                const CharT* name;
            } SIZE_INFO[] =
            {
                1024 * 1024 * 1024, utility::select<CharT>("GB", L"GB"),
                1024 * 1024,        utility::select<CharT>("MB", L"MB"),
                1024,               utility::select<CharT>("KB", L"KB"),
                1,                  utility::select<CharT>("B",  L"B"),
            };

			std::basic_string<CharT> text;
            for(size_t i = 0; i < _countof(SIZE_INFO); ++i)
            {
                if( size >= SIZE_INFO[i].size )
                {
                    text = stdex::to_string(double(size) / SIZE_INFO[i].size, 2);
                    text += SIZE_INFO[i].name;
                    break;
                }
            }

			return std::move(text);
        }
	}
	
}



#endif