#include "parse_recordset.hpp"

#include "convertion_traits.hpp"
#include "exception.hpp"

namespace proto { namespace detail {

	using namespace google::protobuf;

	msg_ptr create_message(const std::string &name)
	{
		msg_ptr msg;
		const Descriptor* descriptor = DescriptorPool::generated_pool()->FindMessageTypeByName(name);

		if( descriptor )
		{
			const Message* prototype = MessageFactory::generated_factory()->GetPrototype(descriptor);
			if( prototype )
			{
				msg.reset(prototype->New());
			}
		}

		return msg;
	}

	void translate_db_val(const _variant_t &val, Message *msg, const FieldDescriptor *field_desc, const Reflection *reflection)
	{
		static db_2_proto_t db_2_proto;
		db_2_proto[val.vt](val, msg, reflection, field_desc);
	}


	struct type_conversion_t
	{
		const msg_ptr &msg_;
		const detail::recordset_ptr &rds_;

		type_conversion_t(const msg_ptr &msg, const detail::recordset_ptr &rds)
			: msg_(msg)
			, rds_(rds)
		{}

		void run()
		{
			std::uint32_t filed_cnt = rds_->get_field_count();

			const Descriptor *desc = msg_->GetDescriptor();
			const Reflection *reflection = msg_->GetReflection();


			for(std::uint32_t i = 0; i != filed_cnt; ++i)
			{
				std::string filed_name = rds_->get_field_name(i);
				_variant_t filed_value = rds_->get_field_value(i);

				const FieldDescriptor *field_desc = desc->FindFieldByName(filed_name);
				if( !field_desc )
					continue;

				translate_db_val(filed_value, msg_.get(), field_desc, reflection);
			}
		}
	};


	msg_ptr parse_rds::parse(const std::string &table_name, const recordset_ptr &rds)
	{
		msg_ptr msg_val = create_message(table_name);
		
		assert(msg_val);
		if( !msg_val )
			throw proto_exception("msg create failed");

		type_conversion_t type_conversion_val(msg_val, rds);
		type_conversion_val.run();

		return msg_val;
	}
}
}