/*
 *
 *   Copyright (C) 2005-2019 by C.H. Huang
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
#include <memory.h>
// uglib
#include "UgDataset.h"
#include "UgCategory.h"
#include "UgData-download.h"


// ----------------------------------------------------------------------------
// UgDataset

static void	ug_dataset_init      (UgDataset* dataset);
static void	ug_dataset_finalize  (UgDataset* dataset);
static void	ug_dataset_assign    (UgDataset* dataset, UgDataset* src);
static void	ug_dataset_in_markup (UgDataset* dataset, GMarkupParseContext* context);
static void	ug_dataset_to_markup (UgDataset* dataset, UgMarkup* markup);

static const UgDataEntry	ug_dataset_entry[] =
{
	{"DataList",	0,	UG_TYPE_CUSTOM,	ug_dataset_in_markup,	ug_dataset_to_markup},
	{NULL}		// null-terminated
};

const UgData1Interface	ug_dataset_iface =
{
	sizeof (UgDataset),		// instance_size
	"dataset",				// name
	ug_dataset_entry,		// entry

	(UgInitFunc)		ug_dataset_init,
	(UgFinalizeFunc)	ug_dataset_finalize,
	(UgAssign1Func)		ug_dataset_assign,
};


static void	ug_dataset_init	(UgDataset* dataset)
{
	dataset->ref_count = 1;

	// for macros
	ug_dataset_alloc_list (dataset, UgCommonInfo);	// UG_DATASET_COMMON   0
	ug_dataset_alloc_list (dataset, UgProxyInfo);	// UG_DATASET_PROXY    1
	ug_dataset_alloc_list (dataset, UgProgressInfo);		// UG_DATASET_PROGRESS 2
	ug_dataset_alloc_list (dataset, UgRelationInfo);		// UG_DATASET_RELATION 3
}

static void	ug_dataset_finalize (UgDataset* dataset)
{
	guint	index;

	if (dataset->destroy.func)
		dataset->destroy.func (dataset->destroy.data);

	for (index = 0;  index < dataset->data_len;  index += 2)
		ug_datalist_free (dataset->data[index]);
	g_free (dataset->data);
}

static void	ug_dataset_assign (UgDataset* dataset, UgDataset* src)
{
	const UgData1Interface*	iface;
	UgDatalist**		data_list;
	guint				index;

	for (index = 0;  index < src->data_len;  index += 2) {
		iface = src->key[index];
		if (iface == NULL || iface->assign == NULL)
			continue;
		// assign list
		data_list = ug_dataset_get_list (dataset, iface);
		if (data_list == NULL)
			data_list = ug_dataset_alloc_list (dataset, iface);
		*data_list = ug_datalist_assign (*data_list, src->data[index]);
	}
}

UgDataset*	ug_dataset_new (void)
{
	return ug_data1_new (&ug_dataset_iface);
}

void	ug_dataset_ref   (UgDataset* dataset)
{
	dataset->ref_count++;
}

void	ug_dataset_unref (UgDataset* dataset)
{
	dataset->ref_count--;
	if (dataset->ref_count == 0)
		ug_data1_free (dataset);
}

// Gets the element at the given position in a list.
gpointer	ug_dataset_get	(UgDataset* dataset, const UgData1Interface* iface, guint nth)
{
	UgDatalist**	list;

	list = ug_dataset_get_list (dataset, iface);
	if (list == NULL)
		return NULL;

	return ug_datalist_nth (*list, nth);
}

void	ug_dataset_remove (UgDataset* dataset, const UgData1Interface* iface, guint nth)
{
	UgDatalist**	list;
	UgDatalist*		link;

	list = ug_dataset_get_list (dataset, iface);
	if (list == NULL || *list == NULL)
		return;

	if (nth == 0) {
		link  = *list;
		*list = link->next;
	}
	else
		link = ug_datalist_nth (*list, nth);

	if (link) {
		ug_datalist_unlink (link);
		ug_data1_free (link);
	}
}

// If nth instance of iface exist, return nth instance.
// If nth instance of iface not exist, alloc new instance in tail and return it.
gpointer	ug_dataset_realloc (UgDataset* dataset, const UgData1Interface* iface, guint nth)
{
	UgDatalist**	list;
	UgDatalist*		link;

//	assert (iface != NULL);

	list = ug_dataset_get_list (dataset, iface);
	if (list == NULL)
		list = ug_dataset_alloc_list (dataset, iface);

	if (*list == NULL) {
		*list = ug_data1_new (iface);
		return *list;
	}

	for (link = *list;  ;  link = link->next, nth--) {
		if (nth == 0)
			return link;
		if (link->next == NULL) {
			link->next = ug_data1_new (iface);
			link->next->prev = link;
			return link->next;
		}
	}
}

gpointer	ug_dataset_alloc_front (UgDataset* dataset, const UgData1Interface* iface)
{
	UgDatalist**	list;
	UgDatalist*		link;

//	assert (iface != NULL);

	list = ug_dataset_get_list (dataset, iface);
	if (list == NULL)
		list = ug_dataset_alloc_list (dataset, iface);

	link = ug_data1_new (iface);
	*list = ug_datalist_prepend (*list, link);

	return link;
}

gpointer	ug_dataset_alloc_back  (UgDataset* dataset, const UgData1Interface* iface)
{
	UgDatalist**	list;
	UgDatalist*		link;

//	assert (iface != NULL);

	list = ug_dataset_get_list (dataset, iface);
	if (list == NULL)
		list = ug_dataset_alloc_list (dataset, iface);

	link  = ug_data1_new (iface);
	*list = ug_datalist_append (*list, link);

	return link;
}


// ----------------------------------------------
// UgDataset list functions
guint	ug_dataset_list_length (UgDataset* dataset, const UgData1Interface* iface)
{
	UgDatalist**	list;

	list = ug_dataset_get_list (dataset, iface);
	return ug_datalist_length (*list);
}

UgDatalist**	ug_dataset_alloc_list (UgDataset* dataset, const UgData1Interface* iface)
{
	guint	data_len = dataset->data_len;

	if (dataset->alloc_len == data_len) {
		dataset->alloc_len += 8 * 2;
		dataset->data = g_realloc (dataset->data, sizeof (gpointer) * dataset->alloc_len);
		dataset->key  = (const UgData1Interface**) dataset->data + 1;
	}
	dataset->key[data_len]  = iface;
	dataset->data[data_len] = NULL;
	dataset->data_len += 2;

	return &dataset->data[data_len];
}

UgDatalist**	ug_dataset_get_list (UgDataset* dataset, const UgData1Interface* iface)
{
	guint	index;

	for (index = 0;  index < dataset->data_len;  index += 2) {
		if (dataset->key[index] == iface)
			return &dataset->data[index];
	}

	return NULL;
}

// free old list in dataset and set list with new_list.
void	ug_dataset_set_list (UgDataset* dataset, const UgData1Interface* iface, gpointer new_list)
{
	UgDatalist**	list;
	UgDatalist*		old_list;

	list = ug_dataset_get_list (dataset, iface);
	if (list == NULL)
		list = ug_dataset_alloc_list (dataset, iface);

	old_list = *list;
	*list = new_list;
	ug_datalist_free (old_list);
}

// Cuts the element at the given position in a list.
gpointer	ug_dataset_cut_list (UgDataset* dataset, const UgData1Interface* iface, guint nth)
{
	UgDatalist**	list;
	UgDatalist*		link;

	list = ug_dataset_get_list (dataset, iface);
	if (list == NULL)
		return NULL;

	if (nth == 0) {
		link = *list;
		*list = NULL;
	}
	else {
		// nth > 0
		link = ug_datalist_nth (*list, nth);
		if (link) {
			((UgDatalist*)link)->prev->next = NULL;
			((UgDatalist*)link)->prev = NULL;
		}
	}

	return link;
}

// ----------------------------------------------------------------------------
// UgMarkup parse/write
static void ug_dataset_parser_start_element (GMarkupParseContext*	context,
                                              const gchar*		element_name,
                                              const gchar**		attr_names,
                                              const gchar**		attr_values,
                                              UgDataset*		dataset,
                                              GError**			error)
{
	const UgData1Interface*	iface;
	UgDatalist*			datalist;
	guint				index;

	if (strcmp (element_name, "DataClass") != 0) {
		g_markup_parse_context_push (context, &ug_markup_skip_parser, NULL);
		return;
	}

	for (index=0; attr_names[index]; index++) {
		if (strcmp (attr_names[index], "name") != 0)
			continue;

		// find registered data interface (UgData1Interface)
		iface = ug_data1_interface_find (attr_values[index]);
		if (iface) {
			// Create new instance by UgData1Interface and prepend it to list.
			datalist = ug_dataset_alloc_front (dataset, iface);
			g_markup_parse_context_push (context, &ug_data1_parser, datalist);
		}
		else {
			// Skip unregistered interface, don't parse anything.
			g_markup_parse_context_push (context, &ug_markup_skip_parser, NULL);
		}
		break;
	}
}

static GMarkupParser	ug_dataset_parser =
{
	(gpointer) ug_dataset_parser_start_element,
	(gpointer) g_markup_parse_context_pop,
	NULL,
	NULL,
	NULL
};

static void	ug_dataset_in_markup (UgDataset* dataset, GMarkupParseContext* context)
{
	g_markup_parse_context_push (context, &ug_dataset_parser, dataset);
}

static void	ug_dataset_to_markup (UgDataset* dataset, UgMarkup* markup)
{
	const UgData1Interface*	iface;
	UgDatalist*			datalist;
	guint				index;

	for (index = 0;  index < dataset->data_len;  index += 2) {
		// output from tail to head
		datalist = ug_datalist_last (dataset->data[index]);
		for (;  datalist;  datalist = datalist->prev) {
			iface = datalist->iface;
			if (iface->entry == NULL)
				continue;
			ug_markup_write_element_start (markup, "DataClass name='%s'", iface->name);
			ug_data1_write_markup ((UgData1*)datalist, markup);
			ug_markup_write_element_end   (markup, "DataClass");
		}
	}
}

