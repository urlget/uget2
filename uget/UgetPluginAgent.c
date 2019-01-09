/*
 *
 *   Copyright (C) 2016-2019 by C.H. Huang
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

#include <curl/curl.h>
#include <UgString.h>
#include <UgetPluginCurl.h>
#include <UgetPluginAgent.h>

#if defined _WIN32 || defined _WIN64
#include <windows.h>
#endif // _WIN32 || _WIN64

// ----------------------------------------------------------------------------
// global data

static struct
{
	const UgetPluginInfo* default_plugin;
	int   ref_count;
} global = {NULL, 0};

// ----------------------------------------------------------------------------
// global functions

UgetResult  uget_plugin_agent_global_init(void)
{
	if (global.default_plugin == NULL) {
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
		global.default_plugin = UgetPluginCurlInfo;
	}
	global.ref_count++;
	return UGET_RESULT_OK;
}

void  uget_plugin_agent_global_ref(void)
{
	global.ref_count++;
}

void  uget_plugin_agent_global_unref(void)
{
	if (global.default_plugin == NULL)
		return;

	global.ref_count--;
	if (global.ref_count == 0) {
		global.default_plugin = NULL;
		curl_global_cleanup();
#if defined _WIN32 || defined _WIN64
		WSACleanup();
#endif
	}
}

UgetResult  uget_plugin_agent_global_set(int option, void* parameter)
{
	switch (option) {
	case UGET_PLUGIN_GLOBAL_INIT:
		// do global initialize/finalize here
		if (parameter)
			return uget_plugin_agent_global_init();
		else
			uget_plugin_agent_global_unref();
		break;

	case UGET_PLUGIN_AGENT_GLOBAL_PLUGIN:
		global.default_plugin = parameter;
		break;

	default:
		return UGET_RESULT_UNSUPPORT;
	}

	return UGET_RESULT_OK;
}

UgetResult  uget_plugin_agent_global_get(int option, void* parameter)
{
	switch (option) {
	case UGET_PLUGIN_GLOBAL_INIT:
		if (parameter)
			*(int*)parameter = global.ref_count;
		break;

	case UGET_PLUGIN_AGENT_GLOBAL_PLUGIN:
		*(void**)parameter = (void*)global.default_plugin;
		break;

	default:
		return UGET_RESULT_UNSUPPORT;
	}

	return UGET_RESULT_OK;
}

// ----------------------------------------------------------------------------
// instance functions

void uget_plugin_agent_init(UgetPluginAgent* plugin)
{
	if (global.ref_count == 0)
		uget_plugin_agent_global_init();
	else
		uget_plugin_agent_global_ref();
}

void uget_plugin_agent_final(UgetPluginAgent* plugin)
{
	// extent data and plug-in
	if (plugin->target_info)
		ug_info_unref(plugin->target_info);
	if (plugin->target_plugin)
		uget_plugin_unref(plugin->target_plugin);

	uget_plugin_agent_global_unref();
}

int   uget_plugin_agent_ctrl(UgetPluginAgent* plugin, int code, void* data)
{
	switch (code) {
	case UGET_PLUGIN_CTRL_START:
		return FALSE;

	case UGET_PLUGIN_CTRL_STOP:
		plugin->paused = TRUE;
		return TRUE;

	case UGET_PLUGIN_CTRL_SPEED:
		// speed control
		return uget_plugin_agent_ctrl_speed(plugin, data);

	// state ----------------
	case UGET_PLUGIN_GET_STATE:
		*(int*)data = (plugin->stopped) ? FALSE : TRUE;
		return TRUE;

	default:
		break;
	}
	return FALSE;
}

int  uget_plugin_agent_ctrl_speed(UgetPluginAgent* plugin, int* speed)
{
	UgetCommon* common;
	int         value;

	// notify plug-in that speed limit has been changed
	if (plugin->limit[0] != speed[0] || plugin->limit[1] != speed[1])
		plugin->limit_changed = TRUE;
	// decide speed limit by user specified data.
	if (plugin->target_info)
		common = ug_info_get(plugin->target_info, UgetCommonInfo);
	else
		common = NULL;

	if (common == NULL) {
		plugin->limit[0] = speed[0];
		plugin->limit[1] = speed[1];
	}
	else {
		// download
		value = speed[0];
		if (common->max_download_speed) {
			if (value > common->max_download_speed || value == 0) {
				value = common->max_download_speed;
				plugin->limit_changed = TRUE;
			}
		}
		plugin->limit[0] = value;
		// upload
		value = speed[1];
		if (common->max_upload_speed) {
			if (value > common->max_upload_speed || value == 0) {
				value = common->max_upload_speed;
				plugin->limit_changed = TRUE;
			}
		}
		plugin->limit[1] = value;
	}
	return plugin->limit_changed;
}

// ----------------------------------------------------------------------------
// sync functions

void  uget_plugin_agent_sync_common(UgetPluginAgent* plugin,
                                    UgetCommon* common,
                                    UgetCommon* target)
{
	if (target == NULL)
		target = ug_info_realloc(plugin->target_info, UgetCommonInfo);

	// sync speed limit from common to target
	if (target->max_upload_speed   != common->max_upload_speed ||
		target->max_download_speed != common->max_download_speed)
	{
		target->max_upload_speed   = common->max_upload_speed;
		target->max_download_speed = common->max_download_speed;
		plugin->limit[1] = common->max_upload_speed;
		plugin->limit[0] = common->max_download_speed;
		uget_plugin_agent_ctrl_speed(plugin, plugin->limit);
	}
	target->max_connections = common->max_connections;

	target->retry_limit = common->retry_limit;
	common->retry_count = target->retry_count;
}

void  uget_plugin_agent_sync_progress(UgetPluginAgent* plugin,
                                      UgetProgress* progress,
                                      UgetProgress* target)
{
	if (target == NULL)
		target = ug_info_realloc(plugin->target_info, UgetProgressInfo);

	// sync progress from target to progress
	progress->complete       = target->complete;
	progress->total          = target->total;
	progress->download_speed = target->download_speed;
	progress->upload_speed   = target->upload_speed;
	progress->uploaded       = target->uploaded;
	progress->elapsed        = target->elapsed;
	progress->percent        = target->percent;
	progress->left           = target->left;
}

// ----------------------------------------------------------------------------
// thread functions

int   uget_plugin_agent_start(UgetPluginAgent* plugin,
                              UgThreadFunc thread_func)
{
	UgThread     thread;
	int          ok;

	// try to start thread
	plugin->paused = FALSE;
	plugin->stopped = FALSE;
	uget_plugin_ref((UgetPlugin*) plugin);
	ok = ug_thread_create(&thread, (UgThreadFunc) thread_func, plugin);
	if (ok == UG_THREAD_OK)
		ug_thread_unjoin(&thread);
	else {
		// failed to start thread -----------------
		plugin->paused = TRUE;
		plugin->stopped = TRUE;
		// post error message and decreases the reference count
		uget_plugin_post((UgetPlugin*) plugin,
				uget_event_new_error(UGET_EVENT_ERROR_THREAD_CREATE_FAILED,
				                     NULL));
		uget_plugin_unref((UgetPlugin*) plugin);
		return FALSE;
	}

	return TRUE;
}
