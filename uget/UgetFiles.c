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

static void ug_json_write_files_array(UgJson* json, UgetFilesArray* array);
static UgJsonError ug_json_parse_files_array(UgJson* json, const char* name,
                                             const char* value,
                                             void* array, void* none);
static void uget_files_element_deleted(UgetFilesElement* element);


static const UgEntry  UgetFilesEntry[] =
{	{"source",   offsetof(UgetFiles, source),   UG_ENTRY_ARRAY,
			ug_json_parse_files_array, ug_json_write_files_array},
	{"output",   offsetof(UgetFiles, output),   UG_ENTRY_ARRAY,
			ug_json_write_files_array, ug_json_parse_files_array},
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
	ug_array_init(&files->source, sizeof(UgetFilesElement), 8);
	ug_array_init(&files->output, sizeof(UgetFilesElement), 8);
}

static void uget_files_final(UgetFiles* files)
{
	ug_array_foreach_ptr(&files->source, (UgForeachFunc)ug_free, NULL);
	ug_array_foreach_ptr(&files->output, (UgForeachFunc)ug_free, NULL);
	ug_array_clear(&files->source);
	ug_array_clear(&files->output);
}

int  uget_files_assign(UgetFiles* files, UgetFiles* src)
{
	ug_array_foreach_ptr(&files->source, (UgForeachFunc)ug_free, NULL);
	ug_array_foreach_ptr(&files->output, (UgForeachFunc)ug_free, NULL);

	files->source.length = 0;
	files->output.length = 0;
	uget_files_array_copy(&files->source, &src->source);
	uget_files_array_copy(&files->output, &src->output);
	return TRUE;
}

static void uget_files_reset(UgetFiles* files, UgetFiles* src, int keep_deleted)
{
	// source
	ug_array_foreach_ptr(&files->source, (UgForeachFunc)ug_free, NULL);
	files->source.length = 0;
	uget_files_array_copy(&files->source, &src->source);

	// output
	if (keep_deleted) {
		ug_array_foreach_ptr(&files->output,
				(UgForeachFunc)uget_files_element_deleted, NULL);
	}
	else {
		ug_array_foreach_ptr(&files->output, (UgForeachFunc)ug_free, NULL);
		files->output.length = 0;
	}
	uget_files_array_copy(&files->output, &src->output);

	files->source_count = src->source_count;
	files->output_count = src->output_count;
}

int  uget_files_sync(UgetFiles* files, UgetFiles* src, int keep_deleted)
{
	UgetFilesElement* element;
	UgetFilesElement* element_src;
	int  index;
	int  index_src;
	int  result;

	// sync UgetFiles::source
	if (files->source.length != src->source.length) {
		uget_files_reset(files, src, keep_deleted);
		return TRUE;
	}
	else {
		for (index = 0;  index < src->source.length;  index++) {
			element_src = src->source.at   + index;
			element     = files->source.at + index;
			if (strcmp(element->name, element_src->name) != 0) {
				uget_files_reset(files, src, keep_deleted);
				return TRUE;
			}
		}
	}

	// sync UgetFiles::output
	for (index_src = 0;  index_src < src->output.length;  index_src++) {
		element_src = src->output.at + index_src;
		element = ug_array_find_sorted(&files->output, &element_src->name,
		                               ug_array_compare_string, &index);
		// file has been deleted
		if (element_src->state & UGET_FILES_DELETED) {
			ug_free(element_src->name);
			ug_array_erase(&src->output, index_src, 1);
			if (element) {
				element->state = element_src->state;
				result = TRUE;
				if (keep_deleted == FALSE) {
					ug_free(element->name);
					ug_array_erase(&files->output, index, 1);
				}
			}
			index_src--;  // rollback
			continue;
		}

		// add new filename
		if (element == NULL) {
			element = ug_array_insert(&files->output, index, 1);
			element->name = ug_strdup(element_src->name);
			result = TRUE;
		}
		element->state = element_src->state;
	}

	files->source_count = src->source_count;
	files->output_count = src->output_count;
	return result;
}

UgetFilesElement*  uget_files_realloc(UgetFiles* files, const char* name)
{
	UgetFilesElement* element;
	int  index;

	element = ug_array_find_sorted(&files->output, &name,
	                               ug_array_compare_string, &index);
    if (element == NULL) {
		element = ug_array_insert(&files->output, index, 1);
		element->name  = NULL;
		element->state = 0;
		files->output_count++;
    }
	return element;
}

// ----------------------------------------------------------------------------
// UgetFilesArray

void uget_files_array_copy(UgetFilesArray* array, UgetFilesArray* src)
{
	UgetFilesElement* element;
	int  index;

	element = ug_array_alloc(array, src->length);
	for(index = 0;  index < src->length;  index++) {
		element[index].name  = ug_strdup(src->at[index].name);
		element[index].state = src->at[index].state;
	}
}

static void uget_files_element_deleted(UgetFilesElement* element)
{
	element->state |= UGET_FILES_DELETED;
}

// ----------------------------------------------------------------------------
// JSON

static const UgEntry  UgetFilesElementEntry[] =
{	{"name",   offsetof(UgetFilesElement, name),   UG_ENTRY_STRING,
			NULL, NULL},
	{"state",  offsetof(UgetFilesElement, state),  UG_ENTRY_UINT,
			NULL, NULL},
	{NULL}    // null-terminated
};

static void ug_json_write_files_array(UgJson* json, UgetFilesArray* array)
{
	UgetFilesElement* element;
	int  index;

	for (index = 0;  index < array->length;  index++) {
		element = array->at + index;
		ug_json_write_object_head(json);
		ug_json_write_entry(json, element, UgetFilesElementEntry);
		ug_json_write_object_tail(json);
	}
}

static UgJsonError ug_json_parse_files_array(UgJson* json, const char* name,
                                             const char* value,
                                             void* array, void* none)
{
	UgetFilesElement* element;

	if (json->type != UG_JSON_OBJECT) {
//		if (json->type >= UG_JSON_OBJECT)
//			ug_json_push(json, ug_json_parse_unknown, NULL, NULL);
		return UG_JSON_ERROR_TYPE_NOT_MATCH;
	}

	((UgetFilesArray*)array)->element_size = sizeof(UgetFilesElement);
	element = ug_array_alloc(array, 1);
	ug_json_push(json, ug_json_parse_entry, element,
	             (void*)&UgetFilesElementEntry);
	return UG_JSON_ERROR_NONE;
}
