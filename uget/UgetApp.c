/*
 *
 *   Copyright (C) 2012-2017 by C.H. Huang
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

#include <UgString.h>
#include <UgUri.h>
#include <UgUtil.h>
#include <UgFileUtil.h>
#include <UgStdio.h>
#include <UgJsonFile.h>
#include <UgetApp.h>
#include <UgetData.h>

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#if defined _WIN32 || defined _WIN64
#include <windows.h> // Sleep()
#define  ug_sleep       Sleep
#else
#include <unistd.h>  // sleep(), usleep()
#define  ug_sleep(millisecond)    usleep (millisecond * 1000)
#endif // _WIN32 || _WIN64

#ifdef HAVE_GLIB
#include <glib/gi18n.h>
#else
#define _(x)   x
#endif

static struct UgetNodeNotification  notification_real =
		{NULL, NULL, NULL, NULL,
		(UgCompareFunc) NULL, FALSE,
		NULL};
static struct UgetNodeNotification  notification_split =
		{uget_node_create_split, NULL, NULL, NULL,
		(UgCompareFunc) NULL, FALSE,
		NULL};
static struct UgetNodeNotification  notification_sorted =
		{uget_node_create_sorted, NULL, NULL, NULL,
		(UgCompareFunc) NULL, FALSE,
		NULL};
static struct UgetNodeNotification  notification_mix =
		{uget_node_create_mix, NULL, NULL, NULL,
		(UgCompareFunc) NULL, FALSE,
		NULL};
static struct UgetNodeNotification  notification_mix_split =
		{uget_node_create_mix_split, NULL, NULL, NULL,
		(UgCompareFunc) NULL, FALSE,
		NULL};


void  uget_app_init (UgetApp* app)
{
	UgetNode*  node;

	// real and virtual root nodes
	uget_node_init (&app->real, NULL);
	uget_node_init (&app->split, &app->real);
	uget_node_init (&app->sorted, &app->real);
	uget_node_init (&app->mix, &app->real);
	uget_node_init (&app->mix_split, &app->mix);
	app->real.notification = &notification_real;
	app->split.notification = &notification_split;
	app->sorted.notification = &notification_sorted;
	app->mix.notification = &notification_mix;
	app->mix_split.notification = &notification_mix_split;
	// add virtual category - "All Category"
	node = uget_node_new (NULL);
	node->name = ug_strdup (_("All Category"));
	uget_node_append (&app->mix, node);

	uget_task_init (&app->task);
	ug_array_init (&app->nodes, sizeof (void*), 32);

	// plug-in registry
	app->plugin_default = NULL;
	ug_registry_init (&app->plugins);
	// info registry
	ug_registry_init (&app->infos);
	ug_registry_add (&app->infos, UgetCommonInfo);
	ug_registry_add (&app->infos, UgetProgressInfo);
	ug_registry_add (&app->infos, UgetProxyInfo);
	ug_registry_add (&app->infos, UgetHttpInfo);
	ug_registry_add (&app->infos, UgetFtpInfo);
	ug_registry_add (&app->infos, UgetLogInfo);
	ug_registry_add (&app->infos, UgetRelationInfo);
	ug_registry_add (&app->infos, UgetCategoryInfo);
	ug_registry_sort (&app->infos);
	ug_info_set_registry (&app->infos);
}

void  uget_app_final (UgetApp* app)
{
	ug_array_clear (&app->nodes);
	uget_task_final (&app->task);
	uget_app_clear_plugins (app);

	uget_node_unref_children (&app->mix_split);
	uget_node_unref_children (&app->mix);
	uget_node_unref_children (&app->sorted);
	uget_node_unref_children (&app->split);
	uget_node_unref_children (&app->real);

	ug_registry_final (&app->plugins);
	ug_registry_final (&app->infos);

	uget_uri_hash_free (app->uri_hash);
	app->uri_hash = NULL;
}

static UgArrayPtr* uget_app_store_nodes (UgetApp* app, UgetNode* parent)
{
	UgetNode*   dnode;
	UgArrayPtr* array;
	int         index;

	array = &app->nodes;
//	array->length = 0;
	ug_array_alloc (array, parent->n_children);
	for (index = 0, dnode = parent->children;  dnode;  dnode = dnode->next)
		array->at[index++] = dnode->data;

	return array;
}

static void     uget_app_clear_nodes (UgetApp* app)
{
	app->nodes.length = 0;
}

static int  uget_app_activate (UgetApp* app, UgetNode* cnode, UgetCategory* category)
{
	UgetNode*   dnode;
	UgetNode*   sibling;
	UgetLog*    log;
	UgArrayPtr* array;
	int         index;

	// Because this function will change node linking,
	// program must store active nodes to array.
	array = uget_app_store_nodes (app, category->active);

	for (index = 0;  index < array->length;  index++) {
		dnode = array->at[index];
		uget_node_updated (dnode);
		if (dnode->state & UGET_STATE_ACTIVE) {
			// remove node and insert it again to sort node
			if (app->mix.notification->compare) {
				sibling = dnode->next;
				uget_node_remove (cnode, dnode);
				uget_node_unref_fake (dnode);
				uget_node_insert (cnode, sibling, dnode);
				app->n_moved++;
			}
			continue;
		}

		uget_task_remove (&app->task, dnode);
		uget_node_remove (cnode, dnode);
		uget_node_unref_fake (dnode);
		if (dnode->state & UGET_STATE_COMPLETED) {
			dnode->state |= UGET_STATE_FINISHED;
			sibling = category->finished->children;
			// completed time
			log = ug_info_realloc (&dnode->info, UgetLogInfo);
			log->completed_time = time(NULL);    // get current time
			app->n_completed++;
		}
		else {
			dnode->state |= UGET_STATE_QUEUING;
			sibling = category->queuing->children;
			if (dnode->state & UGET_STATE_ERROR)
				app->n_error++;
		}

		// try to insert download before finished & recycled
		if (sibling == NULL)
			sibling = category->finished->children;
		if (sibling == NULL)
			sibling = category->recycled->children;
		// get real sibling
		if (sibling)
			sibling = sibling->real;
		uget_node_insert (cnode, sibling, dnode);
		app->n_moved++;
	}

	uget_app_clear_nodes (app);    // clear stored nodes
	return category->active->n_children;
}

static void uget_app_queuing (UgetApp* app, UgetNode* cnode, UgetCategory* category)
{
	UgetNode*   dnode;
	UgArrayPtr* array;
	int         index;

	// Because uget_app_activate_download() will change node linking,
	// program must store queuing nodes to array before calling uget_app_activate_download()
	array = uget_app_store_nodes (app, category->queuing);

	for (index = 0;  index < array->length;  index++) {
		dnode = array->at[index];
		if (category->active->n_children >= category->active_limit)
			break;
		if (dnode->state & UGET_STATE_INACTIVE)
			continue;
		uget_app_activate_download (app, dnode);
		app->n_moved++;
	}

	uget_app_clear_nodes (app);    // clear stored nodes
}

// return number of active download
int  uget_app_grow (UgetApp* app, int no_queuing)
{
	UgetCategory*  category;
	UgetNode*      cnode;
	UgetNode*      dnode;
	int            n_active = 0;

	// dispatch plug-in event, calc speed
	uget_task_dispatch (&app->task);
	// active, queuing, finished, recycled
	for (cnode = app->real.children;  cnode;  cnode = cnode->next) {
		category = ug_info_realloc (&cnode->info, UgetCategoryInfo);
		if (category == NULL)
			continue;
		uget_app_activate (app, cnode, category);
		if (no_queuing == FALSE)
			uget_app_queuing (app, cnode, category);
		while (category->finished->n_children > category->finished_limit) {
			dnode = category->finished->last->real;
			uget_uri_hash_remove_download (app->uri_hash, dnode);
			uget_node_remove (cnode, dnode);
			uget_node_unref (dnode);
			app->n_deleted++;
		}
		while (category->recycled->n_children > category->recycled_limit) {
			dnode = category->recycled->last->real;
			uget_uri_hash_remove_download (app->uri_hash, dnode);
			uget_node_remove (cnode, dnode);
			uget_node_unref (dnode);
			app->n_deleted++;
		}
		n_active += category->active->n_children;
	}

	return n_active;
}

void  uget_app_set_config_dir (UgetApp* app, const char* dir)
{
	ug_free (app->config_dir);
	app->config_dir = (dir) ? ug_strdup(dir) : NULL;
}

void  uget_app_set_sorting (UgetApp* app, UgCompareFunc compare, int reversed)
{
	UgetNode*  node;
	UgetNode*  real;

	node = app->mix.children;
	if (app->mix.notification->reversed != reversed) {
		app->mix.notification->reversed  = reversed;
		app->mix_split.notification->reversed = reversed;
		app->sorted.notification->reversed = reversed;
		if (app->mix.notification->compare == compare && compare) {
			// reverse first category in app->mix
			ug_node_reverse ((UgNode*) node);
			// reverse each category in app->mix_split
			for (node = app->mix_split.children;  node;  node = node->next)
				ug_node_reverse ((UgNode*) node);
			// reverse each category in app->sorted
			for (node = app->sorted.children;  node;  node = node->next)
				ug_node_reverse ((UgNode*) node);
			return;
		}
	}

	if (app->mix.notification->compare != compare) {
		app->mix.notification->compare  = compare;
		app->mix_split.notification->compare = compare;
		app->sorted.notification->compare = compare;
		if (compare == NULL) {
			// reorder first category in app->mix
			for (real = app->real.last;  real;  real = real->prev)
				uget_node_reorder_by_real (node, real);
			// reorder each category in app->mix_split
			for (node = app->mix_split.children;  node;  node = node->next) {
				uget_node_reorder_by_real (node, NULL);
				// reorder app->mix by state
				uget_node_reorder_by_fake (app->mix.children, node);
			}
			// reorder each category in app->sorted
			for (node = app->sorted.children;  node;  node = node->next)
				uget_node_reorder_by_real (node, NULL);
		}
		else {
			// sort first category in app->mix
			uget_node_sort (node, compare, reversed);
			// reorder each category in app->mix_split
			for (node = app->mix_split.children;  node;  node = node->next)
				uget_node_reorder_by_real (node, NULL);
			// sort each category in app->sorted
			for (node = app->sorted.children;  node;  node = node->next)
				uget_node_sort (node, compare, reversed);
		}
	}
}

void  uget_app_set_notification (UgetApp* app, void* data,
                                 UgetNodeFunc inserted,
                                 UgetNodeFunc removed,
                                 UgNotifyFunc updated)
{
	notification_real.inserted = inserted;
	notification_real.removed  = removed;
	notification_real.updated  = updated;
	notification_real.data     = data;

	notification_split.inserted = inserted;
	notification_split.removed  = removed;
	notification_split.updated  = updated;
	notification_split.data     = data;

	notification_sorted.inserted = inserted;
	notification_sorted.removed  = removed;
	notification_sorted.updated  = updated;
	notification_sorted.data     = data;

	notification_mix.inserted = inserted;
	notification_mix.removed  = removed;
	notification_mix.updated  = updated;
	notification_mix.data     = data;

	notification_mix_split.inserted = inserted;
	notification_mix_split.removed  = removed;
	notification_mix_split.updated  = updated;
	notification_mix_split.data     = data;
}

void  uget_app_add_category (UgetApp* app, UgetNode* cnode, int save_file)
{
	UgetCategory*  category;
	UgetNode*      node;
	char*          path_base;
	char*          path;

	cnode->type = UGET_NODE_CATEGORY;
	uget_node_append (&app->real, cnode);
	uget_uri_hash_add_category (app->uri_hash, cnode);
	category = ug_info_realloc (&cnode->info, UgetCategoryInfo);
	for (node = cnode->fake;  node;  node = node->peer) {
		switch (node->state) {
		case UGET_STATE_ACTIVE:
			category->active = node;
			break;

		case UGET_STATE_QUEUING:
			category->queuing = node;
			break;

		case UGET_STATE_FINISHED:
			category->finished = node;
			break;

		case UGET_STATE_RECYCLED:
			category->recycled = node;
			break;

		default:
			break;
		}
	}

	// save new category
	if (save_file) {
		path_base = ug_build_filename (app->config_dir, "category", NULL);
#if defined _WIN32 || defined _WIN64
		path = ug_strdup_printf ("%s%c%.4d.json", path_base, '\\',
		                         uget_node_child_position (&app->real, cnode));
#else
		path = ug_strdup_printf ("%s%c%.4d.json", path_base, '/',
		                         uget_node_child_position (&app->real, cnode));
#endif // defined
		uget_app_save_category ((UgetApp*) app, cnode, path);
		ug_free (path_base);
		ug_free (path);
	}
}

int  uget_app_move_category (UgetApp* app, UgetNode* cnode, UgetNode* position)
{
	char* path1;
	char* path2;
	char* path3;
	char* path_base;
	int   from_nth;
	int   to_nth;

	from_nth = uget_node_child_position (&app->real, cnode);
	if (position)
		to_nth = uget_node_child_position (&app->real, position);
	else
		to_nth = app->real.n_children - 1;
	if (from_nth == -1 || to_nth == -1)
		return FALSE;
	uget_node_move (&app->real, position, cnode);

	if (app->config_dir == NULL)
		path_base = ug_strdup ("category");
	else
		path_base = ug_build_filename (app->config_dir, "category", NULL);

#if defined _WIN32 || defined _WIN64
	path1 = ug_strdup_printf ("%s%c%.4d.json", path_base, '\\', from_nth);
	path2 = ug_strdup_printf ("%s%c%.4d.json", path_base, '\\', to_nth);
	path3 = ug_strdup_printf ("%s%cTemp.json", path_base, '\\');
#else
	path1 = ug_strdup_printf ("%s%c%.4d.json", path_base, '/', from_nth);
	path2 = ug_strdup_printf ("%s%c%.4d.json", path_base, '/', to_nth);
	path3 = ug_strdup_printf ("%s%cTemp.json", path_base, '/');
#endif // _WIN32 || _WIN64

	ug_rename (path1, path3);
	ug_rename (path2, path1);
	ug_rename (path3, path2);
	ug_free (path1);
	ug_free (path2);
	ug_free (path3);
	ug_free (path_base);

	return TRUE;
}

void  uget_app_delete_category (UgetApp* app, UgetNode* cnode)
{
	char* path1;
	char* path2;
	char* path_base;
	int   position;
	int   count;

	position = ug_node_child_position ((UgNode*)&app->real, (UgNode*)cnode);
	if (position == -1)
		return;

	uget_app_stop_category (app, cnode);
	uget_uri_hash_remove_category (app->uri_hash, cnode);
	uget_node_remove (&app->real, cnode);
	uget_node_unref (cnode);

	if (app->config_dir == NULL)
		path_base = ug_strdup ("category");
	else
		path_base = ug_build_filename (app->config_dir, "category", NULL);

	for (count = position;  ; count++) {
#if defined _WIN32 || defined _WIN64
		path1 = ug_strdup_printf ("%s%c%.4d.json", path_base, '\\', count);
		path2 = ug_strdup_printf ("%s%c%.4d.json", path_base, '\\', count+1);
#else
		path1 = ug_strdup_printf ("%s%c%.4d.json", path_base, '/',  count);
		path2 = ug_strdup_printf ("%s%c%.4d.json", path_base, '/',  count+1);
#endif // _WIN32 || _WIN64

		ug_unlink (path1);
		if (ug_rename (path2, path1) == -1) {
			ug_free (path1);
			ug_free (path2);
			break;
		}

		ug_free (path1);
		ug_free (path2);
	}

	ug_free (path_base);
}

void  uget_app_stop_category (UgetApp* app, UgetNode* cnode)
{
	UgetCategory*  category;
	UgetNode*      dnode;
	UgArrayPtr*    array;
	int            index;

	category = ug_info_realloc (&cnode->info, UgetCategoryInfo);

	// Because uget_app_queue_download() will change node linking,
	// program must store active nodes to array before calling uget_app_queue_download()
	array = uget_app_store_nodes (app, category->active);

	for (index = array->length-1;  index >= 0;  index--) {
		dnode = array->at[index];
		uget_app_queue_download (app, dnode);
	}

	uget_app_clear_nodes (app);    // clear stored nodes
}

static int  ug_match_file_exts (const char* file, char** exts)
{
	const char* beg = NULL;
	const char* end;
	int         index;

	// get file ext
	for (end = file + strlen (file) - 1;  end >= file;  end--) {
		if (end[0] == '.') {
			beg = end + 1;   // + '.'
			break;
		}
	}

	if (beg == NULL)
		return -1;

	for (index = 0;  *exts;  exts++, index++) {
		if (strcasecmp (beg, *exts) == 0)
			return index;
	}
	return -1;
}

UgetNode* uget_app_match_category (UgetApp* app, UgUri* uuri, const char* file)
{
	UgetCategory* category;
	UgetNode*     cnode;
	int           count;
	struct {
		UgetNode* cnode;
		int       count;
	} matched;

	matched.cnode = NULL;
	matched.count = 0;

	for (cnode = app->real.children;  cnode;  cnode = cnode->next) {
		category = ug_info_realloc (&cnode->info, UgetCategoryInfo);
		if (category == NULL)
			continue;
		// null-terminated
		*(char**)ug_array_alloc (&category->hosts, 1) = NULL;
		*(char**)ug_array_alloc (&category->schemes, 1) = NULL;
		*(char**)ug_array_alloc (&category->file_exts, 1) = NULL;
		category->hosts.length--;
		category->schemes.length--;
		category->file_exts.length--;
		// match download and category
		count = 0;
		if (ug_uri_match_hosts (uuri, category->hosts.at) >= 0)
			count++;
		if (ug_uri_match_schemes (uuri, category->schemes.at) >= 0)
			count++;
		if (ug_uri_match_file_exts (uuri, category->file_exts.at) >= 0)
			count++;
		else if (file && ug_match_file_exts (file, category->file_exts.at) >= 0)
			count++;

		if (matched.count < count) {
			matched.count = count;
			matched.cnode = cnode;
			if (matched.count == 3)
				break;
		}
	}

	return matched.cnode;
}

int  uget_app_add_download_uri (UgetApp* app, const char* uri, UgetNode* cnode, int apply)
{
	UgetNode*   dnode;
	UgetCommon* common;

	dnode = uget_node_new (NULL);
	common = ug_info_realloc (&dnode->info, UgetCommonInfo);
	common->uri = ug_strdup (uri);
	return uget_app_add_download (app, dnode, cnode, apply);
}

int  uget_app_add_download (UgetApp* app, UgetNode* dnode, UgetNode* cnode, int apply)
{
	UgetNode*     sibling;
	UgetLog*      log;
	UgUri         uuri;
	int           value;
	struct {
		UgetCommon*   common;
		UgetCategory* category;
	} temp;

	temp.common = ug_info_realloc (&dnode->info, UgetCommonInfo);
	// replace invalid characters \/:*?"<>| by _ in filename.
	if (temp.common->file)
		ug_str_replace_chars (temp.common->file, "\\/:*?\"<>|", '_');
	// decode name, filename, and category
	if (temp.common->uri) {
		ug_uri_init (&uuri, temp.common->uri);
		// assign node name if it's name is NULL
		if (dnode->name == NULL) {
			if (temp.common->file)
				dnode->name = ug_strdup (temp.common->file);
			else
				uget_node_set_name_by_uri (dnode, &uuri);
		}
		if (cnode == NULL)
			cnode = uget_app_match_category (app, &uuri, temp.common->file);
	}

	if (cnode == NULL)
		cnode = app->real.children;
	if (cnode) {
		dnode->type   = UGET_NODE_DOWNLOAD;
		dnode->state &= UGET_STATE_CATEGORY | UGET_STATE_PAUSED;
		dnode->state |= UGET_STATE_QUEUING;
		log = ug_info_realloc (&dnode->info, UgetLogInfo);
		log->added_time = time (NULL);    // get current time
		if (apply) {
			value = temp.common->keeping.enable;
			temp.common->keeping.enable = TRUE;
			temp.common->keeping.uri = TRUE;
			ug_info_assign (&dnode->info, &cnode->info, UgetCategoryInfo);
			temp.common->keeping.enable = value;
			if (cnode->state & UGET_STATE_PAUSED)
				dnode->state |= UGET_STATE_PAUSED;
		}
		temp.category = ug_info_realloc (&cnode->info, UgetCategoryInfo);
		// try to insert download before finished and recycled
		sibling = temp.category->finished->children;
		if (sibling == NULL)
			sibling = temp.category->recycled->children;
		// get real sibling
		if (sibling)
			sibling = sibling->real;
		uget_node_insert (cnode, sibling, dnode);
		uget_uri_hash_add_download (app->uri_hash, dnode);
		return TRUE;
	}
	return FALSE;
}

int   uget_app_move_download (UgetApp* app, UgetNode* dnode, UgetNode* dnode_position)
{
	UgetNode*  cnode;

	cnode = dnode->parent;
	if (dnode_position) {
		if (cnode != dnode_position->parent)
			return FALSE;
	}
	else if (dnode->next == NULL) {
		return FALSE;
	}

	uget_node_move (cnode, dnode_position, dnode);
	return TRUE;
}

int   uget_app_move_download_to (UgetApp* app, UgetNode* dnode, UgetNode* cnode)
{
	UgetNode*      sibling;
	UgetCategory*  category;

	if (dnode->parent == cnode)
		return FALSE;
	category = ug_info_realloc (&cnode->info, UgetCategoryInfo);

	switch (dnode->state & UGET_STATE_CATEGORY) {
	case UGET_STATE_ACTIVE:
		sibling = category->queuing->children;
		if (sibling == NULL)
			sibling = category->finished->children;
		if (sibling == NULL)
			sibling = category->recycled->children;
		break;

	case UGET_STATE_QUEUING:
	default:
		sibling = category->finished->children;
		if (sibling == NULL)
			sibling = category->recycled->children;
		break;

	case UGET_STATE_FINISHED:
		sibling = category->recycled->children;
		break;

	case UGET_STATE_RECYCLED:
		sibling = NULL;
		break;
	}
	if (sibling)
		sibling = sibling->real;

	uget_node_remove (dnode->parent, dnode);
	uget_node_unref_fake (dnode);
	uget_node_insert (cnode, sibling, dnode);
	return TRUE;
}

static int  delete_dnode_files (UgetNode* dnode, int  has_aria2_file)
{
	UgetNode*  fnode;   // filenode
	UgetNode*  fnode_next;
	int        error;
	int        error_count = 0;

	if (has_aria2_file == TRUE) {
		// move first aria2 to tail
		for (fnode = dnode->children;  fnode;  fnode = fnode_next) {
			fnode_next = fnode->next;
			if (strstr (fnode->name, ".aria2")) {
				uget_node_move (dnode, NULL, fnode);
				break;
			}
		}
	}

	for (fnode = dnode->children;  fnode;  fnode = fnode_next) {
		fnode_next = fnode->next;

		if (fnode->name == NULL) {
			uget_node_remove (dnode, fnode);
			continue;
		}
		if (ug_file_is_exist (fnode->name) == FALSE) {
			uget_node_remove (dnode, fnode);
			continue;
		}

		error = ug_remove (fnode->name);
		if (error != 0)
			error_count++;
		else if (error_count == 0 || strstr (fnode->name, ".aria2") == NULL)
			uget_node_remove (dnode, fnode);
	}

	if (dnode->children == NULL)
		return TRUE;
	else
		return FALSE;
}

static UG_THREAD_RETURN_TYPE  delete_file_thread (UgetNode* dnode)
{
	int  count;

	for (count = 0;  count < 3;  count++) {
		ug_sleep (2 * 1000);    // sleep 2 seconds
		delete_dnode_files (dnode, FALSE);
		if (dnode->children == NULL)
			break;
	}

	uget_node_unref (dnode);
	return UG_THREAD_RETURN_VALUE;
}

int  uget_app_delete_download (UgetApp* app, UgetNode* dnode, int delete_file)
{
	UgetNode*  cnode;
	UgThread   thread;
	int        is_active;

	cnode = dnode->parent;
	is_active = uget_task_remove (&app->task, dnode);
	uget_node_remove (cnode, dnode);
	uget_node_unref_fake (dnode);
	uget_uri_hash_remove_download (app->uri_hash, dnode);

	if (delete_file == TRUE) {
		if (is_active == TRUE || delete_dnode_files (dnode, TRUE) == FALSE) {
			ug_thread_create (&thread, (UgThreadFunc) delete_file_thread, dnode);
			ug_thread_unjoin (&thread);
			return FALSE;
		}
	}

	uget_node_unref (dnode);
	return TRUE;
}

int  uget_app_recycle_download (UgetApp* app, UgetNode* dnode)
{
	UgetNode*  cnode;
	UgetNode*  sibling;
	UgetCategory* category;

	cnode = dnode->parent;
	uget_task_remove (&app->task, dnode);
	uget_node_remove (cnode, dnode);

	if (dnode->state & UGET_STATE_RECYCLED) {
		uget_node_unref (dnode);
		return FALSE;
	}
	else {
		dnode->state &= ~UGET_STATE_CATEGORY;
		dnode->state |=  UGET_STATE_RECYCLED;
		uget_node_unref_fake (dnode);
		category = ug_info_realloc (&cnode->info, UgetCategoryInfo);
		// try to insert download before recycled
		sibling = category->recycled->children;
		if (sibling)
			sibling = sibling->data;
		uget_node_insert (cnode, sibling, dnode);
		return TRUE;
	}
}

int   uget_app_activate_download (UgetApp* app, UgetNode* dnode)
{
	UgetLog*         log;
	UgetNode*        cnode;
	UgetNode*        sibling;
	union {
		UgetCategory*    category;
		UgetPluginInfo*  pinfo;
	} temp;

	if (dnode->state & UGET_STATE_ACTIVE)
		return FALSE;
	// match plug-in
	log = ug_info_realloc (&dnode->info, UgetLogInfo);
	temp.pinfo = uget_app_match_plugin (app, dnode, NULL);
	if (temp.pinfo == NULL) {
		// no plug-in support
		uget_app_queue_download (app, dnode);
		dnode->state |= UGET_STATE_ERROR;
		ug_list_prepend (&log->messages,
				(UgLink*) uget_event_new_error (
						UGET_EVENT_ERROR_UNSUPPORTED_SCHEME, NULL));
		uget_node_updated (dnode);
		return FALSE;
	}
	else {
		// clear event message before starting
		ug_list_foreach (&log->messages,
		                 (UgForeachFunc) uget_event_free, NULL);
		log->messages.size = 0;
		log->messages.head = NULL;
		log->messages.tail = NULL;
	}
	// start node with plug-in
	cnode = dnode->parent;
	if (uget_task_add (&app->task, dnode, temp.pinfo) == FALSE) {
		// plug-in start failed.
		uget_app_queue_download (app, dnode);
		dnode->state |=  UGET_STATE_ERROR;
		uget_node_updated (dnode);
		return FALSE;
	}
	// change node state and move node position
	uget_node_remove (cnode, dnode);
	uget_node_unref_fake (dnode);
	dnode->state =  UGET_STATE_ACTIVE;
	temp.category = ug_info_realloc (&cnode->info, UgetCategoryInfo);
	// try to insert download before queuing, finished, and recycled
	sibling = temp.category->queuing->children;
	if (sibling == NULL)
		sibling = temp.category->finished->children;
	if (sibling == NULL)
		sibling = temp.category->recycled->children;
	// get real sibling
	if (sibling)
		sibling = sibling->real;
	uget_node_insert (cnode, sibling, dnode);
	return TRUE;
}

int   uget_app_pause_download (UgetApp* app, UgetNode* dnode)
{
#if 0
	UgetCategory* category;
	UgetNode*     sibling;
	UgetNode*     cnode;
	UgetNode*     fake;

	if (dnode->state & UGET_STATE_UNRUNNABLE)
		return FALSE;
	uget_task_remove (&app->task, dnode);
	dnode->state |= UGET_STATE_PAUSED;

	cnode = dnode->parent;
	category = ug_info_realloc (&cnode->info, UgetCategoryInfo);
	for (fake = dnode->fake;  fake;  fake = fake->peer) {
		if (fake->parent == category->active) {
			uget_node_remove (cnode, dnode);
			uget_node_unref_fake (dnode);

			dnode->state |= UGET_STATE_QUEUING;
			// try to insert download before queuing, finished, and recycled
			sibling = category->queuing->children;
			if (sibling == NULL)
				sibling = category->finished->children;
			if (sibling == NULL)
				sibling = category->recycled->children;
			// get real sibling
			if (sibling)
				sibling = sibling->real;
			uget_node_insert (cnode, sibling, dnode);
			break;
		}
	}
#else
	if (dnode->state & UGET_STATE_ACTIVE) {
//		uget_task_remove (&app->task, dnode);
		UgetRelation*  relation;
		relation = ug_info_get (&dnode->info, UgetRelationInfo);
		if (relation && relation->task.plugin)
			uget_plugin_stop (relation->task.plugin);
		dnode->state &= ~UGET_STATE_ACTIVE;
	}
	else if (dnode->state & UGET_STATE_UNRUNNABLE)
		return FALSE;
	dnode->state |= UGET_STATE_PAUSED;
#endif
	return TRUE;
}

int   uget_app_queue_download (UgetApp* app, UgetNode* dnode)
{
	UgetNode*     cnode;
	UgetNode*     sibling;
	UgetCategory* category;

	if ((dnode->state & UGET_STATE_ACTIVE)     == 0 &&
		(dnode->state & UGET_STATE_UNRUNNABLE) == 0)
		return FALSE;

	if (dnode->state & UGET_STATE_QUEUING)
		dnode->state = UGET_STATE_QUEUING;
	else {
		cnode = dnode->parent;
		uget_node_remove (cnode, dnode);
		uget_node_unref_fake (dnode);

		// --- decide sibling position & insert before it ---
		// if current download is in active, insert it before queuing,
		// otherwise insert it before finished and recycled.
		category = ug_info_realloc (&cnode->info, UgetCategoryInfo);
		sibling = NULL;
		if (dnode->state & UGET_STATE_ACTIVE) {
			uget_task_remove (&app->task, dnode);
			sibling = category->queuing->children;
		}
		if (sibling == NULL)
			sibling = category->finished->children;
		if (sibling == NULL)
			sibling = category->recycled->children;
		// get real sibling
		if (sibling)
			sibling = sibling->real;
		dnode->state = UGET_STATE_QUEUING;
		uget_node_insert (cnode, sibling, dnode);
	}
	return TRUE;
}

void  uget_app_reset_download_name (UgetApp* app, UgetNode* dnode)
{
	UgetCommon*  common;
	UgetNode*    sibling;
	UgetNode*    cnode = NULL;
	UgUri        uuri;

	common = ug_info_realloc (&dnode->info, UgetCommonInfo);
	if (common->file) {
		if (dnode->name && strcmp (common->file, dnode->name) == 0)
			return;
		ug_free (dnode->name);
		dnode->name = ug_strdup (common->file);
		cnode = dnode->parent;
	}
	else if (common->uri) {
		if (dnode->name && strcmp (common->uri, dnode->name) == 0)
			return;
		ug_uri_init (&uuri, common->uri);
		uget_node_set_name_by_uri (dnode, &uuri);
		cnode = dnode->parent;
	}

	// reinsert && resort fake nodes
	if (cnode) {
//		cnode   = dnode->parent;
		sibling = dnode->next;
		uget_node_remove (cnode, dnode);
		uget_node_unref_fake (dnode);
		uget_node_insert (cnode, sibling, dnode);
	}
}

#ifndef NO_URI_HASH
void  uget_app_use_uri_hash (UgetApp* app)
{
	UgetNode*  cnode;

	if (app->uri_hash == NULL)
		app->uri_hash = uget_uri_hash_new ();
	for (cnode = app->real.children;  cnode;  cnode = cnode->next)
		uget_uri_hash_add_category (app->uri_hash, cnode);
}

void  uget_app_clear_attachment (UgetApp* app)
{
	UgetNode*   dnode;
	UgetHttp*   http;
	UgDir*      dir;
	void*       hash;
	const char* name;
	char*       folder;
	char*       path;

	hash = uget_uri_hash_new ();
	// add attachment
	for (dnode = app->mix.children->children;  dnode;  dnode = dnode->next) {
		if ((http = ug_info_get (&dnode->data->info, UgetHttpInfo)) == NULL)
			continue;
		if (http->cookie_file)
			uget_uri_hash_add (hash, http->cookie_file);
		if (http->post_file)
			uget_uri_hash_add (hash, http->post_file);
	}

	folder = ug_build_filename (app->config_dir, "attachment", NULL);
#ifdef HAVE_GLIB
	path = g_filename_from_utf8 (folder, -1, NULL, NULL, NULL);
	dir = ug_dir_open (path);
	g_free (path);
#else
	dir = ug_dir_open (folder);
#endif // HAVE_GLIB

	if (dir == NULL)
		ug_create_dir_all (folder, -1);
	else {
		while ((name = ug_dir_read (dir)) != NULL) {
			path = ug_strdup_printf ("%s" UG_DIR_SEPARATOR_S "%s",
			                         folder, name);
			if (uget_uri_hash_find (hash, path) == FALSE)
				ug_unlink (path);
			ug_free (path);
		}
		ug_dir_close (dir);
	}

	ug_free (folder);
	uget_uri_hash_free (hash);
}
#endif // NO_URI_HASH

// ----------------------------------------------------------------------------
// plug-in

void  uget_app_clear_plugins (UgetApp* app)
{
	UgPair*  pair;
	UgPair*  pend;

	pair = app->plugins.at;
	pend = app->plugins.at + app->plugins.length;
	for (pair = app->plugins.at;  pair < pend;  pair++) {
		if (pair->data) {
			uget_plugin_set (pair->data, UGET_PLUGIN_INIT, (void*) FALSE);
			pair->data = NULL;
		}
	}
	ug_array_clear (&app->plugins);
}

void  uget_app_add_plugin (UgetApp* app, const UgetPluginInfo* pinfo)
{
	UgPair*  pair;

	pair = ug_registry_find (&app->plugins, pinfo->name, NULL);
	if (pair == NULL || pair->data == NULL)
		uget_plugin_set (pinfo, UGET_PLUGIN_INIT, (void*) TRUE);
	if (pair == NULL)
		ug_registry_add (&app->plugins, (const UgDataInfo*)pinfo);
}

void  uget_app_remove_plugin (UgetApp* app, const UgetPluginInfo* pinfo)
{
	UgPair*  pair;

	pair = ug_registry_find (&app->plugins, pinfo->name, NULL);
	if (pair && pair->data) {
		uget_plugin_set (pair->data, UGET_PLUGIN_INIT, (void*) FALSE);
		pair->data = NULL;
	}
}

int   uget_app_find_plugin (UgetApp* app, const char* name, const UgetPluginInfo** pinfo)
{
	UgPair*  pair;

	pair = ug_registry_find (&app->plugins, name, NULL);
	if (pair && pair->data) {
		if (pinfo)
			*pinfo = pair->data;
		return TRUE;
	}
	return FALSE;
}

void  uget_app_set_default_plugin (UgetApp* app, const UgetPluginInfo* pinfo)
{
	uget_app_add_plugin (app, pinfo);
	app->plugin_default = (UgetPluginInfo*) pinfo;
}

UgetPluginInfo*  uget_app_match_plugin (UgetApp* app, UgetNode* node, const UgetPluginInfo* exclude)
{
	UgetCommon*      common;
	UgetPluginInfo*  info;
	int              count;
	UgUri            uuri;
	int              index;
	struct {
		UgetPluginInfo*  info;
		int              count;
	} matched = {NULL, 0};

	common = ug_info_get (&node->info, UgetCommonInfo);
	if (common == NULL || common->uri == NULL)
		return NULL;

	ug_uri_init (&uuri, common->uri);
	if (app->plugin_default && app->plugin_default != exclude) {
		matched.info  = app->plugin_default;
		matched.count = uget_plugin_match (matched.info, &uuri);
		if (matched.count >= 3)
			return matched.info;
	}
	for (index = 0;  index < app->plugins.length;  index++) {
		info = app->plugins.at[index].data;
		if (info != exclude) {
			count = uget_plugin_match (info, &uuri);
			if (matched.count < count) {
				matched.count = count;
				matched.info  = info;
				if (matched.count >= 3)
					break;
			}
		}
	}

	// detect file type by plug-in
	if (matched.count == 0) {
		if (matched.info->file_exts    == NULL ||
		    matched.info->file_exts[0] == NULL)
	    {
			return NULL;
	    }
	}
	return matched.info;
}

// ----------------------------------------------------------------------------
// save/load categories

int   uget_app_save_category (UgetApp* app, UgetNode* cnode, const char* filename)
{
	UgJsonFile*  jfile;

	jfile = ug_json_file_new (4096);
	if (ug_json_file_begin_write (jfile, filename, UG_JSON_FORMAT_ALL) == FALSE) {
		ug_json_file_free (jfile);
		return FALSE;
	}

	ug_json_write_object_head (&jfile->json);
	ug_json_write_entry (&jfile->json, cnode, UgetNodeEntry);
	ug_json_write_object_tail (&jfile->json);

	ug_json_file_end_write (jfile);
	ug_json_file_free (jfile);
	return TRUE;
}

UgetNode* uget_app_load_category (UgetApp* app, const char* filename)
{
	UgJsonFile*  jfile;
	UgJsonError  error;
	UgetNode*    cnode;

	jfile = ug_json_file_new (4096);
	if (ug_json_file_begin_parse (jfile, filename) == FALSE) {
		ug_json_file_free (jfile);
		return FALSE;
	}

	cnode = uget_node_new (NULL);
	ug_json_push (&jfile->json, ug_json_parse_entry,
			cnode, (void*)UgetNodeEntry);
	ug_json_push (&jfile->json, ug_json_parse_object,
			NULL, NULL);

	error = ug_json_file_end_parse (jfile);
	ug_json_file_free (jfile);

	if (error == UG_JSON_ERROR_NONE) {
		uget_app_add_category (app, cnode, FALSE);
		// create fake node
		uget_node_make_fake (cnode);
		// move all downloads from active to queuing in this category
		uget_app_stop_category (app, cnode);
		return cnode;
	}
	else {
		uget_node_unref (cnode);
		return NULL;
	}
}

int   uget_app_save_categories (UgetApp* app, const char* folder)
{
	int             count;
	char*           path;
	char*           path_base;
	char*           path_new;
	UgetNode*       cnode;
	UgJsonFile*     jfile;

	if (folder)
		path_base = ug_build_filename (folder, "category", NULL);
	else if (app->config_dir)
		path_base = ug_build_filename (app->config_dir, "category", NULL);
	else
		path_base = ug_strdup ("category");
	ug_create_dir_all (path_base, -1);

	jfile = ug_json_file_new (4096);
	cnode = app->real.children;
	for (count = 0;  cnode;  cnode = cnode->next, count++) {
#if defined _WIN32 || defined _WIN64
		path = ug_strdup_printf ("%s%c%.4d.temp", path_base, '\\', count);
#else
		path = ug_strdup_printf ("%s%c%.4d.temp", path_base, '/',  count);
#endif // _WIN32 || _WIN64

		if (ug_json_file_begin_write (jfile, path, UG_JSON_FORMAT_ALL) == FALSE) {
			ug_free (path);
			break;
		}

		ug_json_write_object_head (&jfile->json);
		ug_json_write_entry (&jfile->json, cnode, UgetNodeEntry);
		ug_json_write_object_tail (&jfile->json);

		ug_json_file_end_write (jfile);

#if defined _WIN32 || defined _WIN64
		path_new = ug_strdup_printf ("%s%c%.4d.json", path_base, '\\', count);
#else
		path_new = ug_strdup_printf ("%s%c%.4d.json", path_base, '/',  count);
#endif // _WIN32 || _WIN64
		ug_unlink (path_new);
		ug_rename (path, path_new);

		ug_free (path_new);
		ug_free (path);
	}

	ug_free (path_base);
	ug_json_file_free (jfile);
	return count;
}

int   uget_app_load_categories (UgetApp* app, const char* folder)
{
	int             count, file_ok;
	char*           path;
	char*           path_base;
	char*           path_temp;
	UgetNode*       cnode;
	UgJsonFile*     jfile;

	if (folder)
		path_base = ug_build_filename (folder, "category", NULL);
	else if (app->config_dir)
		path_base = ug_build_filename (app->config_dir, "category", NULL);
	else
		path_base = ug_strdup ("category");

	jfile = ug_json_file_new (4096);

	for (count = 0;  ;  count++) {
#if defined _WIN32 || defined _WIN64
		path = ug_strdup_printf ("%s%c%.4d.json", path_base, '\\', count);
		path_temp = ug_strdup_printf ("%s%c%.4d.temp", path_base, '\\', count);
#else
		path = ug_strdup_printf ("%s%c%.4d.json", path_base, '/',  count);
		path_temp = ug_strdup_printf ("%s%c%.4d.temp", path_base, '/',  count);
#endif // _WIN32 || _WIN64

		file_ok = ug_json_file_begin_parse (jfile, path);
		if (file_ok)
			ug_unlink (path_temp);
		else if (ug_rename (path_temp, path) != -1)
			file_ok = ug_json_file_begin_parse (jfile, path);
		ug_free (path_temp);
		ug_free (path);
		if (file_ok == FALSE)
			break;

		// parse and add category node
		cnode = uget_node_new (NULL);
		ug_json_push (&jfile->json, ug_json_parse_entry,
				cnode, (void*)UgetNodeEntry);
		ug_json_push (&jfile->json, ug_json_parse_object,
				NULL, NULL);
		ug_json_file_end_parse (jfile);

		uget_app_add_category (app, cnode, FALSE);
		// create fake node
		uget_node_make_fake (cnode);
		// move all downloads from active to queuing in this categoey
		uget_app_stop_category (app, cnode);
	}

	ug_json_file_free (jfile);
	ug_free (path_base);
	return count;
}

// ----------------------------------------------------------------------------
// keeping status

void  uget_node_set_keeping (UgetNode* node, int enable)
{
	union {
		UgetCommon*  common;
		UgetProxy*   proxy;
		UgetHttp*    http;
		UgetFtp*     ftp;
	} temp;

	temp.common = ug_info_realloc (&node->info, UgetCommonInfo);
	if (temp.common) {
		temp.common->keeping.enable = enable;
		if (enable) {
			if (temp.common->uri)
				temp.common->keeping.uri = TRUE;
			if (temp.common->mirrors)
				temp.common->keeping.mirrors = TRUE;
			if (temp.common->file)
				temp.common->keeping.file = TRUE;
			if (temp.common->folder)
				temp.common->keeping.folder = TRUE;
			if (temp.common->user)
				temp.common->keeping.user = TRUE;
			if (temp.common->password)
				temp.common->keeping.password = TRUE;
//			if (temp.common->connect_timeout)
//				temp.common->keeping.connect_timeout = TRUE;
//			if (temp.common->transmit_timeout)
//				temp.common->keeping.transmit_timeout = TRUE;
//			if (temp.common->retry_delay)
//				temp.common->keeping.retry_delay = TRUE;
//			if (temp.common->retry_limit)
//				temp.common->keeping.retry_limit = TRUE;

//			if (temp.common->max_connections)
//				temp.common->keeping.max_connections = TRUE;
			if (temp.common->max_upload_speed)
				temp.common->keeping.max_upload_speed = TRUE;
			if (temp.common->max_download_speed)
				temp.common->keeping.max_download_speed = TRUE;
			if (temp.common->timestamp)
				temp.common->keeping.timestamp = TRUE;
			if (temp.common->debug_level)
				temp.common->keeping.debug_level = TRUE;
		}
	}

	temp.proxy = ug_info_get (&node->info, UgetProxyInfo);
	if (temp.proxy) {
		temp.proxy->keeping.enable = enable;
		if (enable) {
			if (temp.proxy->host)
				temp.proxy->keeping.host = TRUE;
			if (temp.proxy->port)
				temp.proxy->keeping.port = TRUE;
			if (temp.proxy->type)
				temp.proxy->keeping.type = TRUE;
			if (temp.proxy->user)
				temp.proxy->keeping.user = TRUE;
			if (temp.proxy->password)
				temp.proxy->keeping.password = TRUE;
		}
	}

	temp.http = ug_info_get (&node->info, UgetHttpInfo);
	if (temp.http) {
		temp.http->keeping.enable = enable;
		if (enable) {
			if (temp.http->user)
				temp.http->keeping.user = TRUE;
			if (temp.http->password)
				temp.http->keeping.password = TRUE;
			if (temp.http->referrer)
				temp.http->keeping.referrer = TRUE;
			if (temp.http->user_agent)
				temp.http->keeping.user_agent = TRUE;
			if (temp.http->post_data)
				temp.http->keeping.post_data = TRUE;
			if (temp.http->post_file)
				temp.http->keeping.post_file = TRUE;
			if (temp.http->cookie_data)
				temp.http->keeping.cookie_data = TRUE;
			if (temp.http->cookie_file)
				temp.http->keeping.cookie_file = TRUE;
//			if (temp.http->redirection_limit)
//				temp.http->keeping.redirection_limit = TRUE;
		}
	}

	temp.ftp = ug_info_get (&node->info, UgetFtpInfo);
	if (temp.ftp) {
		temp.ftp->keeping.enable = enable;
		if (enable) {
			if (temp.ftp->user)
				temp.ftp->keeping.user = TRUE;
			if (temp.ftp->password)
				temp.ftp->keeping.password = TRUE;
			if (temp.ftp->active_mode)
				temp.ftp->keeping.active_mode = TRUE;
		}
	}
}
