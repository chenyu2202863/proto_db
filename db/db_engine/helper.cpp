#include "helper.hpp"

#include <cassert>


namespace proto { 

	// -- where
	where_t::where_t()
	{

	}
	where_t::where_t(const std::string &msg)
	{
		assert(!msg.empty());
		os_ << " where " << msg;
	}

	where_t &where_t::operator&&(const where_t &rhs)
	{
		os_ << " and " << rhs.os_.str();
		return *this;
	}

	std::string where_t::to_string() const
	{
		std::string msg = os_.str();

		std::uint32_t off = 0;
		std::uint32_t pos = msg.find("where", off);
		while( pos != std::string::npos )
		{
			if( pos > 6 )
				msg.erase(pos, 6);

			off += pos;
			pos = msg.find("where", off);
		}

		return std::move(msg);
	}


	// -- order by
	order_by_t::order_by_t(const std::string &msg)
	{
		os_ << msg;
	}

	order_by_t &order_by_t::desc()
	{
		os_ << " desc";
		return *this;
	}

	std::string order_by_t::to_string() const
	{
		return os_.str();
	}


	limit_t::limit_t(std::uint32_t n)
		: num_(n)
	{

	}


	std::string limit_t::to_string() const
	{
		std::ostringstream os;
		os << num_;
		return os.str();
	}

	select_t::select_t()
	{
		os_ << "select ";
	}

	select_t::~select_t()
	{

	}


	/*into_t::into_t(std::vector<google::protobuf::Message> &msg)
	{
	os_ << " from ";
	}*/


	std::string into_t::to_string() const
	{
		return os_.str();
	}

}