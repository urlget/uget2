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

#ifndef UG_DATA_H
#define UG_DATA_H

#include <UgArray.h>
#include <UgRegistry.h>
#include <UgGroupData.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct  UgData      UgData;

// ----------------------------------------------------------------------------
// This UgRegistry used by UgData.
// User can only store UgGroupDataInfo in this UgRegistry.
// key-data pair:
// const char*            key;   // UgGroupDataInfo->name
// const UgGroupDataInfo* data;  // UgGroupDataInfo

UgRegistry*  ug_data_get_registry(void);
void         ug_data_set_registry(UgRegistry* registry);

// ----------------------------------------------------------------------------
// UgData - UgGroupDataInfo and it's instance collection
//        - It uses UgGroupDataInfo to get/alloc it's instance.

UgData* ug_data_new(int allocated_length, int cache_length);
void    ug_data_ref(UgData* data);
void    ug_data_unref(UgData* data);

void    ug_data_init(UgData* data, int allocated_length, int cache_length);
void    ug_data_final(UgData* data);

void*   ug_data_realloc(UgData* data, const UgGroupDataInfo* key);
void    ug_data_remove(UgData* data, const UgGroupDataInfo* key);
void*   ug_data_get(UgData* data, const UgGroupDataInfo* key);
UgPair* ug_data_find(UgData* data, const UgGroupDataInfo* key, int* inserted_index);

void    ug_data_assign(UgData* data, UgData* src, const UgGroupDataInfo* exclude);

// ----------------
// JSON parser/writer that used with UG_ENTRY_CUSTOM.
// if (UgRegistry*)registry == NULL, use default registry.

UgJsonError ug_json_parse_data_ptr(UgJson* json,
                               const char* name, const char* value,
                               void** data, void* registry);
void        ug_json_write_data_ptr(UgJson* json, UgData** data);

UgJsonError ug_json_parse_data(UgJson* json,
                               const char* name, const char* value,
                               void* data, void* registry);
void        ug_json_write_data(UgJson* json, UgData* data);

// JSON:
//
// {
//    "progress": {
//    },
//    "common": {
//    },
//    "log": {
//    }
// }
//

#ifdef __cplusplus
}
#endif

struct UgData
{
	UG_ARRAY_MEMBERS(UgPair);
//	UgPair* at;
//	int     length;
//	int     allocated;
//	int     element_size;

	int     cache_length;
	int     ref_count;

#ifdef __cplusplus
	// C++11 standard-layout
	inline UgData(void) {}
	inline UgData(int allocatedLength, int cacheLength)
		{ ug_data_init(this, allocatedLength, cacheLength); }

	static inline UgData* create(int allocatedLength, int cacheLength)
		{ return ug_data_new(allocatedLength, cacheLength); }

	inline void  init(int allocatedLength, int cacheLength)
		{ ug_data_init(this, allocatedLength, cacheLength); }
	inline void  final(void)
		{ ug_data_final(this); }

	inline void  remove(const UgGroupDataInfo* key)
		{ ug_data_remove(this, key); }
	inline Ug::GroupDataMethod* realloc(const UgGroupDataInfo* key)
		{ return (Ug::GroupDataMethod*) ug_data_realloc(this, key); }
	inline Ug::GroupDataMethod* get(const UgGroupDataInfo* key)
		{ return (Ug::GroupDataMethod*) ug_data_get(this, key); }

	// static method
	static inline UgRegistry* getRegistry(void)
		{ return ug_data_get_registry(); }
	static inline void        setRegistry(UgRegistry* registry)
		{ ug_data_set_registry(registry); }
#endif  // __cplusplus
};

// ----------------------------------------------------------------------------
// C++11 standard-layout

#ifdef __cplusplus

namespace Ug
{
// This one is for directly use only. You can NOT derived it.
typedef struct UgData    Data;
};  // namespace Ug

#endif  // __cplusplus

#endif  // UG_DATA_H

