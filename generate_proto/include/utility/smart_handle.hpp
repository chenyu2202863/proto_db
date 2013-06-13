#ifndef __UTILITY_SMART_HANDLE_HPP
#define __UTILITY_SMART_HANDLE_HPP

/** @smart_handle.hpp
*
* @author <陈煜>
* [@author <chenyu2202863@yahoo.com.cn>]
* @date <2012/10/12>
* @version <0.1>
*
* 利用RAII机制，退出作用域时关闭句柄
*/

#include <Windows.h>
#include <Winsvc.h>
#include <type_traits>
#include <memory>



namespace utility
{

	/**
	* @class <smart_handle_t>
	* @brief 
	*
	* 利用C++RAII机制，退出作用域时关闭已打开句柄，资源自动管理
	*/
	template <
		typename HandleT, 
		template< typename > class ReleasePolicyT, 
		HandleT NULL_VALUE = NULL
	>
	class smart_handle_t 
		: private ReleasePolicyT<HandleT>
	{
		typedef HandleT	value_type;

	private:
		value_type handle_;

	public:
		/**
		* @brief 默认构造函数，初始化句柄为空
		* @param <无>
		* @exception <无任何异常抛出>
		* @return <无>
		* @note <无>
		* @remarks <无>
		*/
		smart_handle_t()
			: handle_(NULL_VALUE)
		{
		}

		/**
		* @brief 接收一个句柄的构造函数
		* @param <h> <windows句柄>
		* @exception <无任何异常抛出>
		* @return <无>
		* @note <无>
		* @remarks <无>
		*/
		smart_handle_t(const value_type &h)
			: handle_(h)
		{
		}

		/**
		* @brief 拷贝构造函数
		* @param <h> <smart_handle_t对象>
		* @exception <无任何异常抛出>
		* @return <无>
		* @note <无>
		* @remarks <无>
		*/
		smart_handle_t(const smart_handle_t &h)
		{
			cleanup();
			handle_ = h.handle_;
		}

		~smart_handle_t()
		{
			cleanup();
		}

		smart_handle_t &operator=(const smart_handle_t &rhs) 
		{ 
			if( &rhs != this )
			{
				cleanup();
				handle_ = rhs.handle_;
			}

			return(*this);  
		}

		value_type &operator=(const value_type &hande) 
		{ 
			if( hande != handle_ )
			{
				cleanup();
				handle_ = hande;
			}

			return handle_;  
		}

		/**
		* @brief 隐式转换函数，返回一个windows句柄
		* @param <无>
		* @exception <无任何异常抛出>
		* @return <无>
		* @note <无>
		* @remarks <无>
		*/
		operator value_type &()
		{
			return handle_;
		}

		operator const value_type &() const
		{
			return handle_;
		}

		/**
		* @brief 显示获取一个windows句柄
		* @param <无>
		* @exception <无任何异常抛出>
		* @return <无>
		* @note <无>
		* @remarks <无>
		*/
		value_type &get()
		{
			return handle_;
		}

		const value_type &get() const
		{
			return handle_;
		}

		/**
		* @brief 显示判断该句柄是否有效
		* @param <无>
		* @exception <无任何异常抛出>
		* @return <无>
		* @note <无>
		* @remarks <无>
		*/
		bool is_valid() const
		{
			return handle_ != NULL_VALUE;
		}

		/**
		* @brief 隐式判断该句柄是否有效
		* @param <无>
		* @exception <无任何异常抛出>
		* @return <无>
		* @note <无>
		* @remarks <无>
		*/
		operator bool() const
		{
			return is_valid();
		}

		/**
		* @brief 弹出句柄，不进行自动管理
		* @param <无>
		* @exception <无任何异常抛出>
		* @return <返回windows句柄>
		* @note <无>
		* @remarks <无>
		*/
		value_type detach()
		{
			value_type hHandle = handle_;
			handle_ = NULL_VALUE;

			return hHandle;
		}

		void cleanup()
		{
			if ( handle_ != NULL_VALUE )
			{
				operator()(handle_);
				handle_ = NULL_VALUE;
			}
		}
	};


	namespace detail
	{
		// Release algorithms (release policies)

		template< typename T >
		struct close_handle_t
		{
			void operator()(T handle)
			{
				BOOL suc = ::CloseHandle(handle);
				assert(suc);
			}
		};



		template < typename T >
		struct close_reg_t
		{
			void operator()(T handle)
			{
				LONG ret = ::RegCloseKey(handle);
				assert(ERROR_SUCCESS == ret);
			}
		};


		template < typename T >
		struct close_libaray_t
		{
			void operator()(T handle)
			{
				BOOL suc = ::FreeLibrary(handle);
				assert(suc);
			}
		};


		template < typename T >
		struct close_mmap_file_t
		{
			void operator()(T handle)
			{
				BOOL suc = ::UnmapViewOfFile(handle);
				assert(suc);
			}
		};


		template < typename T >
		struct close_service_t
		{
			void operator()(T handle)
			{
				BOOL suc = ::CloseServiceHandle(handle);
				assert(suc);
			}
		};

		template < typename T >
		struct close_find_file_t
		{
			void operator()(T handle)
			{
				BOOL suc = ::FindClose(handle);
				assert(suc);
			}
		};

		template < typename T >
		struct close_icon_t
		{
			void operator()(T handle)
			{
				BOOL suc = ::DestroyIcon(handle);
				assert(suc);
			}
		};

	}


	// definitions of standard Windows handles
	
	typedef smart_handle_t<HANDLE,	detail::close_handle_t>		                    auto_handle;
	typedef smart_handle_t<HKEY,	detail::close_reg_t>		                    auto_reg;
	typedef smart_handle_t<PVOID,	detail::close_mmap_file_t>	                    auto_mmap_file;
	typedef smart_handle_t<HMODULE,	detail::close_libaray_t>						auto_libaray;
	typedef smart_handle_t<HANDLE,	detail::close_handle_t,		INVALID_HANDLE_VALUE>	auto_file;
	typedef smart_handle_t<SC_HANDLE, detail::close_service_t>						auto_service;
	typedef smart_handle_t<HANDLE,	detail::close_find_file_t,	INVALID_HANDLE_VALUE> auto_find_file;
	typedef smart_handle_t<HICON,	detail::close_icon_t>							auto_icon;
	typedef smart_handle_t<HANDLE,	detail::close_handle_t,		INVALID_HANDLE_VALUE>	auto_tool_help;
	typedef smart_handle_t<HANDLE,	detail::close_handle_t>							auto_token;

	

}


#endif