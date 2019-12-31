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

#include <UgSLink.h>

void  ug_slinks_init (UgSLinks* slinks, int allocated_len)
{
	ug_array_init (slinks, sizeof (UgSLink), allocated_len);
	slinks->n_links = 0;
	slinks->used  = NULL;
	slinks->freed = NULL;
}

void  ug_slinks_final (UgSLinks* slinks)
{
	ug_array_clear (slinks);
}

void  ug_slinks_add (UgSLinks* slinks, void* data)
{
	UgSLink*  link;
	UgSLink*  old_used;
	UgSLink*  old_at;
	UgSLink*  current;

	old_used = slinks->used;
	if (slinks->freed != NULL) {
		link = slinks->freed;
		slinks->freed = link->next;
		// move link from freed list to used list.
		link->next = old_used;
	}
	else {
		old_at = slinks->at;
		link = (UgSLink*) ug_array_alloc (slinks, 1);
		if (old_at != slinks->at && old_used) {
			slinks->used = slinks->at + (old_used - old_at);
			for (current = slinks->used;  ;  current = current->next) {
				if (current->next == NULL)
					break;
				current->next = slinks->at + (current->next - old_at);
			}
		}
		// move link to used list.
		link->next = slinks->used;
	}
	link->data = data;
	slinks->used = link;
	slinks->n_links++;
}

UgSLink*  ug_slinks_find (UgSLinks* slinks, void* data, UgSLink** prev_result)
{
	UgSLink*   prev;
	UgSLink*   link;

	for (prev = NULL, link = slinks->used;  link;  link = link->next) {
		if (link->data == data) {
			if (prev_result)
				prev_result[0] = prev;
			return link;
		}
		prev = link;
	}
	return NULL;
}

void  ug_slinks_remove (UgSLinks* slinks, void* data, UgSLink* prev)
{
	UgSLink*   link;

	if (prev == NULL)
		link = ug_slinks_find (slinks, data, &prev);
	else
		link = prev->next;

	if (link) {
		// remove link from used links
		if (prev)
			prev->next = link->next;
		else
			slinks->used = link->next;
		// add link to freed links
		link->next = slinks->freed;
		slinks->freed = link;
	}
	slinks->n_links--;
}

void  ug_slinks_foreach (UgSLinks* slinks, UgForeachFunc func, void* user_data)
{
	UgSLink*   link;

	for (link = slinks->used;  link;  link = link->next)
		func (link->data, user_data);
}
