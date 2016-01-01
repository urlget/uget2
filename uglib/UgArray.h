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

#ifndef UG_ARRAY_H
#define UG_ARRAY_H

// uintptr_t is an unsigned int that is guaranteed to be the same size as a pointer.
#include <stdint.h>     // uintptr_t, int64_t
#include <stdlib.h>     // qsort(), malloc(), free()
#include <UgJson.h>
#include <UgEntry.h>

#ifdef __cplusplus
extern "C" {
#endif

// ----------------------------------------------------------------------------
// UgArray is a template C array. It used by UgEntry with UG_ENTRY_ARRAY.

#define UG_ARRAY_MEMBERS(Type)  \
	Type*  at;                  \
	int    length;              \
	int    allocated;           \
	int    element_size

// implement C++ template by C macro
#define	UG_ARRAY(Type)          struct { UG_ARRAY_MEMBERS (Type); }

typedef UG_ARRAY(char)          UgArrayChar;
typedef UG_ARRAY(char*)         UgArrayStr;
typedef UG_ARRAY(void*)         UgArrayPtr;
typedef UG_ARRAY(int)           UgArrayInt;
typedef UG_ARRAY(unsigned int)  UgArrayUint;
typedef UG_ARRAY(int64_t)       UgArrayInt64;
typedef UG_ARRAY(double)        UgArrayDouble;

void	ug_array_init (void* array, int element_size, int allocated_len);
void	ug_array_clear (void* array);
void*	ug_array_alloc (void* array, int nElements);
void    ug_array_foreach (void* array, UgForeachFunc func, void* data);
void    ug_array_foreach_ptr (void* array, UgForeachFunc func, void* data);

#define ug_array_foreach_str    ug_array_foreach_ptr

#define	ug_array_append(array, values, len)	\
		memcpy (ug_array_alloc ((array), len), values, ((UgArrayChar*)(array))->element_size * len)

#define	ug_array_terminate0(array)	\
		memset (ug_array_alloc ((array), 1), 0, ((UgArrayChar*)(array))->element_size)

#define	ug_array_end0(array)	\
		*((char*) ug_array_alloc ((array), 1)) = 0

// Quick sort and Binary search:
// int compareFunc(const void *s1, const void *s2);
#define	ug_array_sort(array, compareFunc)	\
		qsort ((array)->at, (array)->length, (array)->element_size, compareFunc)
#define	ug_array_bsearch(array, key, compareFunc)	\
		bsearch(key, (array)->at, (array)->length, (array)->element_size, compareFunc)

// ----------------------------------------------------------------------------
// UgJsonParseFunc for JSON array elements
UgJsonError ug_json_parse_array_bool (UgJson* json,
                                      const char* name, const char* value,
                                      void* array, void* none);
UgJsonError ug_json_parse_array_int (UgJson* json,
                                     const char* name, const char* value,
                                     void* array, void* none);
UgJsonError ug_json_parse_array_uint (UgJson* json,
                                      const char* name, const char* value,
                                      void* array, void* none);
UgJsonError ug_json_parse_array_int64 (UgJson* json,
                                       const char* name, const char* value,
                                       void* array, void* none);
UgJsonError ug_json_parse_array_double (UgJson* json,
                                        const char* name, const char* value,
                                        void* array, void* none);
UgJsonError ug_json_parse_array_string (UgJson* json,
                                        const char* name, const char* value,
                                        void* array, void* none);

// ----------------------------------------------------------------------------
// write JSON array elements
void	ug_json_write_array_bool (UgJson* json, UgArrayInt* array);
void	ug_json_write_array_int (UgJson* json, UgArrayInt* array);
void	ug_json_write_array_uint (UgJson* json, UgArrayUint* array);
void	ug_json_write_array_int64 (UgJson* json, UgArrayInt64* array);
void	ug_json_write_array_double (UgJson* json, UgArrayDouble* array);
void	ug_json_write_array_string (UgJson* json, UgArrayStr* array);


#ifdef __cplusplus
}
#endif

// ----------------------------------------------------------------------------
// C++11 standard-layout

#ifdef __cplusplus

namespace Ug
{
// This one is for derived use only. No data members here.
// Your derived struct/class must be C++11 standard-layout
template<class Type> struct ArrayMethod
{
	inline void  init (int allocated_len)
		{ ug_array_init (this, sizeof (Type), allocated_len); }
	inline void  final (void)
		{ ug_array_clear (this); }

	inline Type* alloc (int nElements)
		{ return (Type*) ug_array_alloc (this, nElements); }
};

// This one is for directly use only. You can NOT derived it.
template<class Type> struct Array : ArrayMethod<Type>
{
	UG_ARRAY_MEMBERS (Type);
//	Type*  at;
//	int    length;
//	int    allocated;
//	int    element_size;

	inline Array<Type> (int allocated_len = 0)
		{ ug_array_init (this, sizeof (Type), allocated_len); }
	inline ~Array<Type> (void)
		{ ug_array_clear (this); }
};

};  // namespace Ug

#endif  // __cplusplus


#endif  // UG_ARRAY_H

