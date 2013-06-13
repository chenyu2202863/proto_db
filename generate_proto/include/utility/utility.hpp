#ifndef __UTILITY_UTILITY_HPP
#define __UTILITY_UTILITY_HPP

/** @utility.hpp
*
* @author <陈煜>
* [@author <chenyu2202863@yahoo.com.cn>]
* @date <2012/10/12>
* @version <0.1>
*
* 杂项函数
*/


#include <vector>
#include <cassert>
#include <algorithm>
#include <atlenc.h>

#include "../extend_stl/string/algorithm.hpp"
#include "../extend_stl/unicode.hpp"
#include "select.hpp"



#include <Winver.h>
#pragma comment(lib, "Version.lib")


#include <rpcdce.h>
#pragma comment(lib, "rpcrt4.lib")



namespace utility 
{
	/**
	* @brief 获取主程序路径
	* @param <无>
	* @exception <无任何异常抛出>
	* @return <返回一个unicode字符串>
	* @note <GetModuleFileName>
	* @remarks <无>
	*/
	inline const stdex::tString &get_app_path()
	{
		static struct Helper 
		{
			stdex::tString path_;

			Helper() 
			{
				TCHAR szFilePath[MAX_PATH] = {0};
				DWORD ret = ::GetModuleFileName(NULL, szFilePath, _countof(szFilePath));
				assert(ret != 0);

				path_ = szFilePath;

				size_t pos = path_.find_last_of('\\');
				path_ = path_.substr(0, pos + 1);
			}
		} helper;

		return helper.path_;
	}

	/**
	* @brief 获取当前模块路径
	* @param <无>
	* @exception <无任何异常抛出>
	* @return <返回一个unicode字符串>
	* @note <VirtualQuery GetModuleFileName>
	* @remarks <无>
	*/
	inline const stdex::tString &get_module_path()
	{
		struct module_handle
		{
			static HMODULE self_module()
			{
				MEMORY_BASIC_INFORMATION mbi = {0};

				return ((::VirtualQuery(&self_module, &mbi, sizeof(mbi)) != 0) 
					? (HMODULE) mbi.AllocationBase : NULL);
			}
		};

		static struct Helper
		{
			stdex::tString path_;

			Helper() 
			{
				TCHAR szFilePath[MAX_PATH] = {0};
				DWORD ret = ::GetModuleFileName(module_handle::self_module(), szFilePath, _countof(szFilePath));
				assert(ret != 0);

				path_ = szFilePath;

				size_t pos = path_.find_last_of('\\');
				path_ = path_.substr(0, pos + 1);
			}
		} helper;

		return helper.path_;
	}



	/**
	* @brief 获取系统路径
	* @param <无>
	* @exception <无任何异常抛出>
	* @return <返回一个unicode字符串>
	* @note <GetSystemDirectory>
	* @remarks <无>
	*/
    inline const stdex::tString& get_system_path()
    {
        static struct Helper 
        {
            stdex::tString path_;
            Helper() 
            {
                TCHAR path[MAX_PATH] = {0};
				DWORD ret = ::GetSystemDirectory(path, MAX_PATH);
				assert(ret != 0);

				::PathRemoveBackslash(path);
				::PathAddBackslash(path);

                path_ = path;
            }
        } helper;

        return helper.path_;
    }
	

	/**
	* @brief 获取Windows路径
	* @param <无>
	* @exception <无任何异常抛出>
	* @return <返回一个unicode字符串>
	* @note <GetWindowsDirectory>
	* @remarks <无>
	*/
	inline const stdex::tString& get_windows_path()
	{
		static struct Helper 
		{
			stdex::tString path_;
			Helper() 
			{
				TCHAR path[MAX_PATH] = {0};
				DWORD ret = ::GetWindowsDirectory(path, MAX_PATH);
				assert(ret != 0);

				::PathRemoveBackslash(path);
				::PathAddBackslash(path);

				path_ = path;
			}
		} helper;

		return helper.path_;
	}


	/**
	* @brief 获取文件版本号
	* @param <path> <文件绝对路径>
	* @exception <无任何异常抛出>
	* @return <返回文件版本号>
	* @note <无>
	* @remarks <无>
	*/
	template < typename CharT >
	inline std::basic_string<CharT> get_file_version(const std::basic_string<CharT>& path)
	{
		CharT version[MAX_PATH] = {0};
		DWORD size = select<CharT>(::GetFileVersionInfoSizeA, ::GetFileVersionInfoSizeW)(path.c_str(), 0);

		if( size )
		{
			std::vector<CharT> block(size + 1);	// add null character
			
			BOOL suc = select<CharT>(::GetFileVersionInfoA, ::GetFileVersionInfoW)(path.c_str(), 0, size, block.data());
			if( !suc )
				return std::basic_string<CharT>();

			UINT query_size = 0;
			DWORD* pTransTable = 0;
			suc = select<CharT>(::VerQueryValueA, ::VerQueryValueW)(block.data(), 
				select<CharT>("\\VarFileInfo\\Translation", L"\\VarFileInfo\\Translation"), (void **)&pTransTable, &query_size);
			if( !suc )
				return std::basic_string<CharT>();

			LONG lang_charset = MAKELONG(HIWORD(pTransTable[0]), LOWORD(pTransTable[0]));
			
			CharT sub_block[MAX_PATH] = {0};
			select<CharT>(::sprintf_s, ::swprintf_s)(sub_block, MAX_PATH,
				select<CharT>("\\StringFileInfo\\%08lx\\FileVersion", L"\\StringFileInfo\\%08lx\\FileVersion"), lang_charset);	
			CharT *lpData = 0;
			suc = select<CharT>(::VerQueryValueA, ::VerQueryValueW)(block.data(), sub_block, (PVOID*)&lpData, &query_size);
			if( !suc )
				return std::basic_string<CharT>();

			select<CharT>(::strcpy_s, ::wcscpy_s)(version, MAX_PATH, lpData);
		}


		std::basic_string<CharT> str(version);

		// replace ',' to '.'
		std::replace_if(str.begin(), str.end(), 
			std::bind2nd(std::equal_to<std::basic_string<CharT>::value_type>(), ','), '.');

		// delete [Space]
		std::basic_string<CharT>::iterator iter = std::remove_if(str.begin(), str.end(),
			std::bind2nd(std::equal_to<std::basic_string<CharT>::value_type>(), ' '));

		// remove redundant character
		if( iter != str.end() )
			str.erase(iter, str.end());

		return str;
	}

	template < typename CharT >
	inline std::basic_string<CharT> get_file_version(const CharT *path)
	{
		return get_file_version(std::basic_string<CharT>(path));
	}

	/**
	* @brief 比较两个文件版本号
	* @param <lhs> <文件版本号>
	* @param <lhs> <文件版本号>
	* @exception <无任何异常抛出>
	* @return <返回bool>
	* @note <如果左版本号大于右版本号则为true，否则为false>
	* @remarks <无>
	*/
	template < typename CharT >
	bool compare_file_version(const std::basic_string<CharT> &lhs, const std::basic_string<CharT> &rhs) 
	{
		std::vector<std::basic_string<CharT>> l_ver, r_ver;
		stdex::split(l_ver, lhs, '.');
		stdex::split(r_ver, rhs, '.');

		if( l_ver.size() != r_ver.size() )
			return false;

		for(size_t i = 0; i != l_ver.size(); ++i)
		{
			size_t l_num = stdex::to_number(l_ver[i]);
			size_t r_num = stdex::to_number(r_ver[i]);

			if( l_num != r_num )
				return l_num > r_num;
		}

		return lhs > rhs;
	}

	template < typename CharT >
	bool compare_file_version(const CharT *lhs, const CharT *rhs) 
	{
		return compare_file_version(std::string(lhs), std::string(rhs));
	}
	

	/**
	* @brief 创建GUID字符串
	* @param <无>
	* @exception <无任何异常抛出>
	* @return <返回GUID字符串>
	* @note <无>
	* @remarks <无>
	*/
	inline stdex::tString create_guid_string()
	{
		GUID guid;
		RPC_STATUS status = ::UuidCreate(&guid);
		if( RPC_S_OK != status )
		{
			assert(0 && "RPC_S_OK != status");
			return stdex::tString();
		}

#ifdef _UNICODE
		typedef RPC_WSTR RPC_TSTR;
#else
		typedef RPC_CSTR RPC_TSTR;
#endif

		RPC_TSTR rpcStr = 0;
		status = ::UuidToString(&guid, &rpcStr);
		if( RPC_S_OK != status )
		{
			assert(0 && "RPC_S_OK != status");
			return stdex::tString();
		}

		stdex::tOstringstream os;
		os << _T("{") << reinterpret_cast<TCHAR *>(rpcStr) << _T("}");

		status = ::RpcStringFree(&rpcStr);
		if( RPC_S_OK != status )
		{
			assert(0 && "RPC_S_OK != status");
		}

		return std::move(os.str());
	}

} 

#endif 