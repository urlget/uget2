/*
 *
 *   Copyright (C) 2012-2015 by C.H. Huang
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

#ifndef UG_VALUE_H
#define UG_VALUE_H

#include <stdint.h>     // int64_t
#include <stdlib.h>     // qsort(), malloc(), free()
#include <UgJson.h>
#include <UgDefine.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct UgValue          UgValue;
typedef struct UgValue          UgMember;
typedef struct UgValueArray     UgValueArray;
typedef struct UgValueArray     UgValueObject;
typedef struct UgValueCustom    UgValueCustom;
typedef union  UgValueC         UgValueC;

typedef void (*UgValueForeachFunc) (UgValue* value, void* data);

// If value is NULL pointer and type is UG_VALUE_STRING,
// you will get JSON output 'null'.
typedef enum
{
	UG_VALUE_NONE = 0,  // JSON will not output this value.
	UG_VALUE_BOOL,      // JSON true or false, C int
	UG_VALUE_INT,       // JSON number, C int
	UG_VALUE_UINT,      // JSON number, C unsigned int
	UG_VALUE_INT64,     // JSON number, C int64_t, long long
	UG_VALUE_UINT64,    // JSON number, C uint64_t, unsigned long long
	UG_VALUE_DOUBLE,    // JSON number, C double
	UG_VALUE_STRING,    // JSON string, C string

	UG_VALUE_OBJECT,    // JSON object, C UgValueObject (UgValueArray)
	UG_VALUE_ARRAY,     // JSON array,  C UgValueArray

#ifdef HAVE_UG_VALUE_CUSTOM
	UG_VALUE_CUSTOM,
#endif
} UgValueType;

// ----------------------------------------------------------------------------
// UgValueC: C language value for UgValueType

union  UgValueC
{
	int            boolean;
	int            integer;
	unsigned int   uinteger;
	int64_t        integer64;
	uint64_t       uinteger64;
	char*          string;
	double         fraction;

	UgValueArray*  array;
	UgValueObject* object;

#ifdef HAVE_UG_VALUE_CUSTOM
	UgValueCustom* custom;
#endif
//	void*          pointer;
};

// ----------------------------------------------------------------------------
// UgValue

struct UgValue
{
	char*        name;
	UgValueType  type;
	UgValueC     c;
};

void  ug_value_init (UgValue* value);
void  ug_value_clear (UgValue* value);

void  ug_value_init_array  (UgValue* value, int nElements);
void  ug_value_init_object (UgValue* value, int nMembers);

// You can use this if UgValue.type == UG_VALUE_ARRAY or UG_VALUE_OBJECT.
UgValue* ug_value_alloc (UgValue* uvalue, int nValue);
#if 0
UgValue* ug_value_alloc_front (UgValue* uvalue, int nValue);
#endif

// void ug_value_sort (UgValue* value, UgCompareFunc compare);
#define ug_value_sort(varray, compareFunc)   \
		qsort ((varray)->c.array->at, (varray)->c.array->length,    \
		       sizeof (UgValue), compareFunc)

// UgValue*  ug_value_find (UgValue* value, UgValue* key, UgCompareFunc func);
#define ug_value_find(varray, key, compareFunc)   \
		bsearch (key, (varray)->c.array->at, (varray)->c.array->length,    \
		         sizeof (UgValue), compareFunc)

// void ug_value_sort_name (UgValue* value)
#define ug_value_sort_name(vobj)   \
		qsort ((vobj)->c.object->at, (vobj)->c.object->length,   \
		       sizeof (UgValue), ug_value_compare_name)

UgValue*  ug_value_find_name (UgValue* value, const char* name);

// recursive functions
void  ug_value_sort_recursive (UgValue* value, UgCompareFunc compare);
//void  ug_value_sort_name_recursive (UgValue* value);
#define ug_value_sort_name_recursive(vobj)   \
		ug_value_sort_recursive (vobj, ug_value_compare_name)

int   ug_value_compare_name (const void* value1, const void* value2);
int   ug_value_compare_string (const void* value1, const void* value2);

// If you set static string in all UgValue.name or UgValue.c.string,
// run ug_value_foreach() before you call ug_value_clear().
// e.g.
// ug_value_foreach (value, ug_value_set_name_string, NULL);
void  ug_value_set_name (UgValue* value, void* data);
void  ug_value_set_string (UgValue* value, void* data);
void  ug_value_set_name_string (UgValue* value, void* data);
void  ug_value_foreach (UgValue* value, UgValueForeachFunc func, void* data);

int   ug_value_get_int (UgValue* uvalue);
int64_t  ug_value_get_int64 (UgValue* uvalue);
uint64_t ug_value_get_uint64 (UgValue* uvalue);

UgJsonError ug_json_parse_value (UgJson* json,
                                 const char* name, const char* value,
                                 void* uvalue, void* none);
void        ug_json_write_value (UgJson* json, UgValue* value);

// ----------------------------------------------------------------------------
// UgValueArray

struct UgValueArray
{
	int       length;
	int       allocated;
	UgValue   at[1];
};

UgValueArray*  ug_value_array_new (int preAllocate);
void           ug_value_array_free (UgValueArray* varray);

// ----------------------------------------------------------------------------
// UgValueObject = UgValueArray

#define ug_value_object_new     ug_value_array_new
#define ug_value_object_free    ug_value_array_free

// ----------------------------------------------------------------------------
// UgValueCustom: used by UG_VALUE_CUSTOM. It can't parse value in JSON array.

#ifdef HAVE_UG_VALUE_CUSTOM

struct UgValueCustom
{
	UgJsonParseFunc  parse;
	UgJsonWriteFunc  write;
	void*            data;
	void*            data2;

	int              free_this;
	struct {
		UgNotifyFunc func;
		void*        data;
	} destroy;
};

UgValueCustom*  ug_value_custom_new (void);
void            ug_value_custom_free (UgValueCustom* vcustom);

#endif  // HAVE_UG_VALUE_CUSTOM

#ifdef __cplusplus
}
#endif

#endif  // UG_VALUE_H

