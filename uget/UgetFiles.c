/*
 *
 *   Copyright (C) 2018 by C.H. Huang
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
#include <UgString.h>
#include <UgJson.h>
#include <UgetFiles.h>
#include <UgJson-custom.h>

// ----------------------------------------------------------------------------
// UgetFiles

static void uget_files_init(UgetFiles* files);
static void uget_files_final(UgetFiles* files);
static void uget_files_copy(UgetFiles* files, UgetFiles* src);

static void        ug_json_write_collection(UgJson* json, void* collection);
static UgJsonError ug_json_parse_collection(UgJson* json, const char* name,
                                            const char* value,
                                            void* collection, void* none);

static const UgEntry  UgetFilesEntry[] =
{
	{"collection",   offsetof(UgetFiles, collection),   UG_ENTRY_ARRAY,
			ug_json_parse_collection, ug_json_write_collection},
	{NULL}    // null-terminated
};

static const UgDataInfo  UgetFilesInfoStatic =
{
	"files",               // name
	sizeof(UgetFiles),     // size
	(UgInitFunc)   uget_files_init,
	(UgFinalFunc)  uget_files_final,
	(UgAssignFunc) uget_files_assign,
	UgetFilesEntry,
};
// extern
const UgDataInfo*  UgetFilesInfo = &UgetFilesInfoStatic;

static void uget_files_init(UgetFiles* files)
{
	ug_array_init(&files->collection, sizeof(UgetFile), 8);
	files->sync_count = 0;
}

static void uget_files_final(UgetFiles* files)
{
	// free UgetFile.path in collection
	ug_array_foreach_ptr(&files->collection, (UgForeachFunc)ug_free, NULL);
	ug_array_clear(&files->collection);
}

int  uget_files_assign(UgetFiles* files, UgetFiles* src)
{
	// free UgetFile.path in collection
	ug_array_foreach_ptr(&files->collection, (UgForeachFunc)ug_free, NULL);
	files->collection.length = 0;

	uget_files_copy(files, src);
	files->sync_count = src->sync_count;
	return TRUE;
}

void  uget_files_clear(UgetFiles* files)
{
	ug_array_foreach_ptr(&files->collection, (UgForeachFunc)ug_free, NULL);
	files->collection.length = 0;
}

// sync elements from 'src' to 'files.
// 1. all elements in 'src' will insert/replace into 'files'.
// 2. remove deleted (state == UGET_FILE_STATE_DELETED) elements in 'src'.
// return TRUE if 'files' have added or removed elements.
int  uget_files_sync(UgetFiles* files, UgetFiles* src)
{
	UgetFile* element;
	UgetFile* element_src;
	int  index;
	int  index_src;

	if (files->sync_count == src->sync_count)
		return FALSE;

	// sync element from 'src'
	for (index_src = 0;  index_src < src->collection.length;  index_src++) {
		element_src = src->collection.at + index_src;
		element = ug_array_find_sorted(&files->collection, &element_src->path,
		                               ug_array_compare_string, &index);
		// add new element in files
		if (element == NULL) {
			element = ug_array_insert(&files->collection, index, 1);
			if (element_src->path)
				element->path = ug_strdup(element_src->path);
			else
				element->path = NULL;
		}
		element->type  = element_src->type;
		element->state = element_src->state;
//		element->order = element_src->order;
		element->total = element_src->total;
		element->complete = element_src->complete;

		// remove deleted element in 'src'
		if (element_src->type & UGET_FILE_STATE_DELETED) {
			// delete file from src
			ug_free(element_src->path);
			ug_array_erase(&src->collection, index_src, 1);
			index_src--;  // rollback
			continue;
		}
	}
	files->sync_count = src->sync_count;
	return TRUE;
}

UgetFile* uget_files_realloc(UgetFiles* files, const char* path)
{
	UgetFile* element;
	int       index;

	element = ug_array_find_sorted(&files->collection, &path,
	                               ug_array_compare_string, &index);
    if (element == NULL) {
		element = ug_array_insert(&files->collection, index, 1);
		element->path  = ug_strdup(path);
		element->type  = 0;
		element->state = 0;
//		element->order = 0;
		element->total = 0;
		element->complete = 0;
		files->sync_count++;
    }
	return element;
}

UgetFile* uget_files_replace(UgetFiles* files, const char* path,
                             int type, int state)
{
	UgetFile* element;

	element = uget_files_realloc(files, path);
	element->type  = type;
	element->state = state;
	files->sync_count++;
	return element;
}

void uget_files_apply(UgetFiles* files, int type, int state)
{
	UgetFile* element;
	int       index;

	for (index = 0;  index < files->collection.length;  index++) {
		element = files->collection.at + index;
        if (element->type == type || type == UGET_FILE_ALL)
			element->state |= state;
	}
	files->sync_count++;
}

void uget_files_erase(UgetFiles* files, int type, int state)
{
	UgetFile* element;
	int       index;

	for (index = 0;  index < files->collection.length;  index++) {
		element = files->collection.at + index;
        if (element->type != type && type != UGET_FILE_ALL)
			continue;
		if (element->state & state) {
			// delete file from src
			ug_free(element->path);
			ug_array_erase(&files->collection, index, 1);
			index--;  // rollback
		}
	}
	files->sync_count++;
}

// copy elements from 'src' to 'files'.
static void uget_files_copy(UgetFiles* files, UgetFiles* src)
{
	UgetFile* element;
	int       index;

	element = ug_array_alloc(&files->collection, src->collection.length);
	for(index = 0;  index < src->collection.length;  index++) {
		if (src->collection.at[index].path)
			element[index].path = ug_strdup(src->collection.at[index].path);
		else
			element[index].path = NULL;
		element[index].type  = src->collection.at[index].type;
		element[index].state = src->collection.at[index].state;
//		element[index].order = src->collection.at[index].order;
		element[index].total = src->collection.at[index].total;
		element[index].complete = src->collection.at[index].complete;
	}
    files->sync_count++;
}

// ----------------------------------------------------------------------------
// JSON

static const UgEntry  UgetFileEntry[] =
{	{"path",     offsetof(UgetFile, path),     UG_ENTRY_STRING,
			NULL, NULL},
	{"type",     offsetof(UgetFile, type),     UG_ENTRY_CUSTOM,
			ug_json_parse_int16, ug_json_write_int16},
	{"state",    offsetof(UgetFile, state),    UG_ENTRY_CUSTOM,
			ug_json_parse_int16, ug_json_write_int16},
//	{"order",    offsetof(UgetFile, order),    UG_ENTRY_CUSTOM,
//			ug_json_parse_int32, ug_json_write_int32},
	{"total",    offsetof(UgetFile, total),    UG_ENTRY_INT64,
			NULL, NULL},
	{"complete", offsetof(UgetFile, complete), UG_ENTRY_INT64,
			NULL, NULL},
	{NULL}    // null-terminated
};

static void ug_json_write_collection(UgJson* json, void* collection)
{
	UgetFileCollection* array = collection;
	UgetFile* element;
	int       index;

	for (index = 0;  index < array->length;  index++) {
		element = array->at + index;
		ug_json_write_object_head(json);
		ug_json_write_entry(json, element, UgetFileEntry);
		ug_json_write_object_tail(json);
	}
}

static UgJsonError ug_json_parse_collection(UgJson* json, const char* name,
                                       const char* value,
                                       void* collection, void* none)
{
	UgetFileCollection* array = collection;
	UgetFile* element;

	if (json->type != UG_JSON_OBJECT) {
//		if (json->type >= UG_JSON_OBJECT)
//			ug_json_push(json, ug_json_parse_unknown, NULL, NULL);
		return UG_JSON_ERROR_TYPE_NOT_MATCH;
	}

	array->element_size = sizeof(UgetFile);
	element = ug_array_alloc(array, 1);
	element->path = NULL;
	ug_json_push(json, ug_json_parse_entry,
	             element, (void*)UgetFileEntry);
	return UG_JSON_ERROR_NONE;
}
