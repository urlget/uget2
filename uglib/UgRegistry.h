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

#ifndef UG_REGISTRY_H
#define UG_REGISTRY_H

#include <UgGroupData.h>
#include <UgArray.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct  UgPair        UgPair;
typedef struct  UgRegistry    UgRegistry;

// ----------------------------------------------------------------------------
// UgPair

struct UgPair
{
	void*  key;
	void*  data;
};

// ----------------------------------------------------------------------------
// UgRegistry store UgTypeInfo and it's name and search UgTypeInfo by name.
// key-data pair:
// const char*       key;   // UgTypeInfo->name
// const UgTypeInfo* data;  // UgTypeInfo

void	ug_registry_init(UgRegistry* reg);
void	ug_registry_final(UgRegistry* reg);

void    ug_registry_add(UgRegistry* reg, const void* type_info);
void    ug_registry_remove(UgRegistry* reg, const void* type_info);
UgPair* ug_registry_find(UgRegistry* reg, const char* type_name, int* index);

void    ug_registry_sort(UgRegistry* reg);

#ifdef __cplusplus
}
#endif

struct UgRegistry
{
	UG_ARRAY_MEMBERS(UgPair);
//	UgPair**  at;
//	int       length;
//	int       allocated;
//	int       element_size;

	int       sorted;

#ifdef __cplusplus
	inline void  init(void)
		{ ug_registry_init(this); }
	inline void  final(void)
		{ ug_registry_final(this); }

	inline void  add(const void* typeInfo)
		{ ug_registry_add(this, typeInfo); }
	inline void  remove(const void* typeInfo)
		{ ug_registry_remove(this, typeInfo); }
	inline UgPair* find(const char* typeName, int* index = NULL)
		{ return ug_registry_find(this, typeName, index); }
#endif  // __cplusplus
};

// ----------------------------------------------------------------------------
// C++11 standard-layout

#ifdef __cplusplus

namespace Ug
{
// This two is for directly use only. You can NOT derived it.
typedef struct UgPair        Pair;
typedef struct UgRegistry    Registry;
};  // namespace Ug

#endif  // __cplusplus

#endif  // UG_REGISTRY_H

