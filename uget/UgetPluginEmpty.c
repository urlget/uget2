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

#include <stdio.h>
#include <UgetData.h>
#include <UgetEvent.h>
#include <UgetPluginEmpty.h>

// ----------------------------------------------------------------------------
// UgetPluginInfo (derived from UgDataInfo)

static void plugin_init  (UgetPluginEmpty* plugin);
static void plugin_final (UgetPluginEmpty* plugin);
static int  plugin_ctrl  (UgetPluginEmpty* plugin, int code, void* data);
static int  plugin_sync  (UgetPluginEmpty* plugin);
static UgetResult  global_set (int code, void* parameter);
static UgetResult  global_get (int code, void* parameter);

static const char* schemes[] = {"http", "ftp", NULL};
static const char* types[]   = {"torrent", NULL};

static const UgetPluginInfo UgetPluginEmptyInfoStatic =
{
	"empty",
	sizeof (UgetPluginEmpty),
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
const UgetPluginInfo* UgetPluginEmptyInfo = &UgetPluginEmptyInfoStatic;

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
		//
		// your global initialized code
		// if initialization failed,
		//     return UGET_RESULT_ERROR;
		//
		global.initialized = TRUE;
		puts ("UgetPluginEmpty global initialize");
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
		global.initialized = 0;
		//
		// your global finalized code
		//
		puts ("UgetPluginEmpty global finalize");
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

	case UGET_PLUGIN_ERROR_CODE:
		if (parameter)
			*(int*)parameter = 0;
		break;

	default:
		return UGET_RESULT_UNSUPPORT;
	}

	return UGET_RESULT_OK;
}

// ----------------------------------------------------------------------------
// control functions

static void plugin_init (UgetPluginEmpty* plugin)
{
	if (global.initialized == FALSE)
		global_init ();
	else
		global_ref ();
	//
	// your initialized code.
	//
}

static void plugin_final (UgetPluginEmpty* plugin)
{
	//
	// your finalized code.
	//
	if (plugin->node)
		uget_node_unref (plugin->node);
	global_unref ();
}

// ----------------------------------------------------------------------------
// plugin_ctrl

static int  plugin_ctrl_speed (UgetPluginEmpty* plugin, int* speed);
static int  plugin_start (UgetPluginEmpty* plugin, UgetNode* node);
static void plugin_stop (UgetPluginEmpty* plugin);

static int  plugin_ctrl (UgetPluginEmpty* plugin, int code, void* data)
{
	switch (code) {
	case UGET_PLUGIN_CTRL_START:
		if (plugin->node == NULL)
			return plugin_start (plugin, data);
		break;

	case UGET_PLUGIN_CTRL_STOP:
		plugin_stop (plugin);
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

static int  plugin_ctrl_speed (UgetPluginEmpty* plugin, int* speed)
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
		common = ug_info_realloc (&plugin->node->info, UgetCommonInfo);
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
	// notify plugin that speed limit has been changed
	plugin->limit_changed = TRUE;
	return TRUE;
}

// ----------------------------------------------------------------------------
// plugin_sync

static int  plugin_sync (UgetPluginEmpty* plugin)
{
	// if plugin was stopped, return FALSE.
	return FALSE;
}

// ----------------------------------------------------------------------------

static int  plugin_start (UgetPluginEmpty* plugin, UgetNode* node)
{
	UgetCommon*  common;

	common = ug_info_get (&node->info, UgetCommonInfo);
	if (common == NULL || common->uri == NULL)
		return FALSE;

	plugin->node = node;
	uget_node_ref (node);
	return TRUE;
}

static void plugin_stop (UgetPluginEmpty* plugin)
{
}
