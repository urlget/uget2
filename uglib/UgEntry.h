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

#ifndef UG_ENTRY_H
#define UG_ENTRY_H

#include <stddef.h>		// NULL, offsetof()
#include <UgJson.h>
#include <UgDefine.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct UgEntry          UgEntry;

// ----------------------------------------------------------------------------
// UgEntryType : UgEntry use this to parse and write JSON value

typedef enum
{
	UG_ENTRY_NONE = 0,
	UG_ENTRY_BOOL,      // JSON true or false, C int
	UG_ENTRY_INT,       // JSON number, C int
	UG_ENTRY_UINT,      // JSON number, C unsigned int
	UG_ENTRY_INT64,     // JSON number, C int64_t, long long
	UG_ENTRY_UINT64,    // JSON number, C uint64_t, unsigned long long
	UG_ENTRY_DOUBLE,    // JSON number, C double
	UG_ENTRY_STRING,    // JSON string, C string

	UG_ENTRY_OBJECT,    // JSON object, C struct, C++11 standard layout class
	UG_ENTRY_ARRAY,     // JSON array,  C array or others

	UG_ENTRY_CUSTOM,    // JSON value,  C functions were used.
} UgEntryType;

// You can set this in UgEntry.param2 when UgEntry.type is UG_ENTRY_STRING.
// ug_json_write_entry() will not output this field when value is NULL.
#define  UG_ENTRY_NO_NULL         ((void*)(uintptr_t) 0x0001)

// ----------------------------------------------------------------------------
// UgEntry: It can defines a object member and it's offset of data structure.

// ------------------------------------
// sample for JSON

//	typedef struct
//	{
//		char*  user;
//		int    number;
//	} Foo;
//
//	static UgEntry FooEntry[] =
//	{
//		{ "user",   offsetof(Foo, user),   UG_ENTRY_STRING, NULL, NULL},
//		{ "number", offsetof(Foo, number), UG_ENTRY_INT,    NULL, NULL},
//		{ NULL }    // null-terminated
//	};
//
//	JSON output:
//	{
//		"user": "guest3",
//		"number": 2500,
//	}

// if UgEntry.type == 0, this entry is null-terminated.

// if UgEntry.name == NULL, it can match no name or any name.
// it usually uses at first or last entry.

// UgEntryType = UG_ENTRY_STRING
// If you don't want to output anything when string value is NULL,
// set UgEntry.param2 to UG_ENTRY_NO_NULL.

// UgEntryType = UG_ENTRY_OBJECT
// UgEntry.param1 pointer to UgEntry
// UgEntry.param2 pointer to UgInitFunc
// ---------
// if (UgInitFunc)
//     UgInitFunc(UserData);

// UgEntryType = UG_ENTRY_ARRAY
// UgEntry.param1 = UgJsonParseFunc, how to parse JSON array elements.
// UgEntry.param2 = UgJsonWriteFunc, how to write JSON array elements.
// ---------
// parser call UgEntry.param1 to parse JSON array.
// writer call UgEntry.param2 to write JSON array.

// UgEntryType = UG_ENTRY_CUSTOM
// UgEntry.param1 = UgJsonParseFunc, how to parse JSON value.
// UgEntry.param2 = UgJsonWriteFunc, how to write JSON value.
// ---------
// parser call UgEntry.param1 to parse JSON value.
// writer call UgEntry.param2 to write JSON value.

struct UgEntry
{
	char*        name;      // JSON name
	int          offset;    // offsetof (TYPE, MEMBER)
	UgEntryType  type;

	void*        param1;    // parameter 1
	void*        param2;    // parameter 2
};

// parse JSON value by UgEntry
UgJsonError ug_json_parse_entry(UgJson* json,
                                const char* name, const char* value,
                                void* dest, void* entry);

// write JSON value by UgEntry
void  ug_json_write_entry(UgJson* json, void* src, const UgEntry* entry);

#ifdef __cplusplus
}
#endif

// ----------------------------------------------------------------------------
// C++11 standard-layout

#ifdef __cplusplus

namespace Ug
{
// This one is for directly use only. You can NOT derived it.
typedef struct UgEntry    Entry;
};  // namespace Ug

#endif  // __cplusplus


#endif  // UG_ENTRY_H

