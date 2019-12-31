/*
 *
 *   Copyright (C) 2016-2020 by C.H. Huang
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
// UgetPluginInfo (derived from UgTypeInfo)

static void plugin_init (UgetPluginMedia* plugin);
static void plugin_final(UgetPluginMedia* plugin);
static int  plugin_ctrl (UgetPluginMedia* plugin, int code, void* data);
static int  plugin_accept(UgetPluginMedia* plugin, UgInfo* node_info);
static int  plugin_sync  (UgetPluginMedia* plugin, UgInfo* node_info);
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
	(UgetPluginSyncFunc) plugin_accept,
	(UgetPluginSyncFunc) plugin_sync,
	(UgetPluginCtrlFunc) plugin_ctrl,
	hosts,
	schemes,
	NULL,
	(UgetPluginGlobalFunc) global_set,
	(UgetPluginGlobalFunc) global_get
};
// extern
const UgetPluginInfo* UgetPluginMediaInfo = &UgetPluginMediaInfoStatic;

// ----------------------------------------------------------------------------
// global data and it's functions.

static struct
{
	UgetMediaMatchMode    match_mode;
	UgetMediaQuality      quality;
	UgetMediaType         type;
} global = {UGET_MEDIA_MATCH_NEAR, UGET_MEDIA_QUALITY_360P, UGET_MEDIA_TYPE_MP4};

static UgetResult  global_set(int option, void* parameter)
{
	switch (option) {
	case UGET_PLUGIN_MEDIA_GLOBAL_MATCH_MODE:
		global.match_mode = (intptr_t) parameter;
		break;

	case UGET_PLUGIN_MEDIA_GLOBAL_QUALITY:
		global.quality = (intptr_t) parameter;
		break;

	case UGET_PLUGIN_MEDIA_GLOBAL_TYPE:
		global.type = (intptr_t) parameter;
		break;

	default:
		// call parent's global_set()
		return uget_plugin_agent_global_set(option, parameter);
	}

	return UGET_RESULT_OK;
}

static UgetResult  global_get(int option, void* parameter)
{
	switch (option) {
	case UGET_PLUGIN_GLOBAL_MATCH:
		if (uget_site_get_id(parameter) < UGET_SITE_MEDIA)
			return UGET_RESULT_FAILED;
		break;

	default:
		// call parent's global_get()
		return uget_plugin_agent_global_get(option, parameter);
	}

	return UGET_RESULT_OK;
}

// ----------------------------------------------------------------------------
// control functions

static void plugin_init(UgetPluginMedia* plugin)
{
	// initialize UgetPluginAgent
	uget_plugin_agent_init((UgetPluginAgent*) plugin);
}

static void plugin_final(UgetPluginMedia* plugin)
{
	ug_free(plugin->title);
	uget_plugin_agent_final((UgetPluginAgent*) plugin);
}

// ----------------------------------------------------------------------------
// plugin_ctrl

static UgThreadResult  plugin_thread(UgetPluginMedia* plugin);

static int  plugin_ctrl(UgetPluginMedia* plugin, int code, void* data)
{
	switch (code) {
	case UGET_PLUGIN_CTRL_START:
		if (plugin->target_info) {
			return uget_plugin_agent_start((UgetPluginAgent*)plugin,
			                               (UgThreadFunc)plugin_thread);
		}
		break;

	default:
		// call parent's plugin_ctrl()
		return uget_plugin_agent_ctrl((UgetPluginAgent*)plugin, code, data);
	}
	return FALSE;
}

// ----------------------------------------------------------------------------
// plugin_sync

static int  plugin_sync(UgetPluginMedia* plugin, UgInfo* node_info)
{
	UgetFiles*     files;
	UgetCommon*    common;
	UgetProgress*  progress;

	if (plugin->stopped) {
		if (plugin->synced)
			return FALSE;
		plugin->synced = TRUE;
	}
	// avoid crash if plug-in failed to start.
	if (plugin->target_info == NULL)
		return FALSE;
	// sync data between plug-in and foreign UgData
	common = ug_info_realloc(node_info, UgetCommonInfo);
	// sum retry count
	common->retry_count = plugin->target_common->retry_count + plugin->retry_count;

	// change name by title ,item_index, and item_total
	if (plugin->named == FALSE && plugin->title) {
		plugin->named = TRUE;
		ug_free(common->name);
		// decide to show "(current/total) title" or "title"
		if (plugin->item_total > 1) {
			common->name = ug_strdup_printf("(%d/%d) %s",
					plugin->item_index + 1,
					plugin->item_total,
					plugin->title);
		}
		else
			common->name = ug_strdup(plugin->title);
	}

	// sync common data (include speed limit) between foreign data and target_info
	uget_plugin_agent_sync_common((UgetPluginAgent*) plugin,
	                              common, plugin->target_common);
	// downloading file name changed
	if (plugin->file_renamed == TRUE) {
		plugin->file_renamed = FALSE;
		uget_plugin_lock(plugin);
		ug_free(common->file);
		common->file = ug_strdup(plugin->target_common->file);
		uget_plugin_unlock(plugin);
	}
	// update UgetFiles
	files = ug_info_realloc(node_info, UgetFilesInfo);
	uget_plugin_lock(plugin);
    uget_files_sync(files, plugin->target_files);
    uget_plugin_unlock(plugin);

	// sync progress data from target_info to foreign data
	progress = ug_info_realloc(node_info, UgetProgressInfo);
	uget_plugin_agent_sync_progress((UgetPluginAgent*) plugin,
	                                progress, plugin->target_progress);
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

static int  plugin_accept(UgetPluginMedia* plugin, UgInfo* node_info)
{
	UgetCommon*  common;

	common = ug_info_get(node_info, UgetCommonInfo);
	if (common == NULL || common->uri == NULL)
		return FALSE;

	plugin->target_info = ug_info_new(8, 0);
	ug_info_assign(plugin->target_info, node_info, NULL);
	plugin->target_files  = ug_info_realloc(plugin->target_info, UgetFilesInfo);
	plugin->target_common = ug_info_get(plugin->target_info, UgetCommonInfo);
	plugin->target_proxy  = ug_info_get(plugin->target_info, UgetProxyInfo);
	plugin->target_progress = ug_info_realloc(plugin->target_info, UgetProgressInfo);

	return TRUE;
}

static int   is_file_completed(const char* file, const char* folder);

static UgThreadResult  plugin_thread(UgetPluginMedia* plugin)
{
	UgetPluginInfo*  plugin_info;
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
	// tell plugin_sync() to change foreign UgetCommon::name
	if (umedia->title) {
		plugin->title = umedia->title;
		umedia->title = NULL;
		plugin->synced = FALSE;
	}

	umitem = uget_media_match(umedia, global.match_mode,
	                          global.quality, global.type);
	if (umitem == NULL) {
		umitem = uget_media_match(umedia, global.match_mode,
		                          global.quality, global.type | UGET_MEDIA_TYPE_DEMUX);
	}
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
	http = ug_info_realloc(plugin->target_info, UgetHttpInfo);
	if (http->referrer == NULL)
		http->referrer = ug_strdup_printf("%s%s", common->uri, "# ");
	// clear copied common URI
	ug_free(common->uri);
	common->uri = NULL;
	// reset grand total data
	plugin->elapsed = 0;
	plugin->retry_count = 0;

	for (;  umitem;  umitem = umitem->next) {
		// stop this loop when user paused this plug-in.
		if (plugin->paused)
			break;
		// Don't increase item_index when running this loop first time.
		if (type)
			plugin->item_index++;

		switch (umitem->type & UGET_MEDIA_TYPE_MUX) {
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
		case UGET_MEDIA_TYPE_UNKNOWN:
		default:
			type = "unknown";
			break;
		}

		switch (umitem->quality) {
		case UGET_MEDIA_QUALITY_144P:
			quality = "144p";
			break;
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

		case UGET_MEDIA_QUALITY_UNKNOWN:
		default:
			if (umitem->type & UGET_MEDIA_TYPE_AUDIO)
				quality = "audio";
			else
				quality = "unknown";
			break;
		}

		// generate file name by title, quality, and type.
		uget_plugin_lock(plugin);
		ug_free(common->file);
		common->file = ug_strdup_printf("%s_%s.%s",
				(plugin->title) ? plugin->title : "unknown",
				quality, type);
		ug_str_replace_chars(common->file, "\\/:*?\"<>|", '_');
		uget_plugin_unlock(plugin);
		plugin->file_renamed = TRUE;

		// skip completed file
		if (is_file_completed(common->file, common->folder))
			continue;

		// use media link to replace common->uri
		common->uri = umitem->url;

		// tell plugin_sync() to change foreign UgetCommon::name
		plugin->named = FALSE;
		// save/reset retry count
		plugin->retry_count += common->retry_count;
		common->retry_count = 0;
		// save/reset elapsed
		plugin->elapsed += plugin->target_progress->elapsed;
		plugin->target_progress->elapsed = 0;

		uget_plugin_agent_global_get(UGET_PLUGIN_AGENT_GLOBAL_PLUGIN,
		                             &plugin_info);
		// create target_plugin to download
		plugin->target_plugin = uget_plugin_new(plugin_info);
		uget_plugin_accept(plugin->target_plugin, plugin->target_info);
		uget_plugin_ctrl_speed(plugin->target_plugin, plugin->limit);
		if (uget_plugin_start(plugin->target_plugin) == FALSE) {
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
			uget_plugin_lock(plugin);
			uget_plugin_sync(plugin->target_plugin,
			                 plugin->target_info);
			uget_plugin_unlock(plugin);
		} while (uget_plugin_get_state(plugin->target_plugin));

		// free target_plugin
		uget_plugin_unref(plugin->target_plugin);
		plugin->target_plugin = NULL;
	}

	common->uri = NULL;  // Don't free common->uri again.

	if (plugin->paused == FALSE && umitem == NULL) {
		uget_plugin_post((UgetPlugin*) plugin,
		                 uget_event_new(UGET_EVENT_COMPLETED));
	}
	uget_plugin_post((UgetPlugin*)plugin,
			uget_event_new(UGET_EVENT_STOP));

exit:
	plugin->stopped = TRUE;
	uget_media_free(umedia);
	uget_plugin_unref((UgetPlugin*) plugin);
	return UG_THREAD_RESULT;
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
