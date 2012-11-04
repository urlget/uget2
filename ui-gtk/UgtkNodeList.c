/*
 *
 *   Copyright (C) 2012-2014 by C.H. Huang
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

#include <UgtkNodeList.h>
#define NODE_LIST_N_COLUMNS    1

/* boring declarations of local functions */

static void      init (UgtkNodeList* ugtree);

static void      class_init (UgtkNodeListClass *klass);

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

static void  class_init (UgtkNodeListClass* klass)
{
	GObjectClass *object_class;

	parent_class = (GObjectClass*) g_type_class_peek_parent (klass);
	object_class = (GObjectClass*) klass;

	object_class->finalize = finalize;
}

/*****************************************************************************
 *
 *  tree_model_init: init callback for the interface registration
 *                   in ugtk_node_list_get_type. Here we override
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

static void  init (UgtkNodeList* ulist)
{
	ulist->stamp = g_random_int();  /* Random int to check whether an iter belongs to our model */
}


/*****************************************************************************
 *
 *  finalize: this is called just before a object is
 *            destroyed. Free dynamically allocated memory here.
 *
 *****************************************************************************/

static void  finalize (GObject* object)
{
//	UgtkNodeList *ulist = UGTK_NODE_LIST(object);

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
	g_return_val_if_fail (UGTK_IS_NODE_LIST(tree_model), (GtkTreeModelFlags)0);

	return (GTK_TREE_MODEL_LIST_ONLY | GTK_TREE_MODEL_ITERS_PERSIST);
}


/*****************************************************************************
 *
 *  get_n_columns: tells the rest of the world how many data
 *                 columns we export via the tree model interface
 *
 *****************************************************************************/

static gint  get_n_columns (GtkTreeModel* tree_model)
{
	g_return_val_if_fail (UGTK_IS_NODE_LIST(tree_model), 0);

	return NODE_LIST_N_COLUMNS;
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
	g_return_val_if_fail (UGTK_IS_NODE_LIST(tree_model), G_TYPE_INVALID);
	g_return_val_if_fail (index < NODE_LIST_N_COLUMNS && index >= 0, G_TYPE_INVALID);

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
	UgtkNodeList*  ulist;
	UgetNode*      node;
	gint*          indices;
	gint           depth;

	g_assert (UGTK_IS_NODE_LIST(tree_model));
	g_assert (path != NULL);

	ulist = UGTK_NODE_LIST(tree_model);
	if (ulist->root == NULL)
		return FALSE;

	indices = gtk_tree_path_get_indices(path);
	depth   = gtk_tree_path_get_depth(path);

	g_assert (depth == 1);

	if (ulist->root_visible) {
		if (indices[0] == 0)
			node = ulist->root;
		else if (indices[0] <= ulist->n_fake)
			node = uget_node_nth_fake (ulist->root, indices[0] - 1);
		else
			return FALSE;
	}
	else {
		if (indices[0] < ulist->n_fake)
			node = uget_node_nth_fake (ulist->root, indices[0]);
		else
			return FALSE;
	}

	if (node == NULL)
		return FALSE;
	else {
		// We store a pointer to UgNode in the iter
		iter->stamp      = ulist->stamp;
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
	UgtkNodeList* ulist;
	UgetNode*     node;
	gint          n;

	g_return_val_if_fail (UGTK_IS_NODE_LIST(tree_model), NULL);
	g_return_val_if_fail (iter != NULL,             NULL);
	g_return_val_if_fail (iter->user_data != NULL,  NULL);

	node = iter->user_data;
	path = gtk_tree_path_new();
	ulist = UGTK_NODE_LIST (tree_model);

	if (ulist->root_visible == FALSE)
		n = uget_node_fake_position (node->real, node);
	else {
		if (ulist->root == node)
			n = 0;
		else
			n = uget_node_fake_position (node->real, node) + 1;
	}

	gtk_tree_path_prepend_index (path, n);
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
//	UgtkNodeList*  ulist;

	g_return_if_fail (UGTK_IS_NODE_LIST (tree_model));
	g_return_if_fail (iter != NULL);
	g_return_if_fail (column < NODE_LIST_N_COLUMNS);

	g_value_init (value, G_TYPE_POINTER);

//	ulist = UGTK_NODE_LIST (tree_model);
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
	UgtkNodeList* ulist;
	UgetNode*     node;

	g_return_val_if_fail (UGTK_IS_NODE_LIST (tree_model), FALSE);

	ulist = UGTK_NODE_LIST(tree_model);
	if (ulist->root == NULL)
		return FALSE;

	node = iter->user_data;
	if (ulist->root_visible && ulist->root == node)
		node = node->fake;
	else
		node = node->peer;

	if (node == NULL || uget_node_fake_position (node->real, node) >= ulist->n_fake)
		return FALSE;
	else {
		iter->stamp     = ulist->stamp;
		iter->user_data = node;
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
	UgtkNodeList*  ulist;
	UgetNode*      node;

	g_return_val_if_fail (UGTK_IS_NODE_LIST (tree_model), FALSE);
	g_return_val_if_fail (parent == NULL, FALSE);

	ulist = UGTK_NODE_LIST (tree_model);
	if (ulist->root == NULL)
		return FALSE;

	if (ulist->root_visible)
		node = ulist->root;
	else
		node = ulist->root->fake;

	// Set iter to first child item.
	if (node == NULL)
		return FALSE;
	else {
		iter->stamp     = ulist->stamp;
		iter->user_data = node;
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
	return FALSE;
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
	UgtkNodeList*  ulist;
	UgetNode*      node;
	gint           n = 0;

	g_return_val_if_fail (UGTK_IS_NODE_LIST (tree_model), -1);
	g_return_val_if_fail (iter == NULL, -1);

	ulist = UGTK_NODE_LIST (tree_model);
	if (ulist->root == NULL)
		return 0;

	node = iter->user_data;
	for (n = 0, node = node->fake;  node;  node = node->peer)
		n++;

	if (n > ulist->n_fake)
		n = ulist->n_fake;
	if (ulist->root_visible)
		n++;

	return n;
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
	UgtkNodeList*  ulist;
	UgetNode*      node;

	g_return_val_if_fail (UGTK_IS_NODE_LIST (tree_model), FALSE);
	g_return_val_if_fail (parent == NULL, FALSE);

	ulist = UGTK_NODE_LIST (tree_model);
	if (ulist->root == NULL)
		return FALSE;

	if (ulist->root_visible) {
		if (n == 0)
			node = ulist->root;
		else if (n <= ulist->n_fake)
			node = uget_node_nth_fake (ulist->root, n - 1);
		else
			return FALSE;
	}
	else {
		if (n < ulist->n_fake)
			node = uget_node_nth_fake (ulist->root, n);
		else
			return FALSE;
	}

	if (node == NULL)
		return FALSE;
	else {
		iter->stamp = ulist->stamp;
		iter->user_data = node;
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
	return FALSE;
}


/*****************************************************************************
 *
 *  ugtk_node_list_get_type: here we register our new type and its interfaces
 *                      with the type system. If you want to implement
 *                      additional interfaces like GtkTreeSortable, you
 *                      will need to do it here.
 *
 *****************************************************************************/

GType  ugtk_node_list_get_type (void)
{
	static GType ugtk_node_list_type = 0;

	/* Some boilerplate type registration stuff */
	if (ugtk_node_list_type == 0) {
		static const GTypeInfo ugtk_node_list_info =
		{
			sizeof (UgtkNodeListClass),
			NULL,                                         /* base_init */
			NULL,                                         /* base_finalize */
			(GClassInitFunc) class_init,
			NULL,                                         /* class finalize */
			NULL,                                         /* class_data */
			sizeof (UgtkNodeList),
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
		ugtk_node_list_type = g_type_register_static (G_TYPE_OBJECT, "UgtkNodeList",
				&ugtk_node_list_info, (GTypeFlags)0);

		/* Now register our GtkTreeModel interface with the type system */
		g_type_add_interface_static (ugtk_node_list_type,
				GTK_TYPE_TREE_MODEL, &tree_model_info);
	}

	return ugtk_node_list_type;
}

/*****************************************************************************
 *
 *  ugtk_node_list_new:  This is what you use in your own code to create a
 *                  new tree model for you to use.
 *
 *****************************************************************************/

UgtkNodeList*  ugtk_node_list_new (UgetNode* root, gint n_fake, gboolean root_visible)
{
	UgtkNodeList* ulist;

	ulist = (UgtkNodeList*) g_object_new (UGTK_TYPE_NODE_LIST, NULL);
	ulist->root = root;
	ulist->n_fake = n_fake;
	ulist->root_visible = root_visible;

	g_assert( ulist != NULL );
	return ulist;
}
