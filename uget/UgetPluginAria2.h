/*
 *
 *   Copyright (C) 2011-2015 by C.H. Huang
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

typedef struct Aria2File                 Aria2File;
typedef struct UgetPluginAria2           UgetPluginAria2;
typedef struct UgetPluginAria2Setting    UgetPluginAria2Setting;

extern  const  UgetPluginInfo*           UgetPluginAria2Info;

typedef enum {
	UGET_PLUGIN_ARIA2_OPTION = UGET_PLUGIN_OPTION_DERIVED,
	UGET_PLUGIN_ARIA2_URI,       // set parameter = (char* )
//	UGET_PLUGIN_ARIA2_LOCAL,     // set parameter = (int* )
	UGET_PLUGIN_ARIA2_PATH,      // set parameter = (char* )
	UGET_PLUGIN_ARIA2_ARGUMENT,  // set parameter = (char* )
	UGET_PLUGIN_ARIA2_TOKEN,     // set parameter = (char* )
	UGET_PLUGIN_ARIA2_LAUNCH,    // get/set parameter = (int* )
	UGET_PLUGIN_ARIA2_SHUTDOWN,  // set parameter = (int* )
	UGET_PLUGIN_ARIA2_SHUTDOWN_NOW,  // set parameter = (int* )
} UgetPluginAria2Code;

typedef enum {
	UGET_PLUGIN_ARIA2_ERROR_NONE,
	UGET_PLUGIN_ARIA2_ERROR_RPC,
	UGET_PLUGIN_ARIA2_ERROR_LAUNCH,
} UgetPluginAria2Error;

// ----------------------------------------------------------------------------
// UgetPluginAria2File

struct Aria2File
{
	char*    path;
	int64_t  completedLength;
	int64_t  length;
};

typedef UG_ARRAY(Aria2File)    Aria2FileArray;

// ----------------------------------------------------------------------------
// UgetPluginAria2: It derived from UgetPlugin.

struct UgetPluginAria2
{
	UGET_PLUGIN_MEMBERS;               // It derived from UgetPlugin
//	const UgetPluginInfo*  info;
//	UgetEvent*    messages;
//	UgMutex       mutex;
//	int           ref_count;

	UgetNode*     node;
	// aria2.addUri, aria2.addTorrent, aria2.addMetalink
	UgJsonrpcObject*  start_request;
	time_t            start_time;
	UgUri             uri_part;
	int               uri_type;
	unsigned int      retry_delay;
	// all gids and it's files
	UgArrayStr        gids;
	Aria2FileArray    files;
	int               files_per_gid;
	int               files_per_gid_prev;

	// speed limit control
	// limit[0] = download speed limit
	// limit[1] = upload speed limit
	int               limit[2];
	uint8_t           limit_changed:1;
	uint8_t           limit_by_user:1; // speed limit changed by user

	// flags
	uint8_t    synced:1;
	uint8_t    paused:1;    // paused by user or program
	uint8_t    stopped:1;   // download is stopped
	uint8_t    restart:1;   // for retry
	uint8_t    node_named:1;

	// aria2.tellStatus
	int        status;
	int        errorCode;
	int64_t    totalLength;
	int64_t    completedLength;
	int64_t    uploadLength;
	int        downloadSpeed;
	int        uploadSpeed;
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

// This one is for derived use only. No data members here.
// Your derived struct/class must be C++11 standard-layout
struct PluginAria2Method : Uget::PluginMethod
{
};

// This one is for directly use only. You can NOT derived it.
struct PluginAria2 : Uget::PluginAria2Method, UgetPluginAria2 {};

};  // namespace Uget

#endif  // __cplusplus


#endif  // End of UGET_PLUGIN_ARIA2_H

