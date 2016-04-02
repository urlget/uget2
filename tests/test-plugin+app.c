/*
 *
 *   Copyright (C) 2012-2016 by C.H. Huang
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

#include <stdio.h>
#include <UgString.h>
#include <UgetApp.h>
#include <UgetPluginCurl.h>
#include <UgetPluginAria2.h>
#include <UgetPluginMedia.h>
#include <UgetPluginEmpty.h>

#if defined _WIN32 || defined _WIN64
#include <windows.h>
#define  ug_sleep                 Sleep
#else
#include <unistd.h>               // usleep()
#define  ug_sleep(millisecond)    usleep (millisecond * 1000)
#endif // _WIN32 || _WIN64

// ----------------------------------------------------------------------------
// test_node

void  download_node (UgetNode* node, const UgetPluginInfo* info)
{
	UgetCommon*   common;
	UgetPlugin*   plugin;
	UgetEvent*    events;
	UgetEvent*    cur;
	UgetEvent*    next;
	UgetProgress* progress;
	int           integer[2];
	char*         string[5];

	plugin = uget_plugin_new (info);
	integer[0] = 1000000;
	integer[1] = 1000000;
	uget_plugin_ctrl (plugin, UGET_PLUGIN_CTRL_SPEED, integer);
	uget_plugin_start (plugin, node);
	while (uget_plugin_sync (plugin)) {
		ug_sleep (1000);
		// event
		events = uget_plugin_pop (plugin);
		for (cur = events;  cur;  cur = next) {
			next = cur->next;
			printf ("\n" "Event type: %d - %s\n", cur->type, cur->string);
			uget_event_free (cur);
		}
		// progress
		progress = ug_info_get (&node->info, UgetProgressInfo);
		if (progress == NULL)
			continue;
		string[0] = ug_str_from_int_unit (progress->complete, NULL);
		string[1] = ug_str_from_int_unit (progress->total, NULL);
		string[2] = ug_str_from_int_unit (progress->uploaded, NULL);
		string[3] = ug_str_from_int_unit (progress->download_speed, "/s");
		string[4] = ug_str_from_int_unit (progress->upload_speed, "/s");
		printf ("\r" "DL: %s / %s, %d%%, %s | UL: %s, %s" "      ",
				string[0], string[1], progress->percent,
				string[3], string[2], string[4]);
		for (integer[0] = 0;  integer[0] < 5;  integer[0]++)
			ug_free (string[integer[0]]);
	}
	// these data may changed, print them.
	common = ug_info_get (&node->info, UgetCommonInfo);
	printf ("\n"
			"node->name : %s\n"
			"common->uri : %s\n"
			"common->file : %s\n",
			node->name, common->uri, common->file);
	// print children
	for (node = node->children;  node;  node = node->next)
		printf ("child node name : %s\n", node->name);

	uget_plugin_unref (plugin);
}

void  test_node_download (void)
{
	UgetCommon*  common;
	UgetHttp*    http;
	UgetNode*    node;
	char*        referrer;
	char*        uri;
	char*        mirrors;

	uri = "https://www.youtube.com/watch?v=y2004Xaz2HU";
//	uri = "http://download.tuxfamily.org/notepadplus/6.5.3/npp.6.5.3.Installer.exe";
//	uri = "http://ftp.gimp.org/pub/gimp/v2.8/windows/gimp-2.8.10-setup.exe";
//	mirrors = "ftp://195.220.108.108/linux/fedora/linux/updates/19/x86_64/kernel-3.11.2-201.fc19.x86_64.rpm";
	mirrors = NULL;

//	referrer = "http://code.google.com/p/tortoisegit/wiki/Download?tm=2";
	referrer = NULL;

	node = uget_node_new (NULL);
	// commom options
	common = ug_info_realloc (&node->info, UgetCommonInfo);
	common->uri = ug_strdup (uri);
	if (mirrors)
		common->mirrors = ug_strdup (mirrors);
	common->folder = ug_strdup ("D:\\Downloads");
	common->max_connections = 2;
	common->debug_level = 1;
	// http options
	http = ug_info_realloc (&node->info, UgetHttpInfo);
	if (referrer)
		http->referrer = ug_strdup (referrer);

//	download_node (node, UgetPluginCurlInfo);
//	download_node (node, UgetPluginAria2Info);
	download_node (node, UgetPluginMediaInfo);
	uget_node_unref (node);
}

// ----------------------------------------------------------------------------
// test_plugin

void  test_setup_plugin_aria2 (void)
{
	const UgetPluginInfo* pinfo;

	pinfo = UgetPluginAria2Info;
	uget_plugin_set (pinfo, UGET_PLUGIN_INIT, (void*) TRUE);
	uget_plugin_set (pinfo, UGET_PLUGIN_ARIA2_URI,
			"http://localhost/jsonrpc");
#if defined _WIN32 || defined _WIN64
	uget_plugin_set (pinfo, UGET_PLUGIN_ARIA2_PATH,
			"C:\\Program Files\\uGet\\bin\\aria2c.exe");
#endif
	uget_plugin_set (pinfo, UGET_PLUGIN_ARIA2_ARGUMENT,
			"--enable-rpc=true -D --check-certificate=false");
	uget_plugin_set (pinfo, UGET_PLUGIN_ARIA2_LAUNCH, (void*) TRUE);
	uget_plugin_set (pinfo, UGET_PLUGIN_ARIA2_SHUTDOWN, (void*) TRUE);
	ug_sleep (1000);
	uget_plugin_set (pinfo, UGET_PLUGIN_INIT, (void*) FALSE);
	ug_sleep (1000);
}

// ----------------------------------------------------------------------------
// test_task

void  print_node_speed_limit (UgetNode** dnode, int count)
{
	int           total[2] = {0,0};
	UgetRelation* relation;

	for (count = 0;  count < 7;  count++) {
		relation = ug_info_get (&dnode[count]->info, UgetRelationInfo);
		printf ("limit D: %d, U: %d | speed D: %d, U: %d\n",
				relation->task.limit[0],
				relation->task.limit[1],
				relation->task.speed[0],
				relation->task.speed[1]);
		total[0] += relation->task.limit[0];
		total[1] += relation->task.limit[1];
	}

	printf ("total limit D: %d, U: %d\n",
			total[0],
			total[1]);
}

void  test_task (void)
{
	UgetTask*     task;
	UgetNode*     dnode[7];
	UgetRelation* relation;
	int           count;

	task = calloc (1, sizeof (UgetTask));
	uget_task_init (task);

	// task speed control -------------------
	for (count = 0;  count < 7;  count++) {
		dnode[count] = uget_node_new (NULL);
		relation = ug_info_realloc (&dnode[count]->info, UgetRelationInfo);
		relation->task.limit[0] = 2000;
		relation->task.limit[1] = 1500;
		relation->task.speed[0] = 2000 - count * 200;
		relation->task.speed[1] = 1500 - count * 200;
		uget_task_add (task, dnode[count], UgetPluginEmptyInfo);
	}
	relation->task.limit[0] = 0;
	relation->task.limit[1] = 0;

	uget_task_set_speed (task, 12000, 8000);
	print_node_speed_limit(dnode, 7);
	puts ("---");
	uget_task_adjust_speed (task);
	print_node_speed_limit(dnode, 7);

	uget_task_remove_all (task);
	uget_task_final (task);
	free (task);

	for (count = 0;  count < 7;  count++)
		uget_node_unref (dnode[count]);
}

// ----------------------------------------------------------------------------
// test_app

void  setup_app (UgetApp* app)
{
	UgetNode*      cnode;
	UgetCategory*  category;
	UgetCommon*    common;

	cnode = uget_node_new (NULL);
	cnode->name = ug_strdup ("Home Category");
	category = ug_info_realloc (&cnode->info, UgetCategoryInfo);
	*(char**)ug_array_alloc (&category->schemes, 1) = ug_strdup ("http");
	*(char**)ug_array_alloc (&category->schemes, 1) = ug_strdup ("https");
	*(char**)ug_array_alloc (&category->schemes, 1) = ug_strdup ("ftp");
	common = ug_info_realloc (&cnode->info, UgetCommonInfo);
	common->max_connections = 2;
	uget_app_add_category (app, cnode, TRUE);

	uget_app_add_plugin (app, UgetPluginAria2Info);
	uget_app_add_plugin (app, UgetPluginCurlInfo);
	uget_app_clear_plugins (app);
	uget_app_set_default_plugin (app, UgetPluginCurlInfo);
}

void  start_app (UgetApp* app)
{
	UgetNode*      dnode;
	UgetCommon*    common;
	char*          string[2];

	dnode = uget_node_new (NULL);
	dnode->name = ug_strdup ("Download");
	common = ug_info_realloc (&dnode->info, UgetCommonInfo);
	common->uri = ug_strdup ("http://www.utorrent.com/scripts/dl.php?track=stable&build=29812&client=utorrent");
	common->folder = ug_strdup ("D:\\Downloads");
	common->debug_level = 1;
//	common->keeping.enable = TRUE;
//	common->keeping.uri = TRUE;
//	common->keeping.folder = TRUE;
//	common->keeping.debug_level = TRUE;

	uget_app_add_download (app, dnode, NULL, FALSE);
	puts ("uget_app_grow()");
	while (uget_app_grow (app, FALSE)) {
		ug_sleep (1000);
		string[0] = ug_str_from_int_unit (app->task.speed.download, "/s");
		string[1] = ug_str_from_int_unit (app->task.speed.upload, "/s");
		printf ("\r" "DL: %s | UL %s" "    ", string[0], string[1]);
		ug_free (string[0]);
		ug_free (string[1]);
	}
	puts ("uget_app_grow() return 0");

	puts ("call uget_app_grow() again");
	uget_app_grow (app, FALSE);
}

void  close_app (UgetApp* app)
{
	uget_app_clear_plugins (app);
}

void  test_app_node (UgetApp* app)
{
	UgetNode*     dnode[7];
	UgetNode*     cnode;
	UgetCommon*   common;
	int           count;

	for (count = 0;  count < 7;  count++) {
		dnode[count] = uget_node_new (NULL);
		common = ug_info_realloc (&dnode[count]->info, UgetCommonInfo);
		common->uri = ug_strdup ("ftp://127.0.0.1/");
		common->folder = ug_strdup ("D:\\Downloads");
		common->keeping.enable = TRUE;
		common->keeping.uri = FALSE;
		common->keeping.folder = FALSE;
		uget_app_add_download (app, dnode[count], NULL, TRUE);
	}

	uget_app_move_download (app, dnode[5], NULL);
	uget_app_delete_download (app, dnode[2], TRUE);
	uget_app_delete_download (app, dnode[4], FALSE);
	uget_app_delete_download (app, dnode[6], FALSE);

	cnode = app->real.children;
	if (cnode)
		printf ("%d\n", cnode->n_children);

	cnode = app->split.children;
	if (cnode)
		printf ("%d\n", cnode->n_children);

	cnode = app->mix.children;
	if (cnode)
		printf ("%d\n", cnode->n_children);

	cnode = app->mix_split.children;
	if (cnode)
		printf ("%d\n", cnode->n_children);
}

void  test_app (void)
{
	UgetApp*   app;

	app = calloc (1, sizeof (UgetApp));
	uget_app_init (app);
	setup_app (app);
	start_app (app);
//	test_app_node (app);
	uget_app_save_categories (app, NULL);
//	uget_app_load_categories (app, NULL);
	uget_app_final (app);
	free (app);
}

// ----------------------------------------------------------------------------
// main

int   main (void)
{
	// initialize plug-in
	uget_plugin_set (UgetPluginCurlInfo, UGET_PLUGIN_INIT, (void*) TRUE);
	uget_plugin_set (UgetPluginAria2Info, UGET_PLUGIN_INIT, (void*) TRUE);
	uget_plugin_set (UgetPluginMediaInfo, UGET_PLUGIN_INIT, (void*) TRUE);
//	test_setup_plugin_aria2 ();

	test_node_download ();
//	test_task ();
//	test_app ();

//	uget_plugin_set (UgetPluginAria2Info, UGET_PLUGIN_ARIA2_SHUTDOWN_NOW, (void*) TRUE);
	// finalize plug-in
	uget_plugin_set (UgetPluginCurlInfo, UGET_PLUGIN_INIT, (void*) FALSE);
	uget_plugin_set (UgetPluginAria2Info, UGET_PLUGIN_INIT, (void*) FALSE);
	uget_plugin_set (UgetPluginMediaInfo, UGET_PLUGIN_INIT, (void*) FALSE);
	ug_sleep(1000);

	return 0;
}

