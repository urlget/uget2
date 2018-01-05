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

/*
 * This file base on GTK+ 2.0 Tree View Tutorial - custom-list.c
 */

#include <UgtkNodeTree.h>
#define NODE_TREE_N_COLUMNS    1

/* boring declarations of local functions */

static void      init (UgtkNodeTree* ugtree);

static void      class_init (UgtkNodeTreeClass *klass);

static void      tree_model_init (GtkTreeModelIface *iface);

static void      finalize (GObject* object);

static GtkTreeModelFlags  get_flags (GtkTreeModel* tree_model);

static gint      get_n_columns (GtkTreeModel* tree_model);

static GType     get_column_type (GtkTreeModel* tree_model, gint index);

static gboolean  get_iter (GtkTreeModel* tree_model, GtkTreeIter* iter, GtkTreePath* path);

static GtkTreePath*  get_path (GtkTreeModel* tree_model, GtkTreeIter* iter);

static void      get_value (GtkTreeModel* tree_model, GtkTreeIter* iter, gint column, GValue* value);

static gboolean  iter_next (GtkTreeModel* tree_model, GtkTreeIter* iter);

static gboolean  iter_children (GtkTreeModel* tree_model, GtkTreeIter* iter, GtkTreeIter* parent);

static gboolean  iter_has_child (GtkTreeModel* tree_model, GtkTreeIter* iter);

static gint      iter_n_children (GtkTreeModel* tree_model, GtkTreeIter* iter);

static gboolean  iter_nth_child (GtkTreeModel* tree_model, GtkTreeIter* iter, GtkTreeIter* parent, gint n);

static gboolean  iter_parent (GtkTreeModel* tree_model, GtkTreeIter* iter, GtkTreeIter* child);



static GObjectClass* parent_class = NULL;  /* GObject stuff - nothing to worry about */


/*****************************************************************************
 *
 *  class_init: more boilerplate GObject/GType stuff.
 *              Init callback for the type system,
 *              called once when our new class is created.
 *
 *****************************************************************************/

static void  class_init (UgtkNodeTreeClass* klass)
{
	GObjectClass *object_class;

	parent_class = (GObjectClass*) g_type_class_peek_parent (klass);
	object_class = (GObjectClass*) klass;

	object_class->finalize = finalize;
}

/*****************************************************************************
 *
 *  tree_model_init: init callback for the interface registration
 *                   in ugtk_node_tree_get_type. Here we override
 *                   the GtkTreeModel interface functions that
 *                   we implement.
 *
 *****************************************************************************/

static void  tree_model_init (GtkTreeModelIface* iface)
{
	iface->get_flags       = get_flags;
	iface->get_n_columns   = get_n_columns;
	iface->get_column_type = get_column_type;
	iface->get_iter        = get_iter;
	iface->get_path        = get_path;
	iface->get_value       = get_value;
	iface->iter_next       = iter_next;
	iface->iter_children   = iter_children;
	iface->iter_has_child  = iter_has_child;
	iface->iter_n_children = iter_n_children;
	iface->iter_nth_child  = iter_nth_child;
	iface->iter_parent     = iter_parent;
}


/*****************************************************************************
 *
 *  init: this is called everytime a new object object
 *        instance is created (we do that in g_object_new).
 *        Initialise the list structure's fields here.
 *
 *****************************************************************************/

static void  init (UgtkNodeTree* utree)
{
	utree->stamp = g_random_int();  /* Random int to check whether an iter belongs to our model */
}


/*****************************************************************************
 *
 *  finalize: this is called just before a object is
 *            destroyed. Free dynamically allocated memory here.
 *
 *****************************************************************************/

static void  finalize (GObject* object)
{
//	UgtkNodeTree *utree = UGTK_NODE_TREE(object);

	// must chain up - finalize parent
	(*parent_class->finalize) (object);
}


/*****************************************************************************
 *
 *  get_flags: tells the rest of the world whether our tree model
 *             has any special characteristics. In our case,
 *             we have a list model (instead of a tree), and each
 *             tree iter is valid as long as the row in question
 *             exists, as it only contains a pointer to our struct.
 *
 *****************************************************************************/

static GtkTreeModelFlags  get_flags (GtkTreeModel* tree_model)
{
	g_return_val_if_fail (UGTK_IS_NODE_TREE(tree_model), (GtkTreeModelFlags)0);

	if (UGTK_NODE_TREE (tree_model)->list_only)
		return (GTK_TREE_MODEL_LIST_ONLY | GTK_TREE_MODEL_ITERS_PERSIST);
	else
		return (GTK_TREE_MODEL_ITERS_PERSIST);
}


/*****************************************************************************
 *
 *  get_n_columns: tells the rest of the world how many data
 *                 columns we export via the tree model interface
 *
 *****************************************************************************/

static gint  get_n_columns (GtkTreeModel* tree_model)
{
	g_return_val_if_fail (UGTK_IS_NODE_TREE(tree_model), 0);

	return NODE_TREE_N_COLUMNS;
}


/*****************************************************************************
 *
 *  get_column_type: tells the rest of the world which type of
 *                   data an exported model column contains
 *
 *****************************************************************************/

static GType  get_column_type (GtkTreeModel* tree_model,
                               gint          index)
{
	g_return_val_if_fail (UGTK_IS_NODE_TREE(tree_model), G_TYPE_INVALID);
	g_return_val_if_fail (index < NODE_TREE_N_COLUMNS && index >= 0, G_TYPE_INVALID);

	return G_TYPE_POINTER;
}


/*****************************************************************************
 *
 *  get_iter: converts a tree path (physical position) into a
 *            tree iter structure (the content of the iter
 *            fields will only be used internally by our model).
 *            We simply store a pointer to our CustomRecord
 *            structure that represents that row in the tree iter.
 *
 *****************************************************************************/

static gboolean  get_iter (GtkTreeModel* tree_model,
                           GtkTreeIter*  iter,
                           GtkTreePath*  path)
{
	UgtkNodeTree*  utree;
	UgetNode*  node;
	gint*      indices;
	gint       n, depth;

	g_assert (UGTK_IS_NODE_TREE(tree_model));
	g_assert (path != NULL);

	utree = UGTK_NODE_TREE(tree_model);

	indices = gtk_tree_path_get_indices(path);
	depth   = gtk_tree_path_get_depth(path);

	// prefix.root
	if (utree->prefix.root) {
		n = utree->prefix.len;
		if (n > utree->prefix.root->n_children || n == 0)
			n = utree->prefix.root->n_children;

		if (indices[0] < n)
			node = uget_node_nth_child (utree->prefix.root, indices[0]);
		else if (utree->root)
			node = uget_node_nth_child (utree->root, indices[0] - n);
		else
			return FALSE;

		for (n = 1;  n < depth;  n++) {
			if (node == NULL)
				return FALSE;
			node = uget_node_nth_child (node, indices[n]);
		}
	}
	else {
		for (node = utree->root, n = 0;  n < depth;  n++) {
			if (node == NULL)
				return FALSE;
			node = uget_node_nth_child (node, indices[n]);
		}
	}

	if (node == NULL)
		return FALSE;
	else {
		// We store a pointer to UgetNode in the iter
		iter->stamp      = utree->stamp;
		iter->user_data  = node;
		iter->user_data2 = NULL;   /* unused */
		iter->user_data3 = NULL;   /* unused */
		return TRUE;
	}
}


/*****************************************************************************
 *
 *  get_path: converts a tree iter into a tree path (ie. the
 *            physical position of that row in the list).
 *
 *****************************************************************************/

static GtkTreePath*  get_path (GtkTreeModel* tree_model,
                               GtkTreeIter*  iter)
{
	GtkTreePath*  path;
	UgtkNodeTree* utree;
	UgetNode*     node;
	gint          n;

	g_return_val_if_fail (UGTK_IS_NODE_TREE(tree_model), NULL);
	g_return_val_if_fail (iter != NULL,             NULL);
	g_return_val_if_fail (iter->user_data != NULL,  NULL);

	node = iter->user_data;
	path = gtk_tree_path_new();
	utree = UGTK_NODE_TREE (tree_model);

	// prefix.root
	if (utree->prefix.root) {
		n = utree->prefix.len;
		if (n > utree->prefix.root->n_children || n == 0)
			n = utree->prefix.root->n_children;

		while (node->parent) {
			if (node->parent == utree->prefix.root) {
				gtk_tree_path_prepend_index (path,
						uget_node_child_position (node->parent, node));
				return path;
			}
			else if (node->parent == utree->root) {
				gtk_tree_path_prepend_index (path,
						uget_node_child_position (node->parent, node) + n);
				return path;
			}
			gtk_tree_path_prepend_index (path,
					uget_node_child_position (node->parent, node));
			node = node->parent;
		}
	}
	else {
		while (node->parent && node != utree->root) {
			gtk_tree_path_prepend_index (path,
					uget_node_child_position (node->parent, node));
			node = node->parent;
		}
	}

	return path;
}


/*****************************************************************************
 *
 *  get_value: Returns a row's exported data columns
 *             (_get_value is what gtk_tree_model_get uses)
 *
 *****************************************************************************/

static void  get_value (GtkTreeModel* tree_model,
                        GtkTreeIter*  iter,
                        gint          column,
                        GValue*       value)
{
//	UgtkNodeTree*  utree;

	g_return_if_fail (UGTK_IS_NODE_TREE (tree_model));
	g_return_if_fail (iter != NULL);
	g_return_if_fail (column < NODE_TREE_N_COLUMNS);

	g_value_init (value, G_TYPE_POINTER);

//	utree = UGTK_NODE_TREE (tree_model);
	g_value_set_pointer (value, iter->user_data);
}


/*****************************************************************************
 *
 *  iter_next: Takes an iter structure and sets it to point
 *             to the next row.
 *
 *****************************************************************************/

static gboolean  iter_next (GtkTreeModel* tree_model,
                            GtkTreeIter*  iter)
{
	UgtkNodeTree* utree;
	UgetNode* node;
	gint      n;

	g_return_val_if_fail (UGTK_IS_NODE_TREE (tree_model), FALSE);

	utree = UGTK_NODE_TREE (tree_model);
	node = iter->user_data;
	if (node == NULL)
		return FALSE;

	// prefix.root
	if (utree->prefix.root && utree->prefix.root == node->parent) {
		n = utree->prefix.len;
		if (n > utree->prefix.root->n_children || n == 0)
			n = utree->prefix.root->n_children;

		if (uget_node_child_position (node->parent, node) == n - 1) {
			if (utree->root == NULL || utree->root->children == NULL)
				return FALSE;
			iter->stamp     = utree->stamp;
			iter->user_data = utree->root->children;
			return TRUE;
		}
	}

	if (node->next == NULL)
		return FALSE;
	else {
		iter->stamp     = utree->stamp;
		iter->user_data = node->next;
		return TRUE;
	}
}


/*****************************************************************************
 *
 *  iter_children: Returns TRUE or FALSE depending on whether
 *                 the row specified by 'parent' has any children.
 *                 If it has children, then 'iter' is set to
 *                 point to the first child. Special case: if
 *                 'parent' is NULL, then the first top-level
 *                 row should be returned if it exists.
 *
 *****************************************************************************/

static gboolean  iter_children (GtkTreeModel* tree_model,
                                GtkTreeIter*  iter,
                                GtkTreeIter*  parent)
{
	UgtkNodeTree*  utree;
	UgetNode*    node;

	g_return_val_if_fail (UGTK_IS_NODE_TREE (tree_model), FALSE);

	utree = UGTK_NODE_TREE (tree_model);

	// parent == NULL is a special case; we need to return the first top-level row
	if (parent)
		node = parent->user_data;
	else {
		// prefix.root
		if (utree->prefix.root && utree->prefix.root->children) {
			iter->stamp     = utree->stamp;
			iter->user_data = utree->prefix.root->children;
			return TRUE;
		}
		node = utree->root;
	}

	// Set iter to first child item.
	if (node == NULL || node->children == NULL)
		return FALSE;
	else {
		iter->stamp     = utree->stamp;
		iter->user_data = node->children;
		return TRUE;
	}
}


/*****************************************************************************
 *
 *  iter_has_child: Returns TRUE or FALSE depending on whether
 *                  the row specified by 'iter' has any children.
 *                  We only have a list and thus no children.
 *
 *****************************************************************************/

static gboolean  iter_has_child (GtkTreeModel* tree_model,
                                 GtkTreeIter*  iter)
{
	UgetNode*  node;

	node = iter->user_data;
	if (node->children == NULL)
		return FALSE;
	else
		return TRUE;
}


/*****************************************************************************
 *
 *  iter_n_children: Returns the number of children the row
 *                   specified by 'iter' has. This is usually 0,
 *                   as we only have a list and thus do not have
 *                   any children to any rows. A special case is
 *                   when 'iter' is NULL, in which case we need
 *                   to return the number of top-level nodes,
 *                   ie. the number of rows in our list.
 *
 *****************************************************************************/

static gint  iter_n_children (GtkTreeModel* tree_model,
                              GtkTreeIter*  iter)
{
	UgtkNodeTree*  utree;
	UgetNode*  node;
	gint  n = 0;

	g_return_val_if_fail (UGTK_IS_NODE_TREE (tree_model), -1);

	if (iter == NULL) {
		utree = UGTK_NODE_TREE (tree_model);
		// prefix.root
		if (utree->prefix.root) {
			n = utree->prefix.len;
			if (n > utree->prefix.root->n_children || n == 0)
				n = utree->prefix.root->n_children;
		}
		if (utree->root)
			n += utree->root->n_children;
		return n;
	}
	node = iter->user_data;
	return node->n_children;
}


/*****************************************************************************
 *
 *  iter_nth_child: If the row specified by 'parent' has any
 *                  children, set 'iter' to the n-th child and
 *                  return TRUE if it exists, otherwise FALSE.
 *                  A special case is when 'parent' is NULL, in
 *                  which case we need to set 'iter' to the n-th
 *                  row if it exists.
 *
 *****************************************************************************/

static gboolean  iter_nth_child (GtkTreeModel* tree_model,
                                 GtkTreeIter*  iter,
                                 GtkTreeIter*  parent,
                                 gint          n)
{
	UgtkNodeTree*  utree;
	UgetNode*  node;
	gint       n_prefix;

	g_return_val_if_fail (UGTK_IS_NODE_TREE (tree_model), FALSE);

	utree = UGTK_NODE_TREE (tree_model);
	// special case: if parent == NULL, set iter to n-th top-level row
	if (parent)
		node = parent->user_data;
	else {
		// prefix.root
		if (utree->prefix.root) {
			n_prefix = utree->prefix.len;
			if (n_prefix > utree->prefix.root->n_children)
				n_prefix = utree->prefix.root->n_children;
			if (n >= n_prefix)
				n -= n_prefix;
			else {
				iter->stamp = utree->stamp;
				iter->user_data = uget_node_nth_child (utree->prefix.root, n);
				return TRUE;
			}
		}
		node = utree->root;
	}

	if (node == NULL || n >= node->n_children)
		return FALSE;
	else {
		iter->stamp = utree->stamp;
		iter->user_data = uget_node_nth_child (node, n);
		return TRUE;
	}
}


/*****************************************************************************
 *
 *  iter_parent: Point 'iter' to the parent node of 'child'. As
 *               we have a list and thus no children and no
 *               parents of children, we can just return FALSE.
 *
 *****************************************************************************/

static gboolean  iter_parent (GtkTreeModel* tree_model,
                              GtkTreeIter*  iter,
                              GtkTreeIter*  child)
{
	UgtkNodeTree*  utree;
	UgetNode*  node;

	if (child == NULL || child->user_data == NULL)
		return FALSE;

	utree = UGTK_NODE_TREE (tree_model);
	node = child->user_data;

	// prefix.root
	if (node->parent == utree->prefix.root)
		return FALSE;

	if (node->parent == utree->root)
		return FALSE;
	else {
		iter->stamp = utree->stamp;
		iter->user_data = node->parent;
		return TRUE;
	}
}


/*****************************************************************************
 *
 *  ugtk_node_tree_get_type: here we register our new type and its interfaces
 *                      with the type system. If you want to implement
 *                      additional interfaces like GtkTreeSortable, you
 *                      will need to do it here.
 *
 *****************************************************************************/

GType  ugtk_node_tree_get_type (void)
{
	static GType ugtk_node_tree_type = 0;

	/* Some boilerplate type registration stuff */
	if (ugtk_node_tree_type == 0) {
		static const GTypeInfo ugtk_node_tree_info =
		{
			sizeof (UgtkNodeTreeClass),
			NULL,                                         /* base_init */
			NULL,                                         /* base_finalize */
			(GClassInitFunc) class_init,
			NULL,                                         /* class finalize */
			NULL,                                         /* class_data */
			sizeof (UgtkNodeTree),
			0,                                           /* n_preallocs */
			(GInstanceInitFunc) init
		};

		static const GInterfaceInfo tree_model_info =
		{
			(GInterfaceInitFunc) tree_model_init,
			NULL,
			NULL
		};

		/* First register the new derived type with the GObject type system */
		ugtk_node_tree_type = g_type_register_static (G_TYPE_OBJECT, "UgtkNodeTree",
				&ugtk_node_tree_info, (GTypeFlags)0);

		/* Now register our GtkTreeModel interface with the type system */
		g_type_add_interface_static (ugtk_node_tree_type,
				GTK_TYPE_TREE_MODEL, &tree_model_info);
	}

	return ugtk_node_tree_type;
}

/*****************************************************************************
 *
 *  ugtk_node_tree_new:  This is what you use in your own code to create a
 *                  new uget tree model for you to use.
 *
 *****************************************************************************/

UgtkNodeTree*  ugtk_node_tree_new (UgetNode* root, gboolean list_only)
{
	UgtkNodeTree* utree;

	utree = (UgtkNodeTree*) g_object_new (UGTK_TYPE_NODE_TREE, NULL);
	utree->root = root;
	utree->list_only = list_only;

	g_assert( utree != NULL );
	return utree;
}

void  ugtk_node_tree_set_prefix (UgtkNodeTree* utree, UgetNode* prefix_root, gint prefix_len)
{
	utree->prefix.root = prefix_root;
	utree->prefix.len = prefix_len;
}
