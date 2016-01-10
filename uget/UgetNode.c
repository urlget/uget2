/*
 *
 *   Copyright (C) 2012-2016 by C.H. Huang
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <stdint.h>
#include <UgUtil.h>
#include <UgString.h>
#include <UgetNode.h>
#include <UgetData.h>

#ifdef HAVE_GLIB
#include <glib.h>    // g_slice_xxx
#endif

// ----------------------------------------------------------------------------
// UgetNode

static struct UgetNodeNotification  notification = {
	(UgetNodeFunc) NULL, NULL, NULL,
	(UgNotifyFunc) NULL, NULL, 0, NULL
};

const UgEntry  UgetNodeEntry[] =
{
	{"name",     offsetof (UgetNode, name),  UG_ENTRY_STRING, NULL, NULL},
	{"type",     offsetof (UgetNode, type),  UG_ENTRY_INT,    NULL, NULL},
	{"state",    offsetof (UgetNode, state), UG_ENTRY_INT,    NULL, NULL},
	{"info",     offsetof (UgetNode, info),  UG_ENTRY_CUSTOM,
			ug_json_parse_info,   ug_json_write_info},
	{"children", 0,                          UG_ENTRY_ARRAY,
			ug_json_parse_uget_node_children, ug_json_write_uget_node_children},
	{NULL}    // null-terminated
};

UgetNode*  uget_node_new (UgetNode* node_real)
{
	UgetNode*  node;

#ifdef HAVE_GLIB
	node = g_slice_alloc (sizeof (UgetNode));
#else
	node = ug_malloc (sizeof (UgetNode));
#endif // HAVE_GLIB
	uget_node_init (node, node_real);
	return node;
}

void  uget_node_init  (UgetNode* node, UgetNode* node_real)
{
	memset (node, 0, sizeof (UgetNode));

	node->ref_count = 1;
	node->notification = &notification;

	if (node_real == NULL) {
		node->data = node;    // pointer to self
		ug_info_init (&node->info, 8, 2);
		node->info.at[0].key = (void*) UgetRelationInfo;
		node->info.at[1].key = (void*) UgetProgressInfo;
	}
	else {
		// this is a fake node.
//		node->state = 0;
		node->data = node_real->data;
		node->type = node_real->type;
		node->real = node_real;
		node->peer = node_real->fake;
		node_real->fake = node;
		ug_info_init (&node->info, 0, 0);
	}
}

void  uget_node_ref (UgetNode* node)
{
	node->ref_count++;
}

void  uget_node_unref (UgetNode* node)
{
	if (--node->ref_count == 0) {
		if (node->parent)
			uget_node_remove (node->parent, node);
		if (node->real)
			uget_node_remove_fake (node->real, node);

		uget_node_unref_fake (node);
		uget_node_unref_children (node);
//		ug_node_unlink ((UgNode*)node);
		ug_info_final (&node->info);
		ug_free (node->name);

#ifdef HAVE_GLIB
		g_slice_free1 (sizeof (UgetNode), node);
#else
		ug_free (node);
#endif // HAVE_GLIB
	}
}

void  uget_node_unref_children (UgetNode* node)
{
	UgetNode*  next;
	UgetNode*  children;

	for (children = node->children;  children;  children = next) {
		next = children->next;
		uget_node_unref (children);
	}
	node->children = NULL;
}

void  uget_node_unref_fake (UgetNode* node)
{
	UgetNode*  peer;
	UgetNode*  fake;

	for (fake = node->fake;  fake;  fake = peer) {
		peer = fake->peer;
		fake->real = NULL;   // speed up uget_node_unref()
		uget_node_unref (fake);
	}
	node->fake = NULL;       // speed up uget_node_unref()
}

void  uget_node_move (UgetNode* node, UgetNode* sibling, UgetNode* child)
{
	UgetNode*  fake_sibling;
	UgetNode*  fake_child;

	ug_node_remove ((UgNode*) node, (UgNode*) child);
	ug_node_insert ((UgNode*) node, (UgNode*) sibling, (UgNode*) child);

	fake_sibling = NULL;
	for (fake_child = child->fake;  fake_child;  fake_child = fake_child->peer) {
		node = fake_child->parent;

		if (sibling) {
			for (fake_sibling = sibling->fake;  fake_sibling;  fake_sibling = fake_sibling->peer) {
				if (fake_sibling->parent == node)
					break;
			}
			if (fake_sibling == NULL)
				continue;
		}

		uget_node_move (node, fake_sibling, fake_child);
	}
}

void  uget_node_insert (UgetNode* node, UgetNode* sibling, UgetNode* child)
{
	UgetNodeFunc inserted;

	ug_node_insert ((UgNode*) node, (UgNode*) sibling, (UgNode*) child);
	child->notification = node->notification;

	inserted = node->notification->inserted;
	if (inserted)
		inserted (node, sibling, child);

	uget_node_created (node, sibling, child);
}

static void  uget_node_unlink_children_real (UgetNode* node)
{
	for (node = node->children;  node;  node = node->next) {
		uget_node_unlink_children_real (node);
		if (node->real)
			uget_node_remove_fake (node->real, node);
	}
}

static void  uget_node_unlink_fake_parent (UgetNode* child)
{
	UgetNode*    parent;
	UgetNode*    sibling;
	UgetNodeFunc removed;

	for (child = child->fake;  child;  child = child->peer) {
		uget_node_unlink_fake_parent (child);
		parent = child->parent;
		if (parent == NULL)
			continue;
		sibling = child->next;
		ug_node_remove ((UgNode*) parent, (UgNode*) child);
		// notify
		removed = parent->notification->removed;
		if (removed)
			removed (parent, sibling, child);
	}
}

void  uget_node_remove (UgetNode* node, UgetNode* child)
{
	UgetNode*    sibling;
	UgetNodeFunc removed;

	sibling = child->next;
	ug_node_remove ((UgNode*)node, (UgNode*)child);
	uget_node_unlink_fake_parent (child);
	uget_node_unlink_children_real (child);

	removed = node->notification->removed;
	if (removed)
		removed (node, sibling, child);
}

void  uget_node_append (UgetNode* node, UgetNode* child)
{
	UgetNodeFunc inserted;

	ug_node_append ((UgNode*) node, (UgNode*) child);
	child->notification = node->notification;

	inserted = node->notification->inserted;
	if (inserted)
		inserted (node, NULL, child);

	uget_node_created (node, NULL, child);
}

void  uget_node_prepend (UgetNode* node, UgetNode* child)
{
	UgetNode*    sibling;
	UgetNodeFunc inserted;

	sibling = node->children;
	ug_node_prepend ((UgNode*) node, (UgNode*) child);
	child->notification = node->notification;

	inserted = node->notification->inserted;
	if (inserted)
		inserted (node, sibling, child);

	uget_node_created (node, sibling, child);
}

void  uget_node_sort (UgetNode* node, UgCompareFunc compare, int reversed)
{
	UgetNode* beg;
	UgetNode* cur;
	UgetNode* unsorted;

	for (beg = node->children;  beg;  beg = unsorted->next) {
		unsorted = beg;
		if (reversed == FALSE) {
			for (cur = beg->next;  cur;  cur = cur->next) {
				if (compare (cur, unsorted) < 0)
					unsorted = cur;
			}
		}
		else {
			for (cur = beg->next;  cur;  cur = cur->next) {
				if (compare (unsorted, cur) < 0)
					unsorted = cur;
			}
		}

		if (unsorted != beg)
			uget_node_move (node, beg, unsorted);
	}
}

void  uget_node_insert_sorted (UgetNode* node, UgetNode* child)
{
	UgCompareFunc  compare;
	UgetNode*      cur;
	int            reversed;

	compare = node->notification->compare;
	reversed = node->notification->reversed;
	if (compare == NULL)
		return;

	if (reversed == FALSE) {
		for (cur = node->children;  cur;  cur = cur->next) {
			if (compare (cur, child) > 0) {
				uget_node_insert (node, cur, child);
				return;
			}
		}
	}
	else {
		for (cur = node->children;  cur;  cur = cur->next) {
			if (compare (child, cur) > 0) {
				uget_node_insert (node, cur, child);
				return;
			}
		}
	}
	uget_node_insert (node, NULL, child);
}

void  uget_node_reorder_by_real (UgetNode* node, UgetNode* real)
{
	UgetNode*  sibling;
	UgetNode*  fake;

	sibling = node->children;
	if (real == NULL)
		real = node->real;
	if (real == NULL)
		return;

	for (real = real->children;  real;  real = real->next) {
		for (fake = real->fake;  fake;  fake = fake->peer) {
			if (fake->parent != node)
				continue;
			if (fake == sibling) {
				sibling = sibling->next;
				break;
			}
			ug_node_remove ((UgNode*) node, (UgNode*) fake);
			ug_node_insert ((UgNode*) node, (UgNode*) sibling, (UgNode*) fake);
			break;
		}
	}
}

void  uget_node_reorder_by_fake (UgetNode* node, UgetNode* fake)
{
	UgetNode*  sibling;
	UgetNode*  real;

	if (fake == NULL || fake->children == NULL)
		return;
	sibling = fake->children->real;

	for (fake = fake->children;  fake;  fake = fake->next) {
		real = fake->real;
		if (real == NULL || real->parent != node)
			continue;
		if (real == sibling)
			sibling = sibling->next;
		ug_node_remove ((UgNode*) node, (UgNode*) real);
		ug_node_insert ((UgNode*) node, (UgNode*) sibling, (UgNode*) real);
	}
}

void  uget_node_remove_fake (UgetNode* node, UgetNode* fake)
{
	UgetNode*    curr;
	UgetNode*    prev;

	for (prev = NULL, curr = node->fake;  curr;  curr = curr->peer) {
		if (curr == fake) {
			if (prev)
				prev->peer = curr->peer;
			else
				node->fake = curr->peer;
			curr->real = NULL;
			curr->peer = NULL;
			return;
		}
		prev = curr;
	}
}

void  uget_node_make_fake (UgetNode* node)
{
	UgetNode*  child;
//	UgetNode*  sibling;

	if (node->fake == NULL)
		return;
	for (child = node->children;  child;  child = child->next) {
		uget_node_created (node, NULL, child);
		uget_node_make_fake (child);
	}
}

// ----------------------------------------------------------------------------
// position
UgetNode* uget_node_nth_fake (UgetNode* node, int nth)
{
	UgetNode*  fake;

	for (fake = node->fake;  fake;  fake = fake->peer, nth--) {
		if (nth == 0)
			return fake;
	}
	return NULL;
}

UgetNode* uget_node_fake_from_state (UgetNode* node, int state)
{
	for (node = node->fake;  node;  node = node->peer) {
		if (node->state & state)
			return node;
	}
	return NULL;
}

int  uget_node_fake_position (UgetNode* node, UgetNode* fake)
{
	int  position = 0;

	for (node = node->fake;  node;  node = node->peer, position++) {
		if (node == fake)
			return position;
	}
	return -1;
}

// ----------------------------------------------------------------------------

void  uget_node_set_name_by_uri (UgetNode* node, UgUri* uuri)
{
	const char* filename;
	int         length;

	ug_free (node->name);

	if (uuri->scheme_len == 6 && strncmp (uuri->uri, "magnet", 6) == 0) {
		length = 0;
		filename = strstr (uuri->uri + uuri->file, "dn=");
		if (filename) {
			filename = filename + 3;
			length = strcspn (filename, "&");
			node->name = ug_malloc (length + 1);
			ug_decode_uri (filename, length, node->name);
			if (ug_utf8_get_invalid ((uint8_t*) node->name, NULL) != -1) {
				ug_free (node->name);
				node->name = ug_strndup (filename, length);
			}
		}
	}
	else {
		length = ug_uri_file (uuri, &filename);
		if (length == 0)
			node->name = ug_strdup (uuri->uri);
		else
			node->name = ug_uri_get_file (uuri);
	}
}

void  uget_node_set_name_by_uri_string (UgetNode* node, const char* uri)
{
	UgUri  uuri;

	ug_uri_init (&uuri, uri);
	uget_node_set_name_by_uri (node, &uuri);
}

// ----------------------------------------------------------------------------
// notify

void  uget_node_created (UgetNode* parent, UgetNode* sibling, UgetNode* child)
{
	UgetNode*    fake;
	UgetNodeFunc created;

	for (fake = parent->fake;  fake;  fake = fake->peer) {
		created = fake->notification->created;
		if (created)
			created (fake, sibling, child);
	}
}

void  uget_node_updated (UgetNode* node)
{
	UgNotifyFunc updated;

	updated = node->notification->updated;
	if (updated)
		updated (node);
	// update fake node
	for (node = node->fake;  node;  node = node->peer)
		uget_node_updated (node);
}

// ----------------------------------------------------------------------------
// JSON

UgJsonError ug_json_parse_uget_node_children (UgJson* json,
                                const char* name, const char* value,
                                void* node, void* none)
{
	UgetNode*  temp;

	if (json->type != UG_JSON_OBJECT) {
//		if (json->type == UG_JSON_ARRAY)
//			ug_json_push (json, ug_json_parse_unknown, NULL, NULL);
		return UG_JSON_ERROR_TYPE_NOT_MATCH;
	}

	temp = uget_node_new (NULL);
	uget_node_append (node, temp);
	ug_json_push (json, ug_json_parse_entry, temp, (void*)UgetNodeEntry);
	return UG_JSON_ERROR_NONE;
}

void  ug_json_write_uget_node_children (UgJson* json, const UgetNode* node)
{
	for (node = node->children;  node;  node = node->next) {
		ug_json_write_object_head (json);
		ug_json_write_entry (json, (void*) node, UgetNodeEntry);
		ug_json_write_object_tail (json);
	}
}

// ----------------------------------------------------------------------------
// callback functions for UgetNode.notification.created
// These functions used by UgetApp

// sibling_real, child_real
void  uget_node_create_split (UgetNode* node, UgetNode* sibling, UgetNode* child_real)
{
	UgetNode*  child;

	if (node->parent == NULL) {
		// node is root. child_real is category
		//
		child = uget_node_new (child_real);
		child->state = UGET_STATE_RECYCLED;
		uget_node_prepend (node, child);
		//
		child = uget_node_new (child_real);
		child->state = UGET_STATE_FINISHED;
		uget_node_prepend (node, child);
		//
		child = uget_node_new (child_real);
		child->state = UGET_STATE_QUEUING;
		uget_node_prepend (node, child);
		//
		child = uget_node_new (child_real);
		child->state = UGET_STATE_ACTIVE;
		uget_node_prepend (node, child);
	}
	else if (node->parent->parent == NULL) {
		// node is category. child_real is download
		child = child_real->data;
		if ((node->state & child->state) ||
		    ( (child->state & UGET_STATE_CATEGORY) == 0 &&
		      (node->state & UGET_STATE_QUEUING) ) )
		{
			// insert sorted
			if (node->notification->compare) {
				uget_node_insert_sorted (node, uget_node_new (child_real));
				return;
			}

			// original order
			if (sibling) {
				for (sibling = sibling->fake;  sibling;  sibling = sibling->peer) {
					if (sibling->parent == node)
						break;
				}
			}
			uget_node_insert (node, sibling, uget_node_new (child_real));
		}
	}
}

void  uget_node_create_sorted (UgetNode* node, UgetNode* sibling, UgetNode* child)
{
	child = uget_node_new (child);

	if (node->parent == NULL) {
		// node is root.
		uget_node_append (node, child);
	}
	else if (node->parent->parent == NULL) {
		// node is category.
		// insert sorted
		if (node->notification->compare) {
			uget_node_insert_sorted (node, child);
			return;
		}

		// original order
		if (sibling) {
			for (sibling = sibling->fake;  sibling;  sibling = sibling->peer) {
				if (sibling->parent == node)
					break;
			}
		}

		uget_node_insert (node, sibling, child);
	}
}

// This one is not the same with uget_node_create_sorted()
// sibling_real, child_real
void  uget_node_create_mix (UgetNode* node, UgetNode* sibling, UgetNode* child)
{
	UgetNode*  fake;

	child = uget_node_new (child);

	if (node->parent == NULL) {
		// node is root.
		uget_node_append (node, child);
	}
	else if (node->parent->parent == NULL) {
		// node is category.
		// insert sorted
		node = node->parent->children;
		if (node->notification->compare) {
			// add all download to first category
			uget_node_insert_sorted (node, child);
			return;
		}

		// reorder by state
		if ( sibling &&
		    ((sibling->data->state & UGET_STATE_CATEGORY) !=
		     (child->data->state   & UGET_STATE_CATEGORY)) )
		{
			sibling = NULL;
		}
		if (node->fake && sibling == NULL) {
			switch (child->data->state & UGET_STATE_CATEGORY) {
			case UGET_STATE_ACTIVE:
				fake = uget_node_fake_from_state (node, UGET_STATE_QUEUING);
				if (fake == NULL || fake->children == NULL)
					fake = uget_node_fake_from_state (node, UGET_STATE_FINISHED);
				if (fake == NULL || fake->children == NULL)
					fake = uget_node_fake_from_state (node, UGET_STATE_RECYCLED);
				break;

			case UGET_STATE_QUEUING:
			default:
				fake = uget_node_fake_from_state (node, UGET_STATE_FINISHED);
				if (fake == NULL || fake->children == NULL)
					fake = uget_node_fake_from_state (node, UGET_STATE_RECYCLED);
				break;

			case UGET_STATE_FINISHED:
				fake = uget_node_fake_from_state (node, UGET_STATE_RECYCLED);
				break;

			case UGET_STATE_RECYCLED:
				fake = NULL;
				break;
			}
			if (fake && fake->children) {
				uget_node_insert (node, fake->children->real, child);
				return;
			}
		}

		// original order
		if (sibling) {
			for (sibling = sibling->fake;  sibling;  sibling = sibling->peer) {
//				if (sibling->parent == node)
				if (sibling->parent == node)
					break;
			}
		}
		// insert childNode to first (mixed) category
		uget_node_insert (node, sibling, child);
	}
}

void  uget_node_create_mix_split (UgetNode* node, UgetNode* sibling, UgetNode* child_real)
{
	UgetNode*  real;

	// if REAL category node is not first child, this function do nothing.
	real = node->real;
	if (real->parent == NULL && real->children != child_real)
		return;
	uget_node_create_split (node, sibling, child_real);
}
