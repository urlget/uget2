/*
 *
 *   Copyright (C) 2012-2015 by C.H. Huang
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

#ifndef UG_NODE_H
#define UG_NODE_H

#include <memory.h>    // memset

#ifdef __cplusplus
extern "C" {
#endif

typedef struct	UgNode          UgNode;

// ----------------------------------------------------------------------------
// UgNode: Data Node for all data, compatible with GNode in glib

#define	UG_NODE_MEMBERS(NodeType, DataType, DataName)  \
	DataType*  DataName;   \
	NodeType*  next;       \
	NodeType*  prev;       \
	NodeType*  parent;     \
	NodeType*  children;   \
	NodeType*  last;       \
	int        n_children

struct UgNode
{
	UG_NODE_MEMBERS (UgNode, void, data);
//	void*    data;
//	UgNode*  next;
//	UgNode*  prev;
//	UgNode*  parent;
//	UgNode*  children;
//	UgNode*  last;
//	int      n_children;
};

UgNode* ug_node_new  (void);
void    ug_node_free (UgNode* link);

void    ug_node_init (UgNode* node);

void    ug_node_reverse (UgNode* node);
void    ug_node_prepend (UgNode* parent, UgNode* node);
void    ug_node_append (UgNode* parent, UgNode* node);
void    ug_node_insert (UgNode* parent, UgNode* sibling, UgNode* node);
void    ug_node_remove (UgNode* parent, UgNode* node);
void    ug_node_unlink (UgNode* node);

UgNode* ug_node_nth_child (UgNode* node, int nth);
int     ug_node_child_position (UgNode* node, UgNode* child);

#ifdef __cplusplus
}
#endif

// ----------------------------------------------------------------------------
// C++11 standard-layout

#ifdef __cplusplus

namespace Ug
{

// This one is for derived use only. No data members here.
// Your derived struct/class must be C++11 standard-layout
struct NodeMethod
{
	inline void init (void)
		{ ug_node_init ((UgNode*)this); }

	inline void  prepend (UgNode* node)
		{ ug_node_prepend ((UgNode*)this, node); }
	inline void  append (UgNode* node)
		{ ug_node_append ((UgNode*)this, node); }
	inline void  insert (UgNode* sibling, UgNode* node)
		{ ug_node_insert ((UgNode*)this, sibling, node); }
	inline void  remove (UgNode* node)
		{ ug_node_remove ((UgNode*)this, node); }
	inline void  unlink (void)
		{ ug_node_unlink ((UgNode*)this); }
};

// This one is for directly use only. You can NOT derived it.
struct Node : NodeMethod, UgNode
{
	inline Node (void)
		{ ug_node_init ((UgNode*)this);  }
};

};  // namespace Ug

#endif  // __cplusplus


#endif  // UG_NODE_H

