#include "connection.hpp"

#include <cstdint>
#include "exception.hpp"

#import "msado15.dll" no_namespace rename("EOF","adoEOF")


namespace proto { namespace detail {

	struct com_init
	{
		com_init()
		{
			::CoInitialize(0);
		}

		~com_init()
		{
			::CoUninitialize();
		}
	};

	void init_com()
	{
		static com_init com;
	}

	void test_result(HRESULT hr)
	{
		if( hr != S_OK )
		{
			assert(0);
			throw _com_error(hr);
		}
	}

	struct recordset_t::impl
	{
		_RecordsetPtr rds_impl_;

		impl()
		{}
	};


	recordset_t::recordset_t()
		: impl_(new impl)
	{

	}

	recordset_t::~recordset_t()
	{

	}

	bool recordset_t::is_empty() const
	{
		assert(impl_->rds_impl_);
		return impl_->rds_impl_->GetRecordCount() == 0;
	}

	bool recordset_t::is_eof() const
	{
		assert(impl_->rds_impl_);
		return impl_->rds_impl_->GetadoEOF() == -1;
	}

	bool recordset_t::next()
	{
		assert(impl_->rds_impl_);
		return impl_->rds_impl_->MoveNext() == S_OK;
	}

	std::uint32_t recordset_t::get_field_count() const
	{
		try
		{
			FieldsPtr fields_ptr = impl_->rds_impl_->GetFields();
			if( fields_ptr != nullptr ) 
				return fields_ptr->GetCount();
			else
				return 0;
		}
		catch(_com_error &e)
		{
			throw db_exception((const char *)e.Description());
		}
	}

	std::string recordset_t::get_field_name(std::uint32_t index) const
	{
		try
		{
			FieldsPtr fields_ptr = impl_->rds_impl_->GetFields();
			if( fields_ptr ) 
			{
				FieldPtr field_ptr = fields_ptr->GetItem((long)index);
				if( field_ptr ) 
				{
					return std::string(field_ptr->GetName());
				}
			}

			assert(0);
			return std::string();
		}
		catch(_com_error &e)
		{
			throw db_exception((const char *)e.Description());
		}
	}

	_variant_t recordset_t::get_field_value(const std::string &filed_name) const
	{
		assert(impl_->rds_impl_);

		try
		{
			return impl_->rds_impl_->GetCollect(filed_name.c_str());
		}
		catch(_com_error &e)
		{
			throw db_exception((const char *)e.Description());
		}
	}

	_variant_t recordset_t::get_field_value(std::uint32_t index) const
	{
		assert(impl_->rds_impl_);

		try
		{
			return impl_->rds_impl_->GetCollect((long)index);
		}
		catch(_com_error &e)
		{
			throw db_exception((const char *)e.Description());
		}
	}

	

	struct connection_t::impl
	{
		_ConnectionPtr connection_;
		error_handler_t handler_;

		impl(const error_handler_t &handler)
		{
			init_com();
		}

		~impl()
		{
			
		}

		bool start(const std::string &connection_cmd)
		{
			try 
			{
				test_result(connection_.CreateInstance("ADODB.Connection"));
				test_result(connection_->Open(connection_cmd.c_str(), "", "", adModeUnknown));
			} 
			catch(_com_error &e) 
			{
				handler_((const char *)e.Description());
				
				return false;
			}

			return true;
		}

		bool stop()
		{
			try 
			{
				assert(connection_);
				connection_->Close();
			} 
			catch(_com_error &e) 
			{
				handler_((const char *)e.Description());

				return false;
			}

			return true;
		}
	};


	connection_t::connection_t(const error_handler_t &handler)
		: impl_(new impl(handler))
	{

	}

	connection_t::~connection_t()
	{

	}

	bool connection_t::start(const std::string &connection_cmd)
	{
		return impl_->start(connection_cmd); 
	}

	bool connection_t::stop()
	{
		return impl_->stop();
	}

	void connection_t::execute(const std::string &sql)
	{
		get_recordset(sql);
	}

	recordset_ptr connection_t::get_recordset(const std::string &sql)
	{
		try 
		{
 			_CommandPtr command;
			test_result(command.CreateInstance(__uuidof(Command)));
			
			command->PutCommandText(sql.c_str());
			command->PutCommandType(adCmdText);
			command->ActiveConnection = impl_->connection_; 

			_RecordsetPtr recordset;
			test_result(recordset.CreateInstance(__uuidof(Recordset)));

			recordset->CursorLocation = adUseClient;
			test_result(recordset->Open(static_cast<IDispatch *>(command), vtMissing, adOpenStatic, adLockBatchOptimistic, adOptionUnspecified));
			
			recordset_ptr rds = std::make_shared<recordset_t>();
			rds->impl_->rds_impl_ = recordset;

			return rds;
		} 
		catch (_com_error& e) 
		{
			throw db_exception((const char *)e.Description());
		}
	}

}
}