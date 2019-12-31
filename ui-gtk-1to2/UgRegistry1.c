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

#include <string.h>
#include <stdlib.h>
#include <glib.h>
//#include <gmodule.h>

#include "UgRegistry1.h"

static GHashTable*		registry_hash	= NULL;
//static GStaticMutex	registry_mutex	= G_STATIC_MUTEX_INIT;

void	ug_registry1_insert (const char* key, const void* value)
{
	GList*	list;

	if (registry_hash == NULL)
		registry_hash = g_hash_table_new (g_str_hash, g_str_equal);

	list = g_hash_table_lookup (registry_hash, key);
	// if key doesn't exist in registry_hash, duplicate it.
	if (list == NULL)
		key = g_strdup (key);
	// add value to list and update list in registry_hash.
	list = g_list_prepend (list, (gpointer) value);
	g_hash_table_insert (registry_hash, (gpointer) key, list);
}

void	ug_registry1_remove (const char* key, const void* value)
{
	GList*		list;
	gpointer	orig_key;

	if (registry_hash == NULL)
		return;

	list = g_hash_table_lookup (registry_hash, key);
	if (list) {
		// remove specified value from list
		list = g_list_remove (list, value);
		// if list has data, use new list instead of old one.
		// otherwise key and value must be removed.
		if (list)
			g_hash_table_insert (registry_hash, (gpointer) key, list);
		else {
			// the original key must be freed.
			g_hash_table_lookup_extended (registry_hash, key, &orig_key, NULL);
			g_hash_table_remove (registry_hash, key);
			g_free (orig_key);
		}
	}
}

int		ug_registry1_exist  (const char* key, const void* value)
{
	GList*	list;

	if (registry_hash) {
		list = g_hash_table_lookup (registry_hash, key);
		if (g_list_find (list, value))
			return TRUE;
	}

	return FALSE;
}

void*	ug_registry1_find (const char* key)
{
	GList*	list;

	if (registry_hash) {
		list = g_hash_table_lookup (registry_hash, key);
		if (list)
			return list->data;
	}

	return NULL;
}

