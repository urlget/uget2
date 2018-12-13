/*
 *
 *   Copyright (C) 2012-2018 by C.H. Huang
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

#include <iostream>
#include <UgetNode.h>
#include <UgetData.h>
#include <UgetPlugin.h>
#include <UgetTask.h>

using namespace std;

#define CHECK_CXX_STANDARD_LAYOUT        1

// ----------------------------------------------------------------------------
// test C++ standard-layout

void uget_is_standard_layout (void)
{
#if CHECK_CXX_STANDARD_LAYOUT
	cout << "Uget::Node : is_standard_layout = " << is_standard_layout<Uget::Node>::value << endl
	     << "Uget::Common : is_standard_layout = " << std::is_standard_layout<Uget::Common>::value << endl
	     << "Uget::Progress : is_standard_layout = " << std::is_standard_layout<Uget::Progress>::value << endl
	     << "Uget::Plugin : is_standard_layout = " << std::is_standard_layout<Uget::Plugin>::value << endl
	     << endl;
#endif  // CHECK_CXX_STANDARD_LAYOUT
}

// ----------------------------------------------------------------------------
// test Uget::Node

void test_uget_node_cxx (void)
{
	Uget::Node    node;
	Uget::Common* common;

	cout << " --- test_uget_node_cxx()" << endl;
	uget_node_init(&node, NULL);
	common = (Uget::Common*) node.info->realloc (UgetCommonInfo);
	common->retry_limit = 10;
	node.info->remove (UgetCommonInfo);
}

// ----------------------------------------------------------------------------
// main

int   main (void)
{
	uget_is_standard_layout ();

	test_uget_node_cxx ();

	return 0;
}

