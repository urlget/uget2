/*
 *
 *   Copyright (C) 2012-2018 by C.H. Huang
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
#include <UgRegistry.h>

// ----------------------------------------------------------------------------
// UgPairs

void  ug_registry_init (UgRegistry* reg)
{
	ug_array_init (reg, sizeof (UgPair), 16);
	reg->sorted = FALSE;
}

void  ug_registry_final (UgRegistry* reg)
{
	ug_array_clear (reg);
}

void  ug_registry_add (UgRegistry* reg, const UgDataInfo* info)
{
	UgPair* pair;
	int     index;

	if (reg->sorted == FALSE || reg->length == 0)
		pair = ug_array_alloc (reg, 1);
	else {
		pair = ug_registry_find (reg, info->name, &index);
		if (pair == NULL) {
			ug_array_alloc (reg, 1);
			memmove (reg->at + index + 1, reg->at + index,
					sizeof (UgPair) * (reg->length - index - 1));
			pair = reg->at + index;
		}
	}
	pair->key  = (void*) info->name;
	pair->data = (void*) info;
}

void  ug_registry_remove (UgRegistry* reg, const UgDataInfo* info)
{
	UgPair* cur;

	cur = ug_registry_find (reg, info->name, NULL);
	if (cur) {
		memmove (cur, cur +1,
				sizeof (UgPair) * (reg->length - (cur - reg->at) -1));
	}
}

UgPair* ug_registry_find (UgRegistry* reg, const char* key, int* inserted_index)
{
	UgPair* low;
	UgPair* cur;
	UgPair* high;
	int     diff;

	low  = reg->at;
	high = reg->at + reg->length;
	cur  = low;

	if (reg->sorted == FALSE) {
		for (; low < high; low++) {
			if (strcmp (low->key, key) == 0) {
				if (inserted_index)
					inserted_index[0] = low - reg->at;
				return low;
			}
		}
		return NULL;
	}

	while (low < high) {
		cur = low + (high - low) / 2;

		diff = strcmp (cur->key, key);
		if (diff == 0) {
			if (inserted_index)
				inserted_index[0] = cur - reg->at;
			return cur;
		}
		else if (diff > 0)
			high = cur;
		else if (diff < 0)
			low = cur + 1;
	}

	if (inserted_index) {
		if (cur < low)
			cur++;
		*inserted_index = cur - reg->at;
	}
	return NULL;
}

static int  compare_key_string (UgPair* pair1, UgPair* pair2)
{
	return strcmp (pair1->key, pair2->key);
}

void  ug_registry_sort (UgRegistry* reg)
{
	ug_array_sort (reg, (UgCompareFunc) compare_key_string);
	reg->sorted = TRUE;
}
