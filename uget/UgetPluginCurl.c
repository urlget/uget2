/*
 *
 *   Copyright (C) 2012-2014 by C.H. Huang
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

#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <UgUtil.h>
#include <UgStdio.h>
#include <UgString.h>
#include <UgetCurl.h>
#include <UgetPluginCurl.h>

#include <errno.h>

#if defined _WIN32 || defined _WIN64
#include <windows.h>    // Sleep()
#include <winsock2.h>
#define  ug_sleep       Sleep
#else
#include <unistd.h>  // sleep(), usleep()
#define  ug_sleep(millisecond)    usleep (millisecond * 1000)
#endif // _WIN32 || _WIN64

#if defined(_MSC_VER)
#define strtoll		_strtoi64    // stdlib.h
#endif


typedef struct UriLink      UriLink;

struct UriLink {
	UG_LINK_MEMBERS (UriLink, UriLink, self);
//	UriLink* self;
//	UriLink* next;
//	UriLink* prev;

	uint8_t  scheme_type;
	uint8_t  resumable:1;
	uint8_t  tested:1;
	uint8_t  ok:1;
	char     uri[1];
};

// ----------------------------------------------------------------------------
// UgetPluginInfo (derived from UgDataInfo)

static void plugin_init  (UgetPluginCurl* plugin);
static void plugin_final (UgetPluginCurl* plugin);
static int  plugin_ctrl  (UgetPluginCurl* plugin, int code, void* data);
static int  plugin_sync  (UgetPluginCurl* plugin);
static UgetResult  global_set (int code, void* parameter);
static UgetResult  global_get (int code, void* parameter);

static const char* schemes[] = {"http", "https", "ftp", "ftps", NULL};
static const char* types[]   = {NULL};

static const UgetPluginInfo UgetPluginCurlInfoStatic =
{
	"curl",
	sizeof (UgetPluginCurl),
	(const UgEntry*) NULL,
	(UgInitFunc)   plugin_init,
	(UgFinalFunc)  plugin_final,
	(UgAssignFunc) NULL,
	(UgetPluginCtrlFunc) plugin_ctrl,
	(UgetPluginSyncFunc) plugin_sync,
	NULL,
	schemes,
	types,
	(UgetPluginSetFunc) global_set,
	(UgetPluginGetFunc) global_get
};
// extern
const UgetPluginInfo* UgetPluginCurlInfo = &UgetPluginCurlInfoStatic;

// ----------------------------------------------------------------------------
// global data and it's functions.

static struct
{
	int  initialized;
	int  ref_count;
} global = {0, 0};

static UgetResult  global_init  (void)
{
	if (global.initialized == FALSE) {
#if defined _WIN32 || defined _WIN64
		WSADATA  WSAData;
		WSAStartup (MAKEWORD (2, 2), &WSAData);
#endif // _WIN32 || _WIN64

		if (curl_global_init (CURL_GLOBAL_ALL) != CURLE_OK) {
#if defined _WIN32 || defined _WIN64
			WSACleanup ();
#endif
			return UGET_RESULT_ERROR;
		}
		global.initialized = TRUE;
	}
	global.ref_count++;

	return UGET_RESULT_OK;
}

static void  global_ref (void)
{
	global.ref_count++;
}

static void  global_unref (void)
{
	if (global.initialized == FALSE)
		return;

	global.ref_count--;
	if (global.ref_count == 0) {
		global.initialized  = FALSE;
		curl_global_cleanup ();
#if defined _WIN32 || defined _WIN64
		WSACleanup ();
#endif
	}
}

static UgetResult  global_set (int option, void* parameter)
{
	switch (option) {
	case UGET_PLUGIN_INIT:
		// do global initialize/uninitialize here
		if (parameter)
			return global_init ();
		else
			global_unref ();
		break;

	default:
		return UGET_RESULT_UNSUPPORT;
	}

	return UGET_RESULT_OK;
}

static UgetResult  global_get (int option, void* parameter)
{
	switch (option) {
	case UGET_PLUGIN_INIT:
		if (parameter)
			*(int*)parameter = global.initialized;
		break;

	default:
		return UGET_RESULT_UNSUPPORT;
	}

	return UGET_RESULT_OK;
}

// ----------------------------------------------------------------------------
// plugins functions

static void plugin_init (UgetPluginCurl* plugin)
{
	if (global.initialized == FALSE)
		global_init ();
	else
		global_ref ();

	ug_list_init (&plugin->seg.list);
	plugin->file.time = -1;
	plugin->synced = TRUE;
	plugin->stopped = TRUE;
}

static void plugin_final (UgetPluginCurl* plugin)
{
	if (plugin->node)
		uget_node_unref (plugin->node);
	if (plugin->common)
		ug_data_free (plugin->common);
	if (plugin->proxy)
		ug_data_free (plugin->proxy);
	if (plugin->http)
		ug_data_free (plugin->http);
	if (plugin->ftp)
		ug_data_free (plugin->ftp);
	// free uri.list (UriLink), all link will be freed.
	ug_list_foreach (&plugin->uri.list, (UgForeachFunc) ug_free, NULL);

//	curl_slist_free_all (plugin->ftp_command);
	ug_free (plugin->file.path);
	ug_free (plugin->aria2.path);

	global_unref ();
}

// ----------------------------------------------------------------------------
// plugin_ctrl

static int  plugin_ctrl_speed (UgetPluginCurl* plugin, int* speed);
static int  plugin_start (UgetPluginCurl* plugin, UgetNode* node);

static int  plugin_ctrl (UgetPluginCurl* plugin, int code, void* data)
{
	switch (code) {
	case UGET_PLUGIN_CTRL_START:
		if (plugin->node == NULL)
			return plugin_start (plugin, data);
		break;

	case UGET_PLUGIN_CTRL_STOP:
		plugin->stopped = TRUE;
		return TRUE;

	case UGET_PLUGIN_CTRL_SPEED:
		// speed control
		return plugin_ctrl_speed (plugin, data);

	case UGET_PLUGIN_CTRL_DATA_CHANGED:
		break;

	case UGET_PLUGIN_CTRL_LIMIT_CHANGED:
		break;
	}

	return FALSE;
}

static int  plugin_ctrl_speed (UgetPluginCurl* plugin, int* speed)
{
	UgetCommon*  common;
	int          value;

	// Don't do anything if speed limit keep no change.
	if (plugin->limit.download == speed[0] && plugin->limit.upload == speed[1])
		return TRUE;
	// decide speed limit by user specified data.
	if (plugin->node == NULL) {
		plugin->limit.download = speed[0];
		plugin->limit.upload = speed[1];
	}
	else {
		common = ug_info_realloc (&plugin->node->info, UgetCommonInfo);
		// download
		value = speed[0];
		if (common->max_download_speed) {
			if (value > common->max_download_speed || value == 0)
				value = common->max_download_speed;
		}
		plugin->limit.download = value;
		// upload
		value = speed[1];
		if (common->max_upload_speed) {
			if (value > common->max_upload_speed || value == 0)
				value = common->max_upload_speed;
		}
		plugin->limit.upload = value;
	}
	// notify plugin that speed limit has been changed
	plugin->limit_changed = TRUE;
	return TRUE;
}

// ----------------------------------------------------------------------------
// plugin_sync

static void plugin_remove_node (UgetPluginCurl* plugin, const char* fpath);
static int  plugin_insert_node (UgetPluginCurl* plugin,
                                const char* fpath, int is_attachment);

static int  plugin_sync (UgetPluginCurl* plugin)
{
	UgetNode*      node;
	UgetCommon*    common;
	UgetProgress*  progress;
	char*          name;

	if (plugin->stopped) {
		if (plugin->synced)
			return FALSE;
		plugin->synced  = TRUE;
	}
	if (plugin->node == NULL)
		return TRUE;

	node = plugin->node;
	common = ug_info_realloc (&node->info, UgetCommonInfo);
	common->retry_count = plugin->common->retry_count;

	progress = ug_info_realloc (&node->info, UgetProgressInfo);
	progress->upload_speed   = plugin->speed.upload;
	progress->download_speed = plugin->speed.download;

	if (plugin->size.upload || plugin->size.download) {
		progress->uploaded   = plugin->size.upload;
		progress->complete   = plugin->size.download;

		if (plugin->file.size > 0)
			progress->total = plugin->file.size;
		else
			progress->total = progress->complete;

		if (progress->total > 0)
			progress->percent = progress->complete * 100 / progress->total;
		else
			progress->percent = 0;
	}

	// If total size and average speed is unknown, don't calculate remain time.
	if (progress->download_speed > 0 && progress->total > 0) {
		progress->remain_time = (progress->total - progress->complete) /
				progress->download_speed;
	}
	progress->consume_time = time(NULL) - plugin->start_time;

	// add UgetNode for file & attachment
	if (plugin->file_renamed && plugin->file.path) {
		plugin->file_renamed = FALSE;
		plugin_insert_node (plugin, plugin->file.path, FALSE);
		// change node name
#if defined _WIN32 || defined _WIN64
		name = strrchr (plugin->file.path, '\\');
#else
		name = strrchr (plugin->file.path, '/');
#endif
		if (name && name[1]) {
			if (node->name == NULL || strcmp (name, node->name)) {
				ug_free (node->name);
				node->name = ug_strdup (name + 1);
				uget_plugin_post ((UgetPlugin*) plugin,
						uget_event_new (UGET_EVENT_NAME));
			}
			if (common->file == NULL)
				common->file = ug_strdup (name + 1);
		}
	}
	if (plugin->aria2.path) {
		if (plugin->file.size == plugin->size.download)
			plugin_remove_node (plugin, plugin->aria2.path);
		else if (plugin->aria2_created == FALSE) {
			plugin->aria2_created = TRUE;
			plugin_insert_node (plugin, plugin->aria2.path, TRUE);
		}
	}
	return TRUE;
}

static int  plugin_insert_node (UgetPluginCurl* plugin,
                                const char* fpath, int is_attachment)
{
	UgetNode*  node;

	for (node = plugin->node->children;  node;  node = node->next) {
		if (strcmp (node->name, fpath) == 0)
			return FALSE;
	}

	node = uget_node_new (NULL);
	node->name = ug_strdup (fpath);
	uget_node_prepend (plugin->node, node);
	if (is_attachment)
		node->type = UGET_NODE_ATTACHMENT;
	return TRUE;
}

static void plugin_remove_node (UgetPluginCurl* plugin, const char* fpath)
{
	UgetNode*  node;

	for (node = plugin->node->children;  node;  node = node->next) {
		if (strcmp (node->name, fpath) == 0) {
			uget_node_remove (plugin->node, node);
			uget_node_unref (node);
			return;
		}
	}
}

// ----------------------------------------------------------------------------
// plugin_start

static void plugin_setup_uris (UgetPluginCurl* plugin);
static UG_THREAD_RETURN_TYPE plugin_thread (UgetPluginCurl* plugin);

static int  plugin_start (UgetPluginCurl* plugin, UgetNode* node)
{
	UgThread  thread;
	int       ok;
	int       speed[2];

	plugin->common = ug_info_get (&node->info, UgetCommonInfo);
	if (plugin->common == NULL || plugin->common->uri == NULL)
		return  FALSE;
	plugin->common = ug_data_copy (plugin->common);
	plugin_setup_uris (plugin);

	plugin->proxy = ug_info_get (&node->info, UgetProxyInfo);
	if (plugin->proxy)
		plugin->proxy  = ug_data_copy (plugin->proxy);

	plugin->http = ug_info_get (&node->info, UgetHttpInfo);
	if (plugin->http)
		plugin->http = ug_data_copy (plugin->http);

	plugin->ftp = ug_info_get (&node->info, UgetFtpInfo);
	if (plugin->ftp)
		plugin->ftp = ug_data_copy (plugin->ftp);
	// assign node before speed control
	uget_node_ref (node);
	plugin->node = node;
	// speed control
	speed[0] = plugin->limit.download;
	speed[1] = plugin->limit.upload;
	plugin_ctrl_speed (plugin, speed);
	plugin->limit_changed = FALSE;

	plugin->stopped = FALSE;
	uget_plugin_ref ((UgetPlugin*) plugin);
	ok = ug_thread_create (&thread, (UgThreadFunc) plugin_thread, plugin);
	if (ok == UG_THREAD_OK)
		ug_thread_unjoin (&thread);
	else {
		uget_plugin_post ((UgetPlugin*) plugin,
				uget_event_new_error (UGET_EVENT_ERROR_THREAD_CREATE_FAILED,
				                      NULL));
		uget_plugin_unref ((UgetPlugin*) plugin);
		return FALSE;
	}
	return TRUE;
}

static UriLink* plugin_replace_uri (UgetPluginCurl* plugin, UriLink* old_link,
                                    const char* uri, int uri_len)
{
	UriLink*  uri_link;

	if (uri_len == -1)
		uri_len = strlen (uri);
	uri_link = ug_malloc (sizeof (UriLink) + uri_len);
	strncpy (uri_link->uri, uri, uri_len);
	uri_link->uri[uri_len] = 0;   // null terminated
	uri_link->self = uri_link;
	uri_link->next = NULL;
	uri_link->prev = NULL;
	uri_link->scheme_type = 0;
	uri_link->resumable = FALSE;
	uri_link->tested = FALSE;
	uri_link->ok = FALSE;

	// add to list
	if (old_link == NULL)
		ug_list_append (&plugin->uri.list, (void*) uri_link);
	else {
		ug_list_insert (&plugin->uri.list, (void*) old_link,
				(void*) uri_link);
		ug_list_remove (&plugin->uri.list, (void*) old_link);
		if (plugin->uri.link == (void*) old_link)
			plugin->uri.link  = (void*) uri_link;
		ug_free (old_link);
	}

	return uri_link;
}

static void plugin_setup_uris (UgetPluginCurl* plugin)
{
	UgetCommon*  common;
	const char*  curr;
	const char*  prev;

	common = plugin->common;
	// uri
	plugin->uri.link = (void*) plugin_replace_uri (plugin, NULL,
	                                               common->uri, -1);
	ug_free (common->uri);
	common->uri = NULL;
	// mirrors
	for (curr = common->mirrors;  curr && curr[0];) {
		// skip space ' '
		while (curr[0] == ' ')
			curr++;
		prev = curr;
		curr = curr + strcspn (curr, " ");
		// add to uri.list
		plugin_replace_uri (plugin, NULL, prev, curr - prev);
	}
	ug_free (common->mirrors);
	common->mirrors = NULL;
}

// ----------------------------------------------------------------------------
// plugin_thread

#define N_THREAD(plugin)   ((plugin)->seg.list.size - (plugin)->seg.n_recycled)

static void delay_ms (UgetPluginCurl* plugin, int  milliseconds);
static int  switch_uri (UgetPluginCurl* plugin, UgetCurl* ugcurl, int is_resumable);
static int  prepare_file (UgetCurl* ugcurl, UgetPluginCurl* plugin);
static void complete_file (UgetPluginCurl* plugin);
static int  load_file_info (UgetPluginCurl* plugin);
static int  split_download (UgetPluginCurl* plugin, UgetCurl* ugcurl);
static void adjust_speed_limit (UgetPluginCurl* plugin);
static UgetCurl* create_segment (UgetPluginCurl* plugin);

static UG_THREAD_RETURN_TYPE plugin_thread (UgetPluginCurl* plugin)
{
	UgetCommon* common;
	UgetCurl*   ugcurl;
	UgetCurl*   ugnext;
	int         counter;
	struct {
		int64_t upload;
		int64_t download;
	} size, speed;

	common = plugin->common;
	common->retry_count = 0;
	plugin->seg.n_max = common->max_connections;
	if (plugin->seg.n_max == 0)
		plugin->seg.n_max = 1;

	ugcurl = create_segment (plugin);
	if (load_file_info (plugin)) {
		uget_curl_open_file (ugcurl, plugin->file.path);
		ugcurl->beg = plugin->seg.beg;
		uget_a2cf_lack (&plugin->aria2.ctrl,
		                (uint64_t*) &ugcurl->beg,
		                (uint64_t*) &ugcurl->end);
		plugin->seg.beg = ugcurl->end;
	}
	else {
		ugcurl->prepare.func = (UgetCurlFunc) prepare_file;
		ugcurl->prepare.data = plugin;
		ugcurl->header_store = TRUE;
	}
	// add to seg.list
	ug_list_append (&plugin->seg.list, (void*) ugcurl);
	//
	plugin->start_time = time (NULL);
	uget_curl_run (ugcurl, FALSE);

	// main loop
	for (counter = 0;  N_THREAD(plugin) > 0;  counter++) {
		// sleep 0.5 second
		ug_sleep (500);
		// reset data, program will count them later
		plugin->seg.n_active = 0;
		size.upload = 0;
		size.download = 0;
		speed.upload = 0;
		speed.download = 0;

		ugcurl = (UgetCurl*)plugin->seg.list.head;
		for (;  ugcurl;  ugcurl = ugnext) {
			ugnext = ugcurl->next;
			// split download, use with split_download
			if (ugcurl->split) {
				if (ugcurl->prev == NULL || ugcurl->prev->end < ugcurl->beg)
					ugcurl->split = FALSE;
				// if prev one overwrite.
				else if (ugcurl->pos < ugcurl->prev->pos) {
					ugcurl->split = FALSE;
					ugcurl->stopped = TRUE;
					ugcurl->end = ugcurl->beg;
					ugcurl->pos = ugcurl->beg;
#ifndef NDEBUG
					if (common->debug_level) {
						printf ("\n" "overwrite %u KiB\n",
								(unsigned) ugcurl->beg / 1024);
					}
#endif
				}
				else if (ugcurl->state == UGET_CURL_RUN) {
					ugcurl->split = FALSE;
					ugcurl->prev->end = ugcurl->beg;
					if (ugcurl->prev->pos > ugcurl->beg) {
						ugcurl->prev->pos = ugcurl->beg;
						ugcurl->prev->stopped = TRUE;
					}
#ifndef NDEBUG
					if (common->debug_level) {
						printf ("\n" "split %u KiB\n",
								(unsigned) ugcurl->beg / 1024);
					}
#endif
				}
			}
			// if user want to stop plugin, it must stop all UgetCurl in list.
			if (plugin->stopped) {
				ugcurl->stopped = TRUE;
				plugin->seg.n_max = 0;
			}
			// update aria2 ctrl file progress
			if (plugin->aria2.path)
				uget_a2cf_fill (&plugin->aria2.ctrl, ugcurl->beg, ugcurl->pos);
			// progress
			if (ugcurl->state > UGET_CURL_RUN) {
				plugin->base.upload += ugcurl->size[1];
				plugin->base.download += ugcurl->size[0];
			}
			else if (ugcurl->state > UGET_CURL_READY) {
				size.upload += ugcurl->size[1];
				size.download += ugcurl->size[0];
				speed.upload += ugcurl->speed[1];
				speed.download += ugcurl->speed[0];
			}
			// handle UgetCurl by state
			switch (ugcurl->state) {
			case UGET_CURL_RECYCLED:
			case UGET_CURL_READY:
				break;

			case UGET_CURL_RUN:
				plugin->seg.n_active++;
				break;

			case UGET_CURL_OK:
				ugcurl->state = UGET_CURL_READY;
				if (split_download (plugin, ugcurl) == FALSE) {
					// delete download
					ug_list_remove (&plugin->seg.list, (void*)ugcurl);
					uget_curl_free (ugcurl);
				}
				if (plugin->file.size == 0 && N_THREAD (plugin) == 0)
					complete_file (plugin);
				break;

			case UGET_CURL_ERROR:
				if (switch_uri (plugin, ugcurl, FALSE)) {
					uget_curl_run (ugcurl, FALSE);
					break;
				}
				// recycle current UgetCurl
				ugcurl->state = UGET_CURL_RECYCLED;
				plugin->seg.n_recycled++;
				if (N_THREAD (plugin) == 0) {
					plugin->node->state |= UGET_STATE_ERROR;
					if (ugcurl->event) {
						uget_plugin_post ((UgetPlugin*) plugin, ugcurl->event);
						ugcurl->event = NULL;
					}
				}
				break;

			case UGET_CURL_ABORT:
				ugcurl->state = UGET_CURL_READY;
				// delete download
				ug_list_remove (&plugin->seg.list, (void*)ugcurl);
				uget_curl_free (ugcurl);
				// if no download in list, save aria2 ctrl file
				if (N_THREAD (plugin) == 0 && plugin->aria2.path)
					uget_a2cf_save (&plugin->aria2.ctrl, plugin->aria2.path);
				break;

			case UGET_CURL_RETRY:
				ugcurl->state = UGET_CURL_READY;
				if (N_THREAD (plugin) == 1)
					common->retry_count++;
				if (common->retry_count < common->retry_limit || common->retry_limit == 0) {
					ugcurl->beg = ugcurl->pos;
					delay_ms (plugin, common->retry_delay * 1000);
					uget_curl_run (ugcurl, FALSE);
					break;
				}
				// delete download
				ug_list_remove (&plugin->seg.list, (void*)ugcurl);
				uget_curl_free (ugcurl);
				break;

			case UGET_CURL_NOT_RESUMABLE:    // redownload - retry
				ugcurl->state = UGET_CURL_READY;
				if (switch_uri (plugin, ugcurl, TRUE)) {
					uget_curl_run (ugcurl, FALSE);
					break;
				}
				if (N_THREAD (plugin) == 1) {
					uget_plugin_post ((UgetPlugin*) plugin,
							uget_event_new_normal (
									UGET_EVENT_NORMAL_NOT_RESUMABLE, NULL));
					common->retry_count++;
					if (common->retry_count < common->retry_limit) {
						plugin->base.download = 0;
						ugcurl->beg = 0;
						ugcurl->end = plugin->file.size;
						delay_ms (plugin, common->retry_delay * 1000);
						uget_curl_run (ugcurl, FALSE);
						break;
					}
				}
				// delete download
				ug_list_remove (&plugin->seg.list, (void*)ugcurl);
				uget_curl_free (ugcurl);
				break;
			}
		}

		// progress
		plugin->size.upload = plugin->base.upload + size.upload;
		plugin->size.download = plugin->base.download + size.download;
		if (plugin->seg.list.size) {
			// Don't update speed when stopping
			plugin->speed.upload = speed.upload;
			plugin->speed.download = speed.download;
		}
		plugin->synced = FALSE;
		// completed
		if (plugin->file.size && plugin->file.size == plugin->size.download) {
			if (N_THREAD (plugin) > 0)
				continue;    // wait other thread
			else {
				complete_file (plugin);
				plugin->synced = FALSE;
				plugin->stopped = TRUE;
				break;
			}
		}
		// adjust speed every 0.5 x 2 = 1 second.
		if (counter & 1)
			adjust_speed_limit (plugin);
		// save aria2 ctrl file every 0.5 x 4 = 2 seconds.
		if ((counter & 3) == 3 && plugin->aria2.path)
			uget_a2cf_save (&plugin->aria2.ctrl, plugin->aria2.path);
		// split download every 0.5 x 8 = 4 seconds.
		if ((counter & 7) == 7 && plugin->file.size) {
			// If some threads are connecting, It doesn't split new segment.
			if (N_THREAD (plugin) <  plugin->seg.n_max &&
			    N_THREAD (plugin) == plugin->seg.n_active)
			{
				split_download (plugin, NULL);
			}
		}
		// retry
		if (common->retry_count >= common->retry_limit) {
			uget_plugin_post ((UgetPlugin*) plugin,
					uget_event_new_error (
							UGET_EVENT_ERROR_TOO_MANY_RETRIES, NULL));
			plugin->synced = FALSE;
			plugin->stopped = TRUE;
		}
	}

	plugin->stopped = TRUE;
	ug_list_foreach (&plugin->seg.list, (UgForeachFunc) uget_curl_free, NULL);
	ug_list_init (&plugin->seg.list);
	uget_a2cf_clear (&plugin->aria2.ctrl);
	uget_plugin_unref ((UgetPlugin*) plugin);
	return UG_THREAD_RETURN_VALUE;
}

static int prepare_existed (UgetCurl* ugcurl, UgetPluginCurl* plugin)
{
	double  fsize;
	long    ftime;

	// file.size
	if (plugin->file.size) {
		curl_easy_getinfo (ugcurl->curl,
				CURLINFO_CONTENT_LENGTH_DOWNLOAD, &fsize);
		if (plugin->file.size != ugcurl->beg + (int64_t) fsize) {
			ugcurl->event_code = UGET_EVENT_ERROR_INCORRECT_SOURCE;
			ugcurl->size[0] = 0;
			if (plugin->seg.list.size == 1)
				plugin->base.download = uget_a2cf_completed (&plugin->aria2.ctrl);
			return FALSE;
		}
	}
	// file.time
	if (plugin->file.time == -1) {
		curl_easy_getinfo (ugcurl->curl, CURLINFO_FILETIME, &ftime);
		plugin->file.time = (time_t) ftime;
	}

	if (uget_curl_open_file (ugcurl, plugin->file.path))
		return TRUE;
	else {
		ugcurl->event_code = UGET_EVENT_ERROR_FILE_OPEN_FAILED;
		return FALSE;
	}
}

static int prepare_file (UgetCurl* ugcurl, UgetPluginCurl* plugin)
{
	UgetCommon*  common;
	int    length;
	int    counts;
	int    value;
	int    folder_len;
	union {
		long     ftime;
		double   fsize;
		int64_t  val64;
		UriLink* ulink;
	} temp;

	// file.time
	curl_easy_getinfo (ugcurl->curl, CURLINFO_FILETIME, &temp.ftime);
	plugin->file.time = (time_t) temp.ftime;
	// file.size
	curl_easy_getinfo (ugcurl->curl,
			CURLINFO_CONTENT_LENGTH_DOWNLOAD, &temp.fsize);
	plugin->file.size = (int64_t) temp.fsize;
	if (plugin->file.size == -1)
		plugin->file.size = 0;

	common = plugin->common;
	// folder
	if (common->folder == NULL)
		length = 0;
	else {
		length = strlen (common->folder);
		value = common->folder[length - 1];
		if (value != '\\' || value != '/')
			length++;
	}
	folder_len = length;

	// decide filename
	if (common->file == NULL) {
		if (ugcurl->header.filename) {
			common->file = ugcurl->header.filename;
			ugcurl->header.filename = NULL;
		}
		else if (ugcurl->uri.part.file != -1)
			common->file = ug_uri_get_file (&ugcurl->uri.part);
		// if it is still no filename, set default one
		if (common->file == NULL)
			common->file = ug_strdup ("index");
	}
	length += strlen (common->file);

	// path = folder + filename
	ug_free (plugin->file.path);
	plugin->file.path = ug_malloc (length + 11);  // + ".xxx.aria2" + '\0'
	plugin->file.path[0] = 0;  // you need this line if common->folder is NULL.
	if (common->folder) {
		strcpy (plugin->file.path, common->folder);
		if (value != '\\' || value != '/') {
#if defined _WIN32 || defined _WIN64
			strcat (plugin->file.path, "\\");
#else
			strcat (plugin->file.path, "/");
#endif
		}
	}
	strcat (plugin->file.path, common->file);

	// create folder
	if (ug_create_dir_all (plugin->file.path, folder_len) == -1) {
		ugcurl->event_code = UGET_EVENT_ERROR_FOLDER_CREATE_FAILED;
		return FALSE;
	}

	// create file
	for (counts = 0;  counts < 1000;  counts++) {
		value = ug_open (plugin->file.path, UG_O_CREATE | UG_O_EXCL | UG_O_WRONLY,
				UG_S_IREAD | UG_S_IWRITE | UG_S_IRGRP | UG_S_IROTH);
//		value = ug_open (plugin->file.path, UG_O_CREATE | UG_O_EXCL | UG_O_RDWR,
//				UG_S_IREAD | UG_S_IWRITE | UG_S_IRGRP | UG_S_IROTH);
		// check if this directory can't access
//		if (value == -1 && ug_file_is_exist (plugin->file.path) == FALSE)
//			return FALSE;    // error

		// check exist downloaded file & it's control file
		strcat (plugin->file.path + length, ".aria2");
		if (value == -1) {
			if (uget_a2cf_load (&plugin->aria2.ctrl, plugin->file.path)) {
				if (plugin->aria2.ctrl.total_len == plugin->file.size) {
					plugin->aria2.path = ug_strdup (plugin->file.path);
					plugin->file.path[length] = 0;
					plugin->base.download = uget_a2cf_completed (&plugin->aria2.ctrl);
					break;
				}
				uget_a2cf_clear (&plugin->aria2.ctrl);
			}
		}
		else {
			if (plugin->file.size) {
				plugin->aria2.path = ug_strdup (plugin->file.path);
				uget_a2cf_init (&plugin->aria2.ctrl, plugin->file.size);
				uget_a2cf_save (&plugin->aria2.ctrl, plugin->aria2.path);
				// allocate disk space
				ug_seek (value, plugin->file.size - 1, SEEK_SET);
				ug_write (value, "X", 1);
			}
			ug_close (value);
			// remove tail ".aria2"
			*(char*) strstr (plugin->file.path + length, ".aria2") = 0;
			break;
		}

		// filename repeat
		sprintf (plugin->file.path + length, ".%d", counts);
	}

	if (counts == 1000) {
		ugcurl->event_code = UGET_EVENT_ERROR_FILE_CREATE_FAILED;
		return FALSE;
	}

	// set filename if counts > 0
	if (counts) {
		ug_free (common->file);
		common->file = ug_strdup (plugin->file.path + folder_len);
	}
	plugin->file_renamed = TRUE;

	// event
	if (ugcurl->resumable) {
		uget_plugin_post ((UgetPlugin*) plugin,
				uget_event_new_normal (UGET_EVENT_NORMAL_RESUMABLE, NULL));
	}
	else {
		uget_plugin_post ((UgetPlugin*) plugin,
				uget_event_new_normal (UGET_EVENT_NORMAL_NOT_RESUMABLE, NULL));
	}
#ifndef NDEBUG
	if (common->debug_level) {
		printf ("CURL message = %s\n", ugcurl->error_string);
		printf ("plugin->file.path = %s\n", plugin->file.path);
		printf ("plugin->file.size = %d\n", (int)plugin->file.size);
		printf ("plugin->file.time = %d\n", (int)plugin->file.time);
		printf ("resumable = %d\n", ugcurl->resumable);
	}
#endif

	// set flags to UriLink
	if (ugcurl->header.uri) {
		// HTTP redirection
		temp.ulink = plugin_replace_uri (plugin, ugcurl->uri.link,
		                                 ugcurl->header.uri, -1);
		ugcurl->uri.link = temp.ulink;
		ug_free (ugcurl->header.uri);
		ugcurl->header.uri = NULL;
	}
	else
		temp.ulink = ugcurl->uri.link;
	temp.ulink->scheme_type = ugcurl->scheme_type;
	temp.ulink->resumable = ugcurl->resumable;
	temp.ulink->tested = TRUE;
	temp.ulink->ok = TRUE;
	// change callback
	ugcurl->prepare.func = (UgetCurlFunc) prepare_existed;
	ugcurl->prepare.data = plugin;
	// file and it's offset
	temp.val64 = 0;
	uget_a2cf_lack (&plugin->aria2.ctrl,
	                (uint64_t*) &temp.val64,
	                (uint64_t*) &ugcurl->end);
	plugin->seg.beg = ugcurl->end;
	if (ugcurl->beg == temp.val64) {
		if (uget_curl_open_file (ugcurl, plugin->file.path) == FALSE) {
			ugcurl->event_code = UGET_EVENT_ERROR_FILE_OPEN_FAILED;
			return FALSE;
		}
		return TRUE;
	}
	else {
		ugcurl->beg = temp.val64;
		ugcurl->pos = temp.val64;
		curl_easy_setopt (ugcurl->curl, CURLOPT_RESUME_FROM_LARGE,
				(curl_off_t) temp.val64);
		if (uget_curl_open_file (ugcurl, plugin->file.path))
			ugcurl->restart = TRUE;
		return FALSE;
	}
}

static int  load_file_info (UgetPluginCurl* plugin)
{
	UgetCommon*  common;
	char*        path;
	int          value;
	int          length;

	common = plugin->common;
	if (common == NULL || common->file == NULL)
		return FALSE;
	// folder
	if (common->folder == NULL)
		length = 0;
	else {
		length = strlen (common->folder);
		value = common->folder[length - 1];
		if (value != '\\' || value != '/')
			length++;
	}
	// filename
	length += strlen (common->file);
	// path
	path = ug_malloc (length + 11);  // + ".xxx.aria2" + '\0'
	path[0] = 0;  // you need this line if common->folder is NULL.
	if (common->folder) {
		strcpy (path, common->folder);
		if (value != '\\' || value != '/') {
#if defined _WIN32 || defined _WIN64
			strcat (path, "\\");
#else
			strcat (path, "/");
#endif
		}
	}
	strcat (path, common->file);
	if (ug_file_is_exist (path) == FALSE) {
		ug_free (path);
		return FALSE;
	}
	strcat (path, ".aria2");
	// aria2 ctrl file
	if (uget_a2cf_load (&plugin->aria2.ctrl, path)) {
		plugin->file.size = plugin->aria2.ctrl.total_len;
		plugin->file.path = ug_strndup (path, length);
		plugin->aria2.path = path;
		plugin->base.download = uget_a2cf_completed (&plugin->aria2.ctrl);
		return TRUE;
	}
	else {
		uget_a2cf_clear (&plugin->aria2.ctrl);
		ug_free (path);
		return FALSE;
	}
}

static int  switch_uri (UgetPluginCurl* plugin, UgetCurl* ugcurl, int is_resumable)
{
	UriLink*  uri_link;
	UriLink*  uri_begin;
	int       cycled = FALSE;

	if (plugin->uri.link)
		uri_begin = (UriLink*) plugin->uri.link;
	else
		uri_begin = (UriLink*) plugin->uri.list.head;

	for (uri_link = uri_begin;  ;) {
		if (uri_link == uri_begin && cycled)
			break;
		if (uri_link == NULL) {
			uri_link = (UriLink*) plugin->uri.list.head;
			cycled = TRUE;
			continue;
		}
		if (uri_link->tested) {
			if (uri_link->ok == FALSE) {
				uri_link = uri_link->next;
				continue;
			}
			if (is_resumable && uri_link->resumable == FALSE) {
				uri_link = uri_link->next;
				continue;
			}
		}
		break;
	}

	// set flags from UriLink
	plugin->uri.link = (void*) uri_link->next;
	uget_curl_set_url (ugcurl, uri_link->uri);
	uri_link->scheme_type = ugcurl->scheme_type;
	ugcurl->uri.link = uri_link;
	ugcurl->resumable = uri_link->resumable;
	ugcurl->tested = uri_link->tested;
	ugcurl->test_ok = uri_link->ok;

	if (uri_link == uri_begin)
		return FALSE;
	return TRUE;
}

static void complete_file (UgetPluginCurl* plugin)
{
	if (plugin->aria2.path)
		ug_unlink (plugin->aria2.path);
	// modify file time
	if (plugin->common->timestamp == TRUE && plugin->file.time != -1)
		ug_modify_file_time (plugin->file.path, plugin->file.time);
	// completed message
	plugin->node->state |= UGET_STATE_COMPLETED;
	uget_plugin_post ((UgetPlugin*)plugin,
			uget_event_new (UGET_EVENT_COMPLETED));
	uget_plugin_post ((UgetPlugin*)plugin,
			uget_event_new (UGET_EVENT_STOP));
}

static int split_download (UgetPluginCurl* plugin, UgetCurl* ugcurl)
{
	UgetCurl*  temp;
	UgetCurl*  sibling = NULL;
	uint64_t   cur;
	uint64_t   end;

	if (plugin->aria2.path == NULL)
		return FALSE;
	// find recycled UgetCurl and start it.
	for (temp = (void*)plugin->seg.list.head;  temp;  temp = temp->next) {
		if (temp->state == UGET_CURL_RECYCLED) {
			plugin->seg.n_recycled--;
			switch_uri (plugin, temp, TRUE);
			uget_curl_run (temp, FALSE);
			return FALSE;
		}
	}
	// try to find unused space
	cur = plugin->seg.beg;
	if (uget_a2cf_lack (&plugin->aria2.ctrl, &cur, &end)) {
		plugin->seg.beg = end;
#ifndef NDEBUG
		if (plugin->common->debug_level) {
			printf ("\n" "lack %u-%u KiB\n",
					(unsigned) cur / 1024,
					(unsigned) end / 1024);
		}
#endif
	}
	else {
		// if no unused space, try to split current download
		end = 0;
		for (temp = (void*)plugin->seg.list.head;  temp;  temp = temp->next) {
			cur = temp->end - temp->pos;
			if (end < cur) {
				end = cur;
				sibling = temp;
			}
		}
		if (sibling == NULL)
			return FALSE;
		//
		cur = (sibling->end - sibling->pos) >> 1;
		if (cur < 16384 * 3)
			return FALSE;
		cur = sibling->end - cur;
		end = sibling->end;
		if (cur & 16383)
			cur += 16384 - (cur & 16383);

#ifndef NDEBUG
		if (plugin->common->debug_level) {
			printf ("\n" "split %u-%u KiB\n",
					(unsigned) cur / 1024,
					(unsigned) end / 1024);
		}
#endif
	}

	// reuse or create UgetCurl
	// if this UgetCurl has been inserted in seg.list, remove it.
	if (ugcurl)
		ug_list_remove (&plugin->seg.list, (UgLink*) ugcurl);
	else
		ugcurl = create_segment (plugin);

	// add to seg.list
	if (sibling == NULL)
		ug_list_append (&plugin->seg.list, (void*) ugcurl);
	else {
		ugcurl->split = TRUE;
		ug_list_insert (&plugin->seg.list,
				(void*) sibling->next, (void*) ugcurl);
	}

	ugcurl->beg = cur;
	ugcurl->end = end;
	uget_curl_run (ugcurl, FALSE);
	return TRUE;
}

static void delay_ms (UgetPluginCurl* plugin, int  milliseconds)
{
	while (plugin->stopped == FALSE) {
		if (milliseconds >  500) {
			milliseconds -= 500;
			ug_sleep (500);
			continue;
		}
		ug_sleep (milliseconds);
		return;
	}
}

static UgetCurl* create_segment (UgetPluginCurl* plugin)
{
	UgetCurl*  ugcurl;

	ugcurl = uget_curl_new ();
	uget_curl_set_common (ugcurl, plugin->common);
	uget_curl_set_proxy (ugcurl, plugin->proxy);
	uget_curl_set_http (ugcurl, plugin->http);
	uget_curl_set_ftp (ugcurl, plugin->ftp);
	// select URL
	switch_uri (plugin, ugcurl, FALSE);
	// set output function
	ugcurl->prepare.func = (UgetCurlFunc) prepare_existed;
	ugcurl->prepare.data = plugin;
	return ugcurl;
}

// speed control
#define SPEED_MIN    512

static void  adjust_speed_limit_index (UgetPluginCurl* plugin, int idx, int64_t remain)
{
	UgetCurl*  ucurl;

	// balance speed
	remain = remain / plugin->seg.n_active;

	for (ucurl = (UgetCurl*) plugin->seg.list.head; ucurl; ucurl=ucurl->next) {
		if (ucurl->state != UGET_CURL_RUN)
			continue;
		ucurl->limit[idx] = ucurl->speed[idx] + remain;
		if (ucurl->limit[idx] < SPEED_MIN)
			ucurl->limit[idx] = SPEED_MIN;
		ucurl->limit_changed = TRUE;
	}
}

static void  disable_speed_limit (UgetPluginCurl* plugin, int idx)
{
	UgetCurl*  ucurl;

	for (ucurl = (UgetCurl*) plugin->seg.list.head; ucurl; ucurl=ucurl->next) {
		ucurl->limit[idx] = 0;
		ucurl->limit_changed = TRUE;
	}
}

static void  adjust_speed_limit (UgetPluginCurl* plugin)
{
	if (plugin->seg.n_active == 0)
		return;

	// download
	if (plugin->limit.download > 0)
		adjust_speed_limit_index (plugin, 0, plugin->limit.download - plugin->speed.download);
	else if (plugin->limit_changed)
		disable_speed_limit (plugin, 0);
	// upload
	if (plugin->limit.upload > 0)
		adjust_speed_limit_index (plugin, 1, plugin->limit.upload - plugin->speed.upload);
	else if (plugin->limit_changed)
		disable_speed_limit (plugin, 1);

	plugin->limit_changed = FALSE;
}

