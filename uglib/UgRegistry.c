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

void  ug_registry_init(UgRegistry* reg)
{
	ug_array_init(reg, sizeof(UgPair), 16);
	reg->sorted = FALSE;
}

void  ug_registry_final(UgRegistry* reg)
{
	ug_array_clear(reg);
}

void  ug_registry_add(UgRegistry* reg, const UgDataInfo* info)
{
	UgPair* pair;
	int     index;

	if (reg->sorted == FALSE || reg->length == 0)
		pair = ug_array_alloc(reg, 1);
	else {
		pair = ug_registry_find(reg, info->name, &index);
		if (pair == NULL)
			pair = ug_array_insert(reg, index, 1);
	}
	pair->key  = (void*) info->name;
	pair->data = (void*) info;
}

void  ug_registry_remove(UgRegistry* reg, const UgDataInfo* info)
{
	UgPair* cur;
	int     index;

	cur = ug_registry_find(reg, info->name, &index);
	if (cur)
		ug_array_erase(reg, index, 1);
}

UgPair* ug_registry_find(UgRegistry* reg, const char* key, int* inserted_index)
{
	UgPair* cur;
	UgPair* end;

	if (reg->sorted == FALSE) {
		cur = reg->at;
		end = reg->at + reg->length;
		for (; cur < end; cur++) {
			if (strcmp(cur->key, key) == 0) {
				if (inserted_index)
					inserted_index[0] = cur - reg->at;
				return cur;
			}
		}
		return NULL;
	}

	return ug_array_find_sorted(reg, &key,
	                            ug_array_compare_string,
	                            inserted_index);
}

void  ug_registry_sort(UgRegistry* reg)
{
	ug_array_sort(reg, ug_array_compare_string);
	reg->sorted = TRUE;
}
