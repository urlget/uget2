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

#ifndef UGET_NODE_H
#define UGET_NODE_H

#include <stddef.h>    // offsetof()
#include <stdint.h>    // int16_t
#include <UgNode.h>
#include <UgInfo.h>
#include <UgUri.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ----------------------------------------------------------------------------
   UgetNode: extend from UgNode and add pointers (real, fake, and peer).

	* Tree chart 1: (parent and child nodes)

	Root --+-- Category1 --+-- Download1 (URI)
	       |               |
	       |               |
	       |               |
	       |               +-- Download2 (URI)
	       |                   (torrent path)
	       |
	       |
	       +-- Category2 --+-- Download3 (URI)
	                       |   (metalink path)
	                       |
	                       |
	                       +-- Download4 (URI)

	* Tree chart 2:

	                  fake ----- peer -----> fake
	                   /                      /
	           prev   /               prev   /
	             |   /                  |   /
	             |  /                   |  /
	             | /                    | /
	             |/                     |/
	... <---> parent <--------------> child <----> ...
	            /|                     /|
	           / |                    / |
	          /  |                   /  |
	         /   |                  /   |
	        /    |                 /    |
	       /    next              /    next
	      /                      /
	    real                   real
 */

typedef struct UgetNode          UgetNode;
/*
typedef struct UgetNodeControl   UgetNodeControl;
typedef struct UgetNodeNotifier  UgetNodeNotifier;
typedef struct UgetNodeSort      UgetNodeSort;
 */

typedef void (*UgetNodeFunc)(UgetNode* node, UgetNode* sibling, UgetNode* child);

extern const UgEntry  UgetNodeEntry[];

typedef enum {
	UGET_GROUP_NULL       = 0,
	UGET_GROUP_QUEUING    = 1 << 0,

	UGET_GROUP_PAUSED     = 1 << 1,
	UGET_GROUP_ACTIVE     = 1 << 2,
	UGET_GROUP_COMPLETED  = 1 << 3,
	UGET_GROUP_UPLOADING  = 1 << 4,
	UGET_GROUP_ERROR      = 1 << 5,

	UGET_GROUP_FINISHED   = 1 << 6,
	UGET_GROUP_RECYCLED   = 1 << 7,

	UGET_GROUP_MAJOR      = UGET_GROUP_ACTIVE | UGET_GROUP_QUEUING | UGET_GROUP_FINISHED | UGET_GROUP_RECYCLED,
	UGET_GROUP_INACTIVE   = UGET_GROUP_PAUSED | UGET_GROUP_ERROR,
	UGET_GROUP_UNRUNNABLE = UGET_GROUP_PAUSED | UGET_GROUP_ERROR | UGET_GROUP_FINISHED | UGET_GROUP_RECYCLED,
	UGET_GROUP_UNFINISHED = UGET_GROUP_ACTIVE | UGET_GROUP_UPLOADING,
} UgetGroup;

struct UgetNodeNotifier
{
	// notify when node has inserted a child node.
	UgetNodeFunc    inserted;

	// notify when node has removed a child node.
	UgetNodeFunc    removed;

	// notify when a child node has updated.
	UgNotifyFunc    updated;

//	UgNotifyFunc    destroy;

	void*           data;      // extra data for user
};

struct UgetNodeSort
{
	UgCompareFunc   compare;
	int             reverse;  // TRUE or FALSE
};

struct UgetNodeControl
{
//	struct UgetNodeControl*  children;  // control of children node
	struct UgetNodeNotifier* notifier;
	struct UgetNodeSort      sort;

	// filter child of real node and decide how to insert child of fake node.
	// If real node inserted a child node, all fake nodes call this to filter.
	UgetNodeFunc             filter;
};

extern struct UgetNodeControl   uget_node_default_control;
extern struct UgetNodeNotifier  uget_node_default_notifier;

/* ----------------------------------------------------------------------------
   UgetNode
 */

UgetNode*  uget_node_new (UgetNode* node_real);
void  uget_node_free (UgetNode* node);

void  uget_node_init (UgetNode* node, UgetNode* node_real);
void  uget_node_final (UgetNode* node);

void  uget_node_clear_fake (UgetNode* node);
void  uget_node_clear_children (UgetNode* node);

void  uget_node_move (UgetNode* node, UgetNode* sibling, UgetNode* child);
void  uget_node_insert (UgetNode* node, UgetNode* sibling, UgetNode* child);
void  uget_node_remove (UgetNode* node, UgetNode* child);
void  uget_node_append (UgetNode* node, UgetNode* child);
void  uget_node_prepend (UgetNode* node, UgetNode* child);

void  uget_node_sort (UgetNode* node, UgCompareFunc cmp_func, int is_reversed);
void  uget_node_insert_sorted (UgetNode* node, UgetNode* child);
void  uget_node_reorder_by_real (UgetNode* node, UgetNode* real);
void  uget_node_reorder_by_fake (UgetNode* node, UgetNode* fake);

void  uget_node_remove_fake (UgetNode* node, UgetNode* fake);
void  uget_node_make_fake (UgetNode* node);

#define uget_node_nth_child(node,nth)         \
		(UgetNode*) ug_node_nth_child((UgNode*)node, nth)
#define uget_node_child_position(node,child)  \
		ug_node_child_position((UgNode*)node, (UgNode*)child)
UgetNode* uget_node_nth_fake (UgetNode* node, int nth);
int       uget_node_fake_position (UgetNode* node, UgetNode* fake);

// notify
void  uget_node_updated (UgetNode* node);

// ----------------------------------------------------------------------------

// JSON parser used with UG_ENTRY_ARRAY.
UgJsonError ug_json_parse_uget_node_children (UgJson* json,
                                const char* name, const char* value,
                                void* node, void* none);
// JSON writer used with UG_ENTRY_ARRAY.
void        ug_json_write_uget_node_children (UgJson* json, const UgetNode* node);

/* ----------------------------------------------------------------------------
   compare functions for UgetNode.control.sort.compare and uget_node_sort()
   these function implemented in UgetNode-compare.c
 */
int   uget_node_compare_name         (UgetNode* node1, UgetNode* node2);
int   uget_node_compare_complete     (UgetNode* node1, UgetNode* node2);
int   uget_node_compare_size         (UgetNode* node1, UgetNode* node2);
int   uget_node_compare_percent      (UgetNode* node1, UgetNode* node2);
int   uget_node_compare_elapsed      (UgetNode* node1, UgetNode* node2);
int   uget_node_compare_left         (UgetNode* node1, UgetNode* node2);
int   uget_node_compare_speed        (UgetNode* node1, UgetNode* node2);
int   uget_node_compare_upload_speed (UgetNode* node1, UgetNode* node2);
int   uget_node_compare_uploaded     (UgetNode* node1, UgetNode* node2);
int   uget_node_compare_ratio        (UgetNode* node1, UgetNode* node2);
int   uget_node_compare_retry        (UgetNode* node1, UgetNode* node2);
int   uget_node_compare_parent_name  (UgetNode* node1, UgetNode* node2);
int   uget_node_compare_uri          (UgetNode* node1, UgetNode* node2);
int   uget_node_compare_added_time   (UgetNode* node1, UgetNode* node2);
int   uget_node_compare_completed_time (UgetNode* node1, UgetNode* node2);

/* ----------------------------------------------------------------------------
   callback functions for UgetNode.control.filter (they are used by UgetApp)
   these function implemented in UgetNode-filter.c
 */
void  uget_node_filter_mix (UgetNode* node, UgetNode* sibling, UgetNode* child_real);
void  uget_node_filter_split (UgetNode* node, UgetNode* sibling, UgetNode* child_real);
void  uget_node_filter_mix_split (UgetNode* node, UgetNode* sibling, UgetNode* child_real);
void  uget_node_filter_sorted (UgetNode* node, UgetNode* sibling, UgetNode* child_real);

/*
               uget_node_filter_split()
                         v
  ,-----------.      ,--------.
  | real node | ---> | filter | --+---> fake node (UGET_GROUP_ACTIVE)
  `-----------'      `--------'   |
                                  +---> fake node (UGET_GROUP_QUEUING)
                                  |
                                  +---> fake node (UGET_GROUP_FINISHED)
                                  |
                                  `---> fake node (UGET_GROUP_RECYCLED)

 * helper functions for uget_node_filter_split(), uget_node_filter_mix_split()
   uget_node_get_split() use 'group' (UgetGroup) to find fake node.
   uget_node_get_group() return UgetGroup if it is split fake node.
 */
UgetNode* uget_node_get_split(UgetNode* node, int group);
int       uget_node_get_group(UgetNode* node);


#ifdef __cplusplus
}
#endif

struct UgetNode
{
	UG_NODE_MEMBERS(UgetNode, UgetNode, base);
/*	// ------ UgNode members ------
	UgetNode*     base;    // the realest UgetNode (real->real->real-> ...)
	UgetNode*     next;
	UgetNode*     prev;
	UgetNode*     parent;
	UgetNode*     children;
	UgetNode*     last;
	int           n_children;
 */

	UgetNode*     real;
	UgetNode*     fake;
	UgetNode*     peer;

	UgInfo*       info;
	struct UgetNodeControl*  control;

#ifdef __cplusplus
	inline void* operator new(size_t size, UgetNode* node_real = NULL)
		{ return uget_node_new(node_real); }
	inline void  operator delete(void* p)
		{ uget_node_free((UgetNode*)p); }

	inline void init(UgetNode* node_real = NULL)
		{ uget_node_init(this, node_real); }
	inline void final()
		{ uget_node_final(this); }

	inline void clearFake(void)
		{ uget_node_clear_fake(this); }
	inline void clearChildren(void)
		{ uget_node_clear_children(this); }

	inline void move(UgetNode* sibling, UgetNode* child)
		{ uget_node_move(this, sibling, child); }
	inline void insert(UgetNode* sibling, UgetNode* child)
		{ uget_node_insert(this, sibling, child); }
	inline void remove(UgetNode* child)
		{ uget_node_remove(this, child); }
	inline void append(UgetNode* child)
		{ uget_node_append(this, child); }
	inline void prepend(UgetNode* child)
		{ uget_node_prepend(this, child); }

	inline UgetNode* nthChild(int nth)
		{ return uget_node_nth_child(this, nth); }
	inline int  childPosition(UgetNode* child)
		{ return uget_node_child_position(this, child); }

	inline UgetNode* nthFake(int nth)
		{ return uget_node_nth_fake(this, nth); }
	inline int  fakePosition(UgetNode* fake)
		{ return uget_node_fake_position(this, fake); }

	inline UgetNode* getSplit(int group)
		{ return uget_node_get_split(this, group); }
	inline int  getGroup()
		{ return uget_node_get_group(this); }
#endif  // __cplusplus
};

// ----------------------------------------------------------------------------
// C++11 standard-layout

#ifdef __cplusplus

namespace Uget
{
// This one is for directly use only. You can NOT derived it.
typedef struct UgetNode    Node;
};  // namespace Uget

#endif  // __cplusplus


#endif  // UGET_NODE_H

