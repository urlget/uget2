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


#ifndef UG_SLINK_H
#define UG_SLINK_H

#include <stdint.h>
#include <UgArray.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct  UgSLink        UgSLink;
typedef struct  UgSLinks       UgSLinks;

// ----------------------------------------------------------------------------
// UgSLink : for Singly-Linked List

struct UgSLink
{
	void*    data;
	UgSLink* next;
};

// ----------------------------------------------------------------------------
// UgSLinks : array of UgSLink

#define UG_SLINKS_MEMBERS    \
	UG_ARRAY_MEMBERS (UgSLink);   \
	int       n_links;            \
	UgSLink*  used;               \
	UgSLink*  freed               \

struct UgSLinks
{
	UG_SLINKS_MEMBERS;
//	UgSLink*  at;
//	int       length;
//	int       allocated;
//	int       element_size
//	int       n_links;
//	UgSLink*  used;
//	UgSLink*  freed;
};

void  ug_slinks_init (UgSLinks* slinks, int allocated_len);
void  ug_slinks_final (UgSLinks* slinks);

void  ug_slinks_add (UgSLinks* slinks, void* data);
void  ug_slinks_remove (UgSLinks* slinks, void* data, UgSLink* prev);
void  ug_slinks_foreach (UgSLinks* ilinks, UgForeachFunc func, void* user_data);

// return NULL if not found.
UgSLink*  ug_slinks_find (UgSLinks* slinks, void* data, UgSLink** prev);

#define   ug_slinks_index(slinks, link)   ((link) - (slinks)->at)

#ifdef __cplusplus
}
#endif


#endif  // End of UG_SLINK_H

