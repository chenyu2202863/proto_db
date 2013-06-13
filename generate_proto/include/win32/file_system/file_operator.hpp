#ifndef __WIN32_FILESYSTEM_FILE_OPERATOR_HPP
#define __WIN32_FILESYSTEM_FILE_OPERATOR_HPP

/** @file_operator.hpp
*
* @author <陈煜>
* [@author <chenyu2202863@yahoo.com.cn>]
* @date <2012/10/14>
* @version <0.1>
*
* 文件遍历
*/

#include "find_file.hpp"
#include <functional>


/*
遍历目录
	file_depth_search_t (深度遍历)	
	file_range_search_t (广度遍历)

	depth_search
	range_search
*/


namespace win32
{
	namespace filesystem
	{

		namespace detail
		{
			// -----------------------
			struct check_file_time
			{
				enum { DAYS = 7 };
				static const __int64 PerSecDay = (__int64)10000000 * 24 * 60 * 60;

				template<typename FileFindT>
				bool operator()(const FileFindT &filefind)
				{
					// 文件最后写时间
					ULARGE_INTEGER time = {0};
					::memcpy(&time, &filefind.fd_.ftLastWriteTime, sizeof(ULARGE_INTEGER));

					// 系统当前时间
					ULARGE_INTEGER now = {0};
					SYSTEMTIME st = {0};
					::GetSystemTime(&st);
					FILETIME f = {0};
					::SystemTimeToFileTime(&st, &f);
					::memcpy(&now, &f, sizeof(now));

					// 时间差
					__int64 span = now.QuadPart - time.QuadPart;

					if( span >= PerSecDay * DAYS )
						return true;

					return false;
				}
			};

			struct check_file_null
			{
				template<typename FileFindT>
				bool operator()(const FileFindT &filefind)
				{
					return true;
				}
			};

			// -----------------------
			struct delete_file
			{
				void operator()(const stdex::tString &path)
				{
                    ::SetFileAttributes(path.c_str(), FILE_ATTRIBUTE_NORMAL);
					::DeleteFile(path.c_str());
				}
			};

			struct operate_null
			{
				void operator()(const stdex::tString &path)
				{
					// do nothing
				}
			};
		}
		

		// -----------------------
		// 深度文件搜索

		template< typename CondTraits, typename OPTraits >
		struct file_depth_search_t
		{
			typedef file_depth_search_t<CondTraits, OPTraits> ThisType;

			bool is_sub_dir_;
			CondTraits cond_;
			OPTraits op_;

			file_depth_search_t(const CondTraits &cond, const OPTraits &op, bool isSubDir)
				: cond_(cond)
				, op_(op)
				, is_sub_dir_(isSubDir)
			{}

			void run(const stdex::tString &dir)
			{
				filesystem::find_file find;

				assert(dir.length() > 1);
				stdex::tString tmp = dir;
				if( tmp[tmp.length() - 1] != '\\' ||
					tmp[tmp.length() - 1] != '/' )
					tmp += _T("\\");
				tmp += _T("*");

				bool bSuc = find.find(tmp.c_str());
				while( bSuc )
				{
					bSuc = find.find_next();
					if( !bSuc )
						break;

					if( find.is_dots() )
						continue;

					if( cond_(find) )
					{
						op_(find.get_file_path());
					}

					if( is_sub_dir_ && find.is_directory() )
					{
						run(find.get_file_path());
					}
				}
			}
		};


		// ------------------------
		// 广度文件搜索

		template < typename CondTraits, typename OPTraits >
		struct file_range_search_t
		{
			typedef file_range_search_t<CondTraits, OPTraits> ThisType;

			size_t depth_;
			CondTraits cond_;
			OPTraits op_;

			file_range_search_t(const CondTraits &cond, const OPTraits &op, size_t depth = 0)
				: cond_(cond)
				, op_(op)
				, depth_(depth)
			{}

			void run(const stdex::tString &dir)
			{
				_run_impl(dir, 0);
			}

			void _run_impl(const stdex::tString &dir, size_t depth)
			{
				assert(dir.length() > 1);
				if( depth_ != 0 && (depth < 0 || depth >= depth_) )
					return;

				std::vector<stdex::tString> curDirectorys;
				filesystem::find_file find;

				stdex::tString tmp = dir;
				if( tmp[tmp.length() - 1] != '\\' ||
					tmp[tmp.length() - 1] != '/' )
					tmp += _T("\\");
				tmp += _T("*");

				bool bSuc = find.find(tmp.c_str());
				while( bSuc )
				{
					bSuc = find.find_next();
					if( !bSuc )
						break;

					if( find.is_dots() )
						continue;

					if( cond_(find) )
					{
						op_(find.get_file_path());
					}

					if( find.is_directory() )
					{
						curDirectorys.push_back(find.get_file_path());
					}
				}

				std::for_each(curDirectorys.begin(), curDirectorys.end(),
					std::bind(&ThisType::_run_impl, this, std::placeholders::_1, depth + 1));
			}
		};

		/**
		* @brief 深度遍历文件夹
		* @param <path> <文件夹路径>
		* @param <check_cond> <检测条件回调，类型为bool (const find_file &find_cond)>
		* @param <op_cond> <操作回调, 类型为void (const stdex::tString &path)>
		* @param <subdir> <是否需要遍历子文件夹>
		* @exception <无任何异常抛出>
		* @return <无>
		* @note <check_cond回调返回为true，则继续遍历，返回false，则退出遍历。op_cond为执行回调，根据传入的文件路径可以进行相应操作>
		* @remarks <无>
		*/
		template < typename CheckT, typename OpT >
		inline void depth_search(const stdex::tString &path, const CheckT &check_cond, const OpT &op_cond, bool subdir = true)
		{
			file_depth_search_t<CheckT, OpT> file(check_cond, op_cond, subdir);
			file.run(path);
		}

		/**
		* @brief 广度遍历文件夹
		* @param <path> <文件夹路径>
		* @param <check_cond> <检测条件回调，类型为bool (const find_file &find_cond)>
		* @param <op_cond> <操作回调, 类型为void (const stdex::tString &path)>
		* @param <depth> <深入遍历层级>
		* @exception <无任何异常抛出>
		* @return <无>
		* @note <check_cond回调返回为true，则继续遍历，返回false，则退出遍历。op_cond为执行回调，根据传入的文件路径可以进行相应操作>
		* @remarks <无>
		*/
		template < typename CheckT, typename OpT >
		inline void range_search(const stdex::tString &path, const CheckT &check_cond, const OpT &op_cond, int depth = 0)
		{
			file_range_search_t<CheckT, OpT> file(check_cond, op_cond, depth);
			file.run(path);
		}
	}
}




#endif