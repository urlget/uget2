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

#ifndef UG_LIST_H
#define UG_LIST_H

// uintptr_t is an unsigned int that is guaranteed to be the same size as a pointer.
#include <stdint.h>		// uintptr_t, intptr_t
#include <UgDefine.h>
#include <UgJson.h>
#include <UgEntry.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct	UgLink          UgLink;
typedef struct	UgList          UgList;

// ----------------------------------------------------------------------------
// UgLink

#define UG_LINK_MEMBERS(LinkType, DataType, DataName)  \
	DataType*  DataName;    \
	LinkType*  next;        \
	LinkType*  prev

#define UG_LINK_INT_MEMBERS(LinkType, DataName)  \
	intptr_t   DataName;    \
	LinkType*  next;        \
	LinkType*  prev

#define UG_LINK_UINT_MEMBERS(LinkType, DataName)  \
	uintptr_t  DataName;    \
	LinkType*  next;        \
	LinkType*  prev

struct UgLink
{
	UG_LINK_MEMBERS (UgLink, void, data);
//	void*       data;   // uintptr_t size
//	UgLink*     next;   // head
//	UgLink*     prev;   // tail
};

UgLink* ug_link_new  (void);
void    ug_link_free (UgLink* link);

// ----------------------------------------------------------------------------
// UgList is used by UgEntry with UG_ENTRY_LIST.
// UgList doesn't support UG_ENTRY_INT64, UG_ENTRY_DOUBLE, and UG_ENTRY_OBJECT

#define UG_LIST_MEMBERS(LinkType)  \
	uintptr_t   size;  \
	LinkType*   head;  \
	LinkType*   tail

struct UgList
{
	UG_LIST_MEMBERS (UgLink);
//	uintptr_t   size;
//	UgLink*     head;
//	UgLink*     tail;
};

// if all links in list were created by ug_link_new(),
// ug_list_clear (list, TRUE) can free them.

void  ug_list_init  (UgList* list);
void  ug_list_clear (UgList* list, int free_links);

void  ug_list_foreach (UgList* list, UgForeachFunc func, void* data);
void  ug_list_foreach_link (UgList* list, UgForeachFunc func, void* data);
void  ug_list_prepend (UgList* list, UgLink* link);
void  ug_list_append  (UgList* list, UgLink* link);
void  ug_list_insert  (UgList* list, UgLink* sibling, UgLink* link);
void  ug_list_remove  (UgList* list, UgLink* link);
int   ug_list_position(UgList* list, UgLink* link);

// ----------------------------------------------------------------------------
// UgJsonParseFunc for JSON array elements
UgJsonError ug_json_parse_list_bool (UgJson* json,
                                     const char* name, const char* value,
                                     void* list, void* none);
UgJsonError ug_json_parse_list_int (UgJson* json,
                                    const char* name, const char* value,
                                    void* list, void* none);
UgJsonError ug_json_parse_list_uint (UgJson* json,
                                     const char* name, const char* value,
                                     void* list, void* none);
UgJsonError ug_json_parse_list_string (UgJson* json,
                                       const char* name, const char* value,
                                       void* list, void* none);

// ----------------------------------------------------------------------------
// write JSON array elements
// If you use these functions in UgEntry, UgEntry.type must be UG_ENTRY_ARRAY
void	ug_json_write_list_bool (UgJson* json, UgList* list);
void	ug_json_write_list_int (UgJson* json, UgList* list);
void	ug_json_write_list_uint (UgJson* json, UgList* list);
void	ug_json_write_list_string (UgJson* json, UgList* list);


#ifdef __cplusplus
}
#endif

// ----------------------------------------------------------------------------
// C++11 standard-layout

#ifdef __cplusplus

namespace Ug
{
// This one is for directly use only. You can NOT derived it.
typedef struct UgLink    Link;

// This one is for derived use only. No data members here.
// Your derived struct/class must be C++11 standard-layout
struct ListMethod
{
	inline void  init (void)
		{ ug_list_init ((UgList*) this); }
	inline void  clear (bool freeLinks = false)
		{ ug_list_clear ((UgList*) this, (int)freeLinks); }

	inline void  foreach (UgForeachFunc func, void* data)
		{ ug_list_foreach ((UgList*) this, func, data); }
	inline void  prepend (UgLink* link)
		{ ug_list_prepend ((UgList*) this, link); }
	inline void  append (UgLink* link)
		{ ug_list_append ((UgList*) this, link); }
	inline void  insert (UgLink* sibling, UgLink* link)
		{ ug_list_insert ((UgList*) this, sibling, link); }
	inline void  remove (UgLink* link)
		{ ug_list_remove ((UgList*) this, link); }
};

// This one is for directly use only. You can NOT derived it.
struct List : ListMethod, UgList
{
	inline List (void)
		{ ug_list_init (this); }
};

};  // namespace Ug

#endif  // __cplusplus


#endif  // UG_LIST_H

