// linq.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iostream>
#include "../db\db_engine/helper.hpp"

#include "../db/x.pb.h"

#pragma comment(lib, "../db/libprotobuf.lib")

int _tmain(int argc, _TCHAR* argv[])
{
	using namespace proto;

	typedef std::shared_ptr<Person> person_ptr;

	std::vector<person_ptr> persons;

	select_t select;
	select << all_columns_t() 
		<< from_t<Person>()
		<< (where_t("id") == 1 && where_t("name") == "test name"), 
		into_t(persons);

	std::cout << select.to_string() << std::endl;

	return 0;
}

