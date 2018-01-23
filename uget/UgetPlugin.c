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

#include <stdlib.h>
#include <string.h>
#include <UgetPlugin.h>

// ------------------------------------
// UgetPluginInfo global functions

UgetPlugin*  uget_plugin_new(const UgetPluginInfo* info)
{
	UgetPlugin*  plugin;

	plugin = ug_malloc0(info->size);
	plugin->info = info;
	ug_mutex_init(&plugin->mutex);
	plugin->ref_count = 1;
	info->init(plugin);
	return plugin;
}

UgetResult  uget_plugin_set(const UgetPluginInfo* info, int option, void* parameter)
{
	UgetPluginSetFunc  set;

	set = info->set;
	if (set)
		return set(option, parameter);
	return UGET_RESULT_UNSUPPORT;
}

UgetResult  uget_plugin_get(const UgetPluginInfo* info, int option, void* parameter)
{
	UgetPluginGetFunc  get;

	get = info->get;
	if (get)
		return get(option, parameter);
	return UGET_RESULT_UNSUPPORT;
}

int  uget_plugin_match(const UgetPluginInfo* info, UgUri* uuri)
{
	UgetResult   res;
	const char*  str;
	int          len;
	int          matched_count = 0;

	if (info == NULL)
		return 0;

	if (info->hosts && (len = ug_uri_part_host(uuri, &str))) {
		if (ug_uri_match_hosts(uuri, (char**)info->hosts) >= 0) {
			matched_count++;
			// Don't match this plug-in if it is for specify host.
			res = uget_plugin_get(info, UGET_PLUGIN_MATCH, (void*) uuri->uri);
			if (res == UGET_RESULT_FAILED)
				matched_count = -1;
			else if (res == UGET_RESULT_OK)
				matched_count = 2;
		}
	}

	if (info->schemes && (len = ug_uri_part_scheme(uuri, &str))) {
		if (ug_uri_match_schemes(uuri, (char**)info->schemes) >= 0)
			matched_count++;
	}

	if (info->file_exts && (len = ug_uri_part_file_ext(uuri, &str))) {
		if (ug_uri_match_file_exts(uuri, (char**)info->file_exts) >= 0)
			matched_count++;
	}

	return matched_count;
}

// ------------------------------------
// UgetPlugin functions

//void  uget_plugin_init(UgetPlugin* plugin);
//void  uget_plugin_final(UgetPlugin* plugin);
//void  uget_plugin_assign(UgetPlugin* plugin, UgetPlugin* node);

int  uget_plugin_ctrl(UgetPlugin* plugin, int code, void* data)
{
	UgetPluginCtrlFunc  ctrl;

	ctrl = plugin->info->ctrl;
	if (ctrl)
		return ctrl(plugin, code, data);
	return FALSE;
}

int  uget_plugin_sync(UgetPlugin* plugin, UgetNode* node)
{
	UgetPluginSyncFunc  sync;

	sync = plugin->info->sync;
	if (sync)
		return sync(plugin, node);
	return FALSE;
}

void  uget_plugin_ref(UgetPlugin* plugin)
{
	plugin->ref_count++;
}

void  uget_plugin_unref(UgetPlugin* plugin)
{
	UgetEvent*  curr;
	UgetEvent*  next;

	if (--plugin->ref_count == 0) {
		uget_plugin_final(plugin);
		ug_mutex_clear(&plugin->mutex);
		// free events
		for (curr = plugin->events;  curr;  curr = next) {
			next = curr->next;
			uget_event_free(curr);
		}
		// free plug-in
		ug_free(plugin);
	}
}

void  uget_plugin_post(UgetPlugin* plugin, UgetEvent* message)
{
	ug_mutex_lock(&plugin->mutex);
	if (plugin->events)
		plugin->events->prev = message;
	message->next = plugin->events;
	plugin->events = message;
	ug_mutex_unlock(&plugin->mutex);
}

UgetEvent* uget_plugin_pop(UgetPlugin* plugin)
{
	UgetEvent*  curr;
	UgetEvent*  next;

	ug_mutex_lock(&plugin->mutex);
	curr = plugin->events;
	plugin->events = NULL;
	ug_mutex_unlock(&plugin->mutex);

	// revert
	while (curr) {
		next = curr->next;
		curr->next = curr->prev;
		curr->prev = next;
		if (next == NULL)
			break;
		curr = next;
	}

	return curr;
}


