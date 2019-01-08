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

#ifndef UG_INFO_H
#define UG_INFO_H

#include <UgArray.h>
#include <UgData.h>
#include <UgRegistry.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct  UgInfo      UgInfo;

/* ----------------------------------------------------------------------------
   This UgRegistry used by UgInfo.
   User can only store UgDataInfo in this UgRegistry.
     key  pointer to UgDataInfo.name
     data pointer to UgDataInfo
 */

UgRegistry*  ug_info_get_registry(void);
void         ug_info_set_registry(UgRegistry* registry);

/* ----------------------------------------------------------------------------
   UgInfo - collection of UgDataInfo and UgData
          - It uses UgDataInfo to get/alloc UgData.
     key  pointer to UgDataInfo
     data pointer to UgData
 */

UgInfo* ug_info_new(int allocated_length, int cache_length);
void    ug_info_ref(UgInfo* info);
void    ug_info_unref(UgInfo* info);

void    ug_info_init(UgInfo* info, int allocated_length, int cache_length);
void    ug_info_final(UgInfo* info);

// ug_info_get() and ug_info_realloc() return UgData
// ug_info_set() replace old data by new one. It return old UgData
void*   ug_info_realloc(UgInfo* info, const UgDataInfo* key);
void    ug_info_remove(UgInfo* info, const UgDataInfo* key);
void*   ug_info_get(UgInfo* info, const UgDataInfo* key);
void*   ug_info_set(UgInfo* info, const UgDataInfo* key, void* new_data);
UgPair* ug_info_find(UgInfo* info, const UgDataInfo* key, int* inserted_index);

void    ug_info_assign(UgInfo* info, UgInfo* src, const UgDataInfo* exclude);

// ----------------
// JSON parser/writer that used with UG_ENTRY_CUSTOM.
// if 'registry' is NULL, use default registry.

UgJsonError ug_json_parse_info_ptr(UgJson* json,
                               const char* name, const char* value,
                               void** pinfo, void* registry);
void        ug_json_write_info_ptr(UgJson* json, UgInfo** pinfo);

UgJsonError ug_json_parse_info(UgJson* json,
                               const char* name, const char* value,
                               void* info, void* registry);
void        ug_json_write_info(UgJson* json, UgInfo* info);

/*
	// --- JSON output sample ---
	{
		"progress": {
		},
		"common": {
		},
		"log": {
		}
	}
 */

#ifdef __cplusplus
}
#endif

struct UgInfo
{
	UG_ARRAY_MEMBERS(UgPair);
/*	// ------ UgArray members ------
	UgPair* at;
	int     length;
	int     allocated;
	int     element_size;
 */

	int     cache_length;
	int     ref_count;

#ifdef __cplusplus
	// C++11 standard-layout
	inline void* operator new(size_t size, int allocated_length, int cache_length)
		{ return ug_info_new(allocated_length, cache_length); }
	inline void  operator delete(void* p)
		{ ug_info_unref((UgInfo*)p); }

	inline void ref()
		{ ug_info_ref(this); }
	inline void unref()
		{ ug_info_unref(this); }

	inline void  init(int allocatedLength, int cacheLength)
		{ ug_info_init(this, allocatedLength, cacheLength); }
	inline void  final(void)
		{ ug_info_final(this); }

	inline void  remove(const UgDataInfo* key)
		{ ug_info_remove(this, key); }
	inline Ug::DataMethod* realloc(const UgDataInfo* key)
		{ return (Ug::DataMethod*) ug_info_realloc(this, key); }
	inline Ug::DataMethod* get(const UgDataInfo* key)
		{ return (Ug::DataMethod*) ug_info_get(this, key); }
	inline Ug::DataMethod* set(const UgDataInfo* key, void* new_data)
		{ return (Ug::DataMethod*) ug_info_set(this, key, new_data); }

	inline void  assign(UgInfo* src, const UgDataInfo* exclude)
		{ ug_info_assign(this, src, exclude); }

	// static method
	static inline UgRegistry* getRegistry(void)
		{ return ug_info_get_registry(); }
	static inline void        setRegistry(UgRegistry* registry)
		{ ug_info_set_registry(registry); }
#endif  // __cplusplus
};

// ----------------------------------------------------------------------------
// C++11 standard-layout

#ifdef __cplusplus

namespace Ug
{
// This one is for directly use only. You can NOT derived it.
typedef struct UgInfo    Info;
};  // namespace Ug

#endif  // __cplusplus

#endif  // UG_INFO_H

