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

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <UgInfo.h>
#include <UgEntry.h>

// ----------------------------------------------------------------------------
// Test1

typedef struct
{
	UG_DATA_MEMBERS;
//	const UgDataInfo*   info;

	int     type;
} Test1;

const UgEntry  Test1Entry[] =
{
	{"type", offsetof (Test1, type), UG_ENTRY_INT, NULL, NULL},
	{NULL}
};

void  test1_init (Test1* t1);

const UgDataInfo  Test1Info =
{
	"Test1",
	sizeof (Test1),
	Test1Entry,		// UgEntry
	(UgInitFunc)   test1_init,
	(UgFinalFunc)  NULL,
	(UgAssignFunc) NULL
};

void  test1_init (Test1* t1)
{
	t1->info = &Test1Info;
	t1->type = 1;
}

// ----------------------------------------------------------------------------
// Test2

typedef struct
{
	UG_DATA_MEMBERS;
//	const UgDataInfo*   info;

	int     type;
} Test2;

const UgEntry  Test2Entry[] =
{
	{"type", offsetof (Test2, type), UG_ENTRY_INT, NULL, NULL},
	{NULL}
};

void  test2_init (Test2* t2);

const UgDataInfo  Test2Info =
{
	"Test2",
	sizeof (Test2),
	Test2Entry,		// UgEntry
	(UgInitFunc)   test2_init,
	(UgFinalFunc)  NULL,
	(UgAssignFunc) NULL
};

void  test2_init (Test2* t2)
{
	t2->info = &Test2Info;
	t2->type = 2;
}

// ----------------------------------------------------------------------------
// Test3

typedef struct
{
	UG_DATA_MEMBERS;
//	const UgDataInfo*   info;

	int     type;
} Test3;

const UgEntry  Test3Entry[] =
{
	{"type", offsetof (Test3, type), UG_ENTRY_INT, NULL, NULL},
	{NULL}
};

void  test3_init (Test3* t3);

const UgDataInfo  Test3Info =
{
	"Test3",
	sizeof (Test3),
	Test3Entry,		// UgEntry
	(UgInitFunc)   test3_init,
	(UgFinalFunc)  NULL,
	(UgAssignFunc) NULL
};

void  test3_init (Test3* t3)
{
	t3->info = &Test3Info;
	t3->type = 3;
}

// ----------------------------------------------------------------------------
// test UgInfo

UgEntry	InfoCustomEntry[] = {
	{NULL, 0, UG_ENTRY_CUSTOM, (UgJsonParseFunc) ug_json_parse_info,
	                           (UgJsonWriteFunc) ug_json_write_info},
	{NULL}
};

void  test_info (UgInfo* info)
{
	void*  data;

	puts ("\n--- test_info:");
	data = ug_info_realloc (info, &Test2Info);
	printf ("ug_info_realloc (Test2Info) : %d\n", ((Test2*)data)->type);
	data = ug_info_realloc (info, &Test1Info);
	printf ("ug_info_realloc (Test1Info) : %d\n", ((Test1*)data)->type);
	data = ug_info_realloc (info, &Test3Info);
	printf ("ug_info_realloc (Test3Info) : %d\n", ((Test3*)data)->type);
}

void  parse_info (UgInfo* info)
{
	int     code;
	UgJson	json;
	UgRegistry  registry;
	const char* json_string = {
		"{"
		"  \"Test1\": {"
		"    \"type\": 1"
		"  },"
		"  \"Test2\": {"
		"    \"type\": 2"
		"  },"
		"  \"Test3\": {"
		"    \"type\": 3"
		"  }"
		"}"
	};

	// UgRegistry is used by ug_json_parse_info() and ug_json_parse_info_entry()
	ug_registry_init (&registry);
	ug_registry_add (&registry, &Test2Info);
	ug_registry_add (&registry, &Test3Info);
	ug_registry_add (&registry, &Test1Info);
//	ug_registry_sort (&registry);
	// ug_json_parse_info() must use default UgInfoKeys.
	ug_info_set_registry (&registry);

	ug_json_init (&json);
	ug_json_begin_parse (&json);
#if 1
	// method 1: use UgEntry to parse start of object
	ug_json_push (&json, ug_json_parse_entry, info, InfoCustomEntry);
#else
	// method 2: push ug_json_parse_info() to parse start of object
	ug_json_push (&json, ug_json_parse_info, info, &registry);
#endif
	code = ug_json_parse (&json, json_string, -1);
	ug_json_end_parse (&json);
	ug_json_final (&json);
	printf ("ug_json_parse response %d\n", code);

	ug_info_set_registry (NULL);
	ug_registry_final (&registry);
}

void  dump_info (UgInfo* info)
{
	UgPair*	cur;
	UgPair*	end;

	for (cur = info->at, end = info->at + info->length;  cur < end;  cur++) {
		if (cur->key == NULL) {
			puts ("NULL");
			continue;
		}

		printf ("%s", ((UgDataInfo*)cur->key)->name);
		if (cur->data)
			printf (" : %d", ((Test1*)cur->data)->type);
		puts ("");
	}
}

// UgBufferFunc
static int  buffer_to_file (UgBuffer* buffer)
{
	FILE*   file = buffer->data;

	printf ("write %d bytes to file\n", ug_buffer_length (buffer));
	fwrite (buffer->beg, 1, ug_buffer_length(buffer), file);
	buffer->cur = buffer->beg;
	return 0;
}

void  write_info_to_file (UgInfo* info, char* filename)
{
	UgJson   json;
	FILE*    file;
	UgBuffer buffer;

	file = fopen (filename, "w");
	ug_buffer_init (&buffer, 128);
	buffer.more = buffer_to_file;
	buffer.data = file;

	ug_json_init (&json);
	ug_json_begin_write (&json, UG_JSON_FORMAT_INDENT, &buffer);
#if 1
	// method 1: use UgEntry to write start of object
	ug_json_write_entry (&json, info, InfoCustomEntry);
#else
	// method 2: call ug_json_write_object_head() to write start of object
	ug_json_write_object_head (&json);
	ug_json_write_info (&json, info);
	ug_json_write_object_tail (&json);
#endif
	ug_json_end_write (&json);
	ug_json_final (&json);

	ug_buffer_clear (&buffer, 1);
	fclose (file);
}

// ----------------------------------------------------------------------------
// main

int   main (void)
{
	UgInfo  info;

	puts ("\n--- test UgInfo functions:");
	ug_info_init (&info, 8, 2);
#if 0
	test_info (&info);
#else
	parse_info (&info);
#endif
	dump_info (&info);
	write_info_to_file (&info, "test-info.json");
	ug_info_final (&info);

	return 0;
}

