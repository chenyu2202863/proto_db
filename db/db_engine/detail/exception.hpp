#ifndef __DB_PROTOBUF_EXCEPTION_HPP
#define __DB_PROTOBUF_EXCEPTION_HPP

#include <exception>


namespace proto { namespace detail {
	
	// db exception
	class db_exception
		: public std::exception
	{	
	public:
		explicit db_exception(const char * const &msg)
			: std::exception(msg)
		{}

		virtual ~db_exception()
		{}
	};


	// protobuf exception
	class proto_exception
		: public std::exception
	{	
	public:
		explicit proto_exception(const char * const &msg)
			: std::exception(msg)
		{}

		virtual ~proto_exception()
		{}
	};
}
}

#endif