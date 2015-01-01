/*
 *
 *   Copyright (C) 2012-2015 by C.H. Huang
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

#ifndef UGET_PLUGIN_CURL_H
#define UGET_PLUGIN_CURL_H

#include <time.h>
#include <UgUri.h>
#include <UgetData.h>
#include <UgetPlugin.h>
#include <UgetA2cf.h>
#include <curl/curl.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct UgetPluginCurl     UgetPluginCurl;

extern const UgetPluginInfo*  UgetPluginCurlInfo;

// ----------------------------------------------------------------------------
// UgetPluginCurl: an empty plugin. It derived from UgetPlugin.

struct UgetPluginCurl
{
	UGET_PLUGIN_MEMBERS;               // It derived from UgetPlugin
//	const UgetPluginInfo*  info;
//	UgetEvent*    messages;
//	UgMutex       mutex;
//	int           ref_count;

	// UgetNode data
	UgetNode*     node;
	UgetCommon*   common;
	UgetProxy*    proxy;
	UgetHttp*     http;
	UgetFtp*      ftp;

	// run-time info
//	struct curl_slist*  ftp_command;
	struct {
		char*     path;        // folder + filename
		time_t    time;        // date and time
		int64_t   size;        // total size (0 if size unknown)
	} file;

	// aria2 ctrl file
	struct {
		char*     path;
		UgetA2cf  ctrl;
	} aria2;

	// URI and it's mirror
	struct {
		UgList    list;
		UgLink*   link;
	} uri;

	// segment (split download)
	struct {
		UgList    list;
		int64_t   beg;
		uintptr_t n_max;
		uintptr_t n_active;
		uintptr_t n_recycled;
	} seg;

	// progress for uget_plugin_sync()
	time_t        start_time;

	// base.download = base downloaded size
	// base.upload = base uploaded size
	// size.download = downloaded size
	// size.upload = uploaded size
	// speed.download = downloading speed
	// speed.upload = uploading speed
	// limit.download = download speed limit
	// limit.upload = upload speed limit
	struct {
		int64_t   upload;
		int64_t   download;
	} base, size, speed, limit;

	// flags
	uint8_t       limit_by_user:1; // speed limit changed by user
	uint8_t       limit_changed:1; // speed limit changed
	uint8_t       file_renamed:1;  // has file path?
	uint8_t       aria2_created:1; // has aria2 ctrl file?
	uint8_t       synced:1;
	uint8_t       stopped:1;       // ctrl
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
struct PluginCurlMethod : Uget::PluginMethod
{
};

// This one is for directly use only. You can NOT derived it.
struct PluginCurl : Uget::PluginEmptyMethod, UgetPluginCurl {};

};  // namespace Uget

#endif  // __cplusplus


#endif  // End of UGET_PLUGIN_CURL_H

