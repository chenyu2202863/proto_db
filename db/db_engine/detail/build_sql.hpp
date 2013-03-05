#ifndef __DB_PROTOBUF_BUILD_SQL_HPP
#define __DB_PROTOBUF_BUILD_SQL_HPP

#include <string>
#include "../config.hpp"

namespace proto { namespace detail {
	 
	struct build_sql
	{

		std::string build_select(const std::string &table_name, const std::string &columns, const std::string &where);
		std::string build_insert(const google::protobuf::Message &msg);
		std::string build_update(const google::protobuf::Message &msg, const google::protobuf::Message &where);
		std::string build_delete(const google::protobuf::Message &where);
	};
}
}

#endif