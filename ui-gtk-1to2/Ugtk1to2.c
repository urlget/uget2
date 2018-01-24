/*
 *
 *   Copyright (C) 2013-2018 by C.H. Huang
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

#include <glib.h>
#include <UgUtil.h>
#include <UgStdio.h>
#include <UgString.h>
#include <UgFileUtil.h>
#include <UgJsonFile.h>
#include <UgetData.h>
#include "Ugtk1to2.h"
#include "UgData-download.h"

#define UGTK_APP_DIR				"uGet"
#define UGTK_APP_SETTING_FILE		"Setting.json"
#define UGTK_APP_SETTING_FILE1		"Setting.xml"
#define UGTK_APP_CATEGORY_FILE		"CategoryList.xml"
#define UGTK_APP_DOWNLOAD_FILE		"DownloadList.xml"

static void ugtk_setting_set_by_v1 (UgtkSetting* setting, UgSetting* setting1)
{
	int     weekdays, dayhours;
	GList*  glink;
	UgLink* ulink;
	int     length;

	ugtk_setting_init (setting);
	ugtk_setting_reset (setting);

	// WindowSetting
	setting->window.toolbar = setting1->window.toolbar;
	setting->window.statusbar = setting1->window.statusbar;
	setting->window.category = setting1->window.category;
	setting->window.summary = setting1->window.summary;
	setting->window.banner = setting1->window.banner;
	setting->window.toolbar = setting1->window.toolbar;
	setting->window.x = setting1->window.x;
	setting->window.y = setting1->window.y;
	setting->window.width = setting1->window.width;
	setting->window.height = setting1->window.height;
	setting->window.maximized = setting1->window.maximized;
	// SummarySetting
	setting->summary.name = setting1->summary.name;
	setting->summary.folder = setting1->summary.folder;
	setting->summary.category = setting1->summary.category;
	setting->summary.uri = setting1->summary.url;
	setting->summary.message = setting1->summary.message;
	// DownloadColumn
	setting->download_column.complete = setting1->download_column.completed;
	setting->download_column.total = setting1->download_column.total;
	setting->download_column.percent = setting1->download_column.percent;
	setting->download_column.elapsed = setting1->download_column.elapsed;
	setting->download_column.left = setting1->download_column.left;
	setting->download_column.speed = setting1->download_column.speed;
	setting->download_column.upload_speed = setting1->download_column.upload_speed;
	setting->download_column.uploaded = setting1->download_column.uploaded;
	setting->download_column.ratio = setting1->download_column.ratio;
	setting->download_column.category = setting1->download_column.category;
	setting->download_column.uri = setting1->download_column.url;
	setting->download_column.added_on = setting1->download_column.added_on;
	setting->download_column.completed_on = setting1->download_column.completed_on;
	setting->download_column.sort.type = setting1->download_column.sort.order;
	setting->download_column.sort.nth = setting1->download_column.sort.nth + 1;
	// UserInterface
	setting->ui.exit_confirmation = setting1->ui.close_confirmation;
	switch (setting1->ui.close_action) {
	default:
	case 0:  // close_action == 0, Let user decide.
		setting->ui.exit_confirmation = TRUE;
		break;
	case 1:  // close_action == 1, Minimize to tray.
		setting->ui.close_to_tray = TRUE;
		break;
	case 2:  // close_action == 2, Exit Uget.
		setting->ui.close_to_tray = FALSE;
	}
	setting->ui.delete_confirmation = setting1->ui.delete_confirmation;
	setting->ui.show_trayicon = setting1->ui.show_trayicon;
	setting->ui.start_in_tray = setting1->ui.start_in_tray;
	setting->ui.start_in_offline_mode = setting1->ui.start_in_offline_mode;
	setting->ui.start_notification = setting1->ui.start_notification;
	setting->ui.sound_notification = setting1->ui.sound_notification;
	setting->ui.apply_recent = setting1->ui.apply_recently;
#ifdef HAVE_APP_INDICATOR
	setting->ui.app_indicator = TRUE;
#endif

	// ClipboardSetting
	ug_free (setting->clipboard.pattern);
	setting->clipboard.pattern = setting1->clipboard.pattern;
	setting1->clipboard.pattern = NULL;
	setting->clipboard.monitor = setting1->clipboard.monitor;
	setting->clipboard.quiet = setting1->clipboard.quiet;
	setting->clipboard.nth_category = setting1->clipboard.nth_category;
	if (setting->clipboard.nth_category == -1)
		setting->clipboard.nth_category = 0;
	// SchedulerSetting
	setting->scheduler.enable = setting1->scheduler.enable;
	for (weekdays = 0;  weekdays < 7;  weekdays++) {
		for (dayhours = 0;  dayhours < 24;  dayhours++) {
			setting->scheduler.state.at[weekdays*24 + dayhours] =
					setting1->scheduler.state[weekdays][dayhours];
		}
	}
	// CommandlineSetting
	setting->commandline.quiet = setting1->commandline.quiet;
	setting->commandline.nth_category = setting1->commandline.category_index;
	if (setting->commandline.nth_category == -1)
		setting->commandline.nth_category = 0;
	// PluginSetting
	setting->plugin_order = setting1->plugin.aria2.enable;
	setting->aria2.launch = setting1->plugin.aria2.launch;
	setting->aria2.shutdown = setting1->plugin.aria2.shutdown;
	ug_free (setting->aria2.path);
	setting->aria2.path = setting1->plugin.aria2.path;
	setting1->plugin.aria2.path = NULL;
	ug_free (setting->aria2.args);
	setting->aria2.args = setting1->plugin.aria2.args;
	setting1->plugin.aria2.args = NULL;
	ug_free (setting->aria2.uri);
	// aria2 URI
	length = strlen (setting1->plugin.aria2.uri);
	if (setting1->plugin.aria2.uri[length-3] == 'r' &&
	    setting1->plugin.aria2.uri[length-2] == 'p' &&
	    setting1->plugin.aria2.uri[length-1] == 'c')
	{
		setting->aria2.uri = ug_malloc (length + 4 + 1);  // + "json" + '\0'
		setting->aria2.uri[length-3] = 0;
		strncpy (setting->aria2.uri, setting1->plugin.aria2.uri, length-3);
		strcat (setting->aria2.uri, "jsonrpc");
	}
	else {
		setting->aria2.uri = setting1->plugin.aria2.uri;
		setting1->plugin.aria2.uri = NULL;
	}
	// AutoSaveSetting
	setting->auto_save.enable = setting1->auto_save.active;
	setting->auto_save.interval = setting1->auto_save.interval;
	// FolderHistory
	for (glink = setting1->folder_list;  glink;  glink = glink->next) {
		ulink = ug_link_new ();
		ulink->data = glink->data;
		glink->data = NULL;
		ug_list_append (&setting->folder_history, ulink);
	}

	// completion
	setting->completion.remember = TRUE;
}

static void uget_node_set_by_dataset (UgetNode* node, UgDataset* dataset)
{
	union {
		UgProgress* progress;
		UgRelation* relation;
		UgCommon*   common;
		UgProxy*    proxy;
		UgHttp*     http;
		UgFtp*      ftp;
	} old;

	union {
		UgetProgress* progress;
		UgetCommon*   common;
		UgetProxy*    proxy;
		UgetHttp*     http;
		UgetFtp*      ftp;
	} new;

	old.relation = ug_dataset_get (dataset, UgRelationInfo, 0);
	if (old.relation) {
		if (old.relation->hints & UG_HINT_PAUSED)
			node->group |= UGET_GROUP_PAUSED;
		if (old.relation->hints & UG_HINT_ERROR)
			node->group |= UGET_GROUP_ERROR;
		if (old.relation->hints & UG_HINT_COMPLETED)
			node->group |= UGET_GROUP_COMPLETED;

		if (old.relation->hints & UG_HINT_FINISHED)
			node->group |= UGET_GROUP_FINISHED;
		else if (old.relation->hints & UG_HINT_RECYCLED)
			node->group |= UGET_GROUP_RECYCLED;
		else
			node->group |= UGET_GROUP_QUEUING;
	}

	old.common = ug_dataset_get (dataset, UgCommonInfo, 0);
	if (old.common) {
		node->name = old.common->name;
		old.common->name = NULL;
		if (node->name == NULL && old.common->file)
			node->name = ug_strdup (old.common->file);
		new.common = ug_info_realloc (&node->info, UgetCommonInfo);
		new.common->uri = old.common->url;
		old.common->url = NULL;
		new.common->mirrors = old.common->mirrors;
		old.common->mirrors = NULL;
		new.common->folder = old.common->folder;
		old.common->folder = NULL;
		new.common->file = old.common->file;
		old.common->file = NULL;
		new.common->user = old.common->user;
		old.common->user = NULL;
		new.common->password = old.common->password;
		old.common->password = NULL;
		new.common->connect_timeout = old.common->connect_timeout;
		new.common->transmit_timeout = old.common->transmit_timeout;
		new.common->max_connections = old.common->max_connections;
		new.common->max_upload_speed = old.common->max_upload_speed;
		new.common->max_download_speed = old.common->max_download_speed;
		new.common->timestamp = old.common->retrieve_timestamp;
		new.common->retry_delay = old.common->retry_delay;
		new.common->retry_limit = old.common->retry_limit;
		new.common->retry_count = old.common->retry_count;
	}

	old.proxy = ug_dataset_get (dataset, UgProxyInfo, 0);
	if (old.proxy) {
		new.proxy = ug_info_realloc (&node->info, UgetProxyInfo);
		new.proxy->host = old.proxy->host;
		old.proxy->host = NULL;
		new.proxy->port = old.proxy->port;
		new.proxy->type = old.proxy->type;
		new.proxy->user = old.proxy->user;
		old.proxy->user = NULL;
		new.proxy->password = old.proxy->password;
		old.proxy->password = NULL;
	}

	old.http = ug_dataset_get (dataset, UgHttpInfo, 0);
	if (old.http) {
		new.http = ug_info_realloc (&node->info, UgetHttpInfo);
		new.http->user = old.http->user;
		old.http->user = NULL;
		new.http->password = old.http->password;
		old.http->password = NULL;
		new.http->referrer = old.http->referrer;
		old.http->referrer = NULL;
		new.http->user_agent = old.http->user_agent;
		old.http->user_agent = NULL;
		new.http->post_data = old.http->post_data;
		old.http->post_data = NULL;
		new.http->post_file = old.http->post_file;
		old.http->post_file = NULL;
		new.http->cookie_data = old.http->cookie_data;
		old.http->cookie_data = NULL;
		new.http->cookie_file = old.http->cookie_file;
		old.http->cookie_file = NULL;
		new.http->redirection_limit = old.http->redirection_limit;
		new.http->redirection_count = old.http->redirection_count;
	}

	old.ftp = ug_dataset_get (dataset, UgFtpInfo, 0);
	if (old.ftp) {
		new.ftp = ug_info_realloc (&node->info, UgetFtpInfo);
		new.ftp->user = old.ftp->user;
		old.ftp->user = NULL;
		new.ftp->password = old.ftp->password;
		old.ftp->password = NULL;
		new.ftp->active_mode = old.ftp->active_mode;
	}

	old.progress = ug_dataset_get (dataset, UgProgressInfo, 0);
	if (old.progress) {
		new.progress = ug_info_realloc (&node->info, UgetProgressInfo);
		new.progress->complete = old.progress->complete;
		new.progress->total = old.progress->total;
		new.progress->complete = old.progress->complete;
		new.progress->percent = old.progress->percent;
		new.progress->uploaded = old.progress->uploaded;
		new.progress->ratio = old.progress->ratio;
		new.progress->elapsed = old.progress->consume_time;
		new.progress->left = old.progress->remain_time;
	}
}

static UgetNode* uget_node_from_category (UgCategory* category1)
{
	GList*        link;
	UgetNode*     node;
	UgetNode*     dnode;
	UgetCategory* category;

	node = uget_node_new (NULL);
	uget_node_set_by_dataset (node, category1->defaults);
	category = ug_info_realloc (&node->info, UgetCategoryInfo);
	category->active_limit   = category1->active_limit;
	category->finished_limit = category1->finished_limit;
	category->recycled_limit = category1->recycled_limit;
	node->name = category1->name;
	category1->name = NULL;
	// other
	*(char**)ug_array_alloc (&category->schemes, 1) = ug_strdup ("http");
	*(char**)ug_array_alloc (&category->schemes, 1) = ug_strdup ("ftp");
	*(char**)ug_array_alloc (&category->hosts, 1) = ug_strdup (".com");
	*(char**)ug_array_alloc (&category->hosts, 1) = ug_strdup (".org");
	*(char**)ug_array_alloc (&category->file_exts, 1) = ug_strdup ("torrent");
	*(char**)ug_array_alloc (&category->file_exts, 1) = ug_strdup ("metalink");
	//
	for (link = category1->indices;  link;  link = link->next) {
		dnode = uget_node_new (NULL);
		uget_node_set_by_dataset (dnode, (UgDataset*) link->data);
		uget_node_append (node, dnode);
	}
	return node;
}

Ugtk1to2*  ugtk_1to2_new (const char* config_path)
{
	Ugtk1to2*  u1t2;

	u1t2 = g_malloc0 (sizeof (Ugtk1to2));
	ug_setting_init (&u1t2->setting1);
	uget_node_init (&u1t2->real, NULL);
	u1t2->config_path = g_strdup (config_path);
	return u1t2;
}

void  ugtk_1to2_free (Ugtk1to2* u1t2)
{
	g_free (u1t2);
}

int   ugtk_1to2_load_setting (Ugtk1to2* u1t2)
{
	gchar*	path;
	int     result;

	// load setting
	path = g_build_filename (u1t2->config_path,
	                         UGTK_APP_SETTING_FILE1, NULL);
	result = ug_setting_load (&u1t2->setting1, path);
	g_free (path);
	if (result == TRUE)
		ugtk_setting_set_by_v1 (&u1t2->setting, &u1t2->setting1);
	return result;
}

int   ugtk_1to2_save_setting (Ugtk1to2* u1t2)
{
	gchar*  path;
	int     result;

	// save setting
	path = g_build_filename (u1t2->config_path,
	                         UGTK_APP_SETTING_FILE, NULL);
	result = ugtk_setting_save (&u1t2->setting, path);
	g_free (path);
	return result;
}

int  ugtk_1to2_load_category (Ugtk1to2* u1t2)
{
	UgetNode* cnode;
	GList*	category_list;
	GList*	download_list;
	GList*	link;
	gchar*	file;

	// load all download from file
	file = g_build_filename (u1t2->config_path,
	                         UGTK_APP_DOWNLOAD_FILE, NULL);
	download_list = ug_download_list_load (file);
	g_free (file);
	// load all categories
	file = g_build_filename (u1t2->config_path,
	                         UGTK_APP_CATEGORY_FILE, NULL);
	category_list = ug_category_list_load (file);
	g_free (file);
	// link and add tasks to categories
	ug_category_list_link (category_list, download_list);
	// convert old category from ver1 to ver2
	for (link = category_list;  link;  link = link->next) {
		cnode = uget_node_from_category ((UgCategory*) link->data);
		uget_node_append (&u1t2->real, cnode);
	}
	// free list
	g_list_foreach (category_list, (GFunc) ug_category_free, NULL);
	g_list_free (category_list);
	g_list_foreach (download_list, (GFunc) ug_dataset_unref, NULL);
	g_list_free (download_list);

	return u1t2->real.n_children;
}

int  ugtk_1to2_save_category (Ugtk1to2* u1t2)
{
	int             count;
	char*           path;
	char*           path_base;
	char*           path_new;
	UgetNode*       cnode;
	UgJsonFile*     jfile;

	path_base = g_build_filename (u1t2->config_path,
	                              "category", NULL);
	ug_create_dir_all (path_base, -1);

	jfile = ug_json_file_new (4096);
	cnode = u1t2->real.children;
	for (count = 0;  cnode;  cnode = cnode->next, count++) {
#if defined _WIN32 || defined _WIN64
		path = ug_strdup_printf ("%s%c%.4d.temp", path_base, '\\', count);
#else
		path = ug_strdup_printf ("%s%c%.4d.temp", path_base, '/',  count);
#endif // _WIN32 || _WIN64

		if (ug_json_file_begin_write (jfile, path, UG_JSON_FORMAT_ALL) == FALSE) {
			ug_free (path);
			break;
		}

		ug_json_write_object_head (&jfile->json);
		ug_json_write_entry (&jfile->json, cnode, UgetNodeEntry);
		ug_json_write_object_tail (&jfile->json);

		ug_json_file_end_write (jfile);

#if defined _WIN32 || defined _WIN64
		path_new = ug_strdup_printf ("%s%c%.4d.json", path_base, '\\', count);
#else
		path_new = ug_strdup_printf ("%s%c%.4d.json", path_base, '/',  count);
#endif // _WIN32 || _WIN64
		ug_unlink (path_new);
		ug_rename (path, path_new);

		ug_free (path_new);
		ug_free (path);
	}

	ug_free (path_base);
	ug_json_file_free (jfile);
	return count;
}

