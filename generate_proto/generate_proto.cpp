// generate_proto.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"


#include <cstdint>
#include <map>
#include <list>
#include <string>
#include <algorithm>
#include <iostream>
#include <sstream>
#include <fstream>
#include <set>
#include <iterator>

#include <sql_database.h>
#include <sql_error.h>

#include <extend_stl/string/algorithm.hpp>
#include <utility/utility.hpp>
#include <win32\system\system_helper.hpp>
#include <win32\file_system\file_helper.hpp>

struct column_info_t
{
	std::wstring name_;
	std::uint32_t size_;
	DataTypeEnum type_;

	column_info_t()
		: size_(0)
	{}
};
typedef std::list<column_info_t> columns_info_t;
typedef std::map<std::wstring, columns_info_t> all_table_columns_t;


struct type_map
{
	std::map<std::wstring, int> desc_;

	type_map()
	{
		_insert(adEmpty, L"adEmpty");
		_insert(adTinyInt, L"adTinyInt");
		_insert(adSmallInt, L"adSmallInt");
		_insert(adInteger, L"adInteger");
		_insert(adBigInt, L"adBigInt");
		_insert(adUnsignedTinyInt, L"adUnsignedTinyInt");
		_insert(adUnsignedSmallInt, L"adUnsignedSmallInt");
		_insert(adUnsignedInt, L"adUnsignedInt");
		_insert(adUnsignedBigInt, L"adUnsignedBigInt");
		_insert(adSingle, L"adSingle");
		_insert(adDouble, L"adDouble");
		_insert(adCurrency, L"adCurrency");
		_insert(adDecimal, L"adDecimal");
		_insert(adNumeric, L"adNumeric");
		_insert(adBoolean, L"adBoolean");
		_insert(adError, L"adError");
		_insert(adUserDefined, L"adUserDefined");
		_insert(adVariant, L"adVariant");
		_insert(adIDispatch, L"adIDispatch");
		_insert(adIUnknown, L"adIUnknown");
		_insert(adGUID, L"adGUID");
		_insert(adDate, L"adDate");
		_insert(adDBDate, L"adDBDate");
		_insert(adDBTime, L"adDBTime");
		_insert(adDBTimeStamp, L"adDBTimeStamp");
		_insert(adBSTR, L"adBSTR");
		_insert(adChar, L"adChar");
		_insert(adVarChar, L"adVarChar");
		_insert(adLongVarChar, L"adLongVarChar");
		_insert(adWChar, L"adWChar");
		_insert(adVarWChar, L"adVarWChar");
		_insert(adLongVarWChar, L"adLongVarWChar");
		_insert(adBinary, L"adBinary");
		_insert(adVarBinary, L"adVarBinary");
		_insert(adLongVarBinary, L"adLongVarBinary");
		_insert(adChapter, L"adChapter");
		_insert(adFileTime, L"adFileTime");
		_insert(adPropVariant, L"adPropVariant");
		_insert(adVarNumeric, L"adVarNumeric");
		_insert(adArray, L"adArray");
	}

	void _insert(int n, const std::wstring &desc)
	{
		desc_.insert(std::make_pair(desc, n));
	}

	DataTypeEnum operator[](const std::wstring &desc) const
	{
		auto iter = desc_.find(desc);
		assert(iter != desc_.end());
		if( iter == desc_.end() )
			throw std::runtime_error("not find this type description");

		return DataTypeEnum(iter->second);
	}
};



struct type_convertion_t
{
	std::map<DataTypeEnum, std::wstring> define_type_map_;
	type_map type_map_;

	type_convertion_t()
	{}

	void load(const std::wstring &path)
	{
		std::wifstream in(path, std::ios::binary | std::ios::in);
		assert(in.good());

		if( !in.good() )
			throw std::runtime_error("load type cast file error");

		std::wstring line;
		while(std::getline(in, line))
		{
			if( line.empty() )
				continue;

			std::vector<std::wstring> types;
			stdex::split(types, line, L'=');

			if( types.size() != 2 )
				continue;

			stdex::trim(types[0]);
			stdex::trim(types[1]);

			
			define_type_map_.insert(std::make_pair(type_map_[types[0]], types[1]));
		}

	}

	const std::wstring &cast(DataTypeEnum db_type) const
	{
		auto iter = define_type_map_.find(db_type);
		assert(iter != define_type_map_.end());
		if( iter == define_type_map_.end() )
			throw std::runtime_error("not find this type");

		return iter->second;
	}
};


struct scan_db
{
	sql::Database db_;
	all_table_columns_t &all_columns_;

	scan_db(all_table_columns_t &all_columns, const std::wstring &ip, std::uint16_t port, const std::wstring &db_name, const std::wstring &user_name, const std::wstring &psw)
		: all_columns_(all_columns)
	{
		::CoInitialize(0);

		if( !db_.open(ip, port, db_name, user_name, psw, 10 * 1000) )
			throw std::runtime_error("open db error");
	}

	~scan_db()
	{
		db_.close();

		::CoUninitialize();
	}

	void scan_table()
	{
		auto rds = db_.native_connection()->OpenSchema(adSchemaTables);
		while(!(rds ->adoEOF))
		{
			std::wstring table_name = (const wchar_t *)(_bstr_t)(rds->Fields->GetItem("TABLE_NAME")->Value);//获取表格
			std::wstring table_type = (const wchar_t *)(_bstr_t)(rds->Fields->GetItem("TABLE_TYPE")->Value);//获取表格类型

			if( table_type == L"TABLE" )
				all_columns_.insert(std::make_pair(table_name, all_table_columns_t::mapped_type()));
			rds->MoveNext();
		}
		rds->Close();
	}

	void scan_column()
	{
		
		std::for_each(all_columns_.begin(), all_columns_.end(), [this](all_table_columns_t::value_type &val)
		{
			std::wostringstream os;
			os << L"select top 1 * from " << val.first;
			auto rds = db_.execute_sql(os.str());

			std::uint32_t col_cnt = rds->get_field_count();
			FieldsPtr fields_ptr = rds->native_record_set()->GetFields();

			for(long i = 0; i != col_cnt; ++i)
			{
				column_info_t col;
				if (fields_ptr != nullptr)
				{
					FieldPtr field_ptr = fields_ptr->GetItem(i);
					if (field_ptr != nullptr) 
					{ 
						col.name_ = (const wchar_t *)field_ptr->GetName();
						col.size_ = field_ptr->GetDefinedSize();
						col.type_ = field_ptr->GetType();

						val.second.push_back(col);
					}
				}
			}
		});

	}

	void scan()
	{
		try
		{
			scan_table();
			scan_column();
		}
		catch(_com_error &e)
		{
			throw std::runtime_error((const char *)e.Description());
		}
	}
};

struct generate_proto
{
	const all_table_columns_t &all_columns_;
	const type_convertion_t &type_convertion_;

	generate_proto(const all_table_columns_t &all_columns, const type_convertion_t &type_convertion)
		: all_columns_(all_columns)
		, type_convertion_(type_convertion)
	{}

	void generate(const std::wstring &path)
	{
		std::map<std::wstring, std::wstring> files_path;

		std::for_each(all_columns_.begin(), all_columns_.end(), 
			[this, &path, &files_path](const all_table_columns_t::value_type &val)
		{
			auto this_val = this;
			std::wostringstream os;
			os << L"message " << val.first << std::endl
				<< L"{" << std::endl;

			std::uint32_t index = 1;
			std::for_each(val.second.begin(), val.second.end(), 
				[this_val, &os, &index](const column_info_t &col)
			{
				os << L"	optional " << this_val->type_convertion_.cast(col.type_) << L" " << col.name_ << L" = " << index << L";" << std::endl;
				++index;
			});

			os << L"}";
			
			std::wstring file_path = path + L"/" + val.first + L".proto";
			files_path.insert(std::make_pair(val.first + L".proto", file_path));

			std::wofstream out(file_path, std::ios::out | std::ios::binary);

			assert(out.good());
			if( !out.good() )
				throw std::runtime_error("创建proto文件失败");

			std::wstring buffer = os.str();
			out.write(buffer.c_str(), buffer.size());
		});

		const std::wstring &proto_file_path = utility::get_app_path() + L"protoc.exe";
		if( !win32::filesystem::is_file_exist(proto_file_path) )
			throw std::runtime_error("protoc.exe not exist");

		::SetCurrentDirectory(utility::get_app_path().c_str());
		win32::filesystem::mk_directory(utility::get_app_path() + L"proto");

		std::for_each(files_path.begin(), files_path.end(), 
			[&proto_file_path](const std::pair<std::wstring, std::wstring> &path)
		{
			std::wstring param = path.first + L" --cpp_out=./proto/";
			std::wstring run_param = proto_file_path + _T(" ") + param;
			win32::system::start_process(run_param);
		});
	}
};


struct type_desc
{
	std::map<int, std::wstring> desc_;

	type_desc()
	{
		_insert(adEmpty, L"adEmpty");
		_insert(adTinyInt, L"adTinyInt");
		_insert(adSmallInt, L"adSmallInt");
		_insert(adInteger, L"adInteger");
		_insert(adBigInt, L"adBigInt");
		_insert(adUnsignedTinyInt, L"adUnsignedTinyInt");
		_insert(adUnsignedSmallInt, L"adUnsignedSmallInt");
		_insert(adUnsignedInt, L"adUnsignedInt");
		_insert(adUnsignedBigInt, L"adUnsignedBigInt");
		_insert(adSingle, L"adSingle");
		_insert(adDouble, L"adDouble");
		_insert(adCurrency, L"adCurrency");
		_insert(adDecimal, L"adDecimal");
		_insert(adNumeric, L"adNumeric");
		_insert(adBoolean, L"adBoolean");
		_insert(adError, L"adError");
		_insert(adUserDefined, L"adUserDefined");
		_insert(adVariant, L"adVariant");
		_insert(adIDispatch, L"adIDispatch");
		_insert(adIUnknown, L"adIUnknown");
		_insert(adGUID, L"adGUID");
		_insert(adDate, L"adDate");
		_insert(adDBDate, L"adDBDate");
		_insert(adDBTime, L"adDBTime");
		_insert(adDBTimeStamp, L"adDBTimeStamp");
		_insert(adBSTR, L"adBSTR");
		_insert(adChar, L"adChar");
		_insert(adVarChar, L"adVarChar");
		_insert(adLongVarChar, L"adLongVarChar");
		_insert(adWChar, L"adWChar");
		_insert(adVarWChar, L"adVarWChar");
		_insert(adLongVarWChar, L"adLongVarWChar");
		_insert(adBinary, L"adBinary");
		_insert(adVarBinary, L"adVarBinary");
		_insert(adLongVarBinary, L"adLongVarBinary");
		_insert(adChapter, L"adChapter");
		_insert(adFileTime, L"adFileTime");
		_insert(adPropVariant, L"adPropVariant");
		_insert(adVarNumeric, L"adVarNumeric");
		_insert(adArray, L"adArray");
	}

	void _insert(int n, const std::wstring &desc)
	{
		desc_.insert(std::make_pair(n, desc));
	}

	const std::wstring &operator[](int n) const
	{
		auto iter = desc_.find(n);
		assert(iter != desc_.end());

		return iter->second;
	}
};

int _tmain(int argc, _TCHAR* argv[])
{
	try
	{
		all_table_columns_t all_columns;
		sql::set_error_handler([](HRESULT hr, const std::wstring &msg)
		{
			std::wcout << msg << std::endl;
		});

		scan_db db(all_columns, L"10.34.41.97", 1433, L"wxol", L"wxol", L"123");

		db.scan();

		{
			type_desc type_desc_val;
			std::set<std::wstring> desc_vals;
			std::for_each(all_columns.begin(), all_columns.end(), 
				[&desc_vals, &type_desc_val](const all_table_columns_t::value_type &val)
			{
				auto &desc_ = desc_vals;
				const auto &type_desc_ = type_desc_val;
				std::for_each(val.second.begin(), val.second.end(), [&desc_, &type_desc_](const column_info_t &val)
				{
					desc_.insert(type_desc_[val.type_]);
				});

			});

			std::copy(desc_vals.begin(), desc_vals.end(), std::ostream_iterator<std::wstring, wchar_t>(std::wcout, L"\r\n"));

		}
		

		type_convertion_t type_cast;
		type_cast.load(utility::get_app_path() + L"type_convertion.dat");
		
		generate_proto ge(all_columns, type_cast);
		ge.generate(utility::get_app_path());

	}
	catch(std::exception &e)
	{
		::MessageBoxA(NULL, e.what(), "error", MB_OK | MB_ICONERROR);
	}

	system("pause");
	return 0;
}

