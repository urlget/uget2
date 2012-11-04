/*
 *
 *   Copyright (C) 2005-2014 by C.H. Huang
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

// UgDataset		collection of all UgDatalist-based instance

// UgData
// |
// `- UgDataset

#ifndef UG_DATASET_H
#define UG_DATASET_H

#include <glib.h>
#include "UgData1.h"

#ifdef __cplusplus
extern "C" {
#endif

// These macro is for internal use only.
// use ug_dataset_get(dataset, UgetCommonInfo, 0) to instead UG_DATASET_COMMON(dataset)
#define	UG_DATASET_COMMON(dataset)			( (UgCommon*)((dataset)->data[0]) )
#define	UG_DATASET_PROXY(dataset)			( (UgProxy*) ((dataset)->data[2]) )
#define	UG_DATASET_PROGRESS(dataset)		( (UgProgress*)  ((dataset)->data[4]) )
#define	UG_DATASET_RELATION(dataset)		( (UgRelation*)  ((dataset)->data[6]) )

typedef struct	UgDataset			UgDataset;		// collection of all UgDatalist-based instance

extern const	UgData1Interface		ug_dataset_iface;


// ----------------------------------------------------------------------------
// UgDataset : collection of all UgDatalist-based instance.

struct UgDataset
{
	const UgData1Interface*	iface;		// for UgMarkup parse/write

	UgDatalist**			data;
	const UgData1Interface**	key;
	unsigned int			data_len;
	unsigned int			alloc_len;

	unsigned int			ref_count;

	// call destroy.func(destroy.data) when destroying.
	struct
	{
		UgNotifyFunc	func;
		gpointer		data;
	} destroy;

	struct
	{
		gpointer		pointer;
		gpointer		data;
	} user;
};

UgDataset*	ug_dataset_new   (void);
void		ug_dataset_ref   (UgDataset* dataset);
void		ug_dataset_unref (UgDataset* dataset);

// Gets the element at the given position in a list.
gpointer	ug_dataset_get (UgDataset* dataset, const UgData1Interface* iface, guint nth);

// Removes the element at the given position in a list.
void		ug_dataset_remove (UgDataset* dataset, const UgData1Interface* iface, guint nth);

// If nth instance of data_interface exist, return nth instance.
// If nth instance of data_interface not exist, alloc new instance in tail and return it.
gpointer	ug_dataset_realloc (UgDataset* dataset, const UgData1Interface* iface, guint nth);

gpointer	ug_dataset_alloc_front (UgDataset* dataset, const UgData1Interface* iface);
gpointer	ug_dataset_alloc_back  (UgDataset* dataset, const UgData1Interface* iface);

// ------------------------------------
// UgDataset list functions
guint			ug_dataset_list_length (UgDataset* dataset, const UgData1Interface* iface);

UgDatalist**	ug_dataset_alloc_list (UgDataset* dataset, const UgData1Interface* iface);

UgDatalist**	ug_dataset_get_list (UgDataset* dataset, const UgData1Interface* iface);

// free old list in dataset and set list with new_list.
void			ug_dataset_set_list (UgDataset* dataset, const UgData1Interface* iface, gpointer new_list);

// Cuts the element at the given position in a list.
//UgDatalist*	ug_dataset_cut_list (UgDataset* dataset, const UgData1Interface* iface, guint nth);
gpointer		ug_dataset_cut_list (UgDataset* dataset, const UgData1Interface* iface, guint nth);


#ifdef __cplusplus
}
#endif

#endif  // UG_DATASET_H

