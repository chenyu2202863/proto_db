// db.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "db_engine.hpp"
#include "x.pb.h"
#include <iostream>


#pragma comment(lib, "libprotobuf.lib")

int _tmain(int argc, _TCHAR* argv[])
{
	proto::engine query([](const std::string &msg)
	{
		std::cout << msg << std::endl;
	});

	query.start("Provider=Microsoft.Jet.OLEDB.4.0;Data Source=test.mdb;Persist Security Info=False");
	// "Provider=Microsoft.Jet.OLEDB.4.0;Data Source=test.mdb;Persist Security Info=False"
	
	{
		// insert
		
		Person person;
		person.set_name("test name");
		//person.set_email("test@qq.com");
		person.set_id(1);
		assert(query.sql_insert(person));
	}

	{
		// update
		Person person;
		person.set_name("test name");
		person.set_email("test@qq.com");
		
		Person where_t;
		where_t.set_id(1);

		assert(query.sql_update(person, where_t));
	}

	{
		using namespace proto;

		// select
		Person person;
		typedef std::shared_ptr<Person> person_ptr;
		std::vector<person_ptr> persons = query.sql_select<Person>( 
			all_columns_t(), 
			(where_t("id") == 1 && where_t("name") == "test name"));

		std::for_each(persons.begin(), persons.end(), [](const person_ptr &val)
		{
			std::cout << val->name() << " "
				<< val->id() << " "
				<< val->email() << std::endl;
		});
	}

	{
		using namespace proto;

		// select
		Person person;
		typedef std::shared_ptr<Person> person_ptr;
		std::vector<person_ptr> persons = query.sql_select<Person>(
			columns_t("name", "id"), 
			(where_t("id") == 0 && where_t("name") == "test name"));
	}

	{
		// delete
	}

	{
		// helper
		using namespace proto;
		
		Person person;
		typedef std::shared_ptr<Person> person_ptr;

		std::vector<person_ptr> persons;

		select_t select;
		select << all_columns_t() << (where_t("id") == 1 && where_t("name") == "test name"), into_t(persons);

	}

	query.stop();
	system("pause");
	return 0;
}

