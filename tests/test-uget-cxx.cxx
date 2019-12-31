/*
 *
 *   Copyright (C) 2012-2020 by C.H. Huang
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
#include <UgetFiles.h>
#include <UgetPlugin.h>
#include <UgetPluginCurl.h>
#include <UgetApp.h>

using namespace std;

#define CHECK_CXX_STANDARD_LAYOUT        1

// ----------------------------------------------------------------------------
// test C++ standard-layout

void uget_is_standard_layout(void)
{
#if CHECK_CXX_STANDARD_LAYOUT
	cout << "is_standard_layout<Uget::Node> = " << is_standard_layout<Uget::Node>::value << endl
	     << "is_standard_layout<Uget::Common> = " << std::is_standard_layout<Uget::Common>::value << endl
	     << "is_standard_layout<Uget::Progress> = " << std::is_standard_layout<Uget::Progress>::value << endl
	     << "is_standard_layout<Uget::Files> = " << std::is_standard_layout<Uget::Files>::value << endl
	     << "is_standard_layout<Uget::Plugin> = " << std::is_standard_layout<Uget::Plugin>::value << endl
	     << "is_standard_layout<Uget::PluginInfo> = " << std::is_standard_layout<Uget::PluginInfo>::value << endl
	     << endl;
#endif  // CHECK_CXX_STANDARD_LAYOUT
}

// ----------------------------------------------------------------------------
// test UgetCommon, UgetFiles...etc

void test_uget_data_cxx(void)
{
	Uget::Common *common, *common2;
	Uget::Files  *files,  *files2;

	common = new Uget::Common;
	common2 = common->copy();
	delete common;
	delete common2;

	files = new Uget::Files;
	files2 = new Uget::Files;
	files2->assign(files);
	delete files;
	delete files2;
}

// ----------------------------------------------------------------------------
// test Uget::Node

void test_uget_node_cxx(void)
{
	Uget::Node*   node;
	Uget::Common* common;

	cout << " --- test_uget_node_cxx()" << endl;
	node = new Uget::Node;
	common = (Uget::Common*) node->info->realloc(Uget::CommonInfo);
	common->retry_limit = 10;
	node->info->remove(Uget::CommonInfo);
	delete node;
}

// ----------------------------------------------------------------------------
// test Uget::App & Uget::AppMethod

/*
   Uget::App
   |
   `--- UserApp
 */

struct UserApp : Uget::AppMethod
{
	// ------ Uget::App members ------
	UGET_APP_MEMBERS;

	// ------ UserApp members ------
	int   value;
	char* text;

	inline UserApp(void) {
		init();
		value = 10;
		text  = strdup("This is UserApp's text");
	}

	inline ~UserApp(void) {
		free(text);
		value = 0;
		final();
	}
};

void test_uget_app_cxx(void)
{
	UserApp*  app;

	cout << " --- test_uget_app_cxx()" << endl;
#if CHECK_CXX_STANDARD_LAYOUT
	cout << "is_standard_layout<Uget::App> = " << is_standard_layout<Uget::App>::value << endl;
	cout << "is_standard_layout<UserApp> = " << is_standard_layout<UserApp>::value << endl;
#endif // CHECK_CXX_STANDARD_LAYOUT

	app = new UserApp;
	cout << "app->text = " << app->text << endl;
	app->setDefaultPlugin(Uget::PluginCurlInfo);
	app->addPlugin(Uget::PluginCurlInfo);
	app->task.setSpeed(0, 0);
	delete app;
}

// ----------------------------------------------------------------------------
// main

int   main (void)
{
	uget_is_standard_layout();

	test_uget_data_cxx();
	test_uget_node_cxx();
	test_uget_app_cxx();

	return 0;
}

