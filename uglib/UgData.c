/*
 *
 *   Copyright (C) 2005-2017 by C.H. Huang
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef HAVE_GLIB
#include <glib.h>    // g_slice_xxx
#endif

#include <stdlib.h>
#include <UgData.h>
#include <UgJson-custom.h>

// UgData* ug_data_new (const UgDataInfo* iface)
void*	ug_data_new (const UgDataInfo* info)
{
	UgInitFunc  init;
	UgData*     data;

#ifdef HAVE_GLIB
	data = g_slice_alloc0 (info->size);
#else
	data = ug_malloc0 (info->size);
#endif // HAVE_GLIB

	data->info = info;
	init = info->init;
	if (init)
		init (data);
	return data;
}

// void	ug_data_free (UgData* data)
void	ug_data_free (void* data)
{
	UgFinalFunc  final;

	final = ((UgData*)data)->info->final;
	if (final)
		final (data);

#ifdef HAVE_GLIB
	g_slice_free1 (((UgData*)data)->info->size, data);
#else
	ug_free (data);
#endif // HAVE_GLIB
}

void  ug_data_init (void* data)
{
	UgInitFunc init;

	init = ((UgData*)data)->info->init;
	if (init)
		init (data);
}

void  ug_data_final (void* data)
{
	UgFinalFunc final;

	final = ((UgData*)data)->info->final;
	if (final)
		final (data);
}

// UgData* ug_data_copy (UgData* data)
void* ug_data_copy (void* data)
{
	const UgDataInfo* info;
	UgAssignFunc  assign;
	void*         newone;

	if (data) {
		info = ((UgData*)data)->info;
		assign = info->assign;
		if (assign) {
#ifdef HAVE_GLIB
			newone = g_slice_alloc0 (info->size);
#else
			newone = ug_malloc0 (info->size);
#endif
			((UgData*)newone)->info = info;
			assign (newone, data);
			return newone;
		}
	}
	return NULL;
}

//void	ug_data_assign (UgData* dest, UgData* src)
int   ug_data_assign (void* data, void* src)
{
	UgAssignFunc assign;

	if (data) {
		assign = ((UgData*)data)->info->assign;
		if (assign)
			return assign (data, src);
	}

	return FALSE;
}

// UgJsonParseFunc for UgData, used by UgEntry with UG_ENTRY_CUSTOM
UgJsonError ug_json_parse_data (UgJson* json,
                                const char* name, const char* value,
                                void* data, void* none)
{
	UgData*  ugdata = (UgData*)data;

	// UgData's type is UG_JSON_OBJECT
	if (json->type != UG_JSON_OBJECT) {
//		if (json->type == UG_JSON_ARRAY)
//			ug_json_push (json, ug_json_parse_unknown, NULL, NULL);
		return UG_JSON_ERROR_TYPE_NOT_MATCH;
	}

	if (ugdata->info->entry == NULL)
		ug_json_push (json, ug_json_parse_unknown, NULL, NULL);
	else
		ug_json_push (json, ug_json_parse_entry, data, (void*)ugdata->info->entry);
	return UG_JSON_ERROR_NONE;
}

// write UgData, used by UgEntry with UG_ENTRY_CUSTOM
void        ug_json_write_data (UgJson* json, const UgData* data)
{
	ug_json_write_object_head (json);
	if (data->info->entry)
		ug_json_write_entry (json, (void*) data, data->info->entry);
	ug_json_write_object_tail (json);
}

