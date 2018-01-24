/*
 *
 *   Copyright (C) 2016-2018 by C.H. Huang
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

// UgetPluginMedia
// use libcurl to get video info
// use curl plug-in to download media file.

#include <stdio.h>
#include <stdint.h>
#include <UgUtil.h>
#include <UgFileUtil.h>
#include <UgString.h>
#include <UgetData.h>
#include <UgetEvent.h>
#include <UgetMedia.h>

#include <curl/curl.h>
#include <UgetPluginCurl.h>
#include <UgetPluginAria2.h>
#include <UgetPluginMedia.h>

#if defined _WIN32 || defined _WIN64
#include <windows.h>
#define  ug_sleep                 Sleep
#else
#include <unistd.h>               // usleep()
#define  ug_sleep(millisecond)    usleep(millisecond * 1000)
#endif // _WIN32 || _WIN64

#ifdef HAVE_GLIB
#include <glib/gi18n.h>
#undef  printf
#else
#define N_(x)   x
#define  _(x)   x
#endif

// ----------------------------------------------------------------------------
// UgetPluginInfo (derived from UgDataInfo)

static void plugin_init (UgetPluginMedia* plugin);
static void plugin_final(UgetPluginMedia* plugin);
static int  plugin_ctrl (UgetPluginMedia* plugin, int code, void* data);
static int  plugin_sync (UgetPluginMedia* plugin, UgetNode* node);
static UgetResult  global_set(int code, void* parameter);
static UgetResult  global_get(int code, void* parameter);

static const char* schemes[] = {"http", "https", NULL};
static const char* hosts[]   = {"youtube.com", "youtu.be",
                                NULL};

static const UgetPluginInfo UgetPluginMediaInfoStatic =
{
	"media",
	sizeof(UgetPluginMedia),
	(UgInitFunc)   plugin_init,
	(UgFinalFunc)  plugin_final,
	(UgAssignFunc) NULL,
	(UgetPluginCtrlFunc) plugin_ctrl,
	(UgetPluginSyncFunc) plugin_sync,
	hosts,
	schemes,
	NULL,
	(UgetPluginSetFunc) global_set,
	(UgetPluginGetFunc) global_get
};
// extern
const UgetPluginInfo* UgetPluginMediaInfo = &UgetPluginMediaInfoStatic;

// ----------------------------------------------------------------------------
// global data and it's functions.

static struct
{
	const UgetPluginInfo* plugin_info;
	UgetMediaMatchMode    match_mode;
	UgetMediaQuality      quality;
	UgetMediaType         type;
	int   ref_count;
} global = {NULL, 0, 0, 0, 0};

static UgetResult  global_init(void)
{
	if (global.plugin_info == NULL) {
#if defined _WIN32 || defined _WIN64
		WSADATA  WSAData;
		WSAStartup(MAKEWORD(2, 2), &WSAData);
#endif // _WIN32 || _WIN64

		if (curl_global_init(CURL_GLOBAL_ALL) != CURLE_OK) {
#if defined _WIN32 || defined _WIN64
			WSACleanup();
#endif
			return UGET_RESULT_ERROR;
		}
		global.plugin_info = UgetPluginCurlInfo;
//		global.plugin_info = UgetPluginAria2Info;
		// matching mode: UGET_MEDIA_MATCH_1 or UGET_MEDIA_MATCH_NEAR
		global.match_mode = UGET_MEDIA_MATCH_NEAR;
		global.quality = UGET_MEDIA_QUALITY_360P;
		global.type = UGET_MEDIA_TYPE_MP4;
	}
	global.ref_count++;
	return UGET_RESULT_OK;
}

static void  global_ref(void)
{
	global.ref_count++;
}

static void  global_unref(void)
{
	if (global.plugin_info == NULL)
		return;

	global.ref_count--;
	if (global.ref_count == 0) {
		global.plugin_info = NULL;
		curl_global_cleanup();
#if defined _WIN32 || defined _WIN64
		WSACleanup();
#endif
	}
}

static UgetResult  global_set(int option, void* parameter)
{
	switch (option) {
	case UGET_PLUGIN_INIT:
		// do global initialize/uninitialize here
		if (parameter)
			return global_init();
		else
			global_unref();
		break;

	case UGET_PLUGIN_MEDIA_DEFAULT_PLUGIN:
		global.plugin_info = parameter;
		break;

	case UGET_PLUGIN_MEDIA_MATCH_MODE:
		global.match_mode = (intptr_t) parameter;
		break;

	case UGET_PLUGIN_MEDIA_QUALITY:
		global.quality = (intptr_t) parameter;
		break;

	case UGET_PLUGIN_MEDIA_TYPE:
		global.type = (intptr_t) parameter;
		break;

	default:
		return UGET_RESULT_UNSUPPORT;
	}

	return UGET_RESULT_OK;
}

static UgetResult  global_get(int option, void* parameter)
{
	switch (option) {
	case UGET_PLUGIN_INIT:
		if (parameter)
			*(int*)parameter = (global.plugin_info) ? TRUE : FALSE;
		break;

	case UGET_PLUGIN_ERROR_CODE:
		if (parameter)
			*(int*)parameter = 0;
		break;

	case UGET_PLUGIN_MATCH:
		if (uget_site_get_id(parameter) < UGET_SITE_MEDIA)
			return UGET_RESULT_FAILED;
		return UGET_RESULT_OK;

	default:
		return UGET_RESULT_UNSUPPORT;
	}

	return UGET_RESULT_OK;
}

// ----------------------------------------------------------------------------
// control functions

static void plugin_init(UgetPluginMedia* plugin)
{
	if (global.plugin_info == NULL)
		global_init();
	else
		global_ref();
}

static void plugin_final(UgetPluginMedia* plugin)
{
	// extent data and plug-in
	if (plugin->target_node)
		uget_node_unref(plugin->target_node);
	if (plugin->target_node_child)
		uget_node_unref(plugin->target_node_child);
	if (plugin->target_plugin)
		uget_plugin_unref(plugin->target_plugin);
	// other data
	ug_free(plugin->title);
	// unassign node
	if (plugin->node)
		uget_node_unref(plugin->node);

	global_unref();
}

// ----------------------------------------------------------------------------
// plugin_ctrl

static int  plugin_ctrl_speed(UgetPluginMedia* plugin, int* speed);
static int  plugin_start(UgetPluginMedia* plugin, UgetNode* node);

static int  plugin_ctrl(UgetPluginMedia* plugin, int code, void* data)
{
	switch (code) {
	case UGET_PLUGIN_CTRL_START:
		if (plugin->node == NULL)
			return plugin_start(plugin, data);
		break;

	case UGET_PLUGIN_CTRL_STOP:
		plugin->paused = TRUE;
		return TRUE;

	case UGET_PLUGIN_CTRL_SPEED:
		// speed control
		return plugin_ctrl_speed(plugin, data);

	// output ---------------
	case UGET_PLUGIN_CTRL_ACTIVE:
		*(int*)data = (plugin->stopped) ? FALSE : TRUE;
		return TRUE;

	// unused ---------------
	case UGET_PLUGIN_CTRL_NODE_UPDATED:
	default:
		break;
	}
	return FALSE;
}

static int  plugin_ctrl_speed(UgetPluginMedia* plugin, int* speed)
{
	UgetCommon*  common;
	int          value;

	// Don't do anything if speed limit keep no change.
	if (plugin->limit[0] == speed[0] && plugin->limit[1] == speed[1])
		return TRUE;
	// decide speed limit by user specified data.
	if (plugin->node == NULL) {
		plugin->limit[0] = speed[0];
		plugin->limit[1] = speed[1];
	}
	else {
		common = ug_info_realloc(plugin->node->info, UgetCommonInfo);
		// download
		value = speed[0];
		if (common->max_download_speed) {
			if (value > common->max_download_speed || value == 0)
				value = common->max_download_speed;
		}
		plugin->limit[0] = value;
		// upload
		value = speed[1];
		if (common->max_upload_speed) {
			if (value > common->max_upload_speed || value == 0)
				value = common->max_upload_speed;
		}
		plugin->limit[1] = value;
	}
	// notify plug-in that speed limit has been changed
	plugin->limit_changed = TRUE;
	return TRUE;
}

// ----------------------------------------------------------------------------
// plugin_sync
static int  sync_child_node(UgetNode* node, UgetNode* src, int src_is_active);

static int  plugin_sync(UgetPluginMedia* plugin, UgetNode* node)
{
	UgetCommon*    common;
	UgetProgress*  progress;

	if (plugin->stopped) {
		if (plugin->synced)
			return FALSE;
		plugin->synced = TRUE;
	}
	// avoid crash if plug-in plug-in failed to start.
	if (plugin->node == NULL)
		return TRUE;
	if (node == NULL)
		node = plugin->node;
	// change node name by title ,item_index, and item_total
	if (plugin->named == FALSE && plugin->title) {
		plugin->named = TRUE;
		ug_free(node->name);
		// decide to show "(current/total) title" or "title"
		if (plugin->item_total > 1) {
			node->name = ug_strdup_printf("(%d/%d) %s",
					plugin->item_index + 1,
					plugin->item_total,
					plugin->title);
		}
		else
			node->name = ug_strdup(plugin->title);
	}

	// sync data between plug-in and node
	common = ug_info_realloc(node->info, UgetCommonInfo);
	// sum retry count
	common->retry_count = plugin->target_common->retry_count + plugin->retry_count;
	// sync changed limit from UgetNode
	if (plugin->target_common->max_upload_speed != common->max_upload_speed ||
		plugin->target_common->max_download_speed != common->max_download_speed)
	{
		plugin->target_common->max_upload_speed = common->max_upload_speed;
		plugin->target_common->max_download_speed = common->max_download_speed;
	}
	plugin->target_common->max_connections = common->max_connections;
	plugin->target_common->retry_limit = common->retry_limit;

	// downloading file name changed
	if (plugin->file_renamed == TRUE) {
		plugin->file_renamed = FALSE;
		ug_mutex_lock(&plugin->mutex);
		ug_free(common->file);
		common->file = ug_strdup(plugin->target_common->file);
		ug_mutex_unlock(&plugin->mutex);
	}
	// sync child node from target_node_child
	if (plugin->sync_child == TRUE) {
		plugin->sync_child = FALSE;
		ug_mutex_lock(&plugin->mutex);
		sync_child_node(node, plugin->target_node_child,
		                (plugin->stopped) ? FALSE : TRUE);
		ug_mutex_unlock(&plugin->mutex);
	}
	// sync changed limit from UgetNode to plug-in
	if (plugin->target_common->max_upload_speed != common->max_upload_speed ||
		plugin->target_common->max_download_speed != common->max_download_speed)
	{
		plugin->target_common->max_upload_speed = common->max_upload_speed;
		plugin->target_common->max_download_speed = common->max_download_speed;
		plugin->limit_changed = TRUE;
	}
	plugin->target_common->max_connections = common->max_connections;
	plugin->target_common->retry_limit = common->retry_limit;

	// update progress
	progress = ug_info_realloc(node->info, UgetProgressInfo);
	progress->complete       = plugin->target_progress->complete;
	progress->total          = plugin->target_progress->total;
	progress->download_speed = plugin->target_progress->download_speed;
	progress->upload_speed   = plugin->target_progress->upload_speed;
	progress->uploaded       = plugin->target_progress->uploaded;
	progress->elapsed        = plugin->target_progress->elapsed;
	progress->percent        = plugin->target_progress->percent;
	progress->left           = plugin->target_progress->left;
	// recount progress if plug-in download multiple files
	if (plugin->item_total > 1) {
		// recount percent
		progress->percent = (progress->percent + 100 * plugin->item_index)
				/ plugin->item_total;
		// recount/speculate left time
		if (progress->download_speed > 0) {
			progress->left =
					(progress->total *
					(plugin->item_total - plugin->item_index) -
					 progress->complete)
					/ progress->download_speed;
		}
		// sum elapsed time
		progress->elapsed += plugin->elapsed;
	}

	// if plug-in was stopped, return FALSE.
	return TRUE;
}

// ----------------------------------------------------------------------------

static UG_THREAD_RETURN_TYPE  plugin_thread(UgetPluginMedia* plugin);

static int  plugin_start(UgetPluginMedia* plugin, UgetNode* node)
{
	int          ok;
	UgThread     thread;
	UgetCommon*  common;

	common = ug_info_get(node->info, UgetCommonInfo);
	if (common == NULL || common->uri == NULL)
		return FALSE;

	plugin->target_node = uget_node_new(NULL);
	ug_info_assign(plugin->target_node->info, node->info, NULL);
	plugin->target_common = ug_info_get(plugin->target_node->info, UgetCommonInfo);
	plugin->target_proxy  = ug_info_get(plugin->target_node->info, UgetProxyInfo);
	plugin->target_progress = ug_info_realloc(plugin->target_node->info, UgetProgressInfo);

	// assign node
	uget_node_ref(node);
	plugin->node = node;

	// try to start thread
	plugin->paused = FALSE;
	plugin->stopped = FALSE;
	uget_plugin_ref((UgetPlugin*) plugin);
	ok = ug_thread_create(&thread, (UgThreadFunc) plugin_thread, plugin);
	if (ok == UG_THREAD_OK)
		ug_thread_unjoin(&thread);
	else {
		// failed to start thread -----------------
		plugin->paused = TRUE;
		plugin->stopped = TRUE;
		// don't assign node
		uget_node_unref(plugin->node);
		plugin->node = NULL;
		// post error message and decreases the reference count
		uget_plugin_post((UgetPlugin*) plugin,
				uget_event_new_error(UGET_EVENT_ERROR_THREAD_CREATE_FAILED,
				                     NULL));
		uget_plugin_unref((UgetPlugin*) plugin);
		return FALSE;
	}

	return TRUE;
}

static int   is_file_completed(const char* file, const char* folder);

static UG_THREAD_RETURN_TYPE  plugin_thread(UgetPluginMedia* plugin)
{
	UgetMedia*     umedia;
	UgetMediaItem* umitem;
	UgetCommon*    common;
	UgetHttp*      http;
	UgetEvent*     msg_next;
	UgetEvent*     msg;
	const char*    type = NULL;
	const char*    quality = NULL;

	common = plugin->target_common;
	umedia = uget_media_new(common->uri, 0);
	if (uget_media_grab_items(umedia, plugin->target_proxy) == 0) {
		if (umedia->event) {
			uget_plugin_post((UgetPlugin*) plugin, umedia->event);
			umedia->event = NULL;
		}
		else {
			uget_plugin_post((UgetPlugin*) plugin,
					uget_event_new_error(UGET_EVENT_ERROR_CUSTOM,
					                     _("Failed to get media link.")));
		}
		goto exit;
	}
	// tell plugin_sync() to change node name
	if (umedia->title) {
		plugin->title = umedia->title;
		umedia->title = NULL;
		plugin->synced = FALSE;
	}

	umitem = uget_media_match(umedia, global.match_mode,
	                          global.quality, global.type);
	if (umitem == NULL) {
		uget_plugin_post((UgetPlugin*) plugin,
				uget_event_new_error(UGET_EVENT_ERROR_CUSTOM,
				                     _("No matched media.")));
		goto exit;
	}
	// set item_index and item_total
	plugin->item_index = 0;
	plugin->item_total = umedia->size -
			ug_list_position((UgList*) umedia, (UgLink*) umitem);

	// set HTTP referrer
	http = ug_info_realloc(plugin->target_node->info, UgetHttpInfo);
	if (http->referrer == NULL)
		http->referrer = ug_strdup_printf("%s%s", common->uri, "# ");
	// clear copied common URI
	ug_free(common->uri);
	common->uri = NULL;
	// reset grand total data
	plugin->elapsed = 0;
	plugin->retry_count = 0;
	// create children node
	plugin->target_node_child = uget_node_new(NULL);

	for (;  umitem;  umitem = umitem->next) {
		// stop this loop when user paused this plug-in.
		if (plugin->paused)
			break;
		// Don't increase item_index when running this loop first time.
		if (type)
			plugin->item_index++;

		switch (umitem->type) {
		case UGET_MEDIA_TYPE_3GPP:
			type = "3gpp";
			break;
		case UGET_MEDIA_TYPE_FLV:
			type = "flv";
			break;
		case UGET_MEDIA_TYPE_MP4:
			type = "mp4";
			break;
		case UGET_MEDIA_TYPE_WEBM:
			type = "webm";
			break;
		default:
			type = "unknown";
			break;
		}

		switch (umitem->quality) {
		case UGET_MEDIA_QUALITY_240P:
			quality = "240p";
			break;
		case UGET_MEDIA_QUALITY_360P:
			quality = "360p";
			break;
		case UGET_MEDIA_QUALITY_480P:
			quality = "480p";
			break;
		case UGET_MEDIA_QUALITY_720P:
			quality = "720p";
			break;
		case UGET_MEDIA_QUALITY_1080P:
			quality = "1080p";
			break;
		default:
			quality = "unknown";
			break;
		}

		// generate file name by title, quality, and type.
		ug_mutex_lock(&plugin->mutex);
		ug_free(common->file);
		common->file = ug_strdup_printf("%s_%s.%s",
				(plugin->title) ? plugin->title : "unknown",
				quality, type);
		ug_str_replace_chars(common->file, "\\/:*?\"<>|", '_');
		ug_mutex_unlock(&plugin->mutex);
		plugin->file_renamed = TRUE;

		// skip completed file
		if (is_file_completed(common->file, common->folder))
			continue;

		// use media link to replace common->uri
		common->uri = umitem->url;

		// tell plugin_sync() to change node name
		plugin->named = FALSE;
		// save/reset retry count
		plugin->retry_count += common->retry_count;
		common->retry_count = 0;
		// save/reset elapsed
		plugin->elapsed += plugin->target_progress->elapsed;
		plugin->target_progress->elapsed = 0;

		// create target_plugin to download
		plugin->target_plugin = uget_plugin_new(global.plugin_info);
		uget_plugin_ctrl_speed(plugin->target_plugin, plugin->limit);
		if (uget_plugin_start(plugin->target_plugin, plugin->target_node) == FALSE) {
			msg = uget_event_new_error(UGET_EVENT_ERROR_THREAD_CREATE_FAILED,
			                           NULL);
			uget_plugin_post((UgetPlugin*) plugin, msg);
		}

		do {
			// sleep 0.5 second
			ug_sleep(500);
			// stop target_plugin when user paused this plug-in.
			if (plugin->paused) {
				uget_plugin_stop(plugin->target_plugin);
				break;
			}
			if (plugin->limit_changed) {
				plugin->limit_changed = FALSE;
				uget_plugin_ctrl_speed(plugin->target_plugin, plugin->limit);
			}

			// sync child(file) node from target_node to target_node_child
			ug_mutex_lock(&plugin->mutex);
			plugin->sync_child = sync_child_node(plugin->target_node_child,
			                                     plugin->target_node, TRUE);
			ug_mutex_unlock(&plugin->mutex);

			// move event from target_plugin to plug-in
			msg = uget_plugin_pop((UgetPlugin*) plugin->target_plugin);
			for (;  msg;  msg = msg_next) {
				msg_next = msg->next;
				msg->prev = NULL;
				msg->next = NULL;

				// handle or discard some message
				switch (msg->type) {
				case UGET_EVENT_NAME:
					// tell plugin_sync() to change file name
					plugin->file_renamed = TRUE;
					break;

				case UGET_EVENT_ERROR:
					// stop downloading if error occurred
					plugin->paused = TRUE;
					break;

				case UGET_EVENT_STOP:
				case UGET_EVENT_COMPLETED:
					// discard message
					uget_event_free(msg);
					continue;
				}
				// post event to plug-in
				uget_plugin_post((UgetPlugin*) plugin, msg);
			}
			// sync data in plugin_sync()
			plugin->synced = FALSE;
		} while (uget_plugin_sync(plugin->target_plugin, plugin->target_node));

		// sync file node
		ug_mutex_lock(&plugin->mutex);
		plugin->sync_child = sync_child_node(plugin->target_node_child,
		                                     plugin->target_node, FALSE);
		ug_mutex_unlock(&plugin->mutex);

		// free target_plugin
		uget_plugin_unref((UgetPlugin*) plugin->target_plugin);
		plugin->target_plugin = NULL;
	}

	common->uri = NULL;  // Don't free common->uri again.

	if (plugin->paused == FALSE && umitem == NULL) {
		plugin->node->group |= UGET_GROUP_COMPLETED;
		uget_plugin_post((UgetPlugin*) plugin,
				uget_event_new(UGET_EVENT_COMPLETED));
	}
	uget_plugin_post((UgetPlugin*)plugin,
			uget_event_new(UGET_EVENT_STOP));

exit:
	plugin->stopped = TRUE;
	uget_media_free(umedia);
	uget_plugin_unref((UgetPlugin*) plugin);
	return UG_THREAD_RETURN_VALUE;
}

static int   is_file_completed(const char* file, const char* folder)
{
	char* path;
	char* temp;
	int   result = FALSE;

	if (folder == NULL)
		path = ug_strdup(file);
	else
		path = ug_build_filename(folder, file, NULL);

	if (ug_file_is_exist(path)) {
		temp = ug_strdup_printf("%s.%s", path, "aria2");
		ug_free(path);
		path = temp;
		if (ug_file_is_exist(path) == FALSE)
			result = TRUE;
	}

	// debug
//	if (result)
//		printf("%s is completed\n", path);

	ug_free(path);
	return result;
}

static int  sync_child_node(UgetNode* node, UgetNode* src, int src_is_active)
{
	UgetNode*  node_child;
	UgetNode*  src_child;
	UgetNode*  new_child;
	int        link_changed = FALSE;

	for (src_child = src->children;  src_child;  src_child = src_child->next) {
		for (node_child = node->children;  node_child;  node_child = node_child->next) {
			if (strcmp(src_child->name, node_child->name) == 0)
				break;
		}
		// if found node that has the same name
		if (node_child) {
			// clear UGET_GROUP_ACTIVE if not active
			if (src_is_active == FALSE)
				node_child->group = 0;
		}
		else {
			link_changed = TRUE;
			// add new node if not found
			new_child = uget_node_new(NULL);
			new_child->name = ug_strdup(src_child->name);
			new_child->group = (src_is_active) ? UGET_GROUP_ACTIVE : 0;
			uget_node_prepend(node, new_child);
		}
	}

	// delete unused/removed file node
	if (src_is_active == FALSE) {
		for (node_child = node->children;  node_child;  node_child = new_child) {
			new_child = node_child->next;
			if (node_child->group == UGET_GROUP_ACTIVE) {
				uget_node_remove(node, node_child);
				uget_node_unref(node_child);
				link_changed = TRUE;
			}
		}
	}

	return link_changed;
}
