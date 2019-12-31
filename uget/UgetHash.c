/*
 *
 *   Copyright (C) 2012-2020 by C.H. Huang
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

#include <UgDefine.h>
#include <UgString.h>
#include <UgetData.h>
#include <UgetHash.h>

#ifdef NO_URI_HASH
#else

#if defined HAVE_GLIB

#define ug_hash_table_destroy    g_hash_table_destroy
#define ug_hash_table_lookup     g_hash_table_lookup
#define ug_hash_table_insert     g_hash_table_insert
#define ug_hash_table_remove     g_hash_table_remove

#else

// modify GHashTable from GLib
#include <string.h>
#include <stdint.h>
#include <memory.h>

typedef struct UgHashTable    UgHashTable;
typedef unsigned int (*UgHashFunc) (const void* v);

UgHashTable*  ug_hash_table_new (UgHashFunc hash, UgCompareFunc compare);

void   ug_hash_table_destroy (UgHashTable* hash_table);
void*  ug_hash_table_lookup (UgHashTable* hash_table, const void* key);
void   ug_hash_table_insert (UgHashTable* hash_table, void* key, void* value);
int    ug_hash_table_remove (UgHashTable* hash_table, const void* key);

unsigned int  ug_hash_str (const void* v);

// port GHashTable to Android...
struct UgHashTable
{
	int             size;
	int             mod;
	unsigned int    mask;
	int             nnodes;
	int             noccupied;  // nnodes + tombstones

	void**          keys;
	void**          values;
	unsigned int*   hashes;

	int             ref_count;

	UgHashFunc      hash_func;
	UgCompareFunc   compare_func;

	UgNotifyFunc    key_destroy_func;
	UgNotifyFunc    value_destroy_func;
};

// ----------------------------------------------------------------------------

#undef	MAX
#define MAX(a, b)  (((a) > (b)) ? (a) : (b))

#define HASH_TABLE_MIN_SHIFT 3  // 1 << 3 == 8 buckets

#define UNUSED_HASH_VALUE 0
#define TOMBSTONE_HASH_VALUE 1
#define HASH_IS_UNUSED(h_) ((h_) == UNUSED_HASH_VALUE)
#define HASH_IS_TOMBSTONE(h_) ((h_) == TOMBSTONE_HASH_VALUE)
#define HASH_IS_REAL(h_) ((h_) >= 2)

static const int prime_mod [] =
{
  1,          /* For 1 << 0 */
  2,
  3,
  7,
  13,
  31,
  61,
  127,
  251,
  509,
  1021,
  2039,
  4093,
  8191,
  16381,
  32749,
  65521,      /* For 1 << 16 */
  131071,
  262139,
  524287,
  1048573,
  2097143,
  4194301,
  8388593,
  16777213,
  33554393,
  67108859,
  134217689,
  268435399,
  536870909,
  1073741789,
  2147483647  /* For 1 << 31 */
};

static void
ug_hash_table_set_shift (UgHashTable *hash_table, int shift)
{
	int i;
	unsigned int mask = 0;

	hash_table->size = 1 << shift;
	hash_table->mod  = prime_mod [shift];

	for (i = 0; i < shift; i++) {
		mask <<= 1;
		mask |= 1;
	}

	hash_table->mask = mask;
}

static int
ug_hash_table_find_closest_shift (int n)
{
	int i;

	for (i = 0; n; i++)
		n >>= 1;

	return i;
}

static void
ug_hash_table_set_shift_from_size (UgHashTable *hash_table, int size)
{
	int shift;

	shift = ug_hash_table_find_closest_shift (size);
	shift = MAX (shift, HASH_TABLE_MIN_SHIFT);

	ug_hash_table_set_shift (hash_table, shift);
}

static unsigned int
ug_hash_table_lookup_node (UgHashTable*   hash_table,
                           const void*    key,
                           unsigned int*  hash_return)
{
	unsigned int node_index;
	unsigned int node_hash;
	unsigned int hash_value;
	unsigned int first_tombstone = 0;
	int          have_tombstone = FALSE;
	unsigned int step = 0;

	hash_value = hash_table->hash_func (key);
	if (!HASH_IS_REAL (hash_value))
		hash_value = 2;

	*hash_return = hash_value;

	node_index = hash_value % hash_table->mod;
	node_hash = hash_table->hashes[node_index];

	while (!HASH_IS_UNUSED (node_hash)) {
      /* We first check if our full hash values
       * are equal so we can avoid calling the full-blown
       * key equality function in most cases.
       */
		if (node_hash == hash_value) {
			void* node_key = hash_table->keys[node_index];

			if (hash_table->compare_func) {
				if (hash_table->compare_func (node_key, key) == 0)
					return node_index;
			}
			else if (node_key == key) {
				return node_index;
			}
		}
		else if (HASH_IS_TOMBSTONE (node_hash) && !have_tombstone) {
			first_tombstone = node_index;
			have_tombstone = TRUE;
		}

		step++;
		node_index += step;
		node_index &= hash_table->mask;
		node_hash = hash_table->hashes[node_index];
	}

	if (have_tombstone)
		return first_tombstone;

	return node_index;
}

static void
ug_hash_table_remove_node (UgHashTable* hash_table,
                           int          i,
                           int          notify)
{
	void* key;
	void* value;

	key = hash_table->keys[i];
	value = hash_table->values[i];

	/* Erect tombstone */
	hash_table->hashes[i] = TOMBSTONE_HASH_VALUE;

	/* Be GC friendly */
	hash_table->keys[i] = NULL;
	hash_table->values[i] = NULL;

	hash_table->nnodes--;

	if (notify && hash_table->key_destroy_func)
		hash_table->key_destroy_func (key);

	if (notify && hash_table->value_destroy_func)
		hash_table->value_destroy_func (value);
}

static void
ug_hash_table_remove_all_nodes (UgHashTable* hash_table,
                                int          notify)
{
	int   i;
	void* key;
	void* value;

	hash_table->nnodes = 0;
	hash_table->noccupied = 0;

	if (!notify ||
	    (hash_table->key_destroy_func == NULL &&
	     hash_table->value_destroy_func == NULL))
	{
		memset (hash_table->hashes, 0, hash_table->size * sizeof (unsigned int));
		memset (hash_table->keys,   0, hash_table->size * sizeof (void*));
		memset (hash_table->values, 0, hash_table->size * sizeof (void*));

		return;
    }

	for (i = 0; i < hash_table->size; i++) {
		if (HASH_IS_REAL (hash_table->hashes[i])) {
			key = hash_table->keys[i];
			value = hash_table->values[i];

			hash_table->hashes[i] = UNUSED_HASH_VALUE;
			hash_table->keys[i] = NULL;
			hash_table->values[i] = NULL;

			if (hash_table->key_destroy_func != NULL)
				hash_table->key_destroy_func (key);

			if (hash_table->value_destroy_func != NULL)
				hash_table->value_destroy_func (value);
		}
		else if (HASH_IS_TOMBSTONE (hash_table->hashes[i])) {
			hash_table->hashes[i] = UNUSED_HASH_VALUE;
		}
	}
}

static void
ug_hash_table_resize (UgHashTable *hash_table)
{
  void**        new_keys;
  void**        new_values;
  unsigned int* new_hashes;
  int old_size;
  int i;

  old_size = hash_table->size;
  ug_hash_table_set_shift_from_size (hash_table, hash_table->nnodes * 2);

  new_keys = ug_malloc0 (sizeof (void*) * hash_table->size);
  if (hash_table->keys == hash_table->values)
    new_values = new_keys;
  else
    new_values = ug_malloc0 (sizeof (void*) * hash_table->size);
  new_hashes = ug_malloc0 (sizeof (unsigned int) * hash_table->size);

  for (i = 0; i < old_size; i++)
    {
      unsigned int node_hash = hash_table->hashes[i];
      unsigned int hash_val;
      unsigned int step = 0;

      if (!HASH_IS_REAL (node_hash))
        continue;

      hash_val = node_hash % hash_table->mod;

      while (!HASH_IS_UNUSED (new_hashes[hash_val]))
        {
          step++;
          hash_val += step;
          hash_val &= hash_table->mask;
        }

      new_hashes[hash_val] = hash_table->hashes[i];
      new_keys[hash_val] = hash_table->keys[i];
      new_values[hash_val] = hash_table->values[i];
    }

  if (hash_table->keys != hash_table->values)
    ug_free (hash_table->values);

  ug_free (hash_table->keys);
  ug_free (hash_table->hashes);

  hash_table->keys = new_keys;
  hash_table->values = new_values;
  hash_table->hashes = new_hashes;

  hash_table->noccupied = hash_table->nnodes;
}

static inline void
ug_hash_table_maybe_resize (UgHashTable *hash_table)
{
  int noccupied = hash_table->noccupied;
  int size = hash_table->size;

  if ((size > hash_table->nnodes * 4 && size > 1 << HASH_TABLE_MIN_SHIFT) ||
      (size <= noccupied + (noccupied / 16)))
    ug_hash_table_resize (hash_table);
}

UgHashTable*  ug_hash_table_new (UgHashFunc hash_func, UgCompareFunc compare_func)
{
  UgHashTable*  hash_table;

  hash_table = ug_malloc (sizeof (UgHashTable));
  ug_hash_table_set_shift (hash_table, HASH_TABLE_MIN_SHIFT);
  hash_table->nnodes             = 0;
  hash_table->noccupied          = 0;
  hash_table->hash_func          = hash_func ? hash_func : NULL;
  hash_table->compare_func       = compare_func;
  hash_table->ref_count          = 1;
  hash_table->key_destroy_func   = NULL;
  hash_table->value_destroy_func = NULL;
  hash_table->keys               = ug_malloc0 (sizeof (void*) * hash_table->size);
  hash_table->values             = hash_table->keys;
  hash_table->hashes             = ug_malloc0 (sizeof (unsigned int) * hash_table->size);

  return hash_table;
}

static void
ug_hash_table_insert_node (UgHashTable* hash_table,
                           unsigned int node_index,
                           unsigned int key_hash,
                           void*        key,
                           void*        value,
                           int          keep_new_key,
                           int          reusing_key)
{
  unsigned int old_hash;
  void*        old_key;
  void*        old_value;

  if (hash_table->keys == hash_table->values && key != value) {
    hash_table->values = ug_malloc0 (sizeof (void*) * hash_table->size);
    memcpy (hash_table->values, hash_table->keys, sizeof (void*) * hash_table->size);
  }

  old_hash = hash_table->hashes[node_index];
  old_key = hash_table->keys[node_index];
  old_value = hash_table->values[node_index];

  if (HASH_IS_REAL (old_hash))
    {
      if (keep_new_key)
        hash_table->keys[node_index] = key;
      hash_table->values[node_index] = value;
    }
  else
    {
      hash_table->keys[node_index] = key;
      hash_table->values[node_index] = value;
      hash_table->hashes[node_index] = key_hash;

      hash_table->nnodes++;

      if (HASH_IS_UNUSED (old_hash))
        {
          /* We replaced an empty node, and not a tombstone */
          hash_table->noccupied++;
          ug_hash_table_maybe_resize (hash_table);
        }

    }

  if (HASH_IS_REAL (old_hash))
    {
      if (hash_table->key_destroy_func && !reusing_key)
        hash_table->key_destroy_func (keep_new_key ? old_key : key);
      if (hash_table->value_destroy_func)
        hash_table->value_destroy_func (old_value);
    }
}

void
ug_hash_table_destroy (UgHashTable *hash_table)
{
//  ug_hash_table_remove_all (hash_table);
  ug_hash_table_remove_all_nodes (hash_table, TRUE);
  ug_hash_table_maybe_resize (hash_table);

//  ug_hash_table_unref (hash_table);
  ug_hash_table_remove_all_nodes (hash_table, TRUE);
  if (hash_table->keys != hash_table->values)
     ug_free (hash_table->values);
  ug_free (hash_table->keys);
  ug_free (hash_table->hashes);
  ug_free (hash_table);
}

void*
ug_hash_table_lookup (UgHashTable*   hash_table,
                      const void*    key)
{
  unsigned int node_index;
  unsigned int node_hash;

  node_index = ug_hash_table_lookup_node (hash_table, key, &node_hash);

  return HASH_IS_REAL (hash_table->hashes[node_index])
    ? hash_table->values[node_index]
    : NULL;
}

void
ug_hash_table_insert (UgHashTable *hash_table,
                      void*        key,
                      void*        value)
{
//  g_hash_table_insert_internal (hash_table, key, value, FALSE);
  unsigned int key_hash;
  unsigned int node_index;

  node_index = ug_hash_table_lookup_node (hash_table, key, &key_hash);

//  g_hash_table_insert_node (hash_table, node_index, key_hash, key, value, keep_new_key, FALSE);
  ug_hash_table_insert_node (hash_table, node_index, key_hash, key, value, FALSE, FALSE);
}

int
ug_hash_table_remove (UgHashTable*   hash_table,
                      const void*    key)
{
//  return g_hash_table_remove_internal (hash_table, key, TRUE);
  unsigned int node_index;
  unsigned int node_hash;

  node_index = ug_hash_table_lookup_node (hash_table, key, &node_hash);

  if (!HASH_IS_REAL (hash_table->hashes[node_index]))
    return FALSE;

  ug_hash_table_remove_node (hash_table, node_index, TRUE);
  ug_hash_table_maybe_resize (hash_table);

  return TRUE;
}


unsigned int  ug_hash_str (const void* v)
{
	const signed char *p;
	uint32_t  h = 5381;

	for (p = v; *p != '\0'; p++)
		h = (h << 5) + h + *p;

	return h;
}
#endif // HAVE_GLIB

// ----------------------------------------------------------------------------

void* uget_uri_hash_new (void)
{
#if defined HAVE_GLIB
	return g_hash_table_new_full (g_str_hash, g_str_equal, ug_free, NULL);
#else
	UgHashTable*  uuhash;

	uuhash = ug_hash_table_new (ug_hash_str, (UgCompareFunc) strcmp);
	uuhash->key_destroy_func = ug_free;
	return uuhash;
#endif // HAVE_GLIB
}

void  uget_uri_hash_free (void* uuhash)
{
	if (uuhash)
		ug_hash_table_destroy (uuhash);
}

int   uget_uri_hash_find (void* uuhash, const char* uri)
{
	if (uuhash && ug_hash_table_lookup (uuhash, uri))
		return TRUE;
	else
		return FALSE;
}

void  uget_uri_hash_add (void* uuhash, const char* uri)
{
	uintptr_t   counts;

	if (uri) {
		counts = (uintptr_t) ug_hash_table_lookup (uuhash, uri);
		ug_hash_table_insert (uuhash, ug_strdup (uri), (void*) (++counts));
	}
}

void  uget_uri_hash_remove (void* uuhash, const char* uri)
{
	uintptr_t   counts;

	if (uri) {
		counts = (uintptr_t) ug_hash_table_lookup (uuhash, uri);
		if (counts > 1)
			ug_hash_table_insert (uuhash, ug_strdup (uri), (void*) (--counts));
		else
			ug_hash_table_remove (uuhash, uri);
	}
}

void  uget_uri_hash_add_download (void* uuhash, UgInfo* dnode_info)
{
	UgetCommon* common;
	uintptr_t   counts;

	if (uuhash == NULL)
		return;
	common = ug_info_get(dnode_info, UgetCommonInfo);
	if (common && common->uri) {
		counts = (uintptr_t) ug_hash_table_lookup (uuhash, common->uri);
		ug_hash_table_insert (uuhash, ug_strdup (common->uri), (void*) (++counts));
	}
}

void  uget_uri_hash_remove_download (void* uuhash, UgInfo* dnode_info)
{
	UgetCommon* common;
	uintptr_t   counts;

	if (uuhash == NULL)
		return;
	common = ug_info_get(dnode_info, UgetCommonInfo);
	if (common && common->uri) {
		counts = (uintptr_t) ug_hash_table_lookup (uuhash, common->uri);
		if (counts > 1)
			ug_hash_table_insert (uuhash, ug_strdup (common->uri), (void*) (--counts));
		else
			ug_hash_table_remove (uuhash, common->uri);
	}
}

void  uget_uri_hash_add_category (void* uuhash, UgetNode* cnode)
{
	UgetNode*   dnode;
	UgetCommon* common;
	uintptr_t   counts;

	if (uuhash == NULL)
		return;
	for (dnode = cnode->children;  dnode;  dnode = dnode->next) {
		common = ug_info_get (dnode->info, UgetCommonInfo);
		if (common && common->uri) {
			counts = (uintptr_t) ug_hash_table_lookup (uuhash, common->uri);
			ug_hash_table_insert (uuhash, ug_strdup (common->uri), (void*) (++counts));
		}
	}
}

void  uget_uri_hash_remove_category (void* uuhash, UgetNode* cnode)
{
	UgetNode*   dnode;
	UgetCommon* common;
	uintptr_t   counts;

	if (uuhash == NULL)
		return;
	for (dnode = cnode->children;  dnode;  dnode = dnode->next) {
		common = ug_info_get (dnode->info, UgetCommonInfo);
		if (common && common->uri) {
			counts = (uintptr_t) ug_hash_table_lookup (uuhash, common->uri);
			if (counts > 1)
				ug_hash_table_insert (uuhash, ug_strdup (common->uri), (void*) (--counts));
			else
				ug_hash_table_remove (uuhash, common->uri);
		}
	}
}

#endif // NO_URI_HASH
