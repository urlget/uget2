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

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <UgData.h>
#include <UgEntry.h>

// ----------------------------------------------------------------------------
// Test1

typedef struct
{
	UG_GROUP_DATA_MEMBERS;
//	const UgGroupDataInfo*   info;

	int     type;
} Test1;

const UgEntry  Test1Entry[] =
{
	{"type", offsetof(Test1, type), UG_ENTRY_INT, NULL, NULL},
	{NULL}
};

void  test1_init(Test1* t1);

const UgGroupDataInfo  Test1Info =
{
	"Test1",
	sizeof(Test1),
	(UgInitFunc)   test1_init,
	(UgFinalFunc)  NULL,
	(UgAssignFunc) NULL,
	Test1Entry,		// UgEntry
};

void  test1_init(Test1* t1)
{
	t1->info = &Test1Info;
	t1->type = 1;
}

// ----------------------------------------------------------------------------
// Test2

typedef struct
{
	UG_GROUP_DATA_MEMBERS;
//	const UgGroupDataInfo*   info;

	int     type;
} Test2;

const UgEntry  Test2Entry[] =
{
	{"type", offsetof(Test2, type), UG_ENTRY_INT, NULL, NULL},
	{NULL}
};

void  test2_init(Test2* t2);

const UgGroupDataInfo  Test2Info =
{
	"Test2",
	sizeof(Test2),
	(UgInitFunc)   test2_init,
	(UgFinalFunc)  NULL,
	(UgAssignFunc) NULL,
	Test2Entry,		// UgEntry
};

void  test2_init(Test2* t2)
{
	t2->info = &Test2Info;
	t2->type = 2;
}

// ----------------------------------------------------------------------------
// Test3

typedef struct
{
	UG_GROUP_DATA_MEMBERS;
//	const UgGroupDataInfo*   info;

	int     type;
} Test3;

const UgEntry  Test3Entry[] =
{
	{"type", offsetof(Test3, type), UG_ENTRY_INT, NULL, NULL},
	{NULL}
};

void  test3_init(Test3* t3);

const UgGroupDataInfo  Test3Info =
{
	"Test3",
	sizeof(Test3),
	(UgInitFunc)   test3_init,
	(UgFinalFunc)  NULL,
	(UgAssignFunc) NULL,
	Test3Entry,		// UgEntry
};

void  test3_init(Test3* t3)
{
	t3->info = &Test3Info;
	t3->type = 3;
}

// ----------------------------------------------------------------------------
// test UgData

UgEntry	InfoCustomEntry[] = {
	{NULL, 0, UG_ENTRY_CUSTOM, (UgJsonParseFunc) ug_json_parse_data_ptr,
	                           (UgJsonWriteFunc) ug_json_write_data_ptr},
	{NULL}
};

void  test_data(UgData* data)
{
	void*  group_data;

	puts("\n--- test_data:");
	group_data = ug_data_realloc(data, &Test2Info);
	printf("ug_data_realloc (Test2Info) : %d\n", ((Test2*)group_data)->type);
	group_data = ug_data_realloc(data, &Test1Info);
	printf("ug_data_realloc (Test1Info) : %d\n", ((Test1*)group_data)->type);
	group_data = ug_data_realloc(data, &Test3Info);
	printf("ug_data_realloc (Test3Info) : %d\n", ((Test3*)group_data)->type);
}

void  parse_data(UgData* data)
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

	// UgRegistry is used by ug_json_parse_data() and ug_json_parse_data_entry()
	ug_registry_init(&registry);
	ug_registry_add(&registry, &Test2Info);
	ug_registry_add(&registry, &Test3Info);
	ug_registry_add(&registry, &Test1Info);
//	ug_registry_sort(&registry);
	// ug_json_parse_data() must use default UgDataKeys.
	ug_data_set_registry(&registry);

	ug_json_init(&json);
	ug_json_begin_parse(&json);
#if 1
	// method 1: use UgEntry to parse start of object
	ug_json_push(&json, ug_json_parse_entry, &data, InfoCustomEntry);
#else
	// method 2: push ug_json_parse_data() to parse start of object
	ug_json_push(&json, ug_json_parse_data_ptr, &data, &registry);
#endif
	code = ug_json_parse(&json, json_string, -1);
	ug_json_end_parse(&json);
	ug_json_final(&json);
	printf("ug_json_parse response %d\n", code);

	ug_data_set_registry(NULL);
	ug_registry_final(&registry);
}

void  dump_data(UgData* data)
{
	UgPair*	cur;
	UgPair*	end;

	for (cur = data->at, end = data->at + data->length;  cur < end;  cur++) {
		if (cur->key == NULL) {
			puts("NULL");
			continue;
		}

		printf("%s", ((UgGroupDataInfo*)cur->key)->name);
		if (cur->data)
			printf(" : %d", ((Test1*)cur->data)->type);
		puts("");
	}
}

// UgBufferFunc
static int  buffer_to_file(UgBuffer* buffer)
{
	FILE*   file = buffer->data;

	printf("write %d bytes to file\n", ug_buffer_length(buffer));
	fwrite(buffer->beg, 1, ug_buffer_length(buffer), file);
	buffer->cur = buffer->beg;
	return 0;
}

void  write_data_to_file(UgData* data, char* filename)
{
	UgJson   json;
	FILE*    file;
	UgBuffer buffer;

	file = fopen(filename, "w");
	ug_buffer_init(&buffer, 128);
	buffer.more = buffer_to_file;
	buffer.data = file;

	ug_json_init(&json);
	ug_json_begin_write(&json, UG_JSON_FORMAT_INDENT, &buffer);
#if 1
	// method 1: use UgEntry to write start of object
	ug_json_write_entry(&json, data, InfoCustomEntry);
#else
	// method 2: call ug_json_write_object_head() to write start of object
	ug_json_write_object_head(&json);
	ug_json_write_data(&json, data);
	ug_json_write_object_tail(&json);
#endif
	ug_json_end_write(&json);
	ug_json_final(&json);

	ug_buffer_clear(&buffer, 1);
	fclose(file);
}

// ----------------------------------------------------------------------------
// main

int   main(void)
{
	UgData  data;

	puts("\n--- test UgData functions:");
	ug_data_init(&data, 8, 2);
#if 0
	test_data(&data);
#else
	parse_data(&data);
#endif
	dump_data(&data);
	write_data_to_file(&data, "test-data.json");
	ug_data_final(&data);

	return 0;
}

