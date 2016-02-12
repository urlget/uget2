/*
 *
 *   Copyright (C) 2012-2016 by C.H. Huang
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

#ifndef UGET_PLUGIN_MEDIA_H
#define UGET_PLUGIN_MEDIA_H

#include <UgetPlugin.h>
#include <UgetMedia.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct	UgetPluginMedia          UgetPluginMedia;

typedef enum {
	UGET_PLUGIN_MEDIA_BEGIN = UGET_PLUGIN_OPTION_DERIVED,    // begin

	UGET_PLUGIN_MEDIA_DEFAULT_PLUGIN,   // set parameter = (UgetPluginInfo*)
	UGET_PLUGIN_MEDIA_MATCH_MODE,       // set parameter = (UgetMediaMatchMode)
	UGET_PLUGIN_MEDIA_QUALITY,          // set parameter = (UgetMediaQuality)
	UGET_PLUGIN_MEDIA_TYPE,             // set parameter = (UgetMediaType)
} UgetPluginMediaCode;

extern const UgetPluginInfo*  UgetPluginMediaInfo;

// ----------------------------------------------------------------------------
// UgetPluginMedia: It derived from UgetPlugin.

struct UgetPluginMedia
{
	UGET_PLUGIN_MEMBERS;               // It derived from UgetPlugin
//	const UgetPluginInfo*  info;
//	UgetEvent*    messages;
//	UgMutex       mutex;
//	int           ref_count;

	// pointer to UgetNode that store in UgetApp
	UgetNode*     node;

	// This plug-in use other plug-in to download media files,
	// so we need extra UgetPlugin and UgetNode.
	struct
	{
		// plugin->ex.node is a copy of plugin->node
		UgetNode*     node;
		// copy child nodes from ex.node
		UgetNode*     children;
		// ex.plugin use ex.node to download
		UgetPlugin*   plugin;
	} ex;

	// copy of UgetNode data, they store in ex.node
	UgetProxy*    proxy;
	UgetCommon*   common;
	UgetProgress* progress;

	// plug-in use title to rename file
	char*         title;

	// use these data to recount progress if plug-in download multiple files.
	int           elapsed;
	int           retry_count;
	int           item_index;      // downloading nth files
	int           item_total;      // number of files to download

	// speed limit control
	// limit[0] = download speed limit
	// limit[1] = upload speed limit
	int           limit[2];
	uint8_t       limit_changed:1;
	uint8_t       paused:1;        // paused by user or program
	uint8_t       stopped:1;       // all of downloading thread are stopped
	uint8_t       synced:1;        // used by plugin_sync()
	uint8_t       named:1;         // change node name by title
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
struct PluginMediaMethod : Uget::PluginMethod
{
};

// This one is for directly use only. You can NOT derived it.
struct PluginMedia : Uget::PluginMediaMethod, UgetPluginMedia {};

};  // namespace Uget

#endif  // __cplusplus


#endif  // End of UGET_PLUGIN_MEDIA_H

