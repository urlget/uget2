/*
 *
 *   Copyright (C) 2012-2014 by C.H. Huang
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

#include <limits.h>
#include <string.h>
#include <stdlib.h>
#include <UgString.h>
#include <UgValue.h>

#if defined(_MSC_VER)
#define strtoll     _strtoi64
#define strtoull    _strtoui64
#endif

// ----------------------------------------------------------------------------
// UgValue

void  ug_value_init (UgValue* value)
{
	value->type = UG_VALUE_NONE;
	value->name = NULL;
}

void  ug_value_clear (UgValue* value)
{
	switch (value->type) {
	case UG_VALUE_STRING:
		ug_free (value->c.string);
		break;

	case UG_VALUE_OBJECT:
	case UG_VALUE_ARRAY:
		ug_value_array_free (value->c.array);
		break;

#ifdef HAVE_UG_VALUE_CUSTOM
	case UG_VALUE_CUSTOM:
		ug_value_custom_free (value->c.custom);
#endif

	default:
		break;
	}
	ug_free (value->name);
	value->name = NULL;
	value->type = UG_VALUE_NONE;
}

void  ug_value_init_array (UgValue* value, int nElements)
{
	value->type = UG_VALUE_ARRAY;
	value->name = NULL;
	value->c.array = ug_value_array_new (nElements);
}

void  ug_value_init_object (UgValue* value, int nMembers)
{
	value->type = UG_VALUE_OBJECT;
	value->name = NULL;
	value->c.array = ug_value_object_new (nMembers);
}

UgValue* ug_value_alloc (UgValue* uvalue, int nValue)
{
	int       len;
	UgValue*  value;
	UgValueArray*  varray;

//	if (uvalue->type != UG_VALUE_ARRAY)
//		return NULL;

	varray = uvalue->c.array;
	len = varray->length + nValue;
	if (len > varray->allocated) {
		len *= 2;
//		if (len < 16)
//			len = 16;
		varray->allocated = len + 1;
		uvalue->c.array = ug_realloc (varray,
				sizeof (UgValueArray) + sizeof (UgValue[1]) * len);
		varray = uvalue->c.array;
	}
	len = varray->length;
	varray->length += nValue;

	value = varray->at + len;
	uvalue = value;
	while (nValue--) {
		value->name = NULL;
		value->type = UG_VALUE_NONE;
//		value->c.pointer = NULL;
		value++;
	}
	return uvalue;
}

#if 0
UgValue* ug_value_alloc_front (UgValue* uvalue, int nValue)
{
	int       len;
	UgValue*  value;
	UgValueArray*  varray;

//	if (uvalue->type != UG_VALUE_ARRAY)
//		return NULL;

	varray = uvalue->c.array;
	len = varray->length + nValue;
	if (len > varray->allocated) {
		len *= 2;
//		if (len < 16)
//			len = 16;
		varray->allocated = len + 1;
		uvalue->c.array = ug_realloc (varray,
				sizeof (UgValueArray) + sizeof (UgValue[1]) * len);
		varray = uvalue->c.array;
	}

	memmove (varray->at + nValue, varray->at,
			sizeof (UgValue) * varray->length);
	varray->length += nValue;

	value = varray->at;
	while (nValue--) {
		value->name = NULL;
		value->type = UG_VALUE_NONE;
//		value->c.pointer = NULL;
		value++;
	}
	return varray->at;
}
#endif

UgValue* ug_value_find_name (UgValue* value, const char* name)
{
	UgValue  temp;

	if (value->type == UG_VALUE_OBJECT) {
		temp.name = (char*)name;
		return bsearch (&temp, value->c.object->at, value->c.object->length,
				sizeof (UgValue), ug_value_compare_name);
	}
	return NULL;
}

void  ug_value_sort_recursive (UgValue* value, UgCompareFunc compare)
{
	UgValue* end;

	if (value->type == UG_VALUE_OBJECT) {
		ug_value_sort (value, compare);
		end = value->c.object->at + value->c.object->length;
		for (value = value->c.object->at;  value < end;  value++)
			ug_value_sort_recursive (value, compare);
	}
}

int   ug_value_compare_name (const void* value1, const void* value2)
{
	return strcmp (((UgValue*)value1)->name, ((UgValue*)value2)->name);
}

int   ug_value_compare_string (const void* value1, const void* value2)
{
	return strcmp (((UgValue*)value1)->c.string, ((UgValue*)value2)->c.string);
}

void  ug_value_set_name (UgValue* value, void* data)
{
	value->name = data;
}

void  ug_value_set_string (UgValue* value, void* data)
{
	if (value->type == UG_VALUE_STRING)
		value->c.string = data;
}

void  ug_value_set_name_string (UgValue* value, void* data)
{
	value->name = data;
	if (value->type == UG_VALUE_STRING)
		value->c.string = data;
}

void  ug_value_foreach (UgValue* value, UgValueForeachFunc func, void* data)
{
	UgValue*  end;

	func (value, data);
	if (value->type >= UG_VALUE_OBJECT) {
		end = value->c.object->at + value->c.object->length;
		for (value = value->c.object->at;  value < end;  value++)
			ug_value_foreach (value, func, data);
	}
}

int   ug_value_get_int (UgValue* uvalue)
{
	switch (uvalue->type) {
	default:
	case UG_VALUE_INT:
		return uvalue->c.integer;

	case UG_VALUE_UINT:
		return (int) uvalue->c.uinteger;

	case UG_VALUE_INT64:
		return (int) uvalue->c.integer64;

	case UG_VALUE_UINT64:
		return (int) uvalue->c.uinteger64;

	case UG_VALUE_DOUBLE:
		return (int) uvalue->c.fraction;

	case UG_VALUE_STRING:
		return strtol (uvalue->c.string, NULL, 10);
	}
}

int64_t ug_value_get_int64 (UgValue* uvalue)
{
	switch (uvalue->type) {
	case UG_VALUE_INT:
		return (int64_t) uvalue->c.integer;

	case UG_VALUE_UINT:
		return (int64_t) uvalue->c.uinteger;

	default:
	case UG_VALUE_INT64:
		return uvalue->c.integer64;

	case UG_VALUE_UINT64:
		return (int64_t) uvalue->c.uinteger64;

	case UG_VALUE_DOUBLE:
		return (int64_t) uvalue->c.fraction;

	case UG_VALUE_STRING:
		return strtoll (uvalue->c.string, NULL, 10);
	}
}

uint64_t ug_value_get_uint64 (UgValue* uvalue)
{
	switch (uvalue->type) {
	case UG_VALUE_INT:
		return (uint64_t) uvalue->c.integer;

	case UG_VALUE_UINT:
		return (uint64_t) uvalue->c.uinteger;

	case UG_VALUE_INT64:
		return (uint64_t) uvalue->c.integer64;

	default:
	case UG_VALUE_UINT64:
		return uvalue->c.uinteger64;

	case UG_VALUE_DOUBLE:
		return (uint64_t) uvalue->c.fraction;

	case UG_VALUE_STRING:
		return strtoull (uvalue->c.string, NULL, 10);
	}
}

static UgJsonError ug_json_parse_value_array (UgJson* json,
                                       const char* name, const char* value,
                                       void* uvalue, void* none);

UgJsonError ug_json_parse_value (UgJson* json,
                                 const char* name, const char* value,
                                 void* data, void* none)
{
	UgValue*  uvalue;

	uvalue = data;
	if (json->scope == UG_JSON_OBJECT)
		uvalue->name = ug_strdup (name);
	else
		uvalue->name = NULL;

#ifdef HAVE_UG_VALUE_CUSTOM
	if (uvalue->type == UG_VALUE_CUSTOM) {
		return uvalue->c.custom->parse (json, name, value,
				uvalue->c.custom->data, uvalue->c.custom->data2);
	}
#endif

	switch (json->type) {
	case UG_JSON_NULL:
		uvalue->type = UG_VALUE_STRING;
		uvalue->c.string = NULL;
		break;

	case UG_JSON_TRUE:
		uvalue->type = UG_VALUE_BOOL;
		uvalue->c.boolean = 1;
		break;

	case UG_JSON_FALSE:
		uvalue->type = UG_VALUE_BOOL;
		uvalue->c.boolean = 0;
		break;

	case UG_JSON_NUMBER:
		if (strchr (value, '.')) {
			uvalue->type = UG_VALUE_DOUBLE;
			uvalue->c.fraction = strtod (value, NULL);
		}
		else if (value[0] == '-') {
			uvalue->c.integer64 = (int64_t) strtoll (value, NULL, 10);
			if (uvalue->c.integer64 >= INT_MIN) {
				uvalue->c.integer = (int) uvalue->c.integer64;
				uvalue->type = UG_VALUE_INT;
			}
			else
				uvalue->type = UG_VALUE_INT64;
		}
		else {
			uvalue->c.uinteger64 = (uint64_t) strtoull (value, NULL, 10);
			if (uvalue->c.uinteger64 <= INT_MAX) {
				uvalue->c.integer = (int) uvalue->c.uinteger64;
				uvalue->type = UG_VALUE_INT;
			}
			else if (uvalue->c.uinteger64 <= UINT_MAX) {
				uvalue->c.uinteger = (unsigned int) uvalue->c.uinteger64;
				uvalue->type = UG_VALUE_UINT;
			}
			else if (uvalue->c.uinteger64 <= INT64_MAX) {
				uvalue->c.integer64 = (int64_t) uvalue->c.uinteger64;
				uvalue->type = UG_VALUE_INT64;
			}
			else
				uvalue->type = UG_VALUE_UINT64;
		}
		break;

	case UG_JSON_STRING:
		uvalue->type = UG_VALUE_STRING;
		uvalue->c.string = ug_strdup (value);
		break;

	case UG_JSON_OBJECT:
		uvalue->type = UG_VALUE_OBJECT;
		uvalue->c.object = ug_value_object_new (8);
		ug_json_push (json, ug_json_parse_value_array, uvalue, NULL);
		break;

	case UG_JSON_ARRAY:
		uvalue->type = UG_VALUE_ARRAY;
		uvalue->c.array = ug_value_array_new (8);
		ug_json_push (json, ug_json_parse_value_array, uvalue, NULL);
		break;

	default:
		uvalue->type = UG_VALUE_NONE;
		break;
	};

	return UG_JSON_ERROR_NONE;
}

static void  ug_json_write_value_array (UgJson* json, UgValueArray* varray);

void  ug_json_write_value (UgJson* json, UgValue* uvalue)
{
	if (uvalue->type == UG_VALUE_NONE)
		return;
	if (uvalue->name)
		ug_json_write_string (json, uvalue->name);

	switch (uvalue->type) {
	case UG_VALUE_BOOL:
		ug_json_write_bool (json, uvalue->c.boolean);
		break;

	case UG_VALUE_INT:
		ug_json_write_int (json, uvalue->c.integer);
		break;

	case UG_VALUE_UINT:
		ug_json_write_uint (json, uvalue->c.uinteger);
		break;

	case UG_VALUE_INT64:
		ug_json_write_int64 (json, uvalue->c.integer64);
		break;

	case UG_VALUE_UINT64:
		ug_json_write_uint64 (json, uvalue->c.uinteger64);
		break;

	case UG_VALUE_DOUBLE:
		ug_json_write_double (json, uvalue->c.fraction);
		break;

	case UG_VALUE_STRING:
		if (uvalue->c.string)
			ug_json_write_string (json, uvalue->c.string);
		else
			ug_json_write_null (json);
		break;

	case UG_VALUE_OBJECT:
		ug_json_write_object_head (json);
		ug_json_write_value_array (json, uvalue->c.array);
		ug_json_write_object_tail (json);
		break;

	case UG_VALUE_ARRAY:
		ug_json_write_array_head (json);
		ug_json_write_value_array (json, uvalue->c.array);
		ug_json_write_array_tail (json);
		break;

#ifdef HAVE_UG_VALUE_CUSTOM
	case UG_VALUE_CUSTOM:
		uvalue->c.custom->write (json, uvalue->c.custom->data,
				uvalue->c.custom->data2);
		break;
#endif

	default:
		break;
	};
}

// ----------------------------------------------------------------------------
// UgValueArray

UgValueArray*  ug_value_array_new (int preAllocate)
{
	UgValueArray*  varray;

	varray = ug_malloc (sizeof (UgValueArray) + sizeof (UgValue) * preAllocate);
	varray->allocated = preAllocate + 1;
	varray->length = 0;
	return varray;
}

void  ug_value_array_free (UgValueArray* varray)
{
	UgValue*  cur;
	UgValue*  end;

	cur = varray->at;
	end = varray->at + varray->length;
	for (;  cur < end;  cur++)
		ug_value_clear (cur);
	ug_free (varray);
}

static UgJsonError ug_json_parse_value_array (UgJson* json,
                                       const char* name, const char* value,
                                       void* uvalue, void* none)
{
	UgValue*  uvalue1;

	uvalue1 = ug_value_alloc (uvalue, 1);
	return ug_json_parse_value (json, name, value, uvalue1, none);
}

static void  ug_json_write_value_array (UgJson* json, UgValueArray* varray)
{
	UgValue*  cur;
	UgValue*  end;

	cur = varray->at;
	end = varray->at + varray->length;
	for (; cur < end;  cur++)
		ug_json_write_value (json, cur);
}

// ----------------------------------------------------------------------------
// UgValueCustom

#ifdef HAVE_UG_VALUE_CUSTOM

UgValueCustom*  ug_value_custom_new (void)
{
	UgValueCustom*   custom;

	custom = ug_malloc0 (sizeof (UgValueCustom));
	custom->free_this = TRUE;
	return custom;
}

void  ug_value_custom_free (UgValueCustom* custom)
{
	if (custom->destroy.func)
		custom->destroy.func (custom->destroy.data);

	if (custom->free_this)
		ug_free (custom);
}

#endif  // HAVE_UG_VALUE_CUSTOM

