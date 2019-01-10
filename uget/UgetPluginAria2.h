/*
 *
 *   Copyright (C) 2011-2019 by C.H. Huang
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

#ifndef UGET_PLUGIN_ARIA2_H
#define UGET_PLUGIN_ARIA2_H

#include <time.h>
#include <UgJsonrpc.h>
#include <UgetPlugin.h>
#include <UgetData.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct UgetPluginAria2           UgetPluginAria2;
typedef struct UgetPluginAria2Setting    UgetPluginAria2Setting;

extern  const  UgetPluginInfo*           UgetPluginAria2Info;

typedef enum {
	UGET_PLUGIN_ARIA2_GLOBAL = UGET_PLUGIN_GLOBAL_DERIVED,
	UGET_PLUGIN_ARIA2_GLOBAL_URI,       // set parameter = (char* )
//	UGET_PLUGIN_ARIA2_GLOBAL_LOCAL,     // set parameter = (intptr_t)
	UGET_PLUGIN_ARIA2_GLOBAL_PATH,      // set parameter = (char* )
	UGET_PLUGIN_ARIA2_GLOBAL_ARGUMENT,  // set parameter = (char* )
	UGET_PLUGIN_ARIA2_GLOBAL_TOKEN,     // set parameter = (char* )
	UGET_PLUGIN_ARIA2_GLOBAL_LAUNCH,    // get/set parameter = (intptr_t)
	UGET_PLUGIN_ARIA2_GLOBAL_SHUTDOWN,  // set parameter = (intptr_t)
	UGET_PLUGIN_ARIA2_GLOBAL_SHUTDOWN_NOW,  // set parameter = (intptr_t)
} UgetPluginAria2GlobalCode;

typedef enum {
	UGET_PLUGIN_ARIA2_ERROR_NONE,
	UGET_PLUGIN_ARIA2_ERROR_RPC,
	UGET_PLUGIN_ARIA2_ERROR_LAUNCH,
} UgetPluginAria2Error;

/* ----------------------------------------------------------------------------
   UgetPluginAria2: aria2 plug-in that derived from UgetPlugin.

   UgType
   |
   `--- UgetPlugin
        |
        `--- UgetPluginAria2
 */

struct UgetPluginAria2
{
	UGET_PLUGIN_MEMBERS;
/*	// ------ UgType members ------
	const UgetPluginInfo*  info;

	// ------ UgetPlugin members ------
	UgetEvent*    messages;
	UgMutex       mutex;
	int           ref_count;
 */

	// aria2.addUri, aria2.addTorrent, aria2.addMetalink
	UgJsonrpcObject*  start_request;
	time_t            start_time;
	UgUri             uri_part;
	int               uri_type;
	unsigned int      retry_delay;
	// all gids and it's files
	UgArrayStr        gids;
	UgetFiles*        files;
	int               files_per_gid;

	// aria2.tellStatus
	int        status;
	int        errorCode;
	int64_t    totalLength;
	int64_t    completedLength;
	int64_t    uploadLength;
	int        downloadSpeed;
	int        uploadSpeed;

	// speed limit control
	// limit[0] = download speed limit
	// limit[1] = upload speed limit
	int        limit[2];
	int        limit_upper[2];
	uint8_t    limit_changed:1;  // speed limit changed by user or program

	// flags
	uint8_t    synced:1;
	uint8_t    paused:1;    // paused by user or program
	uint8_t    stopped:1;   // download is stopped
	uint8_t    restart:1;   // for retry
	uint8_t    named:1;
};

// ----------------------------------------------------------------------------

struct UgetPluginAria2Setting
{
	uint8_t    launch;
	uint8_t    shutdown;

	// millisecond interval between aria2.tellStatus()
	int        polling_interval;

	char*      uri;
	char*      path;
	char*      arguments;
};


#ifdef __cplusplus
}
#endif

// ----------------------------------------------------------------------------
// C++11 standard-layout

#ifdef __cplusplus

namespace Uget
{

const PluginInfo* const PluginAria2Info = (PluginInfo*) UgetPluginAria2Info;

// This one is for derived use only. No data members here.
// Your derived struct/class must be C++11 standard-layout
struct PluginAria2Interface : PluginInterface {};

// This one is for directly use only. You can NOT derived it.
struct PluginAria2 : PluginAria2Interface, UgetPluginAria2
{
	inline void* operator new(size_t size)
		{ return uget_plugin_new(PluginAria2Info); }
};

};  // namespace Uget

#endif  // __cplusplus


#endif  // End of UGET_PLUGIN_ARIA2_H

