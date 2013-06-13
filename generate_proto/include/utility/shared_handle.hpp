#ifndef __UTILITY_SHARED_HANDLE_HPP
#define __UTILITY_SHARED_HANDLE_HPP


#include <memory>
#include <Windows.h>



namespace utility
{
	

	/*
	struct xxx_tag
	{ 
	typedef xxx type;
	typedef BOOL (__stdcall *deletor_fun_type)(type);

	static deletor_fun_type GetDeletor()
	{ return &::DestroyIcon; }
	};


	*/

	// ----------------------------------------
	// 带引用计数的句柄
	// ResourceTagT 代表某种资源

	template < typename ResouceTagT >
	struct shared_handle_t
	{
		typedef ResouceTagT ResourceType;
		typedef typename ResourceType::type type;

		typedef std::shared_ptr<void> ResType;
		ResType res_;

		shared_handle_t()
		{}
		shared_handle_t(const type &res)
			: res_(res, ResourceType::GetDeletor())
		{}
		shared_handle_t(const shared_handle_t &rhs)
			: res_(rhs.res_)
		{}
		shared_handle_t &operator=(const type &res)
		{
			if( res_.get() != res )
				res_.reset(res, ResourceType::GetDeletor());

			return *this;
		}
		shared_handle_t &operator=(const shared_handle_t &rhs)
		{
			if( this != &rhs )
				res_ = rhs.res_;

			return *this;
		}

		operator ResType &()
		{ return res_; }
		operator const ResType &() const
		{ return res_; }

		template < typename T >
		operator T ()
		{ 
			static_assert(std::is_same<T, type>::value, "T must be a type");
			return static_cast<type>(res_.get()); 
		}

		operator bool() const
		{
			return is_valid();
		}

		bool is_valid() const
		{
			return ((type *)(res_.get())) == 0;
		}
	};

	template < typename T >
	bool operator==(const shared_handle_t<T> &lhs, const shared_handle_t<T> &rhs)
	{
		return lhs.res_ == rhs.res_;
	}

	template < typename T >
	bool operator!=(const shared_handle_t<T> &lhs, const shared_handle_t<T> &rhs)
	{
		return !(lhs.res_ == rhs.res_);
	}

	template < typename T >
	bool operator==(const shared_handle_t<T> &lhs, size_t val)
	{
		assert(val == 0);
		return lhs.res_.get() == 0;
	}

	template < typename T >
	bool operator!=(const shared_handle_t<T> &lhs, size_t val)
	{
		assert(val == 0);
		return !(lhs.res_ == 0);
	}


	namespace detail
	{
		struct icon_tag
		{ 
			typedef HICON type;
			typedef BOOL (__stdcall *deletor_fun_type)(type);

			static deletor_fun_type GetDeletor()
			{ return &::DestroyIcon; }

			static size_t null()
			{
				return 0;
			}
		};

		struct bitmap_tag
		{
			typedef HBITMAP type;
			typedef BOOL (__stdcall *deletor_fun_type)(HGDIOBJ );

			static deletor_fun_type GetDeletor()
			{ return &::DeleteObject; }

			static size_t null()
			{
				return 0;
			}
		};

		struct handle_tag
		{
			typedef HANDLE type;
			typedef BOOL (__stdcall *deletor_fun_type)(type);

			static deletor_fun_type GetDeletor()
			{ return &::CloseHandle; }

			static size_t null()
			{
				return 0;
			}
		};
	}

	
	// 预定义

	typedef shared_handle_t<detail::icon_tag>		icon_ptr;
	typedef shared_handle_t<detail::handle_tag>		handle_ptr;
	typedef shared_handle_t<detail::bitmap_tag>		bitmap_ptr;
}


#endif