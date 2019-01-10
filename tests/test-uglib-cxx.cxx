/*
 *
 *   Copyright (C) 2012-2019 by C.H. Huang
 *   plushuang.tw@gmail.com
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 *  ---
 *
 *  In addition, as a special exception, the copyright holders give
 *  permission to link the code of portions of this program with the
 *  OpenSSL library under certain conditions as described in each
 *  individual source file, and distribute linked combinations
 *  including the two.
 *  You must obey the GNU Lesser General Public License in all respects
 *  for all of the code used other than OpenSSL.  If you modify
 *  file(s) with this exception, you may extend this exception to your
 *  version of the file(s), but you are not obligated to do so.  If you
 *  do not wish to do so, delete this exception statement from your
 *  version.  If you delete this exception statement from all source
 *  files in the program, then also delete it here.
 *
 */

#include <UgJson.h>
#include <UgArray.h>
#include <UgBuffer.h>
#include <UgString.h>
#include <UgValue.h>
#include <UgList.h>
#include <UgNode.h>
#include <UgData.h>
#include <UgInfo.h>
#include <UgUri.h>
#include <iostream>
#include <type_traits>

using namespace std;

#define CHECK_CXX_STANDARD_LAYOUT        1

// ----------------------------------------------------------------------------
// test C++ standard-layout

void uglib_is_standard_layout(void)
{
#if CHECK_CXX_STANDARD_LAYOUT
	cout << "Ug::DataInfo : is_standard_layout = " << is_standard_layout<Ug::DataInfo>::value << endl
	     << "Ug::Data : is_standard_layout = " << std::is_standard_layout<Ug::Data>::value << endl
	     << "Ug::Array<int> : is_standard_layout = " << std::is_standard_layout<Ug::Array<int>>::value << endl
	     << "Ug::Buffer : is_standard_layout = " << std::is_standard_layout<Ug::Buffer>::value << endl
	     << "Ug::Node : is_standard_layout = " << std::is_standard_layout<Ug::Node>::value << endl
	     << "Ug::List : is_standard_layout = " << std::is_standard_layout<Ug::List>::value << endl
	     << "Ug::Info : is_standard_layout = " << std::is_standard_layout<Ug::Info>::value << endl
	     << "Ug::Json : is_standard_layout = " << std::is_standard_layout<Ug::Json>::value << endl
	     << "Ug::Uri : is_standard_layout = " << std::is_standard_layout<Ug::Uri>::value << endl
	     << endl;
#endif  // CHECK_CXX_STANDARD_LAYOUT
}

// ----------------------------------------------------------------------------
// test Ug::Json

void test_json_cxx(void)
{
	Ug::Json  js;

//	js.init();
//	js.final();
}

// ----------------------------------------------------------------------------
// test Ug::Array

void test_array_cxx(void)
{
	Ug::Array<int>  array;
//	UgArrayInt      array;
	int   index;
	int*  intptr;
	int   value = 102;

	cout << endl << "--- Ug::Array testing..." << endl;

	array.init(128);
	intptr = array.alloc(4);
	intptr[0] = 100;
	intptr[1] = 101;
	intptr[2] = 102;
	intptr[3] = 103;
	intptr = array.insert(2, 2);
//	ug_array_insert(&array, 2, 2);
	intptr[0] = 104;
	intptr[1] = 105;

	array.sort();
//	ug_array_sort(&array, ug_array_compare_int);
	intptr = array.findSorted(value, &index);
	cout << "array.findSorted(" << value << ");" << endl;
	if (intptr) {
		cout << "found."
		     << " value = " << intptr[0] << ","
		     << " index = " << index << endl;
	}

	cout << endl << "dump array ---" << endl;
	for(index = 0;  index < array.length;  index++)
		cout << "array.at[" << index << "] = " << array.at[index] << endl;

	array.clear();
}

// ----------------------------------------------------------------------------
// test Ug::Buffer

void test_buffer_cxx(void)
{
	Ug::Buffer  buffer;

	cout << endl << "--- Ug::Buffer testing..." << endl;

	buffer.init(80);
	buffer.write("Write string to buffer.");
	buffer.write("Second string.\n");
	buffer.fill('Z', 10);
	buffer.write('\0');

	cout << buffer.beg << endl;
	buffer.clear(true);
}

// ----------------------------------------------------------------------------
// test Ug::List

void test_list_cxx(void)
{
	Ug::List list;
	Ug::Link link1;
	Ug::Link link2;

	list.append(&link1);
	list.insert(&link1, &link2);
}

// ----------------------------------------------------------------------------
// test Ug::Node

void test_node_cxx(void)
{
	Ug::Node root;
	Ug::Node node1;
	Ug::Node node2;

	root.append(&node1);
	root.insert(&node1, &node2);
}

// ----------------------------------------------------------------------------
// test Ug::DataInterface

struct UgCxxData : public Ug::DataInterface<UgCxxData>
{
	UG_DATA_MEMBERS;
//	const UgDataInfo*  info;    // UgData(UgType) member

	int   value;
	char* string;

	void* operator new(size_t size);
	static void  init(UgCxxData* data);
	static void  final(UgCxxData* data);
	static int   assign(UgCxxData* data, UgCxxData* src);
};

Ug::DataInfo UgCxxInfo =
{
	"UgCxxData",
	sizeof(UgCxxData),
	(UgInitFunc)   UgCxxData::init,
	(UgFinalFunc)  UgCxxData::final,
	(UgAssignFunc) UgCxxData::assign,
	(UgEntry*)     NULL,
};

void* UgCxxData::operator new(size_t size)
{
	return ug_data_new(&UgCxxInfo);
}

void UgCxxData::init(UgCxxData* data)
{
	data->value = 1;
	data->string = NULL;
}

void UgCxxData::final(UgCxxData* data)
{
	free(data->string);
}

int  UgCxxData::assign(UgCxxData* data, UgCxxData* src)
{
	data->value = src->value;
	free(data->string);
	data->string = strdup(src->string);
	return TRUE;
}

void test_data_cxx (void)
{
	UgCxxData*  cxxdata;

#if CHECK_CXX_STANDARD_LAYOUT
	cout << "Ug::Data : is_standard_layout = " << is_standard_layout<Ug::Data>::value << endl;
	cout << "UgCxxData : is_standard_layout = " << is_standard_layout<UgCxxData>::value << endl;
#endif

	cxxdata = new UgCxxData;
	cxxdata->string = strdup("This is a string.");
	cout << "cxxdata->value : " << cxxdata->value << endl
	     << "cxxdata->string: " << cxxdata->string << endl;
	delete cxxdata;
}

// ----------------------------------------------------------------------------
// test Ug::Info

void test_info_cxx(void)
{
	Ug::Info info;

	info.init(16, 3);
}

// ----------------------------------------------------------------------------
// test Ug::Registry

void test_registry_cxx(void)
{
	Ug::Registry  registry;
	Ug::Pair*     pair;

	cout << endl << "--- Ug::Registry testing..." << endl;

	registry.init();
	registry.add(&UgCxxInfo);
//	registry.remove(&UgCxxInfo);
	pair = registry.find("UgCxxData");
	cout << "pair->key = " << (char*)pair->key << endl;
	registry.final();
}

// ----------------------------------------------------------------------------
// test Ug::Value

void test_value_cxx(void)
{
	Ug::Value  vObject(UG_VALUE_OBJECT);
	Ug::Value* member;
	int  index;

	cout << endl << "--- Ug::Value testing..." << endl;

	member = vObject.alloc(2);
	member[0].name = ug_strdup("testInt");
	member[0].type = UG_VALUE_INT;
	member[0].c.integer = 101;
	member[1].name = ug_strdup("aInt");
	member[1].type = UG_VALUE_INT;
	member[1].c.integer = 105;

	// sort vObject's members by name
	vObject.sort();

	for(index = 0;  index < vObject.length();  index++) {
		member = vObject.at(index);
//		member = ug_value_at(&vObject, index);
		cout << member->name << " = " << member->c.integer << endl;
	}
}

// ----------------------------------------------------------------------------
// main

int   main (void)
{
	uglib_is_standard_layout();

	test_json_cxx();
	test_array_cxx();
	test_buffer_cxx();
	test_node_cxx();
	test_data_cxx();
	test_info_cxx();
	test_registry_cxx();
	test_value_cxx();

	return 0;
}

