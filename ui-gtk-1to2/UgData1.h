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

#ifndef UG_DATA1_H
#define UG_DATA1_H

#include <glib.h>
#include "UgMarkup.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct	UgDataEntry			UgDataEntry;
typedef struct	UgData1Interface	UgData1Interface;
typedef struct	UgData1				UgData1;
typedef struct	UgDatalist			UgDatalist;

typedef enum	UgType				UgType;

// UgData1Interface
typedef void	(*UgInitFunc)		(void* instance);
typedef void	(*UgFinalizeFunc)	(void* instance);
typedef void	(*UgAssign1Func)	(void* instance, void* src_instance);
typedef void	(*UgParseFunc)		(void* instance, void* user_data);
typedef void	(*UgWriteFunc)		(void* instance, void* user_data);
// UgParseMarkup : how to parse data in markup.
// UgWriteMarkup : how to write data to markup.
typedef void	(*UgParseMarkup)	(void* instance, GMarkupParseContext* context);
typedef void	(*UgWriteMarkup)	(void* instance, UgMarkup* markup);
// notify callback
typedef void	(*UgNotifyFunc)		(void* user_data);

enum	UgType
{
	UG_TYPE_NONE,
	UG_TYPE_STRING,
	UG_TYPE_INT,
	UG_TYPE_UINT,
	UG_TYPE_INT64,
	UG_TYPE_DOUBLE,

	// used by UgDataEntry
	// UgData-based pointer. if pointer is NULL, it use UgData1Interface to create data.
	UG_TYPE_INSTANCE,		// UgDataEntry.parser set to UgData1Interface
	// UgData-based data that must be initialized.
	UG_TYPE_STATIC,

	// User defined type.
	// UgDataEntry.parser set to UgParseFunc (or UgParseMarkup)
	// UgDataEntry.writer set to UgWriteFunc (or UgWriteMarkup)
	// You must use it with UgParseMarkup & UgWriteMarkup in UgDataEntry.
	UG_TYPE_CUSTOM,
};


// ----------------------------------------------------------------------------
// UgDataEntry: defines a single XML element and it's offset of data structure.

//	typedef struct
//	{
//		gchar*		user;
//		gchar*		pass;
//	} Foo;
//
//	static UgDataEntry foo_tags[] =
//	{
//		{ "user",	G_STRUCT_OFFSET (Foo, user),	UG_TYPE_STRING,	NULL,	NULL},
//		{ "pass",	G_STRUCT_OFFSET (Foo, pass),	UG_TYPE_STRING,	NULL,	NULL},
//		{ NULL }
//	};
//
//	<user value='guest3' />
//	<pass value='unknown' />

struct UgDataEntry
{
	char*			name;			// tag name
	int				offset;
	UgType			type;

	const void*		parser;			// How to parse data.
	const void*		writer;			// How to write data.
};


// ----------------------------------------------------------------------------
// UgData1Interface: All UgData-based structure must use it to save and load from XML file.

struct UgData1Interface
{
	unsigned int		instance_size;
	const char*			name;

	const UgDataEntry*	entry;		// To disable file parse/write, set entry = NULL.

	UgInitFunc			init;
	UgFinalizeFunc		finalize;

	UgAssign1Func		assign;		// overwrite dest by src
};

void	ug_data1_interface_register	(const UgData1Interface*	iface);
void	ug_data1_interface_unregister(const UgData1Interface*	iface);
const UgData1Interface*	ug_data1_interface_find	(const gchar* name);


// ----------------------------------------------------------------------------
// UgData1 : UgData is a base structure.
//          It can save and load from XML file by UgDataEntry in UgData1Interface.

struct UgData1
{
	const UgData1Interface*	iface;
};

// ------------------------------------
// UgData*	ug_data1_new	(const UgData1Interface* iface);
// void		ug_data1_free	(UgData*	data);
gpointer	ug_data1_new	(const UgData1Interface* iface);
void		ug_data1_free	(gpointer	data);

// UgData*	ug_data1_copy	(UgData*	data);
//void		ug_data1_assign	(UgData*	data, UgData*	src);
gpointer	ug_data1_copy	(gpointer	data);
void		ug_data1_assign	(gpointer	data, gpointer	src);	// overwrite (or merge)

// ------------------------------------
// XML parse and write
extern	GMarkupParser		ug_data1_parser;		// UgData*   user_data
void	ug_data1_write_markup (UgData1* data, UgMarkup* markup);


// ----------------------------------------------------------------------------
// UgDatalist : UgDatalist is a UgData structure include Doubly-Linked Lists.
//              All UgDatalist-base structure can store in UgDataset.

//  UgData
//  |
//	`- UgDatalist

#define UG_DATALIST_MEMBERS(Type)	\
	const UgData1Interface*	iface;	\
	Type*					next;	\
	Type*					prev

struct UgDatalist
{
	UG_DATALIST_MEMBERS (UgDatalist);
//	const UgData1Interface*	iface;
//	UgDatalist*				next;
//	UgDatalist*				prev;
};

// --- UgDatalist functions are similar to GList functions.
//void		ug_datalist_free	(UgDatalist* datalist);
void		ug_datalist_free	(gpointer datalist);

guint		ug_datalist_length	(gpointer datalist);
gpointer	ug_datalist_first	(gpointer datalist);
gpointer	ug_datalist_last	(gpointer datalist);
gpointer	ug_datalist_nth		(gpointer datalist, guint nth);

gpointer	ug_datalist_prepend	(gpointer datalist, gpointer datalink);
gpointer	ug_datalist_append	(gpointer datalist, gpointer datalink);
gpointer	ug_datalist_reverse	(gpointer datalist);

void		ug_datalist_unlink	(gpointer datalink);

// --- copy UgDatalist to a new UgDatalist if UgData1Interface::assign exist.
gpointer	ug_datalist_copy	(gpointer datalist);
gpointer	ug_datalist_assign	(gpointer datalist, gpointer src);


#ifdef __cplusplus
}
#endif

#endif  // UG_DATA1_H

