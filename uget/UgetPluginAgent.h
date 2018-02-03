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

#ifndef UGET_PLUGIN_AGENT_H
#define UGET_PLUGIN_AGENT_H

#include <UgetPlugin.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct	UgetPluginAgent          UgetPluginAgent;

typedef enum {
	UGET_PLUGIN_AGENT_BEGIN = UGET_PLUGIN_OPTION_DERIVED,    // begin

	UGET_PLUGIN_AGENT_DEFAULT_PLUGIN,   // set parameter = (UgetPluginInfo*)

	UGET_PLUGIN_AGENT_OPTION_DERIVED,
} UgetPluginAgentCode;

// ----------------------------------------------------------------------------
// UgetPluginAgent: It derived from UgetPlugin.

#define UGET_PLUGIN_AGENT_MEMBERS  \
	UGET_PLUGIN_MEMBERS;           \
	UgData*       data;            \
	UgData*       target_data;     \
	UgetPlugin*   target_plugin;   \
	int           limit[2];        \
	uint8_t       limit_changed:1; \
	uint8_t       paused:1;        \
	uint8_t       stopped:1

struct UgetPluginAgent
{
	UGET_PLUGIN_AGENT_MEMBERS;
/*
	// ------ UgetPlugin members ------
	const UgetPluginInfo*  info;
	UgetEvent*    messages;
	UgMutex       mutex;
	int           ref_count;

	// ------ UgetPluginAgent members ------
	// pointer to UgData that store in UgetApp
	UgData*       data;

	// This plug-in use other plug-in to download files,
	// so we need extra UgetPlugin and UgData.
	//
	// plugin->target_data is a copy of plugin->data
	UgData*       target_data;
	// target_plugin use target_data to download
	UgetPlugin*   target_plugin;

	// control flags
	// speed limit control
	// limit[0] = download speed limit
	// limit[1] = upload speed limit
	int           limit[2];
	uint8_t       limit_changed:1;  // speed limit changed by user
	uint8_t       paused:1;         // paused by user
	uint8_t       stopped:1;        // all downloading thread are stopped
 */
};


#ifdef __cplusplus
}
#endif

// global functions -------------------
UgetResult  uget_plugin_agent_global_init (void);
void        uget_plugin_agent_global_ref (void);
void        uget_plugin_agent_global_unref (void);

UgetResult  uget_plugin_agent_global_set (int option, void* parameter);
UgetResult  uget_plugin_agent_global_get (int option, void* parameter);

// instance functions -----------------
void  uget_plugin_agent_init  (UgetPluginAgent* plugin);
void  uget_plugin_agent_final (UgetPluginAgent* plugin);

int   uget_plugin_agent_ctrl (UgetPluginAgent* plugin, int code, void* data);
int   uget_plugin_agent_ctrl_speed (UgetPluginAgent* plugin, int* speed);

// sync functions ---------------------
// sync common data (include speed limit) between plugin->data and plugin->target_data
// parameter common and target can be NULL.
void  uget_plugin_agent_sync_common (UgetPluginAgent* plugin,
                                     UgetCommon* common,
                                     UgetCommon* target);

// sync progress data from plugin->target_data to plugin->data
// parameter progress and target can be NULL.
void  uget_plugin_agent_sync_progress (UgetPluginAgent* plugin,
                                       UgetProgress* progress,
                                       UgetProgress* target);

// thread functions -------------------
int   uget_plugin_agent_start_thread (UgetPluginAgent* plugin,
                                      UgThreadFunc thread_func);

// handle events from target_plugin by default action.
// return remain events
UgetEvent* uget_plugin_agent_handle_message (UgetPluginAgent* plugin, ...);

// ----------------------------------------------------------------------------
// C++11 standard-layout

#ifdef __cplusplus

namespace Uget
{

// This one is for derived use only. No data members here.
// Your derived struct/class must be C++11 standard-layout
struct PluginAgentMethod : Uget::PluginMethod {};

// This one is for directly use only. You can NOT derived it.
struct PluginAgent : Uget::PluginAgentMethod, UgetPluginAgent {};

};  // namespace Uget

#endif  // __cplusplus


#endif  // End of UGET_PLUGIN_AGENT_H

