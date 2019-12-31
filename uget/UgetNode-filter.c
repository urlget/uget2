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

#include <UgetNode.h>
#include <UgetData.h>

#define N_SPLIT_GROUPS      4

static int   split_groups[N_SPLIT_GROUPS] =
{
	UGET_GROUP_ACTIVE,
	UGET_GROUP_QUEUING,
	UGET_GROUP_FINISHED,
	UGET_GROUP_RECYCLED,
};

// ----------------------------------------------------------------------------
// callback functions for UgetNode.control.filter

// sibling_real, child_real
void  uget_node_filter_split (UgetNode* node, UgetNode* sibling, UgetNode* child_real)
{
	UgetRelation* relation;
	UgetNode*     child;
	int           group;

	if (node->parent == NULL) {
		// node is root. child_real is category
		for (group=0;  group < N_SPLIT_GROUPS;  group++) {
			child = uget_node_new (child_real);
			uget_node_prepend (node, child);
		}
	}
	else if (node->parent->parent == NULL) {
		// node is category. child_real is download
		child = child_real->base;
		relation = ug_info_realloc(child->info, UgetRelationInfo);
		group = uget_node_get_group(node);
		if ((relation->group & UGET_GROUP_MAJOR) == 0)
			relation->group |= UGET_GROUP_QUEUING;
		if (group & relation->group) {
			// insert sorted
			if (node->control->sort.compare) {
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

void  uget_node_filter_sorted (UgetNode* node, UgetNode* sibling, UgetNode* child)
{
	child = uget_node_new (child);

	if (node->parent == NULL) {
		// node is root.
		uget_node_append (node, child);
	}
	else if (node->parent->parent == NULL) {
		// node is category.
		// insert sorted
		if (node->control->sort.compare) {
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

// sibling_real, child_real
void  uget_node_filter_mix (UgetNode* node, UgetNode* sibling, UgetNode* child)
{
	UgetNode*     fake;
	UgetRelation* relation_child;
	UgetRelation* relation_sibling;

	child = uget_node_new (child);

	if (node->parent == NULL) {
		// node is root.
		uget_node_append (node, child);
	}
	else if (node->parent->parent == NULL) {
		// node is category.
		// insert sorted
		node = node->parent->children;
		if (node->control->sort.compare) {
			// add all download to first category
			uget_node_insert_sorted (node, child);
			return;
		}

		// reorder by group
		relation_child = ug_info_realloc(child->info, UgetRelationInfo);
		if (sibling) {
			relation_sibling = ug_info_realloc(sibling->info, UgetRelationInfo);
			if ((relation_sibling->group & UGET_GROUP_MAJOR) !=
			    (relation_child->group   & UGET_GROUP_MAJOR))
			{
				sibling = NULL;
			}
		}
		// get inserting position by group
		if (node->fake && sibling == NULL) {
			switch (relation_child->group & UGET_GROUP_MAJOR) {
			case UGET_GROUP_ACTIVE:
				fake = uget_node_get_split(node, UGET_GROUP_QUEUING);
				if (fake == NULL || fake->children == NULL)
					fake = uget_node_get_split(node, UGET_GROUP_FINISHED);
				if (fake == NULL || fake->children == NULL)
					fake = uget_node_get_split(node, UGET_GROUP_RECYCLED);
				break;

			case UGET_GROUP_QUEUING:
			default:
				fake = uget_node_get_split(node, UGET_GROUP_FINISHED);
				if (fake == NULL || fake->children == NULL)
					fake = uget_node_get_split(node, UGET_GROUP_RECYCLED);
				break;

			case UGET_GROUP_FINISHED:
				fake = uget_node_get_split(node, UGET_GROUP_RECYCLED);
				break;

			case UGET_GROUP_RECYCLED:
				fake = NULL;
				break;
			}
			// insert into specified position by group
			if (fake && fake->children) {
				uget_node_insert (node, fake->children->real, child);
				return;
			}
		}

		// original order
		if (sibling) {
			for (sibling = sibling->fake;  sibling;  sibling = sibling->peer) {
				if (sibling->parent == node)
					break;
			}
		}
		// insert childNode to first (mixed) category
		uget_node_insert (node, sibling, child);
	}
}

void  uget_node_filter_mix_split (UgetNode* node, UgetNode* sibling, UgetNode* child_real)
{
	UgetNode*  real;

	// if REAL category node is not first child, this function do nothing.
	real = node->real;
	if (real->parent == NULL && real->children != child_real)
		return;
	uget_node_filter_split (node, sibling, child_real);
}

// ----------------------------------------------------------------------------
// helper functions for uget_node_filter_split(), uget_node_filter_mix_split()

UgetNode* uget_node_get_split(UgetNode* node, int group)
{
	UgetNodeFunc  filter;
	int           nth;

	for (nth = 0, node = node->fake;  node;  node = node->peer) {
		filter = node->control->filter;
		if (filter == uget_node_filter_split ||
		    filter == uget_node_filter_mix_split)
		{
			if (split_groups[nth] & group)
				return node;
			nth++;  // must place here
		}
	}
	return NULL;
}

int   uget_node_get_group(UgetNode* node)
{
	UgetNodeFunc  filter;
	UgetNode*     fake;
	int           nth;

	if (node->real == NULL)
		return UGET_GROUP_NULL;
	for (nth = 0, fake = node->real->fake;  fake;  fake = fake->peer) {
		filter = fake->control->filter;
		if (filter == uget_node_filter_split ||
		    filter == uget_node_filter_mix_split)
		{
			if (fake == node)
				return split_groups[nth];
			nth++;  // must place here
		}
	}
	return UGET_GROUP_NULL;
}
