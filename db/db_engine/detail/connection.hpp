#ifndef __DB_PROTOBUF_CONNECTION_HPP
#define __DB_PROTOBUF_CONNECTION_HPP

#include <memory>
#include <string>
#include <cstdint>

#include <comutil.h>

#include "../config.hpp"


namespace proto { namespace detail { 

	
	struct recordset_t
	{
		struct impl;
		std::unique_ptr<impl> impl_;

	public:
		recordset_t();
		~recordset_t();

	private:
		recordset_t(const recordset_t &);
		recordset_t &operator=(const recordset_t &);

	public:
		bool is_empty() const;
		bool is_eof() const;
		bool next();

		std::uint32_t get_field_count() const;
		std::string get_field_name(std::uint32_t index) const;
		_variant_t get_field_value(const std::string &filed_name) const;
		_variant_t get_field_value(std::uint32_t index) const;
	};
	typedef std::shared_ptr<recordset_t> recordset_ptr;


	// -----------------------------
	class connection_t
	{
		struct impl;
		std::unique_ptr<impl> impl_;

	public:
		explicit connection_t(const error_handler_t &);
		~connection_t();

	private:
		connection_t(const connection_t &);
		connection_t &operator=(const connection_t &);

	public:
		bool start(const std::string &connection_cmd);
		bool stop();

		void execute(const std::string &sql);
		recordset_ptr get_recordset(const std::string &sql);
	};
	typedef std::shared_ptr<connection_t> connection_ptr;

}
}


#endif