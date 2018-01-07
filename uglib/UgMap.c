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
#include <UgMap.h>

// ----------------------------------------------------------------------------
// UgRegistry for UgMap

static UgRegistry*  ug_map_registry;

UgRegistry*  ug_map_get_registry(void)
{
	return ug_map_registry;
}

void  ug_map_set_registry(UgRegistry* registry)
{
	ug_map_registry = registry;
}

// ----------------------------------------------------------------------------
// UgMap

void  ug_map_init(UgMap* map, int allocated_len, int cache_len)
{
	int     index;

	ug_array_init(map, sizeof(UgPair), allocated_len + cache_len);
	map->length = cache_len;
	map->cache_len = cache_len;
	for (index = 0;  index < map->allocated;  index++) {
		map->at[index].key  = NULL;
		map->at[index].data = NULL;
	}
}

void  ug_map_final(UgMap* map)
{
	UgPair* cur;
	UgPair* end;

	for (cur = map->at, end = cur + map->length;  cur < end;  cur++) {
		if (cur->key == NULL)
			continue;
		if (cur->data)
			ug_data_free(cur->data);
	}

	ug_array_clear(map);
}

UgPair* ug_map_find(UgMap* map, const UgDataInfo* key, int* inserted_index)
{
	UgPair*   low;
	UgPair*   cur;
	UgPair*   high;
	const UgDataInfo* cur_key;

	for (cur = map->at, low = cur + map->cache_len;  cur < low;  cur++) {
		if (cur->key == key)
			return cur;
	}

	high = map->at + map->length;
	while (low < high) {
//		cur = low + ((high - low) / 2);
		cur = low + ((high - low) >> 1);
		cur_key = cur->key;

		if (cur_key == key)
			return cur;
		else if (cur_key > key)
			high = cur;
		else if (cur_key < key)
			low = cur + 1;
	}

	if (inserted_index) {
		if (cur < low)
			cur++;
		*inserted_index = cur - map->at;
	}
	return NULL;
}

void*  ug_map_realloc(UgMap* map, const UgDataInfo* key)
{
	UgPair* cur;
	int     index;

	cur = ug_map_find(map, key, &index);
	if (cur == NULL) {
		ug_array_alloc(map, 1);
		memmove(map->at + index + 1, map->at + index,
				sizeof(UgPair) * (map->length - index - 1));
		cur = map->at + index;
		cur->key = (void*) key;
		cur->data = ug_data_new(key);
	}
	else if (cur->data == NULL)
		cur->data = ug_data_new(key);
	return cur->data;
}

void  ug_map_remove (UgMap* map, const UgDataInfo* key)
{
	UgPair* cur;

	cur = ug_map_find(map, key, NULL);
	if (cur && cur->data) {
		ug_data_free(cur->data);
		cur->data = NULL;
	}
}

void* ug_map_get(UgMap* map, const UgDataInfo* key)
{
	UgPair* cur;

	cur = ug_map_find(map, key, NULL);
	if (cur == NULL)
		return NULL;
	return cur->data;
}

void  ug_map_assign(UgMap* map, UgMap* src, const UgDataInfo* exclude_key)
{
	int      index;
	UgPair*  pair;
	UgData*  data;

	for (index = 0;  index < src->length;  index++) {
		pair = src->at + index;
		if (pair->key == NULL || pair->data == NULL)
			continue;
		if (pair->key == exclude_key)
			continue;
		data = ug_map_realloc(map, pair->key);
		ug_data_assign(data, pair->data);
	}
}

// UgJsonParseFunc for key/data pairs in UgMap
static UgJsonError ug_json_parse_info_reg(UgJson* json,
                                const char* name, const char* value,
                                void* map, void* infoRegistry)
{
	UgRegistry* registry;
	UgPair*     cur;

	if (infoRegistry)
		registry = infoRegistry;
	else if (ug_map_registry)
		registry = ug_map_registry;
	else
		registry = NULL;

	if (registry) {
		if (registry->sorted == FALSE)
			ug_registry_sort(registry);
		cur = ug_registry_find(registry, name, NULL);

		if (cur) {
			ug_json_push(json, ug_json_parse_entry,
					ug_map_realloc(map, cur->data),
					(void*)((UgDataInfo*)cur->data)->entry);
			return UG_JSON_ERROR_NONE;
		}
	}

	if (json->type >= UG_JSON_OBJECT)
		ug_json_push(json, ug_json_parse_unknown, NULL, NULL);
	return UG_JSON_ERROR_CUSTOM;
}

// ----------------

// JSON parser for UgMap.
UgJsonError ug_json_parse_info(UgJson* json,
                               const char* name, const char* value,
                               void* map, void* none)
{
	// UgMap's type is UG_JSON_OBJECT
	if (json->type != UG_JSON_OBJECT) {
//		if (json->type == UG_JSON_ARRAY)
//			ug_json_push (json, ug_json_parse_unknown, NULL, NULL);
		return UG_JSON_ERROR_TYPE_NOT_MATCH;
	}

	ug_json_push(json, ug_json_parse_info_reg, map, NULL);
	return UG_JSON_ERROR_NONE;
}

// JSON writer for UgMap.
void  ug_json_write_info(UgJson* json, const UgMap* map)
{
	UgPair* cur;
	UgPair* end;

	ug_json_write_object_head(json);
	for (cur = map->at, end = cur + map->length;  cur < end;  cur++) {
		if (cur->data == NULL || ((UgDataInfo*)cur->key)->entry == NULL)
			continue;

		ug_json_write_string(json, ((UgDataInfo*)cur->key)->name);
		ug_json_write_object_head(json);
		ug_json_write_entry(json, cur->data,
				((UgDataInfo*)cur->key)->entry);
		ug_json_write_object_tail(json);
	}
	ug_json_write_object_tail(json);
}

