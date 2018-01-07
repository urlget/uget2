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

#ifndef UG_MAP_H
#define UG_MAP_H

#include <UgArray.h>
#include <UgData.h>
#include <UgRegistry.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct  UgMap      UgMap;

// ----------------------------------------------------------------------------
// UgRegistry for UgMap

UgRegistry*  ug_map_get_registry(void);
void         ug_map_set_registry(UgRegistry* registry);

// ----------------------------------------------------------------------------
// UgMap

void    ug_map_init(UgMap* map, int allocated_len, int cache_len);
void    ug_map_final(UgMap* map);

void*   ug_map_realloc(UgMap* map, const UgDataInfo* key);
void    ug_map_remove(UgMap* map, const UgDataInfo* key);
void*   ug_map_get(UgMap* map, const UgDataInfo* key);
UgPair* ug_map_find(UgMap* map, const UgDataInfo* key, int* inserted_index);

void    ug_map_assign(UgMap* map, UgMap* src, const UgDataInfo* exclude);

// ----------------
// JSON parser that used with UG_ENTRY_CUSTOM.
// if (UgRegistry*)registry == NULL, use default registry.
UgJsonError ug_json_parse_info(UgJson* json,
                               const char* name, const char* value,
                               void* map, void* registry);
// JSON writer that used with UG_ENTRY_CUSTOM.
void        ug_json_write_info(UgJson* json, const UgMap* map);

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

struct UgMap
{
	UG_ARRAY_MEMBERS(UgPair);
//	UgPair* at;
//	int     length;
//	int     allocated;
//	int     element_size;

	int     cache_len;

#ifdef __cplusplus
	// C++11 standard-layout
	inline UgMap(void) {}
	inline UgMap(int allocated_len, int cache_len)
		{ ug_map_init(this, allocated_len, cache_len); }

	inline void  init(int allocated_len, int cache_len)
		{ ug_map_init(this, allocated_len, cache_len); }
	inline void  final(void)
		{ ug_map_final(this); }

	inline void  remove(const UgDataInfo* key)
		{ ug_map_remove(this, key); }
	inline Ug::DataMethod* realloc(const UgDataInfo* key)
		{ return (Ug::DataMethod*)ug_map_realloc(this, key); }
	inline Ug::DataMethod* get (const UgDataInfo* key)
		{ return (Ug::DataMethod*)ug_map_get(this, key); }

	// static method
	static inline UgRegistry* getRegistry(void)
		{ return ug_map_get_registry(); }
	static inline void    setRegistry(UgRegistry* registry)
		{ ug_map_set_registry(registry); }
#endif  // __cplusplus
};

// ----------------------------------------------------------------------------
// C++11 standard-layout

#ifdef __cplusplus

namespace Ug
{
// This one is for directly use only. You can NOT derived it.
typedef struct UgMap    Map;
};  // namespace Ug

#endif  // __cplusplus

#endif  // UG_MAP_H

