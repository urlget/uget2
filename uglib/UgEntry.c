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

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <UgString.h>
#include <UgEntry.h>
#include <UgValue.h>

#if defined(_MSC_VER)
#define strtoll     _strtoi64
#define strtoull    _strtoui64
#endif

UgJsonError ug_json_parse_entry (UgJson* json,
                                 const char* name, const char* value,
                                 void* dest, void* entry0)
{
	const UgEntry* entry;
	UgJsonError    error = UG_JSON_ERROR_NONE;

	for (entry = entry0;  entry->type;  entry++) {
		if (entry->name && strcmp (entry->name, name) != 0)
			continue;
		// get destination
		dest = ((char*) dest) + entry->offset;

		// handle data by UgEntryType
		switch (entry->type) {
		case UG_ENTRY_BOOL:
			if (json->type == UG_JSON_TRUE)
				*(int*) dest = 1;
			else if (json->type == UG_JSON_FALSE)
				*(int*) dest = 0;
			else
				error = UG_JSON_ERROR_TYPE_NOT_MATCH;
			break;

		case UG_ENTRY_INT:
			if (json->type == UG_JSON_NUMBER)
				*(int*) dest = strtol (value, NULL, 10);
			else
				error = UG_JSON_ERROR_TYPE_NOT_MATCH;
			break;

		case UG_ENTRY_UINT:
			if (json->type == UG_JSON_NUMBER)
				*(unsigned int*) dest = (unsigned int) strtoul (value, NULL, 10);
			else
				error = UG_JSON_ERROR_TYPE_NOT_MATCH;
			break;

		case UG_ENTRY_INT64:
			// C99 Standard
			if (json->type == UG_JSON_NUMBER)
				*(int64_t*) dest = strtoll (value, NULL, 10);
			else
				error = UG_JSON_ERROR_TYPE_NOT_MATCH;
			break;

		case UG_ENTRY_UINT64:
			// C99 Standard
			if (json->type == UG_JSON_NUMBER)
				*(uint64_t*) dest = strtoull (value, NULL, 10);
			else
				error = UG_JSON_ERROR_TYPE_NOT_MATCH;
			break;

		case UG_ENTRY_DOUBLE:
			if (json->type == UG_JSON_NUMBER)
				*(double*) dest = strtod (value, NULL);
			else
				error = UG_JSON_ERROR_TYPE_NOT_MATCH;
			break;

		case UG_ENTRY_STRING:
			if (json->type == UG_JSON_STRING)
				*(char**) dest = ug_strdup (value);
			else if (json->type == UG_JSON_NULL)
				*(char**) dest = NULL;
			else
				error = UG_JSON_ERROR_TYPE_NOT_MATCH;
			break;

		case UG_ENTRY_OBJECT:
			if (json->type != UG_JSON_OBJECT)
				return UG_JSON_ERROR_TYPE_NOT_MATCH;
			if (entry->param2)
				((UgInitFunc) entry->param2) (dest);
			ug_json_push (json, ug_json_parse_entry, dest, entry->param1);
			return UG_JSON_ERROR_NONE;

		case UG_ENTRY_ARRAY:
			if (json->type != UG_JSON_ARRAY)
				return UG_JSON_ERROR_TYPE_NOT_MATCH;
			ug_json_push (json, (UgJsonParseFunc) entry->param1, dest, NULL);
			return UG_JSON_ERROR_NONE;

		case UG_ENTRY_CUSTOM:
			return ((UgJsonParseFunc) entry->param1) (json, name, value,
					dest, (void*)entry);

		default:
			break;
		}
		// End of switch (entry->type)
		break;
	}

	// if entry->type != UG_ENTRY_OBJECT or UG_ENTRY_ARRAY
	// but json->type == UG_JSON_OBJECT or UG_JSON_ARRAY
//	if (json->type >= UG_JSON_OBJECT) {
//		ug_json_push (json, ug_json_parse_unknown, NULL, NULL);
//		return UG_JSON_ERROR_TYPE_NOT_MATCH;
//	}

	return error;
}

// ----------------------------------------------------------------------------

void  ug_json_write_entry (UgJson* json, void* src, const UgEntry* entry)
{
	union {
		void*           src;
		char**          pstring;
		int*            pint;
		unsigned int*	puint;
		int64_t*        pint64;
		uint64_t*       puint64;
		double*         pdouble;

		UgValue*        pvalue;
	} value;

	for (;  entry->type;  entry++) {
		value.src = ((char*) src) + entry->offset;

		switch (entry->type) {
		case UG_ENTRY_BOOL:
			if (entry->name)
				ug_json_write_string (json, entry->name);
			ug_json_write_bool (json, *(value.pint));
			break;

		case UG_ENTRY_INT:
			if (entry->name)
				ug_json_write_string (json, entry->name);
			ug_json_write_int (json, *(value.pint));
			break;

		case UG_ENTRY_UINT:
			if (entry->name)
				ug_json_write_string (json, entry->name);
			ug_json_write_uint (json, *(value.puint));
			break;

		case UG_ENTRY_INT64:
			if (entry->name)
				ug_json_write_string (json, entry->name);
			ug_json_write_int64 (json, *(value.pint64));
			break;

		case UG_ENTRY_UINT64:
			if (entry->name)
				ug_json_write_string (json, entry->name);
			ug_json_write_uint64 (json, *(value.puint64));
			break;

		case UG_ENTRY_DOUBLE:
			if (entry->name)
				ug_json_write_string (json, entry->name);
			ug_json_write_double (json, *(value.pdouble));
			break;

		case UG_ENTRY_STRING:
			// for UG_ENTRY_SKIP_IF_NULL
			if (entry->param1 && *(value.pstring) == NULL)
				break;
			if (entry->name)
				ug_json_write_string (json, entry->name);
			ug_json_write_string (json, *(value.pstring));
			break;

		case UG_ENTRY_OBJECT:
			if (entry->name)
				ug_json_write_string (json, entry->name);
			ug_json_write_object_head (json);
			ug_json_write_entry (json, value.src, entry->param1);
			ug_json_write_object_tail (json);
			break;

		case UG_ENTRY_ARRAY:
			if (entry->name)
				ug_json_write_string (json, entry->name);
			ug_json_write_array_head (json);
			((UgJsonWriteFunc) entry->param2) (json, value.src, (void*)entry);
			ug_json_write_array_tail (json);
			break;

		case UG_ENTRY_CUSTOM:
			if (entry->param2 == (void*) ug_json_write_value) {
				// for UgValue only
				if (value.pvalue->type == UG_VALUE_NONE)
					continue;
				if (value.pvalue->name == NULL)
					ug_json_write_string (json, entry->name);
			}
			else if (entry->name)
				ug_json_write_string (json, entry->name);
			((UgJsonWriteFunc) entry->param2) (json, value.src, (void*)entry);
			break;

		default:
			break;
		}
	}
}

