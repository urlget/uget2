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

// Uget Tree chart:
//
// Root -+- Category1 -+- Download1 (URI) -+- File
//       |             |                   |
//       |             |                   +  cookie or post file (attachment)
//       |             |
//       |             +- Download2 (URI) -+- File1
//       |             |  (torrent path)   |
//       |             |                   +- File2
//       |             |                   |
//       |             |                   +- torrent file (attachment)
//       |             |
//       |             +- Download3 (URI) -+- File
//       |                (metalink path)  |
//       |                                 +- metalink file (attachment)
//       +-- Category2
//

// UgNode   is base node type.
// UgetNode extend from UgNode and add pointers (real, fake, and peer).
//
//            prev   /               prev   /
//              |  fake                |  fake
//              |  /                   |  /
//              | /                    | /
//              |/                     |/
// ... <---> parent <--------------> child <----> ...
//             /|                     /|
//            / |                    / |
//           /  |                   /  |
//        real  |                real  |
//         /    |                 /    |
//             next                   next
//

#include <stddef.h>    // offsetof ()
#include <stdint.h>    // int16_t
#include <UgNode.h>
#include <UgInfo.h>
#include <UgUri.h>

#ifdef __cplusplus
extern "C" {
#endif

// ----------------------------------------------------------------------------
// UgetNode

typedef struct UgetTree         UgetTree;
typedef struct UgetNode         UgetNode;

typedef void (*UgetNodeFunc)(UgetNode* node, UgetNode* sibling, UgetNode* child);

extern const UgEntry  UgetNodeEntry[];

typedef enum {
	UGET_NODE_ROOT,
	UGET_NODE_CATEGORY,
	UGET_NODE_DOWNLOAD,
	UGET_NODE_FILE,
	UGET_NODE_FOLDER,
	UGET_NODE_ATTACHMENT,
	UGET_NODE_REMOTE_FILE,
} UgetNodeType;

typedef enum {
	UGET_STATE_QUEUING    = 1 << 0,

	UGET_STATE_PAUSED     = 1 << 1,
	UGET_STATE_ACTIVE     = 1 << 2,
	UGET_STATE_COMPLETED  = 1 << 3,
	UGET_STATE_UPLOADING  = 1 << 4,
	UGET_STATE_ERROR      = 1 << 5,

	UGET_STATE_FINISHED   = 1 << 6,
	UGET_STATE_RECYCLED   = 1 << 7,

	UGET_STATE_CATEGORY   = UGET_STATE_ACTIVE | UGET_STATE_QUEUING | UGET_STATE_FINISHED | UGET_STATE_RECYCLED,
	UGET_STATE_INACTIVE   = UGET_STATE_PAUSED | UGET_STATE_ERROR,
	UGET_STATE_UNRUNNABLE = UGET_STATE_PAUSED | UGET_STATE_ERROR | UGET_STATE_FINISHED | UGET_STATE_RECYCLED,
	UGET_STATE_UNFINISHED = UGET_STATE_ACTIVE | UGET_STATE_UPLOADING,
} UgetState;

struct UgetNodeNotifier
{
	// notify when node has inserted a child node.
	UgetNodeFunc    inserted;

	// notify when node has removed a child node.
	UgetNodeFunc    removed;

	// notify when a child node has updated.
	UgNotifyFunc    updated;

//	UgNotifyFunc    destroy;
};

struct UgetNodeSort
{
	UgCompareFunc   compare;
	int             reverse;  // TRUE or FALSE
};

struct UgetNodeControl
{
	struct UgetNodeControl*  children;  // control of children node
	struct UgetNodeNotifier  notifier;
	struct UgetNodeSort      sort;

	// fake filter node from real.
	// If real node inserted a child node, all fake nodes call this to filter.
	UgetNodeFunc             filter;

	void*                    data;      // extra data for user
};

struct UgetNode
{
	UG_NODE_MEMBERS(UgetNode, UgetNode, data);
//	UgetNode*     data;
//	UgetNode*     next;
//	UgetNode*     prev;
//	UgetNode*     parent;
//	UgetNode*     children;
//	UgetNode*     last;
//	int           n_children;

	UgetNode*     real;
	UgetNode*     fake;
	UgetNode*     peer;

	int           ref_count;

	char*         name;    // fake node doesn't use this
	int           type;    // UgetNodeType
	int           state;   // UgetState
	UgInfo        info;    // fake node doesn't use this

	struct UgetNodeControl*  control;
};

UgetNode*  uget_node_new (UgetNode* node_real);
void  uget_node_init  (UgetNode* node, UgetNode* node_real);

void  uget_node_ref   (UgetNode* node);
void  uget_node_unref (UgetNode* node);
void  uget_node_unref_fake (UgetNode* node);
void  uget_node_unref_children (UgetNode* node);

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
UgetNode* uget_node_fake_from_state (UgetNode* node, int state);
int       uget_node_fake_position (UgetNode* node, UgetNode* fake);

// ----------------------------------------------------------------------------

void  uget_node_set_name_by_uri (UgetNode* node, UgUri* uuri);
void  uget_node_set_name_by_uri_string (UgetNode* node, const char* uri);

// notify
void  uget_node_updated (UgetNode* node);

// for UgetNode.children
// JSON parser used with UG_ENTRY_ARRAY.
UgJsonError ug_json_parse_uget_node_children (UgJson* json,
                                const char* name, const char* value,
                                void* node, void* none);
// JSON writer used with UG_ENTRY_ARRAY.
void        ug_json_write_uget_node_children (UgJson* json, const UgetNode* node);

// ----------------------------------------------------------------------------
// compare functions for uget_node_sort()
// UgetNode-compare.c

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

// ----------------------------------------------------------------------------
// callback functions for UgetNode.control.filter
// These functions used by UgetApp
void  uget_node_filter_mix (UgetNode* node, UgetNode* sibling, UgetNode* child_real);
void  uget_node_filter_split (UgetNode* node, UgetNode* sibling, UgetNode* child_real);
void  uget_node_filter_mix_split (UgetNode* node, UgetNode* sibling, UgetNode* child_real);
void  uget_node_filter_sorted (UgetNode* node, UgetNode* sibling, UgetNode* child_real);


#ifdef __cplusplus
}
#endif

// ----------------------------------------------------------------------------
// C++11 standard-layout

#ifdef __cplusplus

namespace Uget
{

struct TreeMethod {};

struct Node : Ug::NodeMethod, UgetNode
{
};

};  // namespace Uget

#endif  // __cplusplus


#endif  // UGET_NODE_H

