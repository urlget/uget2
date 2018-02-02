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

#ifndef UGET_PLUGIN_EMPTY_H
#define UGET_PLUGIN_EMPTY_H

#include <UgetPlugin.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct	UgetPluginEmpty          UgetPluginEmpty;

typedef enum {
	UGET_PLUGIN_EMPTY_BEGIN = UGET_PLUGIN_OPTION_DERIVED,    // begin
	// your setting ID...
} UgetPluginEmptyCode;

extern const UgetPluginInfo*  UgetPluginEmptyInfo;

// ----------------------------------------------------------------------------
// UgetPluginEmpty: an empty plug-in. It derived from UgetPlugin.

struct UgetPluginEmpty
{
	UGET_PLUGIN_MEMBERS;               // It derived from UgetPlugin
//	const UgetPluginInfo*  info;
//	UgetEvent*    messages;
//	UgMutex       mutex;
//	int           ref_count;

	UgInfo*       node_info;

	// speed limit control
	// limit[0] = download speed limit
	// limit[1] = upload speed limit
	int           limit[2];
	uint8_t       limit_changed;
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
struct PluginEmptyMethod : Uget::PluginMethod {};

// This one is for directly use only. You can NOT derived it.
struct PluginEmpty : Uget::PluginEmptyMethod, UgetPluginEmpty {};

};  // namespace Uget

#endif  // __cplusplus


#endif  // End of UGET_PLUGIN_EMPTY_H

