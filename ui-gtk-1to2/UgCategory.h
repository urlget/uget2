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


#ifndef UG_CATEGORY_H
#define UG_CATEGORY_H

#include "UgDataset.h"

#ifdef __cplusplus
extern "C" {
#endif

// interface address for UgDataset
#define	UgRelationInfo		ug_relation_iface_pointer

typedef struct	UgCategory          UgCategory;
typedef struct	UgCategoryFuncs     UgCategoryFuncs;
typedef enum	UgCategoryHints     UgCategoryHints;
typedef struct	UgRelation          UgRelation;

typedef void	(*UgCategoryAddFunc)     (UgCategory* category, UgDataset* dataset);
typedef GList*	(*UgCategoryGetAllFunc)  (UgCategory* category);
typedef GList*	(*UgCategoryGetTasksFunc)(UgCategory* category);
typedef void	(*UgCategoryChangedFunc) (UgCategory* category, UgDataset* dataset);

extern const	UgData1Interface		ug_category_iface;
extern const	UgData1Interface		ug_relation_iface;
extern const	UgData1Interface*		ug_category_iface_pointer;
extern const	UgData1Interface*		ug_relation_iface_pointer;


// ----------------------------------------------------------------------------
// UgCategory

// UgData
// |
// `- UgCategory

struct UgCategory
{
	const UgData1Interface*	iface;			// for UgMarkup parse/write

	const UgCategoryFuncs*	funcs;			// functions

	gchar*				name;

	// limit
	guint				active_limit;
	guint				finished_limit;		// finished: completed and paused
	guint				recycled_limit;

	// default setting of UgDataset
	UgDataset*			defaults;

	// used when program save/load file
	GList*				indices;

	// call destroy.func(destroy.data) when destroying.
	struct
	{
		UgNotifyFunc	func;
		gpointer		data;
	} destroy;
};

UgCategory*	ug_category_new  (void);
void		ug_category_free (UgCategory* category);

void		ug_category_init     (UgCategory* category);
void		ug_category_finalize (UgCategory* category);

// add dataset to category and increase reference count of dataset.
void	ug_category_add (UgCategory* category, UgDataset* dataset);

// get all tasks(UgDataset) in this category.
// To free the returned value, use g_list_free (list).
GList*	ug_category_get_all (UgCategory* category);

// get queuing tasks(UgDataset) in this category.
// This function should be noticed UgCategory::active_limit, because
// application will try to activate all returned dataset.
// To free the returned value, use g_list_free (list).
GList*	ug_category_get_tasks (UgCategory* category);

// used to notify category that it's dataset was changed.
// It may change hints and switch dataset to another internal queue of category.
void	ug_category_changed (UgCategory* category, UgDataset* dataset);


// ------------------------------------
// UgCategoryHints

enum	UgCategoryHints
{
//	UG_HINT_QUEUING				= 1 << 0,
	UG_HINT_PAUSED				= 1 << 1,
	UG_HINT_DOWNLOADING			= 1 << 2,
	UG_HINT_ERROR				= 1 << 3,
	UG_HINT_COMPLETED			= 1 << 4,	// Download completed only
	UG_HINT_UPLOADING			= 1 << 5,	// reserved

	UG_HINT_FINISHED			= 1 << 6,	// Download completed, uget will not use it in future.
	UG_HINT_RECYCLED			= 1 << 7,	// Download will be deleted.

	UG_HINT_ACTIVE				= UG_HINT_DOWNLOADING | UG_HINT_UPLOADING,
	UG_HINT_INACTIVE			= UG_HINT_PAUSED | UG_HINT_ERROR,
	UG_HINT_UNRUNNABLE			= UG_HINT_PAUSED | UG_HINT_ERROR | UG_HINT_FINISHED | UG_HINT_RECYCLED,
};


// ----------------------------------------------------------------------------
// CategoryList

// Before calling ug_category_list_load(), user must register data interface of UgCategory.
GList* ug_category_list_load (const gchar* filename);
void   ug_category_list_link (GList* category_list, GList* download_list);

// ----------------------------------------------------------------------------
// DownloadList
// ug_download_list_load() load file and return list of newly-created UgDataset.
// To free the return value, use:
//	g_list_foreach (list, (GFunc) ug_dataset_unref, NULL);
//	g_list_free (list);
// Before calling ug_download_list_load(), user must register data interface of UgetRelation.
GList*		ug_download_list_load (const gchar* filename);
// Below utility functions can be used by g_list_foreach()
gboolean	ug_download_create_attachment (UgDataset* dataset, gboolean force);
gboolean	ug_download_assign_attachment (UgDataset* dataset, UgDataset* src);
void		ug_download_complete_data (UgDataset* dataset);
void		ug_download_delete_temp (UgDataset* dataset);


// ----------------------------------------------------------------------------
// UgetRelation : relation of UgCategory, UgDataset, and UgPlugin.

// UgData
// |
// `- UgDatalist
//    |
//    `- UgRelation

struct UgRelation
{
	UG_DATALIST_MEMBERS (UgRelation);
//	const UgData1Interface*	iface;
//	UgRelation*				next;
//	UgRelation*				prev;

	// category
	UgCategoryHints		hints;
	UgCategory*			category;

	// use index when program save/load file.
	guint				index;

	// attachment
	struct
	{
		gchar*			folder;
		guint			stamp;
	} attached;

	// call destroy.func(destroy.data) when destroying.
	struct
	{
		UgNotifyFunc	func;
		gpointer		data;
	} destroy;
};


#ifdef __cplusplus
}
#endif

#endif  // UG_CATEGORY_H

