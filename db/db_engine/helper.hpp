#ifndef __DB_ENGINE_HELPER_HPP
#define __DB_ENGINE_HELPER_HPP


#include <string>
#include <sstream>
#include <cstdint>
#include <memory>
#include <vector>

//#include <google/protobuf/message.h>


namespace proto { 

	

	class columns_t
	{
		std::ostringstream os_;

	public:
		enum { is_sql_object = 1 };

	public:
		explicit columns_t(const std::string &col)
		{
			os_ << col;
		}
		columns_t(const std::string &col, const std::string &col1)
		{
			os_ << col << " , " << col1;
		}
		columns_t(const std::string &col, const std::string &col1, const std::string &col2)
		{
			os_ << col << " , " << col1 << " , " << col2;
		}
		columns_t(const std::string &col, const std::string &col1, const std::string &col2, const std::string &col3)
		{
			os_ << col << " , " << col1 << " , " << col2 << " , " << col3;
		}
		columns_t(const std::string &col, const std::string &col1, const std::string &col2, const std::string &col3, const std::string &col4)
		{
			os_ << col << " , " << col1 << " , " << col2 << " , " << col3 << " , " << col4;
		}
		columns_t(const std::string &col, const std::string &col1, const std::string &col2, const std::string &col3, const std::string &col4, const std::string &col5)
		{
			os_ << col << " , " << col1 << " , " << col2 << " , " << col3 << " , " << col4 << " , " << col5;
		}
		explicit columns_t(const std::string &col, const std::string &col1, const std::string &col2, const std::string &col3, const std::string &col4, const std::string &col5, const std::string &col6)
		{
			os_ << col << " , " << col1 << " , " << col2 << " , " << col3 << " , " << col4 << " , " << col5 << " , " << col6;
		}
		columns_t(const std::string &col, const std::string &col1, const std::string &col2, const std::string &col3, const std::string &col4, const std::string &col5, const std::string &col6, const std::string &col7)
		{
			os_ << col << " , " << col1 << " , " << col2 << " , " << col3 << " , " << col4 << " , " << col5 << " , " << col6 << " , " << col7;
		}
		columns_t(const std::string &col, const std::string &col1, const std::string &col2, const std::string &col3, const std::string &col4, const std::string &col5, const std::string &col6, const std::string &col7, const std::string &col8)
		{
			os_ << col << " , " << col1 << " , " << col2 << " , " << col3 << " , " << col4 << " , " << col5 << " , " << col6 << " , " << col7 << " , " << col8;
		}
		columns_t(const std::string &col, const std::string &col1, const std::string &col2, const std::string &col3, const std::string &col4, const std::string &col5, const std::string &col6, const std::string &col7, const std::string &col8, const std::string &col9)
		{
			os_ << col << " , " << col1 << " , " << col2 << " , " << col3 << " , " << col4 << " , " << col5 << " , " << col6 << " , " << col7 << " , " << col8 << " , " << col9;
		}
		~columns_t()
		{}

	private:
		columns_t(const columns_t &);
		columns_t &operator=(const columns_t &);

	public:
		std::string to_string() const
		{
			return os_.str();
		}

		operator std::string() const
		{
			return to_string();
		}
	};


	class all_columns_t
	{
		std::ostringstream os_;

	public:
		enum { is_sql_object = 1 };

	public:
		all_columns_t()
		{
			os_ << " * "; 
		}

	private:
		all_columns_t(const all_columns_t &);
		all_columns_t &operator=(const all_columns_t &);

	public:
		std::string to_string() const
		{
			return os_.str();
		}

		operator std::string() const
		{
			return to_string();
		}
	};

	class where_t
	{
		std::ostringstream os_;

	public:
		enum { is_sql_object = 1 };

	public:
		where_t();
		explicit where_t(const std::string &msg);

	private:
		where_t(const where_t &);
		where_t &operator=(const where_t &);

	public:
		template < typename T >
		where_t &operator==(const T &val)
		{
			os_ << " = " << val;
			return *this;
		}

		where_t &operator==(const std::string &val)
		{
			os_ << " = " << '\'' << val << '\'';
			return *this;
		}

		where_t &operator==(const char *val)
		{
			os_ << " = " << '\'' << val << '\'';
			return *this;
		}

		where_t &operator&&(const where_t &rhs);


		std::string to_string() const;
		operator std::string() const
		{
			return to_string();
		}
	};


	class order_by_t
	{
		std::ostringstream os_;

	public:
		enum { is_sql_object = 1 };

	public:
		explicit order_by_t(const std::string &msg);

	private:
		order_by_t(const order_by_t &);
		order_by_t &operator=(const order_by_t &);

	public:
		order_by_t &desc();

		std::string to_string() const;
		operator std::string() const
		{
			return to_string();
		}
	};


	class limit_t
	{
		std::uint32_t num_;

	public:
		enum { is_sql_object = 1 };

	public:
		explicit limit_t(std::uint32_t n);

	private:
		limit_t(const limit_t &);
		limit_t &operator=(const limit_t &);

	public:
		std::string to_string() const;
		operator std::string() const
		{
			return to_string();
		}
	};



	class into_t
	{
		std::ostringstream os_;

	public:
		enum { is_sql_object = 1 };

		template < typename T >
		struct into_impl_t
		{
			std::vector<std::shared_ptr<T>> &values_;

			into_impl_t(std::vector<std::shared_ptr<T>> &values)
				: values_(values)
			{}

			void handle()
			{

			}
		};

	public:
		template < typename T >
		into_t(std::vector<std::shared_ptr<T>> &msg)
		{
			static_assert(std::is_base_of<google::protobuf::Message, T>::value, 
				"T must be derived from google::protobuf::Message");

			into_impl_t<T>(msg).handle();
		}

	private:
		into_t(const into_t &);
		into_t &operator=(const into_t &);

	public:
		std::string to_string() const;

	};


	template < typename TableT >
	class from_t
	{
		std::ostringstream os_;

	public:
		enum { is_sql_object = 1 };

	public:
		from_t()
		{
			TableT msg;
			os_ << msg.GetTypeName();
		}
		~from_t()
		{}

	private:
		from_t(const from_t &);
		from_t &operator=(const from_t &);

	public:
		std::string to_string() const
		{
			return os_.str();
		}
	};


	class select_t
	{
		std::ostringstream os_;

	public:
		enum { is_sql_object = 1 };

	public:
		select_t();
		~select_t();

	private:
		select_t(const select_t &);
		select_t &operator=(const select_t &);

	public:
		template < typename T >
		select_t &operator<<(const T &val)
		{
			static_assert(T::is_sql_object, "T must sql object"); 
			os_ << val.to_string();

			return *this;
		}


		select_t &operator,(const into_t &val)
		{
			os_ << val.to_string();

			return *this;
		}

		std::string to_string() const
		{
			return os_.str();
		}
	};


}


#endif