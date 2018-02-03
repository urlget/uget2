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

#include <stdlib.h>
#include <string.h>
#include <UgData.h>

// ----------------------------------------------------------------------------
// UgRegistry for UgData

static UgRegistry*  ug_data_registry;

UgRegistry*  ug_data_get_registry(void)
{
	return ug_data_registry;
}

void  ug_data_set_registry(UgRegistry* registry)
{
	ug_data_registry = registry;
}

// ----------------------------------------------------------------------------
// UgData

UgData* ug_data_new(int allocated_length, int cache_length)
{
	UgData*  data;

#ifdef HAVE_GLIB
	data = g_slice_alloc(sizeof(UgData));
#else
	data = ug_malloc(sizeof(UgData));
#endif // HAVE_GLIB
	ug_data_init(data, allocated_length, cache_length);
	return data;
}

void    ug_data_ref(UgData* data)
{
	data->ref_count++;
}

void    ug_data_unref(UgData* data)
{
	if (--data->ref_count == 0) {
		ug_data_final(data);
#ifdef HAVE_GLIB
		g_slice_free1(sizeof(UgData), data);
#else
		ug_free(data);
#endif // HAVE_GLIB
	}
}

void  ug_data_init(UgData* data, int allocated_length, int cache_length)
{
	int     index;

	ug_array_init(data, sizeof(UgPair), allocated_length + cache_length);
	data->length       = cache_length;
	data->cache_length = cache_length;
	data->ref_count    = 1;

	// clear cache
	for (index = 0;  index < data->length;  index++) {
		data->at[index].key  = NULL;
		data->at[index].data = NULL;
	}
}

void  ug_data_final(UgData* data)
{
	UgPair* cur;
	UgPair* end;

	for (cur = data->at, end = cur + data->length;  cur < end;  cur++) {
		if (cur->key == NULL)
			continue;
		if (cur->data)
			ug_group_data_free(cur->data);
	}

	ug_array_clear(data);
}

UgPair* ug_data_find(UgData* data, const UgGroupDataInfo* key, int* index)
{
	UgPair*   end;
	UgPair*   cur;

	// find key in cache space
	for (cur = data->at, end = cur + data->cache_length;  cur < end;  cur++) {
		if (cur->key == key)
			return cur;
	}

	// find key without cache space
	data->at     += data->cache_length;
    data->length -= data->cache_length;
    cur = ug_array_find_sorted(data, &key, ug_array_compare_pointer, index);
	data->at     -= data->cache_length;
    data->length += data->cache_length;
	if (index)
		index[0] += data->cache_length;
	return cur;
}

void*  ug_data_realloc(UgData* data, const UgGroupDataInfo* key)
{
	UgPair* cur;
	int     index;

	cur = ug_data_find(data, key, &index);
	if (cur == NULL) {
		cur = ug_array_insert(data, index, 1);
		cur->key = (void*) key;
		cur->data = ug_group_data_new(key);
	}
	else if (cur->data == NULL)
		cur->data = ug_group_data_new(key);
	return cur->data;
}

void  ug_data_remove(UgData* data, const UgGroupDataInfo* key)
{
	UgPair* cur;

	cur = ug_data_find(data, key, NULL);
	if (cur && cur->data) {
		ug_group_data_free(cur->data);
		cur->data = NULL;
	}
}

void* ug_data_get(UgData* data, const UgGroupDataInfo* key)
{
	UgPair* cur;

	cur = ug_data_find(data, key, NULL);
	if (cur == NULL)
		return NULL;
	return cur->data;
}

void  ug_data_assign(UgData* data, UgData* src, const UgGroupDataInfo* exclude_info)
{
	int           index;
	UgPair*       pair;
	UgGroupData*  group_data;

	for (index = 0;  index < src->length;  index++) {
		pair = src->at + index;
		if (pair->key == NULL || pair->data == NULL)
			continue;
		if (pair->key == exclude_info)
			continue;
		group_data = ug_data_realloc(data, pair->key);
		ug_group_data_assign(group_data, pair->data);
	}
}

// UgJsonParseFunc for key/data pairs in UgData
static UgJsonError ug_json_parse_data_reg(UgJson* json,
                                const char* name, const char* value,
                                void* data, void* dataRegistry)
{
	UgRegistry* registry;
	UgPair*     cur;

	if (dataRegistry)
		registry = dataRegistry;
	else if (ug_data_registry)
		registry = ug_data_registry;
	else
		registry = NULL;

	if (registry) {
		if (registry->sorted == FALSE)
			ug_registry_sort(registry);
		cur = ug_registry_find(registry, name, NULL);

		if (cur) {
			ug_json_push(json, ug_json_parse_entry,
					ug_data_realloc(data, cur->data),
					(void*)((UgGroupDataInfo*)cur->data)->entry);
			return UG_JSON_ERROR_NONE;
		}
	}

	if (json->type >= UG_JSON_OBJECT)
		ug_json_push(json, ug_json_parse_unknown, NULL, NULL);
	return UG_JSON_ERROR_CUSTOM;
}

// ----------------

// JSON parser for UgData.
UgJsonError ug_json_parse_data_ptr(UgJson* json,
                               const char* name, const char* value,
                               void** data, void* none)
{
	// UgData's type is UG_JSON_OBJECT
	if (json->type != UG_JSON_OBJECT) {
//		if (json->type == UG_JSON_ARRAY)
//			ug_json_push(json, ug_json_parse_unknown, NULL, NULL);
		return UG_JSON_ERROR_TYPE_NOT_MATCH;
	}

	ug_json_push(json, ug_json_parse_data_reg, *data, NULL);
	return UG_JSON_ERROR_NONE;
}

// JSON writer for UgData.
void  ug_json_write_data_ptr(UgJson* json, UgData** pdata)
{
	UgData* data = *pdata;
	UgPair* cur;
	UgPair* end;

	ug_json_write_object_head(json);
	for (cur = data->at, end = cur + data->length;  cur < end;  cur++) {
		if (cur->data == NULL || ((UgGroupDataInfo*)cur->key)->entry == NULL)
			continue;

		ug_json_write_string(json, ((UgGroupDataInfo*)cur->key)->name);
		ug_json_write_object_head(json);
		ug_json_write_entry(json, cur->data,
				((UgGroupDataInfo*)cur->key)->entry);
		ug_json_write_object_tail(json);
	}
	ug_json_write_object_tail(json);
}

