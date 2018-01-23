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

#ifndef UGET_PLUGIN_H
#define UGET_PLUGIN_H

#include <stdint.h>
#include <UgUri.h>
#include <UgData.h>
#include <UgThread.h>
#include <UgetNode.h>
#include <UgetEvent.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct  UgetPlugin         UgetPlugin;
typedef struct  UgetPluginInfo     UgetPluginInfo;

typedef enum {
	// input ----------------
	UGET_PLUGIN_CTRL_START,    // UgetNode*
	UGET_PLUGIN_CTRL_STOP,
	UGET_PLUGIN_CTRL_SPEED,    // int*, int[0] = download, int[1] = upload

	// output ---------------
	UGET_PLUGIN_CTRL_ACTIVE,   // int*, TRUE or FALSE

	// unused ---------------
	UGET_PLUGIN_CTRL_NODE_UPDATED,   // unused
	UGET_PLUGIN_CTRL_LIMIT_CHANGED,  // unused
} UgetPluginCtrlCode;

// global
typedef enum {
	UGET_PLUGIN_INIT,            // get/set, parameter = (intptr_t = FALSE or TRUE)
	UGET_PLUGIN_SETTING,         // get/set, parameter = (void* custom_struct)
	UGET_PLUGIN_SPEED_LIMIT,     // get/set, parameter = (int  speed[2])
	UGET_PLUGIN_SPEED,           // get, parameter = (int  speed[2])
	UGET_PLUGIN_ERROR_CODE,      // get, parameter = (int* error_code)
	UGET_PLUGIN_ERROR_STRING,    // get, parameter = (char** error_string)
	UGET_PLUGIN_MATCH,           // get, parameter = (char*  url)

	UGET_PLUGIN_OPTION_DERIVED = 10000,  // for derived plug-ins
} UgetPluginOption;

typedef enum {
	UGET_RESULT_OK = 0,
	UGET_RESULT_ERROR,
	UGET_RESULT_FAILED,
	UGET_RESULT_UNSUPPORT,
} UgetResult;

typedef int        (*UgetPluginSyncFunc)(UgetPlugin* plugin, UgetNode* node);
typedef int        (*UgetPluginCtrlFunc)(UgetPlugin* plugin, int, void* data);
typedef UgetResult (*UgetPluginSetFunc) (int option, void* parameter);
typedef UgetResult (*UgetPluginGetFunc) (int option, void* parameter);

// ----------------------------------------------------------------------------
// UgetPluginInfo

struct UgetPluginInfo
{
	UG_TYPE_INFO_MEMBERS;
//	const char*     name;
//	uintptr_t       size;
//	UgInitFunc      init;
//	UgFinalFunc     final;

	UgAssignFunc    assign;
	UgetPluginCtrlFunc  ctrl;
	UgetPluginSyncFunc  sync;    // UgetTask call this to sync data

	// ----------------------------
	// Global data and functions

	// UgetTask use below data to match UgetPlugin and UgetNode
	const char**    hosts;
	const char**    schemes;
	const char**    file_exts;

	// global set/get function for plug-in special setting.
	UgetPluginSetFunc  set;
	UgetPluginGetFunc  get;
};

// ----------------------------------------------------------------------------
// UgetPlugin: It derived from UgType.
//             It it base class/struct that used by plug-ins.

#define UGET_PLUGIN_MEMBERS  \
	const UgetPluginInfo*  info;  \
	UgetEvent*    events;         \
	UgMutex       mutex;          \
	int           ref_count

struct UgetPlugin
{
	UGET_PLUGIN_MEMBERS;               // It derived from UgType
//	const UgetPluginInfo*  info;
//	UgetEvent*    events;
//	UgMutex       mutex;
//	int           ref_count;
};

// UgetPluginInfo global functions
UgetPlugin*  uget_plugin_new(const UgetPluginInfo* info);
UgetResult   uget_plugin_set(const UgetPluginInfo* info, int option, void* parameter);
UgetResult   uget_plugin_get(const UgetPluginInfo* info, int option, void* parameter);

// return matched count.
// return 3 if URI can be matched hosts, schemes, and file_exts.
int     uget_plugin_match(const UgetPluginInfo* info, UgUri* uuri);

// UgetPlugin functions
//void  uget_plugin_init(UgetPlugin* plugin);
#define uget_plugin_init      ug_type_init

//void  uget_plugin_final(UgetPlugin* plugin);
#define uget_plugin_final     ug_type_final

// return TRUE or FALSE.
int   uget_plugin_ctrl(UgetPlugin* plugin, int code, void* data);

#define uget_plugin_start(plugin, node)   \
		uget_plugin_ctrl(plugin, UGET_PLUGIN_CTRL_START, node)
#define uget_plugin_stop(plugin)          \
		uget_plugin_ctrl(plugin, UGET_PLUGIN_CTRL_STOP, NULL)

#define uget_plugin_ctrl_speed(plugin, dl_ul_int_array)  \
		uget_plugin_ctrl(plugin, UGET_PLUGIN_CTRL_SPEED, dl_ul_int_array)

// unused
// notify plug-in when other data was changed
#define uget_plugin_data_changed(plugin)  \
		uget_plugin_ctrl(plugin, UGET_PLUGIN_CTRL_DATA_CHANGED, NULL)
// unused
// notify plug-in when speed_limit, retry_limit, max_connections...etc was changed
#define uget_plugin_limit_changed(plugin) \
		uget_plugin_ctrl(plugin, UGET_PLUGIN_CTRL_LIMIT_CHANGED, NULL)

// return TRUE  if plug-in running.
// return FALSE if plug-in stopped.
int   uget_plugin_sync(UgetPlugin* plugin, UgetNode* node);

void  uget_plugin_ref(UgetPlugin* plugin);
void  uget_plugin_unref(UgetPlugin* plugin);

void       uget_plugin_post(UgetPlugin* plugin, UgetEvent* message);
UgetEvent* uget_plugin_pop (UgetPlugin* plugin);

#define uget_plugin_lock(plugin)    ug_mutex_lock(&(plugin)->mutex)
#define uget_plugin_unlock(plugin)  ug_mutex_unlock(&(plugin)->mutex)

#ifdef __cplusplus
}
#endif

// ----------------------------------------------------------------------------
// C++11 standard-layout

#ifdef __cplusplus

namespace Uget
{

// This one is for derived use only. No data members here.
// Your derived struct/class must be C++11 standard-layout
struct PluginInfoMethod
{
	inline UgetResult  set(int option, void* parameter)
		{ return uget_plugin_set((UgetPluginInfo*)this, option, parameter); }
	inline UgetResult  get(int option, void* parameter)
		{ return uget_plugin_get((UgetPluginInfo*)this, option, parameter); }

	inline int  match(UgUri* uuri)
		{ return uget_plugin_match((UgetPluginInfo*)this, uuri); }
};

// This one is for directly use only. You can NOT derived it.
struct PluginInfo : Uget::PluginInfoMethod, UgetPluginInfo {};


// This one is for derived use only. No data members here.
// Your derived struct/class must be C++11 standard-layout
struct PluginMethod : Ug::DataMethod
{
	inline void  start(UgetNode* baseNode)
		{ uget_plugin_start((UgetPlugin*) this, baseNode); }

	inline void  stop(void)
		{ uget_plugin_stop((UgetPlugin*) this); }

	inline int   sync(UgetNode* node)
		{ return uget_plugin_sync((UgetPlugin*) this, node); }

	inline void  ref(void)
		{ uget_plugin_ref((UgetPlugin*) this); }
	inline void  unref(void)
		{ uget_plugin_unref((UgetPlugin*) this); }

	inline void  post(UgetEvent* message)
		{ uget_plugin_post((UgetPlugin*) this, message); }
	inline UgetEvent* pop(void)
		{ return uget_plugin_pop((UgetPlugin*) this); }
};

// This one is for directly use only. You can NOT derived it.
struct Plugin : Uget::PluginMethod, UgetPlugin {};

};  // namespace Uget

#endif  // __cplusplus


#endif  // End of UGET_PLUGIN_H

