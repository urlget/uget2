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

#include <stddef.h>		// NULL
#include <string.h>
#include <UgString.h>
#include <UgArray.h>
#include <UgEntry.h>

#if defined(_MSC_VER)
#define strtoll		_strtoi64
#endif

// ----------------------------------------------------------------------------
// UgArray: for UG_ENTRY_ARRAY

void  ug_array_init(void* arr, int element_size, int allocated_len)
{
	UgArrayChar* array = arr;

	if (allocated_len)
		array->at = ug_malloc(element_size * allocated_len);
	else
		array->at = NULL;
	array->length = 0;
	array->allocated = allocated_len;
	array->element_size = element_size;
}

void  ug_array_clear(void* arr)
{
	UgArrayChar* array = arr;

	ug_free(array->at);
	array->at = NULL;
	array->length = 0;
	array->allocated = 0;
}

void* ug_array_alloc(void* arr, int nElements)
{
	UgArrayChar* array = arr;
	int          len;

	len = array->length + nElements;
	if (array->allocated < len) {
		array->allocated = len * 2;
//		if (array->allocated < 16)
//			array->allocated = 16;
		array->at = ug_realloc(array->at, array->allocated * array->element_size);
	}
	arr = array->at + array->length * array->element_size;
	array->length += nElements;
	return arr;
}

void  ug_array_foreach(void* array, UgForeachFunc func, void* data)
{
	char*  cur;
	char*  end;

	cur = ((UgArrayChar*)array)->at;
	end = cur + ((UgArrayChar*)array)->element_size * ((UgArrayChar*)array)->length;

	for (; cur < end;  cur+= ((UgArrayChar*)array)->element_size)
		func(cur, data);
}

void  ug_array_foreach_ptr(void* array, UgForeachFunc func, void* data)
{
	char*  cur = ((UgArrayChar*)array)->at;
	char*  end = cur + ((UgArrayChar*)array)->element_size * ((UgArrayChar*)array)->length;

	for (; cur < end;  cur+= ((UgArrayChar*)array)->element_size)
		func(*(void**)cur, data);
}

void* ug_array_find_sorted(void* array, const void* key,
                           UgCompareFunc compare, int* inserted_index)
{
	int      low;
	int      cur;
	int      high;
	int      diff;
	void*    cur_key;

	low  = 0;
	cur  = 0;
	high = ((UgArrayChar*)array)->length;
	while (low < high) {
//		cur = low + ((high - low) / 2);
		cur = low + ((high - low) >> 1);
		cur_key = ((UgArrayChar*)array)->at + cur *
		          ((UgArrayChar*)array)->element_size;

		diff = compare(cur_key, key);
		if (diff == 0) {
			if (inserted_index)
				inserted_index[0] = cur;
			return ((UgArrayChar*)array)->at + cur *
			       ((UgArrayChar*)array)->element_size;
		}
		else if (diff > 0)
			high = cur;
		else if (diff < 0)
			low = cur + 1;
	}

	if (inserted_index) {
		if (cur < low)
			cur++;
		inserted_index[0] = cur;
	}
	return NULL;
}

// inline function
#if defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L)
// C99
// inline function in UgArray.h
#else
void  ug_array_erase(void* array, int index, int length)
{
	memmove(ug_array_addr(array, index),
	        ug_array_addr(array, index + length),
			ug_array_count(array, ug_array_length(array) - index - 1));
	((UgArrayChar*)array)->length -= length;
}

void* ug_array_insert(void* array, int index, int length)
{
	char* addr;
	ug_array_alloc(array, length);
	memmove(ug_array_addr(array, index + length),
	        addr = ug_array_addr(array, index),
			ug_array_count(array, ug_array_length(array) - index - 1));
	return (void*)addr;
}

void  ug_array_sort(void* array, UgCompareFunc compare)
{
	qsort( ((UgArrayChar*)array)->at, ((UgArrayChar*)array)->length,
	       ((UgArrayChar*)array)->element_size, compare);
}
#endif  // __STDC_VERSION__

int  ug_array_compare_int(const void *s1, const void *s2)
{
	return *(int*)s1 - *(int*)s2;
}

int  ug_array_compare_string(const void *s1, const void *s2)
{
	return strcmp(*(char**)s1, *(char**)s2);
}

int  ug_array_compare_pointer(const void *s1, const void *s2)
{
	return *(char**)s1 - *(char**)s2;
}

// ----------------------------------------------------------------------------
// UgJsonParseFunc for JSON array elements

UgJsonError ug_json_parse_array_bool(UgJson* json,
                                     const char* name, const char* value,
                                     void* array, void* none)
{
	int  boolValue;

	if (json->type != UG_JSON_TRUE && json->type != UG_JSON_FALSE) {
//		if (json->type >= UG_JSON_OBJECT)
//			ug_json_push(json, ug_json_parse_unknown, NULL, NULL);
		return UG_JSON_ERROR_TYPE_NOT_MATCH;
	}

	if (json->type == UG_JSON_TRUE)
		boolValue = TRUE;
	else
		boolValue = FALSE;

	((UgArrayInt*)array)->element_size = sizeof(int);
	*((int*) ug_array_alloc(array, 1)) = boolValue;
	return UG_JSON_ERROR_NONE;
}

UgJsonError ug_json_parse_array_int(UgJson* json,
                                    const char* name, const char* value,
                                    void* array, void* none)
{
	if (json->type != UG_JSON_NUMBER) {
//		if (json->type >= UG_JSON_OBJECT)
//			ug_json_push(json, ug_json_parse_unknown, NULL, NULL);
		return UG_JSON_ERROR_TYPE_NOT_MATCH;
	}

	((UgArrayInt*)array)->element_size = sizeof(int);
	*((int*) ug_array_alloc(array, 1)) = strtol(value, NULL, 10);
	return UG_JSON_ERROR_NONE;
}

UgJsonError ug_json_parse_array_uint(UgJson* json,
                                     const char* name, const char* value,
                                     void* array, void* none)
{
	if (json->type != UG_JSON_NUMBER) {
//		if (json->type >= UG_JSON_OBJECT)
//			ug_json_push(json, ug_json_parse_unknown, NULL, NULL);
		return UG_JSON_ERROR_TYPE_NOT_MATCH;
	}

	((UgArrayInt*)array)->element_size = sizeof(unsigned int);
	*((unsigned int*) ug_array_alloc(array, 1)) = strtoul(value, NULL, 10);
	return UG_JSON_ERROR_NONE;
}

UgJsonError ug_json_parse_array_int64(UgJson* json,
                                      const char* name, const char* value,
                                      void* array, void* none)
{
	if (json->type != UG_JSON_NUMBER) {
//		if (json->type >= UG_JSON_OBJECT)
//			ug_json_push(json, ug_json_parse_unknown, NULL, NULL);
		return UG_JSON_ERROR_TYPE_NOT_MATCH;
	}

	((UgArrayInt*)array)->element_size = sizeof(int64_t);
	*((int64_t*) ug_array_alloc(array, 1)) = strtoll(value, NULL, 10);
	return UG_JSON_ERROR_NONE;
}

UgJsonError ug_json_parse_array_double(UgJson* json,
                                       const char* name, const char* value,
                                       void* array, void* none)
{
	if (json->type != UG_JSON_NUMBER) {
//		if (json->type >= UG_JSON_OBJECT)
//			ug_json_push(json, ug_json_parse_unknown, NULL, NULL);
		return UG_JSON_ERROR_TYPE_NOT_MATCH;
	}

	((UgArrayInt*)array)->element_size = sizeof(double);
	*((double*) ug_array_alloc(array, 1)) = strtod(value, NULL);
	return UG_JSON_ERROR_NONE;
}

UgJsonError ug_json_parse_array_string(UgJson* json,
                                       const char* name, const char* value,
                                       void* array, void* none)
{
	char* string;

	if (json->type == UG_JSON_STRING)
		string = ug_strdup(value);
	else if (json->type == UG_JSON_NULL)
		string = NULL;
	else {
//		if (json->type >= UG_JSON_OBJECT)
//			ug_json_push(json, ug_json_parse_unknown, NULL, NULL);
		return UG_JSON_ERROR_TYPE_NOT_MATCH;
	}

	((UgArrayInt*)array)->element_size = sizeof(char*);
	*((char**) ug_array_alloc(array, 1)) = string;
	return UG_JSON_ERROR_NONE;
}

// ----------------------------------------------------------------------------
// write JSON array elements

void  ug_json_write_array_bool(UgJson* json, UgArrayInt* array)
{
//	UgArrayInt* array = src;
	int         index;

	for (index = 0;  index < array->length;  index++)
		ug_json_write_bool(json, array->at[index]);
}

void  ug_json_write_array_int(UgJson* json, UgArrayInt* array)
{
//	UgArrayInt* array = src;
	int         index;

	for (index = 0;  index < array->length;  index++)
		ug_json_write_int(json, array->at[index]);
}

void  ug_json_write_array_uint(UgJson* json, UgArrayUint* array)
{
	int  index;

	for (index = 0;  index < array->length;  index++)
		ug_json_write_uint(json, array->at[index]);
}

void  ug_json_write_array_int64(UgJson* json, UgArrayInt64* array)
{
	int  index;

	for (index = 0;  index < array->length;  index++)
		ug_json_write_int64(json, array->at[index]);
}

void  ug_json_write_array_double(UgJson* json, UgArrayDouble* array)
{
	int  index;

	for (index = 0;  index < array->length;  index++)
		ug_json_write_double(json, array->at[index]);
}

void  ug_json_write_array_string(UgJson* json, UgArrayStr* array)
{
	int   index;
	char* string;

	for (index = 0;  index < array->length;  index++) {
		string = array->at[index];
		if (string == NULL)
			ug_json_write_null(json);
		else
			ug_json_write_string(json, string);
	}
}

