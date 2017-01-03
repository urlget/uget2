/*
 *
 *   Copyright (C) 2005-2017 by C.H. Huang
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

#include "UgData1.h"
#include "UgRegistry1.h"

#if defined(_MSC_VER)
#define strtoll _strtoi64
#endif

// ----------------------------------------------------------------------------
// UgData1Interface
void	ug_data1_interface_register (const UgData1Interface* iface)
{
	gchar*	key;

	key = g_strconcat ("data.", iface->name, NULL);
	ug_registry1_insert (key, iface);
	g_free (key);
}

void	ug_data1_interface_unregister (const UgData1Interface* iface)
{
	gchar*	key;

	key = g_strconcat ("data.", iface->name, NULL);
	ug_registry1_remove (key, iface);
	g_free (key);
}

const UgData1Interface*	ug_data1_interface_find (const gchar* name)
{
	gpointer	iface;
	gchar*		key;

	key = g_strconcat ("data.", name, NULL);
	iface = ug_registry1_find (key);
	g_free (key);

	return iface;
}


// -----------------------------------------------------------------------------
// UgData : UgData is a base structure.

// UgData*	ug_data1_new	(const UgData1Interface* iface)
gpointer	ug_data1_new (const UgData1Interface* iface)
{
	UgInitFunc	init;
	UgData1*	data;

//	data = g_malloc0 (iface->instance_size);
	data = g_slice_alloc0 (iface->instance_size);
	data->iface = iface;

	init = iface->init;
	if (init)
		init (data);
	return data;
}

// void	ug_data1_free (UgData*  data)
void	ug_data1_free (gpointer data)
{
	UgFinalizeFunc	finalize;

	finalize = ((UgData1*)data)->iface->finalize;
	if (finalize)
		finalize (data);
//	g_free (data);
	g_slice_free1 (((UgData1*)data)->iface->instance_size, data);
}

// UgData*	ug_data1_copy (UgData*  data)
gpointer	ug_data1_copy (gpointer data)
{
	const UgData1Interface*	iface;
	UgAssignFunc		assign;
	gpointer			new_data;

	if (data) {
		iface = ((UgData1*)data)->iface;
		assign = iface->assign;
		if (assign) {
//			new_data = g_malloc0 (iface->instance_size);
			new_data = g_slice_alloc0 (iface->instance_size);
			((UgData1*)new_data)->iface = iface;
			assign (new_data, data);
			return new_data;
		}
	}
	return NULL;
}

//void	ug_data1_assign (UgData*  dest, UgData*  src)
void	ug_data1_assign (gpointer data, gpointer src)
{
	UgAssignFunc	assign;

	if (data) {
		assign = ((UgData1*)data)->iface->assign;
		if (assign)
			assign (data, src);
	}
}

// ------------------------------------
// UgData-base XML input and output
static void ug_data1_parser_start_element (GMarkupParseContext*	context,
                                          const gchar*		element_name,
                                          const gchar**		attr_names,
                                          const gchar**		attr_values,
                                          UgData1*			data,
                                          GError**			error);

// UgData*  user_data
GMarkupParser	ug_data1_parser =
{
	(gpointer) ug_data1_parser_start_element,
	(gpointer) g_markup_parse_context_pop,
	NULL, NULL, NULL
};

void	ug_data1_write_markup (UgData1* data, UgMarkup* markup)
{
	const UgDataEntry*	entry;
	union {
		gpointer	src;
		gchar*		v_string;
		gint		v_int;
		guint		v_uint;
		gint64		v_int64;
		gdouble		v_double;
	} value;

	entry = data->iface->entry;
	if (entry == NULL)
		return;

	for (;  entry->name;  entry++) {
		value.src = ((guint8*) data) + entry->offset;

		switch (entry->type) {
		case UG_TYPE_STRING:
			value.v_string = *(gchar**)value.src;
			if (value.v_string) {
				// ug_markup_write_element_start() must use with ug_markup_write_element_end()
				ug_markup_write_element_start (markup, "%s value='%s'", entry->name, value.v_string);
				ug_markup_write_element_end   (markup, entry->name);
			}
			break;

		case UG_TYPE_INT:
			value.v_int = *(gint*)value.src;
//			if (value.v_int) {
				// ug_markup_write_element_start() must use with ug_markup_write_element_end()
				ug_markup_write_element_start (markup, "%s value='%d'", entry->name, value.v_int);
				ug_markup_write_element_end   (markup, entry->name);
//			}
			break;

		case UG_TYPE_UINT:
			value.v_uint = *(guint*)value.src;
//			if (value.v_int) {
				// ug_markup_write_element_start() must use with ug_markup_write_element_end()
				ug_markup_write_element_start (markup, "%s value='%u'", entry->name, value.v_uint);
				ug_markup_write_element_end   (markup, entry->name);
//			}
			break;

		case UG_TYPE_INT64:
			value.v_int64 = *(gint64*)value.src;
//			if (value.v_int64) {
				// ug_markup_write_element_start() must use with ug_markup_write_element_end()
//#if  defined (_MSC_VER)  ||  defined (__MINGW32__)
//				ug_markup_write_element_start (markup, "%s value='%I64d'", entry->name, value.v_int64);
//#else	// C99 Standard
				ug_markup_write_element_start (markup, "%s value='%lld'", entry->name, value.v_int64);
//#endif
				ug_markup_write_element_end   (markup, entry->name);
//			}
			break;

		case UG_TYPE_DOUBLE:
			value.v_double = *(gdouble*)value.src;
//			if (value.v_double) {
				// ug_markup_write_element_start() must use with ug_markup_write_element_end()
				ug_markup_write_element_start (markup, "%s value='%f'", entry->name, value.v_double);
				ug_markup_write_element_end   (markup, entry->name);
//			}
			break;

		case UG_TYPE_INSTANCE:
			value.src = *(gpointer*) value.src;
			if (value.src == NULL)
				break;
		case UG_TYPE_STATIC:
			ug_markup_write_element_start (markup, entry->name);
			ug_data1_write_markup (value.src, markup);
			ug_markup_write_element_end   (markup, entry->name);
			break;

		case UG_TYPE_CUSTOM:
			// ug_markup_write_element_start() must use with ug_markup_write_element_end()
			if (entry->writer) {
				ug_markup_write_element_start (markup, entry->name);
				((UgWriteFunc) entry->writer) (value.src, markup);
				ug_markup_write_element_end   (markup, entry->name);
			}
			break;

		default:
			break;
		}
		// End of switch (entry->type)
	}
}

// UgData*  user_data
static void ug_data1_parser_start_element (GMarkupParseContext*	context,
                                          const gchar*		element_name,
                                          const gchar**		attr_names,
                                          const gchar**		attr_values,
                                          UgData1*			data,
                                          GError**			error)
{
	const UgDataEntry*	entry;
	const gchar*		src;
	gpointer			dest;
	guint				index;

	entry = data->iface->entry;
	if (entry == NULL) {
		// don't parse anything.
		g_markup_parse_context_push (context, &ug_markup_skip_parser, NULL);
		return;
	}

	// data parser
	for (;  entry->name;  entry++) {
		if (strcmp (entry->name, element_name) != 0)
			continue;

		src = NULL;
		for (index=0; attr_names[index]; index++) {
			if (strcmp (attr_names[index], "value") == 0) {
				src = attr_values[index];
				break;
			}
		}
		dest = ((guint8*) data) + entry->offset;

		switch (entry->type) {
		case UG_TYPE_STRING:
			if (src) {
				g_free (*(gchar**) dest);
				*(gchar**) dest = g_strdup (src);
			}
			break;

		case UG_TYPE_INT:
			if (src)
				*(gint*) dest = strtol (src, NULL, 10);
			break;

		case UG_TYPE_UINT:
			if (src)
				*(guint*) dest = (guint) strtoul (src, NULL, 10);
			break;

		case UG_TYPE_INT64:
			// C99 Standard
			if (src)
				*(gint64*) dest = strtoll (src, NULL, 10);;
			break;

		case UG_TYPE_DOUBLE:
			if (src)
				*(gdouble*) dest = strtod (src, NULL);
			break;

		case UG_TYPE_INSTANCE:
			if (entry->parser == NULL)
				break;
			if (*(gpointer*) dest == NULL)
				*(gpointer*) dest = ug_data1_new (entry->parser);
			dest = *(gpointer*) dest;
		case UG_TYPE_STATIC:
			g_markup_parse_context_push (context, &ug_data1_parser, dest);
			return;

		case UG_TYPE_CUSTOM:
			if (entry->parser) {
				((UgParseFunc) entry->parser) (dest, context);
				return;
			}
//			g_set_error (error, G_MARKUP_ERROR, G_MARKUP_ERROR_UNKNOWN_ELEMENT, "Unknow element");
			break;

		default:
			break;
		}
		// End of switch (entry->type)
		break;
	}

	// don't parse anything.
	g_markup_parse_context_push (context, &ug_markup_skip_parser, NULL);
}


// -----------------------------------------------------------------------------
// UgDatalist functions

#define UG_DATALIST_CAST(instance)    ((UgDatalist*)(instance))

void	ug_datalist_free (gpointer datalist)
{
	UgDatalist*		next;

	while (datalist) {
		next = UG_DATALIST_CAST (datalist)->next;
		ug_data1_free (datalist);
		datalist = next;
	}
}

guint	ug_datalist_length (gpointer datalist)
{
	guint	len = 0;

	while (datalist) {
		datalist = UG_DATALIST_CAST (datalist)->next;
		len++;
	}
	return len;
}

gpointer	ug_datalist_first   (gpointer datalist)
{
	if (datalist) {
		while (UG_DATALIST_CAST (datalist)->prev)
			datalist = UG_DATALIST_CAST (datalist)->prev;
	}
	return datalist;
}

gpointer	ug_datalist_last (gpointer datalist)
{
	if (datalist) {
		while (UG_DATALIST_CAST (datalist)->next)
			datalist = UG_DATALIST_CAST (datalist)->next;
	}
	return datalist;
}

gpointer	ug_datalist_nth (gpointer datalist, guint nth)
{
	for (; datalist && nth; nth--)
		datalist = UG_DATALIST_CAST (datalist)->next;
	return datalist;
}

gpointer	ug_datalist_prepend (gpointer datalist, gpointer link)
{
	if (link) {
		if (datalist)
			UG_DATALIST_CAST (datalist)->prev = link;

		UG_DATALIST_CAST (link)->next = datalist;
		return link;
	}

	return datalist;
}

gpointer	ug_datalist_append (gpointer datalist, gpointer link)
{
	UgDatalist*		last_link;

	if (datalist == NULL)
		return link;

	last_link = ug_datalist_last (datalist);
	UG_DATALIST_CAST (last_link)->next = link;
	UG_DATALIST_CAST (link)->prev = last_link;
	return datalist;
}

gpointer	ug_datalist_reverse (gpointer datalist)
{
	UgDatalist*		temp;

	for (temp = datalist;  temp;  ) {
		datalist = temp;
		temp = UG_DATALIST_CAST (datalist)->next;
		UG_DATALIST_CAST (datalist)->next = UG_DATALIST_CAST (datalist)->prev;
		UG_DATALIST_CAST (datalist)->prev = temp;
	}
	return datalist;
}

void	ug_datalist_unlink (gpointer datalink)
{
	if (datalink) {
		if (UG_DATALIST_CAST (datalink)->next)
			UG_DATALIST_CAST (datalink)->next->prev = UG_DATALIST_CAST (datalink)->prev;
		if (UG_DATALIST_CAST (datalink)->prev)
			UG_DATALIST_CAST (datalink)->prev->next = UG_DATALIST_CAST (datalink)->next;
		UG_DATALIST_CAST (datalink)->next = NULL;
		UG_DATALIST_CAST (datalink)->prev = NULL;
	}
}

gpointer	ug_datalist_copy (gpointer datalist)
{
	UgDatalist*		newlist;
	UgDatalist*		newdata;
	UgDatalist*		src;

	newlist = NULL;
	for (src = ug_datalist_last (datalist);  src;  src = src->prev) {
		if (src->iface->assign) {
			newdata = ug_data1_copy (src);
			newlist = ug_datalist_prepend (newlist, newdata);
		}
	}
	return newlist;
}

gpointer	ug_datalist_assign (gpointer datalist, gpointer src)
{
	UgDatalist*		newlink;
	UgDatalist*		srclink;

	for (newlink = datalist, srclink = src;  srclink;  srclink = srclink->next) {
		if (newlink == NULL) {
			newlink = ug_data1_copy (srclink);
			datalist = ug_datalist_append (datalist, newlink);
		}
		else
			ug_data1_assign (newlink, srclink);
		newlink = newlink->next;
	}

	return datalist;
}

