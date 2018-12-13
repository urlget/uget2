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
#include <UgInfo.h>
#include <UgThread.h>
#include <UgetEvent.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct  UgetPlugin         UgetPlugin;
typedef struct  UgetPluginInfo     UgetPluginInfo;

typedef enum {
	// input ----------------
	UGET_PLUGIN_CTRL_START,
	UGET_PLUGIN_CTRL_STOP,
	UGET_PLUGIN_CTRL_SPEED,    // int*, int[0] = download, int[1] = upload

	// state ----------------
	UGET_PLUGIN_SET_STATE,     // int*, TRUE or FALSE  (unused)
	UGET_PLUGIN_GET_STATE,     // int*, TRUE or FALSE
} UgetPluginCtrlCode;

// global
typedef enum {
	UGET_PLUGIN_GLOBAL_INIT,            // get/set, parameter = (intptr_t = FALSE or TRUE)
	UGET_PLUGIN_GLOBAL_SETTING,         // get/set, parameter = (void* custom_struct)
	UGET_PLUGIN_GLOBAL_SPEED_LIMIT,     // get/set, parameter = (int  speed[2])
	UGET_PLUGIN_GLOBAL_SPEED,           // get, parameter = (int  speed[2])
	UGET_PLUGIN_GLOBAL_ERROR_CODE,      // get, parameter = (int* error_code)
	UGET_PLUGIN_GLOBAL_ERROR_STRING,    // get, parameter = (char** error_string)
	UGET_PLUGIN_GLOBAL_MATCH,           // get, parameter = (char*  url)

	UGET_PLUGIN_GLOBAL_DERIVED = 10000,  // for derived plug-ins
} UgetPluginGlobalOption;

typedef enum {
	UGET_RESULT_OK = 0,
	UGET_RESULT_ERROR,
	UGET_RESULT_FAILED,
	UGET_RESULT_UNSUPPORT,
} UgetResult;

// accept/sync return TRUE or FALSE
typedef int        (*UgetPluginSyncFunc)(UgetPlugin* plugin, UgInfo* info);
// start/stop...etc return TRUE or FALSE.
typedef int        (*UgetPluginCtrlFunc)(UgetPlugin* plugin, int, void* data);
// global_set/global_get
typedef UgetResult (*UgetPluginGlobalFunc)(int option, void* parameter);

/* ----------------------------------------------------------------------------
   UgetPluginInfo

   UgTypeInfo
   |
   `-- UgetPluginInfo
 */

struct UgetPluginInfo
{
	UG_TYPE_INFO_MEMBERS;
/*	// ------ UgTypeInfo members ------
	const char*     name;
	uintptr_t       size;
	UgInitFunc      init;
	UgFinalFunc     final;
 */

	UgetPluginSyncFunc  accept;  // pass data to plug-in.
	UgetPluginSyncFunc  sync;    // call this to sync/exchange data.
	UgetPluginCtrlFunc  ctrl;

	// ----------------------------
	// Global data and functions

	// global data is used for matching UgetPlugin and UgInfo
	const char**    hosts;
	const char**    schemes;
	const char**    file_exts;

	// global set/get function for plug-in special setting.
	UgetPluginGlobalFunc  global_set;
	UgetPluginGlobalFunc  global_get;
};

UgetResult  uget_plugin_global_set(const UgetPluginInfo* info,
                                   int option, void* parameter);
UgetResult  uget_plugin_global_get(const UgetPluginInfo* info,
                                   int option, void* parameter);

// return matched count.
// return 3 if URI can be matched hosts, schemes, and file_exts.
int     uget_plugin_match(const UgetPluginInfo* info, UgUri* uuri);

/* ----------------------------------------------------------------------------
   UgetPlugin: It is base class/struct that used by plug-ins.
               It derived from UgType.

   UgType
   |
   `-- UgetPlugin

                 accept(info)                 accept(info)
  ,----------. -------------> ,-----------. -------------> ,-----------.
  |          |                |           |                |           |
  | User App |                | plug-in 1 |                | plug-in 2 |
  |          |                |           |                |           |
  `----------' <------------> `-----------' <------------> `-----------'
               sync(info)                   sync(info)

	// create and start plug-in
	plugin = uget_plugin_new(UgetPluginCurlInfo);
	uget_plugin_accept(plugin, info);
	if (uget_plugin_start(plugin) == FALSE) {
		uget_plugin_unref(plugin);
		return;
	}

	// Running loop sample 1: use uget_plugin_sync()
	while (uget_plugin_sync(plugin, info)) {
		// sleep();
		// do something here
	}

	// Running loop sample 2: use uget_plugin_get_state()
	do {
		// sleep();
		// do something here
		// If you don't want to exchange data (e.g. progress) with plug-in,
		// you do not need to call uget_plugin_sync() at last.
		uget_plugin_sync(plugin, info);
	} while (uget_plugin_get_state(plugin));
 */

#define UGET_PLUGIN_MEMBERS       \
	const UgetPluginInfo*  info;  \
	UgetEvent*    events;         \
	UgMutex       mutex;          \
	int           ref_count

struct UgetPlugin
{
	UGET_PLUGIN_MEMBERS;
/*	// ------ UgType members ------
	const UgetPluginInfo*  info;

	// ------ UgetPlugin members ------
	UgetEvent*    events;
	UgMutex       mutex;
	int           ref_count;
 */
};

// UgetPlugin functions
UgetPlugin* uget_plugin_new(const UgetPluginInfo* info);

void    uget_plugin_ref(UgetPlugin* plugin);
void    uget_plugin_unref(UgetPlugin* plugin);

// return TRUE  if UgInfo was accepted by plug-in.
// return FALSE if UgInfo lacks necessary data.
int     uget_plugin_accept(UgetPlugin* plugin, UgInfo* info);

// return TRUE  if plug-in is running or some data need to exchange/sync.
// return FALSE if plug-in was stopped and no data need to exchange/sync.
int     uget_plugin_sync(UgetPlugin* plugin, UgInfo* info);

// return TRUE or FALSE.
int     uget_plugin_ctrl(UgetPlugin* plugin, int code, void* data);

#define uget_plugin_start(plugin)  \
		uget_plugin_ctrl(plugin, UGET_PLUGIN_CTRL_START, NULL)
#define uget_plugin_stop(plugin)   \
		uget_plugin_ctrl(plugin, UGET_PLUGIN_CTRL_STOP, NULL)

#define uget_plugin_ctrl_speed(plugin, dl_ul_int_array)  \
		uget_plugin_ctrl(plugin, UGET_PLUGIN_CTRL_SPEED, dl_ul_int_array)

// return > 0 if plug-in is running.
int     uget_plugin_get_state(UgetPlugin* plugin);

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
	inline UgetResult  globalSet(int option, void* parameter)
		{ return uget_plugin_global_set((UgetPluginInfo*)this, option, parameter); }
	inline UgetResult  globalGet(int option, void* parameter)
		{ return uget_plugin_global_get((UgetPluginInfo*)this, option, parameter); }

	inline int  match(UgUri* uuri)
		{ return uget_plugin_match((UgetPluginInfo*)this, uuri); }
};

// This one is for directly use only. You can NOT derived it.
struct PluginInfo : Uget::PluginInfoMethod, UgetPluginInfo {};


// This one is for derived use only. No data members here.
// Your derived struct/class must be C++11 standard-layout
struct PluginMethod
{
	// static method
	static inline UgetPlugin* create(UgetPluginInfo* pinfo)
		{ return uget_plugin_new(pinfo); }

	inline void  ref(void)
		{ uget_plugin_ref((UgetPlugin*) this); }
	inline void  unref(void)
		{ uget_plugin_unref((UgetPlugin*) this); }

	inline int   accept(UgInfo* info)
		{ return uget_plugin_accept((UgetPlugin*) this, info); }

	inline int   sync(UgInfo* info)
		{ return uget_plugin_sync((UgetPlugin*) this, info); }

	inline void  start(UgInfo* info)
		{ uget_plugin_start((UgetPlugin*) this); }

	inline void  stop(void)
		{ uget_plugin_stop((UgetPlugin*) this); }

	inline int   getState(void)
		{ return uget_plugin_get_state((UgetPlugin*) this); }

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

