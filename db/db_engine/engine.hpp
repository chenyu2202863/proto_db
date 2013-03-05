#ifndef __DB_PROTOBUF_ENGINE_HPP
#define __DB_PROTOBUF_ENGINE_HPP

#include <vector>

#include "config.hpp"
#include "helper.hpp"


namespace proto { 

	

	class engine
	{
		struct impl;
		std::unique_ptr<impl> impl_;

		
	public:
		engine(const error_handler_t &);
		~engine();

	private:
		engine(const engine &);
		engine &operator=(const engine &);

	public:
		bool start(const std::string &connection_cmd);
		bool stop();

	public:
		bool sql_insert(const google::protobuf::Message &msg);
		bool sql_update(const google::protobuf::Message &msg, const google::protobuf::Message &where);
		bool sql_delete(const google::protobuf::Message &where);
		
		template < typename T, typename ColumnT >
		std::vector<std::shared_ptr<T>> sql_select(const ColumnT &cols, const where_t &where)
		{
			std::vector<msg_ptr> &&tmp = _select_impl(T().GetTypeName(), cols.to_string(), where.to_string());

			std::vector<std::shared_ptr<T>> values;
			values.reserve(tmp.size());
			std::for_each(tmp.begin(), tmp.end(), [&values](const msg_ptr &val)
			{
				values.push_back(std::static_pointer_cast<T>(val));
			});
			return values;
		}

	private:
		bool _run_impl(const std::string &msg);
		std::vector<msg_ptr> _select_impl(const std::string &table_name, const std::string &columns, const std::string &where);
	};	

	


	
}


#endif