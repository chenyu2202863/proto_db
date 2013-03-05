#include "engine.hpp"

#include <cstdint>
#include <sstream>

#include "detail/connection.hpp"
#include "detail/build_sql.hpp"
#include "detail/parse_recordset.hpp"
#include "detail/exception.hpp"


namespace proto
{
	using namespace google::protobuf;


	struct engine::impl
	{
		error_handler_t handler_;
		detail::connection_t db_impl_;
		

		impl(const error_handler_t &handler)
			: handler_(handler)
			, db_impl_(handler_)
		{

		}

		~impl()
		{

		}

		bool start(const std::string &connection_cmd)
		{
			return db_impl_.start(connection_cmd);
		}

		bool stop()
		{
			return db_impl_.stop();
		}
	};

	engine::engine(const error_handler_t &handler)
		: impl_(new impl(handler))
	{

	}

	engine::~engine()
	{

	}

	bool engine::start(const std::string &connection_cmd)
	{
		return impl_->start(connection_cmd);
	}

	bool engine::stop()
	{
		return impl_->stop();
	}

	bool engine::sql_insert(const Message &msg)
	{
		detail::build_sql build;
		std::string sql = build.build_insert(msg);

		return _run_impl(sql);
	}

	bool engine::sql_update(const Message &msg, const Message &where)
	{
		detail::build_sql build;
		std::string sql = build.build_update(msg, where);
		return _run_impl(sql);
	}

	bool engine::sql_delete(const Message &where)
	{
		detail::build_sql build;
		std::string sql = build.build_delete(where);
		return _run_impl(sql);
	}


	bool engine::_run_impl(const std::string &msg)
	{
		try
		{
			impl_->db_impl_.execute(msg);

			return true;
		}
		catch(std::exception &e)
		{
			impl_->handler_(e.what());
			return false;
		}
	}

	



	std::vector<msg_ptr> engine::_select_impl(const std::string &table_name, const std::string &columns, const std::string &where)
	{
		detail::build_sql build;
		std::string sql = build.build_select(table_name, columns, where);

		try
		{
			const detail::recordset_ptr &rds = impl_->db_impl_.get_recordset(sql);

			std::vector<msg_ptr> values;
			for(; !rds->is_eof(); rds->next())
			{
				detail::parse_rds parse;
				auto msg_val = parse.parse(table_name, rds);

				values.push_back(msg_val);
			}

			return values;
		}
		catch(std::exception &e)
		{
			impl_->handler_(e.what());
			return std::vector<msg_ptr>();
		}

	}
	
}