/*
 *
 *   Copyright (C) 2005-2020 by C.H. Huang
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
#include "UgRegistry1.h"
#include "UgCategory.h"
#include "UgData-download.h"


// ----------------------------------------------------------------------------
// UgCategory

// UgCategory.indices parse/write for UgMarkup
static void	ug_int_list_in_markup (GList** list, GMarkupParseContext* context);
static void	ug_int_list_to_markup (GList** list, UgMarkup* markup);

static const UgDataEntry	ug_category_entry[] =
{
	{"name",			G_STRUCT_OFFSET (UgCategory, name),				UG_TYPE_STRING,		NULL,	NULL},
	{"ActiveLimit",		G_STRUCT_OFFSET (UgCategory, active_limit),		UG_TYPE_UINT,		NULL,	NULL},
	{"FinishedLimit",	G_STRUCT_OFFSET (UgCategory, finished_limit),	UG_TYPE_UINT,		NULL,	NULL},
	{"RecycledLimit",	G_STRUCT_OFFSET (UgCategory, recycled_limit),	UG_TYPE_UINT,		NULL,	NULL},
	{"DownloadDefault",	G_STRUCT_OFFSET (UgCategory, defaults),			UG_TYPE_INSTANCE,	&ug_dataset_iface,	NULL},
	{"DownloadIndices",	G_STRUCT_OFFSET (UgCategory, indices),			UG_TYPE_CUSTOM,		ug_int_list_in_markup,	ug_int_list_to_markup},
	{NULL},			// null-terminated
};
// extern
const UgData1Interface	ug_category_iface =
{
	sizeof (UgCategory),	// instance_size
	"category",				// name
	ug_category_entry,		// entry

	(UgInitFunc)     ug_category_init,
	(UgFinalizeFunc) ug_category_finalize,
	(UgAssign1Func)  NULL,
};
// extern
const UgData1Interface*	ug_category_iface_pointer = &ug_category_iface;


void	ug_category_init (UgCategory* category)
{
//	category->iface = ug_category_iface_pointer;

	category->defaults = ug_dataset_new ();
	category->active_limit   = 3;
	category->finished_limit = 300;
	category->recycled_limit = 300;
}

void	ug_category_finalize (UgCategory* category)
{
	if (category->destroy.func)
		category->destroy.func (category->destroy.data);

	g_free (category->name);
	if (category->defaults)
		ug_dataset_unref (category->defaults);
}

UgCategory*	ug_category_new (void)
{
	return ug_data1_new (ug_category_iface_pointer);
}

void	ug_category_free (UgCategory* category)
{
	ug_data1_free (category);
}

// add dataset to category and increase reference count of dataset.
void	ug_category_add (UgCategory* category, UgDataset* dataset)
{
	UgLog*			datalog;

	// added on
	datalog = ug_dataset_realloc (dataset, UgLogInfo, 0);
	datalog->added_on = time (NULL);
}


// ----------------------------------------------------------------------------
// Category.indices load/save
//
static void ug_int_list_start_element (GMarkupParseContext*	context,
                                       const gchar*			element_name,
                                       const gchar**		attr_names,
                                       const gchar**		attr_values,
                                       GList**				list,
                                       GError**				error)
{
	guint	index;
	int		value;

	for (index=0; attr_names[index]; index++) {
		if (strcmp (attr_names[index], "value") != 0)
			continue;
		value = atoi (attr_values[index]);
		*list = g_list_prepend (*list, GINT_TO_POINTER (value));
	}

	g_markup_parse_context_push (context, &ug_markup_skip_parser, NULL);
}

static GMarkupParser	ug_int_list_parser =
{
	(gpointer) ug_int_list_start_element,
	(gpointer) g_markup_parse_context_pop,
	NULL,
	NULL,
	NULL
};

static void	ug_int_list_in_markup (GList** list, GMarkupParseContext* context)
{
	g_markup_parse_context_push (context, &ug_int_list_parser, list);
}

static void	ug_int_list_to_markup (GList** list, UgMarkup* markup)
{
	GList*		link;
	guint		value;

	for (link = g_list_last (*list);  link;  link = link->prev) {
		value = GPOINTER_TO_INT (link->data);
		ug_markup_write_element_start (markup, "int value='%d'", value);
		ug_markup_write_element_end   (markup, "int");
	}
}


// ----------------------------------------------------------------------------
// CategoryList load/save
//
static void ug_category_data_start_element (GMarkupParseContext*	context,
                                            const gchar*		element_name,
                                            const gchar**		attr_names,
                                            const gchar**		attr_values,
                                            GList**				list,
                                            GError**			error)
{
	UgCategory*		category;

	if (strcmp (element_name, "category") != 0) {
		g_markup_parse_context_push (context, &ug_markup_skip_parser, NULL);
		return;
	}

	// user must register data interface of UgCategory.
	category = ug_data1_new (ug_category_iface_pointer);
	*list = g_list_prepend (*list, category);
	g_markup_parse_context_push (context, &ug_data1_parser, category);
}

static GMarkupParser	ug_category_data_parser =
{
	(gpointer) ug_category_data_start_element,
	(gpointer) g_markup_parse_context_pop,
	NULL,
	NULL,
	NULL
};

static void ug_category_list_start_element (GMarkupParseContext*	context,
                                            const gchar*		element_name,
                                            const gchar**		attr_names,
                                            const gchar**		attr_values,
                                            GList**				list,
                                            GError**			error)
{
//	guint	index;

//	if (strcmp (element_name, "UgCategoryList") == 0) {
//		for (index=0; attr_names[index]; index++) {
//			if (strcmp (attr_names[index], "version") != 0)
//				continue;
//			if (strcmp (attr_values[index], "1") == 0) {
				g_markup_parse_context_push (context, &ug_category_data_parser, list);
				return;
//			}
//			// others...
//			break;
//		}
//	}

//	g_markup_parse_context_push (context, &ug_markup_skip_parser, NULL);
}

static GMarkupParser	ug_category_list_parser =
{
	(gpointer) ug_category_list_start_element,
	(gpointer) g_markup_parse_context_pop,
	NULL,
	NULL,
	NULL
};

GList*	ug_category_list_load (const gchar* file)
{
	GList*		category_list;

	category_list = NULL;
	ug_markup_parse (file, &ug_category_list_parser, &category_list);
	return category_list;
}

void	ug_category_list_link (GList* list, GList* download_list)
{
	GPtrArray*	array;
	UgCategory*	category;
	GList*		link;
	guint		index;

	// create array from download_list
	array = g_ptr_array_sized_new (g_list_length (download_list));
	for (link = download_list;  link;  link = link->next)
		array->pdata[array->len++] = link->data;

	// link tasks in category
	for (;  list;  list = list->next) {
		category = list->data;
		// get tasks from array by index
		for (link = category->indices;  link;  link = link->next) {
			index = GPOINTER_TO_INT (link->data);
			if (index < array->len)
				link->data = g_ptr_array_index (array, index);
		}
	}

	// free array
	g_ptr_array_free (array, TRUE);
}


// ----------------------------------------------------------------------------
// DownloadList load/save
//
static void ug_download_data_start_element (GMarkupParseContext*	context,
                                            const gchar*		element_name,
                                            const gchar**		attr_names,
                                            const gchar**		attr_values,
                                            GList**				list,
                                            GError**			error)
{
	UgDataset*		dataset;

	if (strcmp (element_name, "download") != 0) {
		g_markup_parse_context_push (context, &ug_markup_skip_parser, NULL);
		return;
	}

	dataset = ug_dataset_new ();
	*list = g_list_prepend (*list, dataset);
	g_markup_parse_context_push (context, &ug_data1_parser, dataset);
}

static GMarkupParser	ug_download_data_parser =
{
	(gpointer) ug_download_data_start_element,
	(gpointer) g_markup_parse_context_pop,
	NULL,
	NULL,
	NULL
};

static void ug_download_list_start_element (GMarkupParseContext*	context,
                                            const gchar*		element_name,
                                            const gchar**		attr_names,
                                            const gchar**		attr_values,
                                            GList**				list,
                                            GError**			error)
{
//	guint	index;

//	if (strcmp (element_name, "UgDownloadList") == 0) {
//		for (index=0; attr_names[index]; index++) {
//			if (strcmp (attr_names[index], "version") != 0)
//				continue;
//			if (strcmp (attr_values[index], "1") == 0) {
				g_markup_parse_context_push (context, &ug_download_data_parser, list);
				return;
//			}
			// others...
//			break;
//		}
//	}

//	g_markup_parse_context_push (context, &ug_markup_skip_parser, NULL);
}

static GMarkupParser	ug_download_list_parser =
{
	(gpointer) ug_download_list_start_element,
	(gpointer) g_markup_parse_context_pop,
	NULL,
	NULL,
	NULL
};

GList*	ug_download_list_load (const gchar* download_file)
{
//	UgRelation*		relation;
	GList*			list;
//	GList*			link;

	list = NULL;
	ug_markup_parse (download_file, &ug_download_list_parser, &list);
	// attachment
//	for (link = list;  link;  link = link->next) {
//		relation = UG_DATASET_RELATION ((UgDataset*) link->data);
//		ug_attachment_ref (relation->attached.stamp);
//	}

	return list;
}

// ----------------------------------------------------------------------------
// UgRelation : relation of UgCategory, UgDataset, and UgPlugin.
//
static void	ug_relation_final  (UgRelation* relation);

static const UgDataEntry	ug_relation_entry[] =
{
	{"hints",			G_STRUCT_OFFSET (UgRelation, hints),			UG_TYPE_UINT,	NULL,	NULL},
	{NULL},			// null-terminated
};
// extern
const UgData1Interface	ug_relation_iface =
{
	sizeof (UgRelation),	// instance_size
	"relation",				// name
	ug_relation_entry,		// entry

	(UgInitFunc)     NULL,
	(UgFinalizeFunc) ug_relation_final,
	(UgAssign1Func)  NULL,
};
// extern
const UgData1Interface*	ug_relation_iface_pointer = &ug_relation_iface;


static void	ug_relation_final (UgRelation* relation)
{
	if (relation->destroy.func)
		relation->destroy.func (relation->destroy.data);

	g_free (relation->attached.folder);
//	ug_attachment_unref (relation->attached.stamp);
}

