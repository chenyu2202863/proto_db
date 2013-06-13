#ifndef __EXTEND_STL_STRING_ALGORITHM_HPP
#define __EXTEND_STL_STRING_ALGORITHM_HPP

/** @algorithm.hpp
*
* @author <陈煜>
* [@author <chenyu2202863@yahoo.com.cn>]
* @date <2012/10/08>
* @version <0.1>
*
* 字符串常用算法
*/


#include <iomanip>
#include <algorithm>
#include <functional>
#include <type_traits>
#include <sstream>
#include <string>
#include <vector>
#include <cassert>
#include <cctype>
#include <windows.h>
#include "../../utility/select.hpp"	// for char or wchar_t

/*
大小写转换
	to_upper
	to_lower
	to_number
	to_string

吃掉空格
	trim_left
	trim_right
	trim

删除
	erase

不分大小写比较
	compare_no_case

匹配查找
	find_nocase

起始、结束匹配
	is_start_with
	is_end_with

分割
	split


*/


namespace stdex
{
	namespace detail
	{
		template < typename CharT, typename FuncT >
		struct ret_helper_t
		{
			std::basic_string<CharT> &str_;
			FuncT &func_;
			ret_helper_t(std::basic_string<CharT> &str, FuncT &func)
				: str_(str)
				, func_(func)
			{}

			operator std::basic_string<CharT> &()
			{
				std::transform(str_.begin(), str_.end(), str_.begin(), func_);

				return str_;
			}

			operator size_t() const
			{
				std::transform(str_.begin(), str_.end(), str_.begin(), func_);
				return str_.length();
			}
		};
	}


	/**
	* @brief 字符串所有字母转为大写，支持char、wchar_t
	* @param <str> <string字符串>
	* @exception <不会抛出任何异常>
	* @return <编译期根据接收类型隐式转换>
	* @note <编译期推导字符串是char or wchar_t>
	* @remarks <转换传入字符串所有字符为大写>
	*/
	template < typename CharT >
	void to_upper(std::basic_string<CharT> &&str)
	{
		std::transform(str.begin(), str.end(), str.begin(), ::toupper);
	}
	
	template < typename CharT >
	detail::ret_helper_t<CharT, decltype(::toupper)> to_upper(const std::basic_string<CharT> &str)
	{
		return detail::ret_helper_t<CharT, decltype(::toupper)>(const_cast<std::basic_string<CharT> &>(str), ::toupper);
	}

	// 支持字符串数组
	template < typename CharT, size_t N >
	inline void to_upper(CharT (&str)[N])
	{
		std::transform(str, str + N, str, ::toupper);
	}


	/**
	* @brief 字符串所有字母转为小写，支持char、wchar_t
	* @param <str> <string字符串>
	* @exception <不会抛出任何异常>
	* @return <编译期根据接收类型隐式转换>
	* @note <编译期推导字符串是char or wchar_t>
	* @remarks <转换传入字符串所有字符为小写>
	*/
	template < typename CharT >
	void to_lower(std::basic_string<CharT> &&str)
	{
		std::transform(str.begin(), str.end(), str.begin(), ::tolower);
	}

    template < typename CharT >
    detail::ret_helper_t<CharT, decltype(::tolower)> to_lower(const std::basic_string<CharT> &str)
    {
        return detail::ret_helper_t<CharT, decltype(::tolower)>(const_cast<std::basic_string<CharT> &>(str), ::tolower);
    }

	// 支持字符串数组
	template < typename CharT, size_t N >
	inline void to_lower(CharT (&str)[N])
	{
		std::transform(str, str + N, str, ::tolower);
	}


	namespace detail
	{
		template < typename CharT >
		struct to_number_helper_t
		{
			std::basic_istringstream<CharT> is_;

			to_number_helper_t(const std::basic_string<CharT> &str)
				: is_(str)
			{}

			~to_number_helper_t()
			{
				assert(is_.good() || is_.eof());
			}

			template < typename T >
			operator T()
			{
				static_assert(std::is_integral<T>::value || 
					std::is_enum<T>::value ||
					std::is_floating_point<T>::value, "T must is a number");

				T val = 0;
				is_ >> val;
				return val;
			}

			operator bool()
			{
				bool val = false;
				is_ >> std::boolalpha >> val;
				return val;
			}
		};
	}

	

	/**
	* @brief 把字符串转为number或bool，支持char、wchar_t
	* @param <str> <string字符串>
	* @exception <不会抛出任何异常>
	* @return <编译期根据接收类型隐式转换>
	* @note <编译期推导字符串是char or wchar_t>
	* @remarks <转换传入字符串为number或者bool类型>
	*/
	template < typename CharT >
	inline detail::to_number_helper_t<CharT> to_number(const std::basic_string<CharT> &str)
	{
		return detail::to_number_helper_t<CharT>(str);
	}

	template < typename CharT >
	inline detail::to_number_helper_t<CharT> to_number(const CharT *str)
	{
		std::basic_string<CharT> val(str);
		return detail::to_number_helper_t<CharT>(val);
	}


	
	namespace detail
	{
		template < typename T >
		struct to_string_helper_t
		{
			T val_;
			int prec_;
			bool is_boolalpha_;

			to_string_helper_t(const T &val, int prec = 0, bool is_boolalpha = false)
				: val_(val)
				, prec_(prec)
				, is_boolalpha_(is_boolalpha)
			{
				static_assert(std::is_integral<T>::value || 
					std::is_enum<T>::value ||
					std::is_floating_point<T>::value, "T must is a number");

				if( prec_ != 0 )
					assert(is_boolalpha == false);
			}

			template < typename CharT >
			operator std::basic_string<CharT>() const
			{
				std::basic_ostringstream<CharT> os;
				if( prec_ != 0 )
					os << std::setiosflags(std::ios::fixed) << std::setprecision(prec_);
				if( is_boolalpha_ )
					os << std::boolalpha;

				os << val_;

				assert(os.good() || os.eof());
				return os.str();
			}
		};
	}

	/**
	* @brief 把number或bool转为字符串，支持char、wchar_t
	* @param <val> <val为number或bool>
	* @exception <不会抛出任何异常>
	* @return <编译期根据接收类型隐式转换为std::string或std::wstring>
	* @note <编译期推导传入参数类型，但必须为numer或bool>
	* @remarks <>
	*/
	template < typename T >
	inline detail::to_string_helper_t<T> to_string(const T &val)
	{
		return detail::to_string_helper_t<T>(val);
	}

	template < typename T >
	inline detail::to_string_helper_t<T> to_string(const T &val, int prec)
	{
		return detail::to_string_helper_t<T>(val, prec);
	}

	inline detail::to_string_helper_t<bool> to_string(bool val)
	{
		return detail::to_string_helper_t<bool>(val, 0, true);
	}

	
	
	/**
	* @brief 字符串忽略大小写比较，支持char、wchar_t
	* @param <lhs> <字符串>
	* @param <rhs> <字符串>
	* @exception <不会抛出任何异常>
	* @return <返回int -1: lhs < rhs, 0: lhs == rhs, 1: lhs > rhs>
	* @note <编译期推导传入参数类型>
	* @remarks <>
	*/
	template < typename CharT >
	int compare_no_case(const CharT *lhs, const CharT *rhs)
	{
		return utility::select<CharT>(_stricmp, _wcsicmp)(lhs, rhs);
	}

	template < typename CharT >
	int compare_no_case(const std::basic_string<CharT> &lhs, const std::basic_string<CharT> &rhs)
	{
		return compare_no_case(lhs.c_str(), rhs.c_str());
			/*std::lexicographical_compare(lhs.begin(), lhs.end(),
			rhs.begin(), rhs.end(), 
			[](CharT c1, CharT c2){ return std::tolower(c1) < std::tolower(c2); });*/
	}

	
	/**
	* @brief 字符串按拼音先后比较，支持char、wchar_t
	* @param <lhs> <字符串>
	* @param <rhs> <字符串>
	* @exception <不会抛出任何异常>
	* @return <返回int -1: lhs < rhs, 0: lhs == rhs, 1: lhs > rhs， -2：出现错误>
	* @note <编译期推导传入参数类型>
	* @remarks <>
	*/
	template < typename CharT >
	int compare_phonetic(const std::basic_string<CharT> &lhs, const std::basic_string<CharT> &rhs)
	{
		LCID cid = MAKELCID(MAKELANGID(LANG_CHINESE_SIMPLIFIED, SUBLANG_CHINESE_SIMPLIFIED), SORT_CHINESE_PRCP);

		int ret = utility::select<CharT>(::CompareStringA, ::CompareStringW)(cid, 0, lhs.c_str(), lhs.length(), rhs.c_str(), rhs.length());
		switch(ret)
		{
		case CSTR_LESS_THAN:
			return (-1);
		case CSTR_EQUAL:
			return 0;
		case CSTR_GREATER_THAN:
			return 1;
		default:
			assert(false);
			return (-2);
		}
	}


	/**
	* @brief 字符串替换，支持char、wchar_t
	* @param <str> <需要替换的字符串>
	* @param <old_value> <被替换的匹配字符串>
	* @param <new_value> <用来替换的字符串>
	* @exception <不会抛出任何异常>
	* @return <返回替换后的字符串>
	* @note <编译期推导传入参数类型>
	* @remarks <>
	*/
	template < typename CharT >
	std::basic_string<CharT> &replace_all(std::basic_string<CharT> &str, const std::basic_string<CharT> &old_value, const std::basic_string<CharT> &new_value)     
	{     
		while(true)   
		{     
			std::basic_string<CharT>::size_type pos(0);     
			if( ( pos = str.find(old_value)) != std::basic_string<CharT>::npos )     
				str.replace(pos, old_value.length(), new_value);     
			else   
				break;     
		}     
		return str;     
	}     

	
	/**
	* @brief 删除字符串左边全部空格，支持char、wchar_t
	* @param <str> <字符串>
	* @exception <不会抛出任何异常>
	* @return <无>
	* @note <编译期推导传入参数类型>
	* @remarks <>
	*/
	template< typename CharT >
	inline void trim_left(std::basic_string<CharT> &str)
	{
		str.erase(0, str.find_first_not_of(' '));
	}


	/**
	* @brief 删除字符串右边全部空格，支持char、wchar_t
	* @param <str> <字符串>
	* @exception <不会抛出任何异常>
	* @return <无>
	* @note <编译期推导传入参数类型>
	* @remarks <无>
	*/
	template< typename CharT >
	inline void trim_right(std::basic_string<CharT> &str)
	{
		str.erase(str.find_last_not_of(' ') + 1);
	}


	/**
	* @brief 删除字符串左右两边全部空格，支持char、wchar_t
	* @param <str> <字符串>
	* @exception <不会抛出任何异常>
	* @return <无>
	* @note <编译期推导传入参数类型>
	* @remarks <无>
	*/
	template< typename CharT >
	inline void trim(std::basic_string<CharT> &str)
	{
		str.erase(0, str.find_first_not_of(' '));
		str.erase(str.find_last_not_of(' ') + 1);
	}

	/**
	* @brief 删除字符串中所有指定字符，支持char、wchar_t
	* @param <str> <引用传递需要删除的字符串>
	* @param <charactor> <需要删除的字符>
	* @exception <不会抛出任何异常>
	* @return <无>
	* @note <编译期推导传入参数类型>
	* @remarks <无>
	*/	
	template < typename CharT >
	inline void erase(std::basic_string<CharT> &str, const CharT &charactor)
	{
		str.erase(std::remove_if(str.begin(), str.end(), 
			std::bind2nd(std::equal_to<CharT>(), charactor)), str.end());
	}


	/**
	* @brief 判断字符串是否以指定字符串开头，支持char、wchar_t
	* @param <str> <需要判断的字符串>
	* @param <src> <用以判断标准的字符串>
	* @exception <不会抛出任何异常>
	* @return <如果是str中以src为开头，则返回true，否则返回false>
	* @note <编译期推导传入参数类型>
	* @remarks <无>
	*/	
	template < typename CharT >
	inline bool is_start_with(const std::basic_string<CharT> &str, const std::basic_string<CharT> &src)
	{
		return str.compare(0, src.size(), src) == 0;
	}


	/**
	* @brief 判断字符串是否以指定字符串结尾，支持char、wchar_t
	* @param <str> <需要判断的字符串>
	* @param <src> <用以判断标准的字符串>
	* @exception <不会抛出任何异常>
	* @return <如果是str中以src为结尾，则返回true，否则返回false>
	* @note <编译期推导传入参数类型>
	* @remarks <无>
	*/
	template < typename CharT >
	inline bool is_end_with(const std::basic_string<CharT> &str, const std::basic_string<CharT> &src)
	{
		return str.compare(str.size() - src.size(), src.size(), src) == 0;
	}


	/**
	* @brief 忽略大小写查找字符串是否在目的字符串中存在，支持char、wchar_t
	* @param <str> <需要判断的字符串>
	* @param <val> <需要查找的字符串字符串>
	* @exception <不会抛出任何异常>
	* @return <如果找到则返回true，否则返回false>
	* @note <编译期推导传入参数类型>
	* @remarks <无>
	*/
	template < typename CharT >
	inline bool find_nocase(const std::basic_string<CharT> &str, const std::basic_string<CharT> &val)
	{
		auto iter = std::search(str.begin(), str.end(), val.begin(), val.end(), 
			[](CharT lhs, CharT rhs) { return ::tolower(lhs) == ::tolower(rhs); });
		return iter != str.end();
	}

	
	/**
	* @brief 以指定字符分割字符串，支持char、wchar_t
	* @param <seq> <分割后的字符串集合>
	* @param <str> <需要分割的字符串>
	* @param <separator> <用以分割的字符>
	* @exception <不会抛出任何异常>
	* @return <无>
	* @note <编译期推导传入参数类型>
	* @remarks <分割成功的字符串放入vector中返回>
	*/
	template < typename ValueType, typename CharT >
    inline void split(std::vector<ValueType> &seq, const std::basic_string<CharT> &str, CharT separator)
	{
		if( str.empty() )
			return;

		std::basic_istringstream<CharT> iss(str);
		for(std::basic_string<CharT> s; std::getline(iss, s, separator); )
		{
            ValueType val;
			std::basic_istringstream<CharT> isss(s);

			isss >> val;

			assert(isss.good() || isss.eof());
			seq.push_back(std::move(val));
		}

		return;
	}

	/**
	* @brief 以指定字符分割字符串，返回指定位置的字符串，支持char、wchar_t
	* @param <str> <需要分割的字符串>
	* @param <separator> <用以分割的字符>
	* @param <index> <分割后的字符串位置，下标0开头>
	* @exception <out_of_range>
	* @return <返回指定下标字符串，如果下标越界，则抛出异常out_of_range>
	* @note <编译期推导传入参数类型>
	* @remarks <分割成功的字符串放入vector中返回>
	*/
	template < typename CharT >
	inline std::basic_string<CharT> split(const std::basic_string<CharT> &str, CharT separator, size_t index)
	{
		std::vector<std::basic_string<CharT>> seq;
		split(seq, str, separator);

		if( seq.empty() )
			return std::basic_string<CharT>();

		assert(index < seq.size());
		if( index >= seq.size() )
		{			
			return std::basic_string<CharT>();
		}
		
		return seq[index];
	}


}


#endif