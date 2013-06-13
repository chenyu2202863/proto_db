#ifndef __FILE_SYSTEM_FILE_HELPER_HPP
#define __FILE_SYSTEM_FILE_HELPER_HPP

/** @file_helper.hpp
*
* @author <陈煜>
* [@author <chenyu2202863@yahoo.com.cn>]
* @date <2012/10/14>
* @version <0.1>
*
* 文件操作帮助函数
*/

#include <cstdint>
#include <windows.h>
#include <DbgHelp.h>	// for MakeSureDirectoryPathExists

#include "../../utility/smart_handle.hpp"
#include "../../utility/select.hpp"
#include "../system/system_helper.hpp"
#include "filesystem.hpp"

#include "file_operator.hpp"


#pragma comment(lib, "Dbghelp")

/*
	check_disk_space		检测磁盘空间
	parttion_speed			磁盘运转速度
	del_directory			删除目录(不管目录是否为空)
	mk_directory			新建目录(支持多级目录)
	is_file_exsit			文件是否存在
	is_directory_exsit		目录是否存在
	file_size				文件大小
	file_last_write_time	文件上次写入时间

*/

namespace win32
{

	namespace filesystem
	{
		namespace detail
		{
			struct ms_file_ompare
			{
				template < typename CharT >
				bool operator()(CharT val)
				{
					return val == '/' || val == '\\';
				}
			};

			template < typename T >
			struct less_t
			{
				typedef T value_type;
				bool operator()(const value_type &lhs, const value_type &rhs) const
				{
					size_t lCnt = std::count_if(lhs.begin(), lhs.end(), ms_file_ompare());
					size_t rCnt = std::count_if(rhs.begin(), rhs.end(), ms_file_ompare());

					return lCnt > rCnt;
				}
			};

			struct check_file
			{
				typedef std::vector<stdex::tString> Directorys;
				Directorys directorys_;

				template<typename FileFindT>
				bool operator()(const FileFindT &filefind)
				{
					if( filefind.is_directory() )
					{
						stdex::tString path = filefind.get_file_path();
						directorys_.push_back(path);
						return false;
					}
					else
					{
						::SetFileAttributes(filefind.get_file_path().c_str(), FILE_ATTRIBUTE_NORMAL);
						return true;
					}
				}

				const Directorys &get_dirs() 
				{
					std::sort(directorys_.begin(), directorys_.end(), less_t<stdex::tString>());
					return directorys_;
				}
			};
		}

		/**
		* @brief 检测硬盘剩余空间是否满足指定值
		* @param <path> <磁盘路径>
		* @param <size> <指定检测比较大小>
		* @param <info> <返回来的信息>
		* @exception <无任何异常抛出>
		* @return <成功返回true，否则返回false>
		* @note <无>
		* @remarks <无>
		*/
		template < typename CharT >
		inline bool check_disk_space(const std::basic_string<CharT> &path, std::uint64_t size, std::basic_string<CharT> &info)
		{
			ULARGE_INTEGER FreeBytesAvailableToCaller = {0};
			ULARGE_INTEGER TotalNumberOfBytes = {0};
			ULARGE_INTEGER TotalNumberOfFreeBytes = {0};

			if( utility::select<CharT>(::GetDiskFreeSpaceExA, ::GetDiskFreeSpaceExW)(path.c_str(),
				&FreeBytesAvailableToCaller,
				&TotalNumberOfBytes,
				&TotalNumberOfFreeBytes) )
			{
				if( FreeBytesAvailableToCaller.QuadPart < size )
				{
					std::basic_ostringstream<CharT> os;
					os << _T("磁盘空间偏低, DIR=") << path << _T(", SPACE=") 
						<< FreeBytesAvailableToCaller.QuadPart/1024.0/1024.0/1024.0 << _T("GB");
					info = os.str();
					return false;
				}
			}
			else 
			{
				std::basic_ostringstream<CharT> os;
				os << _T("检测目录失败, DIR=") << path << _T(", CODE=") << ::GetLastError();
				info = os.str();
				return false;
			}

			return true;
		}


		/**
		* @brief 检测分区读取速度
		* @param <partition> <磁盘分区>
		* @exception <无任何异常抛出>
		* @return <返回磁盘读取速度>
		* @note <随机读取指定分区数据，读取2000次>
		* @remarks <无>
		*/
		template < typename CharT >
		inline size_t partition_speed(CharT partition)
		{
			TCHAR device[MAX_PATH] = {0};
			::_stprintf_s(device, _T("\\\\.\\%C:"), partition);

			utility::auto_file hFile = ::CreateFile(device, GENERIC_READ, 
				FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
			if( !hFile )
				return 0;

			LARGE_INTEGER liStart = {0}, liEnd = {0}, Frequency = {0};
			::QueryPerformanceFrequency(&Frequency);

			const size_t bufSize = 64 * 1024;
			char data[bufSize] = {0};
			::srand((unsigned)::time(0));

			::QueryPerformanceCounter(&liStart);
			DWORD dwTotalReadBytes = 0;
			for(size_t idx = 0; idx < 2000; ++idx)
			{
				DWORD dwReadBytes = 0;
				int n = rand() * rand();
				::SetFilePointer(hFile, n, 0, FILE_BEGIN);
				::ReadFile(hFile, data, bufSize, &dwReadBytes, NULL);
				dwTotalReadBytes += dwReadBytes;
			}

			::QueryPerformanceCounter(&liEnd);
			double speed = dwTotalReadBytes * 2000 / 1000.0 / 
				((liEnd.QuadPart - liStart.QuadPart) * 1.0 / Frequency.QuadPart);

			return static_cast<size_t>(speed);
		}


		/**
		* @brief 删除文件夹所有文件
		* @param <path> <文件夹路径>
		* @exception <无任何异常抛出>
		* @return <成功则返回true，否则返回false>
		* @note <无>
		* @remarks <无>
		*/
		template < typename CharT >
		inline bool del_directoy(const std::basic_string<CharT> &path)
        {
			if( path.empty() )
				return false;

			// 删除所有文件
			//detail::check_file checkfile; 
			//detail::delete_file deletefile;
			//depth_search(unicode::to(path), checkfile, deletefile);

			//// 删除目录
			//const detail::check_file::Directorys &directorys = checkfile.get_dirs();

			//bool suc = true;
			//std::for_each(directorys.begin(), directorys.end(), [&suc](const stdex::tString &tmp)
			//{
			//	if( !::RemoveDirectory(tmp.c_str()) )
			//		suc = false;
			//});


			//if( !utility::select<CharT>(::RemoveDirectoryA, ::RemoveDirectoryW)(path.c_str()) )
			//	suc = false;

			typedef utility::selector_t<CharT, win32::file::path_traits, win32::file::wpath_traits>::type path_traits;
			typedef win32::file::basic_path<std::basic_string<CharT>, path_traits> path_type;
			std::uint32_t cnt = win32::file::remove_all(path_type(path));

			return true;
        }

		template < typename CharT >
		inline bool del_directoy(const CharT *path)
		{
			std::basic_string<CharT> dest_path(path);
			return del_directoy(dest_path);
		}

        /**
		* @brief 创建文件夹路径
		* @param <path> <文件夹路径>
		* @exception <无任何异常抛出>
		* @return <成功则返回true，否则返回false>
		* @note <支持嵌套多重路径>
		* @remarks <无>
		*/
        template < typename CharT >
		inline bool mk_directory(const std::basic_string<CharT> &path)
		{
			if( path.empty() )
				return false;

			// 分隔符必须是backslash
			std::string &&dest_path = unicode::to(add_backslash(path));
			std::replace_if(dest_path.begin(), dest_path.end(), std::bind2nd(std::equal_to<char>(), '/'), '\\');
			
			return ::MakeSureDirectoryPathExists(dest_path.c_str()) == TRUE;
		}

		template < typename CharT >
		inline bool mk_directory(const CharT *path)
		{
			std::basic_string<CharT> dest_path(path);
			return mk_directory(dest_path);
		}


		/**
		* @brief 检查文件是否存在
		* @param <path> <文件路径>
		* @exception <无任何异常抛出>
		* @return <存在返回true，不存在返回false>
		* @note <无>
		* @remarks <无>
		*/
		template < typename CharT >
		inline bool is_file_exist(const std::basic_string<CharT> &path)
		{	    
			DWORD att = utility::select<CharT>(::GetFileAttributesA, ::GetFileAttributesW)(path.c_str());
			if( att == INVALID_FILE_ATTRIBUTES )
				return false;

			return true;
		}

		template < typename CharT >
		inline bool is_file_exist(const CharT *path)
		{
			return is_file_exist(std::basic_string<CharT>(path));
		}

		/**
		* @brief 检查文件夹是否存在
		* @param <path> <文件路径>
		* @exception <无任何异常抛出>
		* @return <存在返回true，不存在返回false>
		* @note <无>
		* @remarks <无>
		*/
		template < typename CharT >
		inline bool is_directory_exist(const std::basic_string<CharT> &path)
		{	    
			DWORD att = utility::select<CharT>(::GetFileAttributesA, ::GetFileAttributesW)(path.c_str());
			if( att == INVALID_FILE_ATTRIBUTES )
				return false;

			return (att & FILE_ATTRIBUTE_DIRECTORY) != 0;
		}

		template < typename CharT >
		inline bool is_directory_exist(const CharT *path)
		{
			return is_directory_exist(std::basic_string<CharT>(path));
        }


		/**
		* @brief 添加文件路径分隔符
		* @param <path> <文件路径>
		* @exception <无任何异常抛出>
		* @return <返回带有文件分割符的路径字符串>
		* @note <替换分隔符为统一的'\\'>
		* @remarks <无>
		*/
		template < typename CharT >
		inline std::basic_string<CharT> add_backslash(const std::basic_string<CharT> &path)
		{
			if( path.empty() )
				return std::move(path);

			std::basic_string<CharT> tmp = path;
			if( tmp.back() != '\\' &&
				tmp.back() != '/' )
				tmp.append(1, CharT('\\'));

			return std::move(tmp);
		}

		/**
		* @brief 合并文件夹路径和文件路径
		* @param <directory> <文件夹路径>
		* @param <fileName> <文件名称>
		* @exception <无任何异常抛出>
		* @return <返回合并后的文件路径字符串>
		* @note <替换分隔符为统一的'\\'>
		* @remarks <无>
		*/
        template < typename CharT >
        inline std::basic_string<CharT> combine_file_path(const std::basic_string<CharT> &directory, const std::basic_string<CharT> &fileName)
        {
            if( !directory.empty() && 
				*(directory.rbegin()) != '\\' && 
				*(directory.rbegin()) != '/')
            {
                return std::move(directory + CharT('\\') + fileName);
            }
            else
            {
				return std::move(directory + fileName);
            }
        }

		/**
		* @brief 获取文件大小
		* @param <file> <文件所在路径>
		* @exception <无任何异常抛出>
		* @return <返回文件大小>
		* @note <如果文件不存在,则返回0>
		* @remarks <无>
		*/
        template < typename CharT >
        inline std::uint64_t file_size(const char * const file)
        {
            return file_size(std::basic_string<CharT>(file));
        }

        template < typename CharT >
		inline std::uint64_t file_size(const std::basic_string<CharT> &filePath)
        {
            utility::auto_file file = utility::select<CharT>(::CreateFileA, ::CreateFileW)(filePath.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);

            if( !file )
            {
				assert(0 && "文件不存在");
                return 0;
            }

			LARGE_INTEGER fileSize = {0};
			BOOL result = ::GetFileSizeEx(file, &fileSize);
            assert(result && "取得文件大小失败");
            return fileSize.QuadPart;
        }

		/**
		* @brief 文件最后写入时间
		* @param <file> <文件所在路径>
		* @exception <无任何异常抛出>
		* @return <返回文件最后写入时间>
		* @note <如果文件不存在,则返回0>
		* @remarks <无>
		*/
        template < typename CharT >
        inline time_t file_last_write_time(const char * const file)
        {
            return file_last_write_time(std::basic_string<CharT>(file));
        }

        template < typename CharT >
        inline time_t file_last_write_time(const std::basic_string<CharT> &filePath)
        {
            utility::auto_file file = utility::select<CharT>(::CreateFileA, ::CreateFileW)
				(filePath.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);

            if( !file )
            {
				assert(0 && "文件不存在");
                return 0;
            }
			FILETIME fileTime = {0};
			BOOL result = ::GetFileTime(file, NULL, NULL, &fileTime);
            assert(result && "取得文件时间失败");
            return win32::system::file_time_to_time(fileTime);
        }
	}
}




#endif