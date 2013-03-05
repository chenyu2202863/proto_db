#include "build_sql.hpp"

#include <cstdint>
#include <sstream>

#include "convertion_traits.hpp"


namespace proto { namespace detail {

	using namespace google::protobuf;


	std::string translate_field_val(const Message &msg, const FieldDescriptor *field_desc, const Reflection *ref)
	{
		const auto type = field_desc->type();
		
		static proto_2_db_t type_converion;
		return type_converion[type](msg, field_desc, ref);
	}


	std::string build_sql::build_insert(const Message &msg)
	{
		std::ostringstream os;

		os << "insert into "
			<< msg.GetTypeName() << " "
			<< "(";

		const Descriptor *desc = msg.GetDescriptor();
		const Reflection *ref = msg.GetReflection();


		std::uint32_t field_cnt = desc->field_count();

		bool is_first = true;
		for(std::uint32_t i = 0; i != field_cnt; ++i)
		{
			if( !ref->HasField(msg, desc->field(i)) )
				continue;

			if( !is_first )
				os << ", ";

			is_first = false;

			auto field_desc = msg.GetDescriptor()->field(i);

			assert(field_desc->label() == FieldDescriptor::LABEL_OPTIONAL || 
				field_desc->label() == FieldDescriptor::LABEL_REQUIRED);

			const std::string &name = field_desc->name();
			os << name;
		}

		os << ")";
		os << " values (";

		is_first = true;
		for(std::uint32_t i = 0; i != field_cnt; ++i)
		{
			if( !ref->HasField(msg, desc->field(i)) )
				continue;

			if( !is_first )
				os << ", ";

			is_first = false;

			auto field_desc = msg.GetDescriptor()->field(i);
			os << translate_field_val(msg, field_desc, ref);
		}

		os << ")";

		return std::move(os.str());
	}

	std::string build_sql::build_update(const Message &msg, const Message &where)
	{
		std::ostringstream os;

		os << "update "
			<< msg.GetTypeName() << " "
			<< "set ";

		const Descriptor *desc = msg.GetDescriptor();
		const Reflection *ref = msg.GetReflection();

		std::uint32_t field_cnt = desc->field_count();

		bool is_first = true;
		for(std::uint32_t i = 0; i != field_cnt; ++i)
		{
			if( !ref->HasField(msg, desc->field(i)) )
				continue;

			if( !is_first )
				os << ", ";

			is_first = false;

			auto field_desc = desc->field(i);
			assert(field_desc->label() == FieldDescriptor::LABEL_OPTIONAL || 
				field_desc->label() == FieldDescriptor::LABEL_REQUIRED);

			const std::string &name = field_desc->name();
			os << name << " = " << translate_field_val(msg, field_desc, ref);
		}

		os << " where ";

		// where
		const Descriptor *where_desc = where.GetDescriptor();
		const Reflection *where_ref = where.GetReflection();

		std::uint32_t where_field_cnt = where_desc->field_count();
		bool is_where_first = true;
		for(std::uint32_t i = 0; i != where_field_cnt; ++i)
		{
			if( !ref->HasField(where, where_desc->field(i)) )
				continue;

			if( !is_where_first )
				os << "and";

			is_where_first = false;

			auto field_desc = where.GetDescriptor()->field(i);
			const std::string &name = field_desc->name();
			os << name << " = " << translate_field_val(where, field_desc, where_ref);
		}

		return os.str();
	}

	std::string build_sql::build_delete(const Message &where)
	{
		std::ostringstream os;

		os << "delete from "
			<< where.GetTypeName() << " "
			<< "where ";

		const Descriptor *where_desc = where.GetDescriptor();
		const Reflection *where_ref = where.GetReflection();

		std::uint32_t where_field_cnt = where_desc->field_count();
		bool is_where_first = true;
		for(std::uint32_t i = 0; i != where_field_cnt; ++i)
		{
			if( !where_ref->HasField(where, where_desc->field(i)) )
				continue;

			if( !is_where_first )
				os << "and";

			is_where_first = false;

			auto field_desc = where_desc->field(i);
			assert(field_desc->label() == FieldDescriptor::LABEL_OPTIONAL || 
				field_desc->label() == FieldDescriptor::LABEL_REQUIRED);

			const std::string &name = field_desc->name();
			os << name << " = " << translate_field_val(where, field_desc, where_ref);
		}

		return os.str();
	}


	std::string build_sql::build_select(const std::string &table_name, const std::string &columns, const std::string &where)
	{
		std::ostringstream os;
		os << "select " << columns << " from " << table_name << " " << where;

		return os.str();
	}
}}