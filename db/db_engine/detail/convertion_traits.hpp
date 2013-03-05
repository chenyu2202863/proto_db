#ifndef __DB_PROTOBUF_CONVVERTION_TRAITS_HPP
#define __DB_PROTOBUF_CONVVERTION_TRAITS_HPP

#include <map>
#include <functional>
#include <sstream>
#include <comutil.h>
#include <google/protobuf/descriptor.h>

#include "../config.hpp"


namespace proto { namespace detail {

	using namespace google::protobuf;

	// ----------------- type ---------
	template < int >
	struct type_traits_t
	{
		typedef void value_type;
		enum { value = FieldDescriptor::MAX_TYPE };
	};


	// --- int64
	template <>
	struct type_traits_t<FieldDescriptor::TYPE_FIXED64>
	{
		typedef std::int64_t value_type;

		enum{ value = FieldDescriptor::TYPE_FIXED64 };
	}; 

	template <>
	struct type_traits_t<FieldDescriptor::TYPE_INT64>
	{
		typedef std::int64_t value_type;

		enum{ value = FieldDescriptor::TYPE_INT64 };
	};

	template <>
	struct type_traits_t<FieldDescriptor::TYPE_UINT64>
	{
		typedef std::uint64_t value_type;

		enum{ value = FieldDescriptor::TYPE_UINT64 };
	};


	// --------int32
	template <>
	struct type_traits_t<FieldDescriptor::TYPE_FIXED32>
	{
		typedef std::int32_t value_type;

		enum{ value = FieldDescriptor::TYPE_FIXED32 };
	}; 

	template <>
	struct type_traits_t<FieldDescriptor::TYPE_INT32>
	{
		typedef std::int32_t value_type;

		enum{ value = FieldDescriptor::TYPE_INT32 };
	};

	template <>
	struct type_traits_t<FieldDescriptor::TYPE_UINT32>
	{
		typedef std::uint32_t value_type;

		enum{ value = FieldDescriptor::TYPE_UINT32 };
	};

	// -----------string
	
	template <>
	struct type_traits_t<FieldDescriptor::TYPE_STRING>
	{
		typedef std::string value_type;

		enum{ value = FieldDescriptor::TYPE_STRING };
	};


	// -----------double

	template <>
	struct type_traits_t<FieldDescriptor::TYPE_DOUBLE>
	{
		typedef double value_type;

		enum{ value = FieldDescriptor::TYPE_DOUBLE };
	};

	// ----------float
	template <>
	struct type_traits_t<FieldDescriptor::TYPE_FLOAT>
	{
		typedef float value_type;

		enum{ value = FieldDescriptor::TYPE_FLOAT };
	};

	// ----------bool
	template <>
	struct type_traits_t<FieldDescriptor::TYPE_BOOL>
	{
		typedef bool value_type;

		enum{ value = FieldDescriptor::TYPE_BOOL };
	};
	

	// --------- convertion -----------

	template < typename T >
	struct proto_conversion_traits_t
	{
		static std::string to(const Message &msg, const FieldDescriptor *field_desc, const Reflection *reflection)
		{
			assert(0);

			return std::string();
		}
	};

	template <>
	struct proto_conversion_traits_t<std::int64_t>
	{
		static std::string to(const Message &msg, const FieldDescriptor *field_desc, const Reflection *reflection)
		{
			std::ostringstream os;
			os << reflection->GetInt64(msg, field_desc);

			return os.str();
		}
	};
	template <>
	struct proto_conversion_traits_t<std::uint64_t>
	{
		static std::string to(const Message &msg, const FieldDescriptor *field_desc, const Reflection *reflection)
		{
			std::ostringstream os;
			os << reflection->GetUInt64(msg, field_desc);

			return os.str();
		}
	};

	template <>
	struct proto_conversion_traits_t<std::int32_t>
	{
		static std::string to(const Message &msg, const FieldDescriptor *field_desc, const Reflection *reflection)
		{
			std::ostringstream os;
			os << reflection->GetInt32(msg, field_desc);

			return os.str();
		}
	};

	template <>
	struct proto_conversion_traits_t<std::uint32_t>
	{
		static std::string to(const Message &msg, const FieldDescriptor *field_desc, const Reflection *reflection)
		{
			std::ostringstream os;
			os << reflection->GetUInt32(msg, field_desc);

			return os.str();
		}
	};

	template <>
	struct proto_conversion_traits_t<std::string>
	{
		static std::string to(const Message &msg, const FieldDescriptor *field_desc, const Reflection *reflection)
		{
			return "'" + reflection->GetString(msg, field_desc) + "'";
		}
	};

	template <>
	struct proto_conversion_traits_t<double>
	{
		static std::string to(const Message &msg, const FieldDescriptor *field_desc, const Reflection *reflection)
		{
			std::ostringstream os;
			os << reflection->GetDouble(msg, field_desc);

			return os.str();
		}
	};

	template <>
	struct proto_conversion_traits_t<float>
	{
		static std::string to(const Message &msg, const FieldDescriptor *field_desc, const Reflection *reflection)
		{
			std::ostringstream os;
			os << reflection->GetFloat(msg, field_desc);

			return os.str();
		}
	};

	template <>
	struct proto_conversion_traits_t<bool>
	{
		static std::string to(const Message &msg, const FieldDescriptor *field_desc, const Reflection *reflection)
		{
			std::ostringstream os;
			os << reflection->GetBool(msg, field_desc);

			return os.str();
		}
	};

	template < int N >
	struct proto_conversion_auto_register_t
	{
		template < typename ImplT >
		static void reg(ImplT &impl)
		{
			impl.insert(static_cast<FieldDescriptor::Type>(N), &proto_conversion_traits_t<type_traits_t<N>::value_type>::to);
			proto_conversion_auto_register_t<N - 1>::reg(impl);
		}
	};

	template <>
	struct proto_conversion_auto_register_t<0>
	{
		template < typename ImplT >
		static void reg(ImplT &impl)
		{
		}
	};


	struct proto_2_db_t
	{
		typedef std::function<std::string(const Message &, const FieldDescriptor *, const Reflection *)> convertion_t;
		typedef std::map<FieldDescriptor::Type, convertion_t> conversions_t;

		conversions_t conversions_;

		proto_2_db_t()
		{
			proto_conversion_auto_register_t<FieldDescriptor::MAX_TYPE>::reg(*this);
		}

		void insert(FieldDescriptor::Type type, const convertion_t &conver)
		{
			conversions_.insert(std::make_pair(type, conver));
		}

		const convertion_t &operator[](FieldDescriptor::Type type) const
		{
			auto iter = conversions_.find(type);
			assert(iter != conversions_.end());
			if( iter == conversions_.end() )
				throw std::runtime_error("not find this type");
			else
				return iter->second;
		}
	};


	template < int N >
	struct db_conversion_traits_t
	{
		static void run(const _variant_t &val, Message *msg, const Reflection *reflection, const FieldDescriptor *field_desc)
		{
			assert(0);
		}
	};

	template <>
	struct db_conversion_traits_t<VT_NULL>
	{
		static void run(const _variant_t &val, Message *msg, const Reflection *reflection, const FieldDescriptor *field_desc)
		{}
	};

	template <>
	struct db_conversion_traits_t<VT_EMPTY>
	{
		static void run(const _variant_t &val, Message *msg, const Reflection *reflection, const FieldDescriptor *field_desc)
		{}
	};

	template <>
	struct db_conversion_traits_t<VT_BOOL>
	{
		static void run(const _variant_t &val, Message *msg, const Reflection *reflection, const FieldDescriptor *field_desc)
		{
			reflection->SetBool(msg, field_desc, val);
		}
	};


	template <>
	struct db_conversion_traits_t<VT_INT>
	{
		static void run(const _variant_t &val, Message *msg, const Reflection *reflection, const FieldDescriptor *field_desc)
		{
			reflection->SetInt32(msg, field_desc, val);
		}
	};

	template <>
	struct db_conversion_traits_t<VT_UINT>
	{
		static void run(const _variant_t &val, Message *msg, const Reflection *reflection, const FieldDescriptor *field_desc)
		{
			reflection->SetUInt32(msg, field_desc, val);
		}
	};

	template <>
	struct db_conversion_traits_t<VT_DECIMAL>
	{
		static void run(const _variant_t &val, Message *msg, const Reflection *reflection, const FieldDescriptor *field_desc)
		{
			reflection->SetDouble(msg, field_desc, val);
		}
	};

	template <>
	struct db_conversion_traits_t<VT_DATE>
	{
		static void run(const _variant_t &val, Message *msg, const Reflection *reflection, const FieldDescriptor *field_desc)
		{
			reflection->SetUInt64(msg, field_desc, val);
		}
	};
	

	template <>
	struct db_conversion_traits_t<VT_I2>
	{
		static void run(const _variant_t &val, Message *msg, const Reflection *reflection, const FieldDescriptor *field_desc)
		{
			reflection->SetInt32(msg, field_desc, val);
		}
	};

	template <>
	struct db_conversion_traits_t<VT_I4>
	{
		static void run(const _variant_t &val, Message *msg, const Reflection *reflection, const FieldDescriptor *field_desc)
		{
			reflection->SetInt32(msg, field_desc, val);
		}
	};

	template <>
	struct db_conversion_traits_t<VT_I8>
	{
		static void run(const _variant_t &val, Message *msg, const Reflection *reflection, const FieldDescriptor *field_desc)
		{
			reflection->SetInt64(msg, field_desc, val);
		}
	};
	
	template <>
	struct db_conversion_traits_t<VT_BSTR>
	{
		static void run(const _variant_t &val, Message *msg, const Reflection *reflection, const FieldDescriptor *field_desc)
		{
			reflection->SetString(msg, field_desc, static_cast<const char *>(_bstr_t(val)));
		}
	};

	template <>
	struct db_conversion_traits_t<VT_LPSTR>
	{
		static void run(const _variant_t &val, Message *msg, const Reflection *reflection, const FieldDescriptor *field_desc)
		{
			reflection->SetString(msg, field_desc, static_cast<const char *>(_bstr_t(val)));
		}
	};
		
	// auto register
	template < int N >
	struct db_conversion_auto_register_t
	{
		template < typename ImplT >
		static void reg(ImplT &impl)
		{
			impl.insert(N, &db_conversion_traits_t<N>::run);
			db_conversion_auto_register_t<N - 1>::reg(impl);
		}
	};


	template < >
	struct db_conversion_auto_register_t<0>
	{
		template < typename ImplT >
		static void reg(ImplT &impl)
		{
		}
	};

	struct db_2_proto_t
	{
		typedef std::function<void(const _variant_t &, Message *, const Reflection *, const FieldDescriptor *)> convertion_t;
		typedef std::map<VARTYPE, convertion_t> convertions_t;

		convertions_t conversions_;

		db_2_proto_t()
		{
			db_conversion_auto_register_t<0xFF>::reg(*this);
		}

		void insert(VARTYPE type, const convertion_t &conver)
		{
			conversions_.insert(std::make_pair(type, conver));
		}

		const convertion_t &operator[](VARTYPE type) const
		{
			auto iter = conversions_.find(type);
			assert(iter != conversions_.end());
			if( iter == conversions_.end() )
				throw std::runtime_error("not find this type");
			else
				return iter->second;
		}

		
	};


}}

#endif
