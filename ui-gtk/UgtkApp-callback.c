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

#include <UgtkApp.h>
#include <UgtkTrayIcon.h>
#include <UgtkConfirmDialog.h>

// static functions
static void  ugtk_window_init_callback   (struct UgtkWindow*  window,  UgtkApp* app);
static void  ugtk_toolbar_init_callback  (struct UgtkToolbar* toolbar, UgtkApp* app);
// functions for UgetNode.notification
static void  node_inserted (UgetNode* node, UgetNode* sibling, UgetNode* child);
static void  node_removed (UgetNode* node, UgetNode* sibling, UgetNode* child);
static void  node_updated (UgetNode* child);

void  ugtk_app_init_callback (UgtkApp* app)
{
//	gtk_accel_group_connect (app->accel_group, GDK_KEY_q, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE,
//	                         g_cclosure_new_swap (G_CALLBACK (ugtk_app_quit), app, NULL));
//	gtk_accel_group_connect (app->accel_group, GDK_KEY_s, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE,
//	                         g_cclosure_new_swap (G_CALLBACK (ugtk_app_save), app, NULL));
//	gtk_accel_group_connect (app->accel_group, GDK_KEY_c, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE,
//	                         g_cclosure_new_swap (G_CALLBACK (on_summary_copy_selected), app, NULL));
	ugtk_window_init_callback  (&app->window,  app);
	ugtk_menubar_init_callback (&app->menubar, app);
	ugtk_toolbar_init_callback (&app->toolbar, app);
	ugtk_trayicon_init_callback (&app->trayicon, app);
	// node notification
	uget_app_set_notification ((UgetApp*) app, app,
			node_inserted, node_removed, (UgNotifyFunc) node_updated);
}

// ----------------------------------------------------------------------------
// Toolbar
//
static void  on_create_download (GtkWidget* widget, UgtkApp* app)
{
	ugtk_app_create_download (app, NULL, NULL);
}

// ----------------------------------------------------------------------------
// UgtkWindow

// UgtkTraveler.download.view selection "changed"
static void  on_download_selection_changed (GtkTreeSelection* selection, UgtkApp* app)
{
	gint  n_selected;

	n_selected = gtk_tree_selection_count_selected_rows (selection);
	ugtk_statusbar_set_info (&app->statusbar, n_selected);
	ugtk_app_decide_download_sensitive (app);
}

// UgtkTraveler.download.view "cursor-changed"
static void  on_download_cursor_changed (GtkTreeView* view, UgtkApp* app)
{
	GtkWidget*    item;
	UgetNode*     node;
	UgetRelation* relation;
	UgetPriority  priority;

	node = app->traveler.download.cursor.node;
	if (node)
		node = node->data;
	ugtk_summary_show (&app->summary, node);

	// UgtkMenubar.download.priority
	if (node == NULL)
		priority = UGET_PRIORITY_NORMAL;
	else {
		relation = ug_info_get (&node->info, UgetRelationInfo);
		if (relation)
			priority = relation->task.priority;
		else
			priority = UGET_PRIORITY_NORMAL;
	}
	// set item
	switch (priority) {
	case UGET_PRIORITY_HIGH:
		item = app->menubar.download.prioriy.high;
		break;

	case UGET_PRIORITY_LOW:
		item = app->menubar.download.prioriy.low;
		break;

	case UGET_PRIORITY_NORMAL:
	default:
		item = app->menubar.download.prioriy.normal;
		break;
	}
	gtk_check_menu_item_set_active ((GtkCheckMenuItem*) item, TRUE);
}

// UgtkTraveler.category.view "cursor-changed"
// UgtkTraveler.state.view "cursor-changed"
static void  on_category_cursor_changed (GtkTreeView* view, UgtkApp* app)
{
	UgtkTraveler*  traveler;

	traveler = &app->traveler;
	if (traveler->state.cursor.pos    != traveler->state.cursor.pos_last ||
	    traveler->category.cursor.pos != traveler->category.cursor.pos_last)
	{
		// Is user select "All Status" or "All Category"?
		if (traveler->state.cursor.pos && traveler->category.cursor.pos)
			ugtk_traveler_set_sorting (traveler, FALSE, 0, 0);
		else {
			// set sorting column
			ugtk_traveler_set_sorting (traveler, TRUE,
					app->setting.download_column.sort.nth,
					app->setting.download_column.sort.type);
		}
	}

	// show/hide download column and setup items in UgtkViewMenu
	if (traveler->state.cursor.pos != traveler->state.cursor.pos_last) {
		struct UgtkDownloadColumnSetting*  setting;
		GtkTreeView*        view;
		GtkTreeViewColumn*  column;
		gboolean            sensitive;

		view = traveler->download.view;
		setting = &app->setting.download_column;
		// Finished
		if (traveler->state.cursor.pos == 3)
			sensitive = FALSE;
		else
			sensitive = TRUE;
		gtk_widget_set_sensitive (app->menubar.view.columns.complete, sensitive);
		gtk_widget_set_sensitive (app->menubar.view.columns.percent, sensitive);
		column = gtk_tree_view_get_column (view, UGTK_NODE_COLUMN_COMPLETE);
		gtk_tree_view_column_set_visible (column, sensitive && setting->complete);
		column = gtk_tree_view_get_column (view, UGTK_NODE_COLUMN_PERCENT);
		gtk_tree_view_column_set_visible (column, sensitive && setting->percent);
		// Recycled
		if (traveler->state.cursor.pos == 4)
			sensitive = FALSE;
		else
			sensitive = TRUE;
		gtk_widget_set_sensitive (app->menubar.view.columns.elapsed, sensitive);
		column = gtk_tree_view_get_column (view, UGTK_NODE_COLUMN_ELAPSED);
		gtk_tree_view_column_set_visible (column, sensitive && setting->elapsed);
		// Finished & Recycled
		if (traveler->state.cursor.pos == 3  ||  traveler->state.cursor.pos == 4)
			sensitive = FALSE;
		else
			sensitive = TRUE;
		gtk_widget_set_sensitive (app->menubar.view.columns.left, sensitive);
		gtk_widget_set_sensitive (app->menubar.view.columns.speed, sensitive);
		gtk_widget_set_sensitive (app->menubar.view.columns.upload_speed, sensitive);
		gtk_widget_set_sensitive (app->menubar.view.columns.uploaded, sensitive);
		gtk_widget_set_sensitive (app->menubar.view.columns.ratio, sensitive);
		column = gtk_tree_view_get_column (view, UGTK_NODE_COLUMN_LEFT);
		gtk_tree_view_column_set_visible (column, sensitive && setting->left);
		column = gtk_tree_view_get_column (view, UGTK_NODE_COLUMN_SPEED);
		gtk_tree_view_column_set_visible (column, sensitive && setting->speed);
		column = gtk_tree_view_get_column (view, UGTK_NODE_COLUMN_UPLOAD_SPEED);
		gtk_tree_view_column_set_visible (column, sensitive && setting->upload_speed);
		column = gtk_tree_view_get_column (view, UGTK_NODE_COLUMN_UPLOADED);
		gtk_tree_view_column_set_visible (column, sensitive && setting->uploaded);
		column = gtk_tree_view_get_column (view, UGTK_NODE_COLUMN_RATIO);
		gtk_tree_view_column_set_visible (column, sensitive && setting->ratio);
	}

	if (view == app->traveler.category.view) {
		ugtk_app_decide_category_sensitive (app);
		// sync UgtkMenubar.download.move_to
		ugtk_menubar_sync_category (&app->menubar, app, FALSE);
	}
}

// button-press-event
static gboolean  on_button_press_event (GtkTreeView* view, GdkEventButton* event, UgtkApp* app)
{
	GtkTreeSelection* selection;
	GtkTreePath*      path;
	GtkMenu*  menu;
	gboolean  is_selected;

	// right button press
//	if (event->type != GDK_BUTTON_PRESS)
//		return FALSE;
	if (event->button != 3)		// right mouse button
		return FALSE;
	// popup a menu
	if (view == app->traveler.category.view)
		menu = (GtkMenu*) app->menubar.category.self;
	else if (view == app->traveler.download.view)
		menu = (GtkMenu*) app->menubar.download.self;
	else if (view == app->summary.view)
		menu = app->summary.menu.self;
	else
		return FALSE;

	gtk_menu_popup (menu, NULL, NULL, NULL, NULL,
			event->button, gtk_get_current_event_time());

	if (gtk_tree_view_get_path_at_pos (view, (gint)event->x, (gint)event->y, &path, NULL, NULL, NULL)) {
		selection = gtk_tree_view_get_selection (view);
		is_selected = gtk_tree_selection_path_is_selected (selection, path);
		gtk_tree_path_free (path);
		if (is_selected)
			return TRUE;
	}
	return FALSE;
}

// UgtkSummary.menu.copy signal handler
static void  on_summary_copy_selected (GtkWidget* widget, UgtkApp* app)
{
	gchar*  text;

	text = ugtk_summary_get_text_selected (&app->summary);
	ugtk_clipboard_set_text (&app->clipboard, text);
}

// UgtkSummary.menu.copy_all signal handler
static void  on_summary_copy_all (GtkWidget* widget, UgtkApp* app)
{
	gchar*  text;

	text = ugtk_summary_get_text_all (&app->summary);
	ugtk_clipboard_set_text (&app->clipboard, text);
}

// This function is used by on_window_key_press_event()
static void  menu_position_func (GtkMenu*	menu,
                                 gint*		x,
                                 gint*		y,
                                 gboolean*	push_in,
                                 gpointer	user_data)
{
	GtkRequisition	menu_requisition;
	GtkAllocation	allocation;
	GtkWidget*		widget;
	gint			max_x, max_y;

	widget = user_data;
	gdk_window_get_origin (gtk_widget_get_window (widget), x, y);
	gtk_widget_get_preferred_size (GTK_WIDGET (menu), &menu_requisition, NULL);

	gtk_widget_get_allocation (widget, &allocation);
	if (gtk_widget_get_direction (widget) == GTK_TEXT_DIR_RTL)
		*x += allocation.width - allocation.width / 3;
	else
		*x += allocation.width / 3;
	*y += allocation.height / 3;

//	if (gtk_widget_get_direction (widget) == GTK_TEXT_DIR_RTL)
//		*x += allocation.width - menu_requisition.width;
//	else
//		*x += allocation.x;
//	*y += allocation.y + allocation.height;

	// Make sure we are on the screen.
	max_x = MAX (0, gdk_screen_width ()  - menu_requisition.width);
	max_y = MAX (0, gdk_screen_height () - menu_requisition.height);

	*x = CLAMP (*x, 0, max_x);
	*y = CLAMP (*y, 0, max_y);
}

// key-press-event
static gboolean  on_window_key_press_event (GtkWidget* widget, GdkEventKey* event, UgtkApp* app)
{
	GtkTreeView*	focus;
	GtkMenu*		menu;

//	g_print ("key-press : 0x%x\n", event->keyval);
	if (event->keyval != GDK_KEY_Menu)
		return FALSE;

	focus = (GtkTreeView*) gtk_window_get_focus (app->window.self);
	if (focus == app->traveler.category.view) {
		widget = (GtkWidget*) app->traveler.category.view;
		menu = (GtkMenu*) app->menubar.category.self;
	}
	else if (focus == app->traveler.download.view) {
		widget = (GtkWidget*) app->traveler.download.view;
		menu = (GtkMenu*) app->menubar.download.self;
	}
	else if (focus == app->summary.view) {
		widget = (GtkWidget*) app->summary.view;
		menu = (GtkMenu*) app->summary.menu.self;
	}
	else
		return FALSE;

	gtk_menu_popup (menu, NULL, NULL,
			menu_position_func, widget,
			0, gtk_get_current_event_time());
	return TRUE;
}

// UgtkWindow.self "delete-event"
static gboolean  on_window_delete_event (GtkWidget* widget, GdkEvent* event, UgtkApp* app)
{
	if (app->setting.ui.close_to_tray == FALSE)
		ugtk_app_decide_to_quit (app);
	else {
		ugtk_tray_icon_set_visible (&app->trayicon, TRUE);
		gtk_widget_hide ((GtkWidget*) app->window.self);
	}
	return TRUE;
}

// UgtkTraveler.download.view "key-press-event"
static gboolean  on_traveler_key_press_event  (GtkWidget* widget, GdkEventKey* event, UgtkApp* app)
{
/*
	// check shift key status
	GdkWindow*		gdk_win;
	GdkDevice*		dev_pointer;
	GdkModifierType	mask;

	// check shift key status
	gdk_win = gtk_widget_get_parent_window ((GtkWidget*) app->cwidget.current.widget->view);
	dev_pointer = gdk_device_manager_get_client_pointer (
			gdk_display_get_device_manager (gdk_window_get_display (gdk_win)));
	gdk_window_get_device_position (gdk_win, dev_pointer, NULL, NULL, &mask);

	if (app->cwidget.current.category) {
		switch (event->keyval) {
		case GDK_KEY_Delete:
		case GDK_KEY_KP_Delete:
			if (mask & GDK_SHIFT_MASK)
				ugtk_app_delete_download (app, TRUE);
			else
				ugtk_app_delete_download (app, FALSE);
			return TRUE;

		case GDK_KEY_Return:
		case GDK_KEY_KP_Enter:
			if (mask & GDK_SHIFT_MASK)
				on_open_download_folder (widget, app);
			else
				on_open_download_file (widget, app);
			return TRUE;
		}
	}
*/
	switch (event->keyval) {
	case GDK_KEY_Delete:
		ugtk_app_delete_download (app, FALSE);
		return TRUE;

	case GDK_KEY_space:
		ugtk_app_switch_download_state (app);
		return TRUE;

	default:
		break;
	}

	return FALSE;
}

static void ugtk_window_init_callback (struct UgtkWindow* window, UgtkApp* app)
{
	GtkTreeSelection* selection;

	// UgtkTraveler
	selection = gtk_tree_view_get_selection (app->traveler.download.view);
	g_signal_connect (selection, "changed",
			G_CALLBACK (on_download_selection_changed), app);
	g_signal_connect (app->traveler.download.view, "cursor-changed",
			G_CALLBACK (on_download_cursor_changed), app);
	g_signal_connect_after (app->traveler.category.view, "cursor-changed",
			G_CALLBACK (on_category_cursor_changed), app);
	g_signal_connect_after (app->traveler.state.view, "cursor-changed",
			G_CALLBACK (on_category_cursor_changed), app);

	// pop-up menu by mouse button
	g_signal_connect (app->traveler.category.view, "button-press-event",
			G_CALLBACK (on_button_press_event), app);
	g_signal_connect (app->traveler.download.view, "button-press-event",
			G_CALLBACK (on_button_press_event), app);
	g_signal_connect (app->summary.view, "button-press-event",
			G_CALLBACK (on_button_press_event), app);

	// UgtkSummary.menu signal handlers
	g_signal_connect (app->summary.menu.copy, "activate",
			G_CALLBACK (on_summary_copy_selected), app);
	g_signal_connect (app->summary.menu.copy_all, "activate",
			G_CALLBACK (on_summary_copy_all), app);

	// UgtkWindow.self signal handlers
	g_signal_connect (window->self, "key-press-event",
			G_CALLBACK (on_window_key_press_event), app);
	g_signal_connect (window->self, "delete-event",
			G_CALLBACK (on_window_delete_event), app);
	g_signal_connect_swapped (window->self, "destroy",
			G_CALLBACK (ugtk_app_quit), app);

	// UgtkTraveler signal handlers
	g_signal_connect (app->traveler.download.view, "key-press-event",
			G_CALLBACK (on_traveler_key_press_event), app);
}

// ----------------------------------------------------------------------------
// UgtkToolbar

static void ugtk_toolbar_init_callback (struct UgtkToolbar* toolbar, UgtkApp* app)
{
	// create new
	g_signal_connect (toolbar->create, "clicked",
			G_CALLBACK (on_create_download), app);
	g_signal_connect (toolbar->create_download, "activate",
			G_CALLBACK (on_create_download), app);
	g_signal_connect_swapped (toolbar->create_category, "activate",
			G_CALLBACK (ugtk_app_create_category), app);
	g_signal_connect_swapped (toolbar->create_sequence, "activate",
			G_CALLBACK (ugtk_app_sequence_batch), app);
	g_signal_connect_swapped (toolbar->create_clipboard, "activate",
			G_CALLBACK (ugtk_app_clipboard_batch), app);
	g_signal_connect_swapped (toolbar->create_torrent, "activate",
			G_CALLBACK (ugtk_app_create_torrent), app);
	g_signal_connect_swapped (toolbar->create_metalink, "activate",
			G_CALLBACK (ugtk_app_create_metalink), app);
	// save
	g_signal_connect_swapped (toolbar->save, "clicked",
			G_CALLBACK (ugtk_app_save), app);
	// change status
	g_signal_connect_swapped (toolbar->runnable, "clicked",
			G_CALLBACK (ugtk_app_queue_download), app);
	g_signal_connect_swapped (toolbar->pause, "clicked",
			G_CALLBACK (ugtk_app_pause_download), app);
	// change data
	g_signal_connect_swapped (toolbar->properties, "clicked",
			G_CALLBACK (ugtk_app_edit_download), app);
	// move
	g_signal_connect_swapped (toolbar->move_up, "clicked",
			G_CALLBACK (ugtk_app_move_download_up), app);
	g_signal_connect_swapped (toolbar->move_down, "clicked",
			G_CALLBACK (ugtk_app_move_download_down), app);
	g_signal_connect_swapped (toolbar->move_top, "clicked",
			G_CALLBACK (ugtk_app_move_download_top), app);
	g_signal_connect_swapped (toolbar->move_bottom, "clicked",
			G_CALLBACK (ugtk_app_move_download_bottom), app);
}

// ----------------------------------------------------------------------------
// functions for UgetNode.notification

static void node_inserted (UgetNode* node, UgetNode* sibling, UgetNode* child)
{
	GtkTreePath* path;
	GtkTreeIter  iter;
	UgtkApp*     app;

	app = node->notification->data;
	if (node == (UgetNode*) app->traveler.category.model->root) {
		// category inserted
		path = gtk_tree_path_new_from_indices (
				uget_node_child_position (node, child) +1, -1);
		iter.stamp = app->traveler.category.model->stamp;
		iter.user_data = child;
		gtk_tree_model_row_inserted (
				GTK_TREE_MODEL (app->traveler.category.model), path, &iter);
		gtk_tree_path_free (path);
		// sync UgtkMenubar.download.move_to
		ugtk_menubar_sync_category (&app->menubar, app, TRUE);
	}
	else if (node == (UgetNode*) app->traveler.download.model->root) {
		// download inserted
		path = gtk_tree_path_new_from_indices (
				uget_node_child_position (node, child), -1);
		iter.stamp = app->traveler.download.model->stamp;
		iter.user_data = child;
		gtk_tree_model_row_inserted (
				GTK_TREE_MODEL (app->traveler.download.model), path, &iter);
		gtk_tree_path_free (path);
	}
}

static void node_removed (UgetNode* node, UgetNode* sibling, UgetNode* child)
{
	GtkTreePath* path;
	UgtkApp*     app;
	int          pos;

	app = node->notification->data;
	if (node == (UgetNode*) app->traveler.category.model->root) {
		// category removed
		if (sibling)
			pos = uget_node_child_position (node, sibling);
		else
			pos = app->traveler.category.model->root->n_children;
		//                                     pos + "All Category"
		path = gtk_tree_path_new_from_indices (pos + 1, -1);
		gtk_tree_model_row_deleted (
				GTK_TREE_MODEL (app->traveler.category.model), path);
		gtk_tree_path_free (path);
		// sync UgtkMenubar.download.move_to
		ugtk_menubar_sync_category (&app->menubar, app, TRUE);
	}
	else if (node == (UgetNode*) app->traveler.download.model->root) {
		// download removed
		if (sibling)
			pos = uget_node_child_position (node, sibling);
		else
			pos = app->traveler.download.model->root->n_children;
		path = gtk_tree_path_new_from_indices (pos, -1);
		gtk_tree_model_row_deleted (
				GTK_TREE_MODEL (app->traveler.download.model), path);
		gtk_tree_path_free (path);
	}
}

static void node_updated (UgetNode* child)
{
	GtkTreePath* path;
	GtkTreeIter  iter;
	UgetNode*    node;
	UgtkApp*     app;

	node = child->parent;
	app = node->notification->data;
	if (node == (UgetNode*) app->traveler.category.model->root) {
		// category changed
		path = gtk_tree_path_new_from_indices (
				uget_node_child_position (node, child) +1, -1);
		iter.stamp = app->traveler.category.model->stamp;
		iter.user_data = child;
		gtk_tree_model_row_changed (
				GTK_TREE_MODEL (app->traveler.category.model), path, &iter);
		gtk_tree_path_free (path);
		// sync UgtkMenubar.download.move_to
		ugtk_menubar_sync_category (&app->menubar, app, TRUE);
	}
	else if (node == (UgetNode*) app->traveler.download.model->root) {
		// download changed
		path = gtk_tree_path_new_from_indices (
				uget_node_child_position (node, child), -1);
		iter.stamp = app->traveler.download.model->stamp;
		iter.user_data = child;
		gtk_tree_model_row_changed (
				GTK_TREE_MODEL (app->traveler.download.model), path, &iter);
		gtk_tree_path_free (path);
	}
}
