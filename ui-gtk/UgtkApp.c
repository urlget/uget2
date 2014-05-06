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

#include <UgUtil.h>
#include <UgStdio.h>
#include <UgString.h>
#include <UgHtmlFilter.h>
#include <UgetPluginCurl.h>
#include <UgetPluginAria2.h>
#include <UgtkApp.h>
#include <UgtkUtil.h>
#include <UgtkNodeDialog.h>
#include <UgtkBatchDialog.h>
#include <UgtkConfirmDialog.h>

#include <glib/gi18n.h>

#define UGTK_APP_DIR        "uGet"

void  ugtk_app_init (UgtkApp* app, UgetIpc* ipc)
{
	char*  dir;

	app->ipc = ipc;
	uget_app_init ((UgetApp*) app);
	// set application config directory for each user
	dir = g_build_filename (g_get_user_config_dir (), UGTK_APP_DIR, NULL);
	uget_app_set_config_dir ((UgetApp*) app, dir);
	g_free (dir);

	uget_rss_init (&app->rss_buildin);
	ugtk_app_load (app);
	ugtk_app_init_ui (app);
	ugtk_app_init_callback (app);
	if (app->real.n_children == 0)
		ugtk_app_add_default_category (app);
	// clipboard
	ugtk_clipboard_init (&app->clipboard, app->setting.clipboard.pattern);

	// apply UgtkSetting
	ugtk_app_set_plugin_setting (app, &app->setting);
	ugtk_app_set_window_setting (app, &app->setting);
	ugtk_app_set_other_setting (app, &app->setting);
	ugtk_app_set_ui_setting (app, &app->setting);

	ugtk_tray_icon_set_info (&app->trayicon, 0, 0, 0);
	ugtk_statusbar_set_speed (&app->statusbar, 0, 0);
	ugtk_menubar_sync_category (&app->menubar, app, TRUE);

	app->last.category_index = 0;
	app->last.infonode = uget_node_new (NULL);
	// RSS
	uget_rss_add_buildin (&app->rss_buildin, UGET_RSS_STABLE);
	uget_rss_add_buildin (&app->rss_buildin, UGET_RSS_NEWS);
	uget_rss_add_buildin (&app->rss_buildin, UGET_RSS_TUTORIALS);
	uget_rss_update (&app->rss_buildin, FALSE);
	gtk_widget_hide (app->banner.self);

	uget_app_use_uri_hash ((UgetApp*) app);
	ugtk_app_clear_attachment (app);
	ugtk_app_init_timeout (app);
}

void  ugtk_app_final (UgtkApp* app)
{
	uget_app_set_notification ((UgetApp*) app, NULL, NULL, NULL, NULL);

	uget_rss_final (&app->rss_buildin);
	uget_app_final ((UgetApp*) app);
}

void  ugtk_app_save (UgtkApp* app)
{
	gchar*    file;

	ug_create_dir_all (app->config_dir, -1);

	file = g_build_filename (app->config_dir, "Setting.json", NULL);
	ugtk_setting_save (&app->setting, file);
	g_free (file);

	// RSS
	file = g_build_filename (app->config_dir, "RSS-build-in.json", NULL);
	uget_rss_save_feeds (&app->rss_buildin, file);
	g_free (file);

//	uget_app_save_categories ((UgetApp*) app, g_get_user_config_dir ());
	uget_app_save_categories ((UgetApp*) app, NULL);
}

void  ugtk_app_load (UgtkApp* app)
{
	int       counts;
	gchar*    file;

	// load setting
	ugtk_setting_init (&app->setting);
	file = g_build_filename (app->config_dir, "Setting.json", NULL);
	counts = ugtk_setting_load (&app->setting, file);
	g_free (file);
	if (counts == FALSE)
		ugtk_setting_reset (&app->setting);
	else if (app->setting.scheduler.state.length < 7*24) {
		ug_array_alloc (&app->setting.scheduler.state,
		                7*24 - app->setting.scheduler.state.length);
	}

	// RSS
	file = g_build_filename (app->config_dir, "RSS-build-in.json", NULL);
	uget_rss_load_feeds (&app->rss_buildin, file);
	g_free (file);

//	uget_app_load_categories ((UgetApp*) app, g_get_user_config_dir ());
	counts = uget_app_load_categories ((UgetApp*) app, NULL);
	if (counts == 0)
		ugtk_app_add_default_category (app);
}

void  ugtk_app_quit (UgtkApp* app)
{
	// stop all tasks
	uget_task_remove_all (&app->task);
	// sync setting and save data
	ugtk_app_get_window_setting (app, &app->setting);
	ugtk_app_save (app);
	// clear plugin
	uget_app_clear_plugins ((UgetApp*) app);
	// hide icon in system tray before quit
	ugtk_tray_icon_set_visible (&app->trayicon, FALSE);
	// hide window
	gtk_widget_hide (GTK_WIDGET (app->window.self));

	gtk_main_quit ();
}

void  ugtk_app_get_window_setting (UgtkApp* app, UgtkSetting* setting)
{
	GdkWindowState  gdk_wstate;
	GdkWindow*      gdk_window;
	GtkAllocation   allocation;

	// get window position, size, and maximzied state
	if (gtk_widget_get_visible (GTK_WIDGET (app->window.self)) == TRUE) {
		gdk_window = gtk_widget_get_window (GTK_WIDGET (app->window.self));
		gdk_wstate = gdk_window_get_state (gdk_window);

		if (gdk_wstate & GDK_WINDOW_STATE_MAXIMIZED)
			setting->window.maximized = TRUE;
		else
			setting->window.maximized = FALSE;
		// get geometry
		if (setting->window.maximized == FALSE) {
			gtk_window_get_position (app->window.self,
					&setting->window.x, &setting->window.y);
			gtk_window_get_size (app->window.self,
					&setting->window.width, &setting->window.height);
		}
	}
	// GtkPaned position
	if (app->setting.window.category)
		setting->window.paned_position_h = gtk_paned_get_position (app->window.hpaned);
	if (app->setting.window.summary) {
		gtk_widget_get_allocation ((GtkWidget*) app->window.vpaned, &allocation);
		setting->window.paned_position_v = gtk_paned_get_position (app->window.vpaned);
		setting->window.paned_position_v = allocation.height - setting->window.paned_position_v;
	}

	// banner
	setting->window.banner = gtk_widget_get_visible (app->banner.self);
	// traveler
	setting->window.nth_category = app->traveler.category.cursor.pos;
	setting->window.nth_state    = app->traveler.state.cursor.pos;
}

void  ugtk_app_set_window_setting (UgtkApp* app, UgtkSetting* setting)
{
	GtkAllocation   allocation;

	// set window position, size, and maximized state
	if (setting->window.width  > 0 &&
	    setting->window.height > 0 &&
	    setting->window.x < gdk_screen_width ()  &&
	    setting->window.y < gdk_screen_height () &&
	    setting->window.x + setting->window.width > 0  &&
	    setting->window.y + setting->window.height > 0)
	{
		gtk_window_move (app->window.self,
				setting->window.x, setting->window.y);
		gtk_window_resize (app->window.self,
				setting->window.width, setting->window.height);
	}
	if (setting->window.maximized)
		gtk_window_maximize (app->window.self);
	// GtkPaned position
	if (setting->window.paned_position_h > 0) {
		gtk_paned_set_position (app->window.hpaned,
		                        setting->window.paned_position_h);
	}
	if (setting->window.paned_position_v > 0) {
		gtk_widget_get_allocation ((GtkWidget*) app->window.vpaned, &allocation);
		gtk_paned_set_position (app->window.vpaned,
				allocation.height - setting->window.paned_position_v);
	}
	// set visible widgets
	gtk_widget_set_visible (app->toolbar.self,
			setting->window.toolbar);
	gtk_widget_set_visible ((GtkWidget*) app->statusbar.self,
			setting->window.statusbar);
	gtk_widget_set_visible (gtk_paned_get_child1 (app->window.hpaned),
			setting->window.category);
	gtk_widget_set_visible (app->summary.self,
			setting->window.summary);
	gtk_widget_set_visible (app->banner.self,
			setting->window.banner);
	// Summary
	app->summary.visible.name     = setting->summary.name;
	app->summary.visible.folder   = setting->summary.folder;
	app->summary.visible.category = setting->summary.category;
	app->summary.visible.uri      = setting->summary.uri;
	app->summary.visible.message  = setting->summary.message;

	// traveler
	ugtk_traveler_select_category (&app->traveler,
				setting->window.nth_category,
				setting->window.nth_state);
	// menu
	ugtk_app_set_menu_setting (app, setting);
}

void  ugtk_app_set_plugin_setting (UgtkApp* app, UgtkSetting* setting)
{
	int  limit[2];
	int  sensitive = FALSE;

	switch (setting->plugin_order) {
	default:
	case UGTK_PLUGIN_ORDER_CURL:
		uget_app_remove_plugin ((UgetApp*) app, UgetPluginAria2Info);
		uget_app_set_default_plugin ((UgetApp*) app, UgetPluginCurlInfo);
		sensitive = FALSE;
		break;

	case UGTK_PLUGIN_ORDER_ARIA2:
		uget_app_remove_plugin ((UgetApp*) app, UgetPluginCurlInfo);
		uget_app_set_default_plugin ((UgetApp*) app, UgetPluginAria2Info);
		sensitive = TRUE;
		break;

	case UGTK_PLUGIN_ORDER_CURL_ARIA2:
		uget_app_add_plugin ((UgetApp*) app, UgetPluginAria2Info);
		uget_app_set_default_plugin ((UgetApp*) app, UgetPluginCurlInfo);
		sensitive = TRUE;
		break;

	case UGTK_PLUGIN_ORDER_ARIA2_CURL:
		uget_app_add_plugin ((UgetApp*) app, UgetPluginCurlInfo);
		uget_app_set_default_plugin ((UgetApp*) app, UgetPluginAria2Info);
		sensitive = TRUE;
		break;
	}

	if (setting->plugin_order >= UGTK_PLUGIN_ORDER_ARIA2) {
		uget_plugin_set (UgetPluginAria2Info, UGET_PLUGIN_ARIA2_URI,
		                 setting->aria2.uri);
		uget_plugin_set (UgetPluginAria2Info, UGET_PLUGIN_ARIA2_PATH,
		                 setting->aria2.path);
		uget_plugin_set (UgetPluginAria2Info, UGET_PLUGIN_ARIA2_ARGUMENT,
		                 setting->aria2.args);
		uget_plugin_set (UgetPluginAria2Info, UGET_PLUGIN_ARIA2_LAUNCH,
		                 (void*)(intptr_t) setting->aria2.launch);
		uget_plugin_set (UgetPluginAria2Info, UGET_PLUGIN_ARIA2_SHUTDOWN,
		                 (void*)(intptr_t) setting->aria2.shutdown);
		limit[0] = setting->aria2.limit.download;
		limit[1] = setting->aria2.limit.upload;
		uget_plugin_set (UgetPluginAria2Info, UGET_PLUGIN_SPEED_LIMIT, limit);
	}

//	app->aria2.remote_updated = FALSE;
	gtk_widget_set_sensitive ((GtkWidget*) app->trayicon.menu.create_torrent, sensitive);
	gtk_widget_set_sensitive ((GtkWidget*) app->trayicon.menu.create_metalink, sensitive);
	gtk_widget_set_sensitive ((GtkWidget*) app->menubar.file.create_torrent, sensitive);
	gtk_widget_set_sensitive ((GtkWidget*) app->menubar.file.create_metalink, sensitive);
	gtk_widget_set_sensitive ((GtkWidget*) app->toolbar.create_torrent, sensitive);
	gtk_widget_set_sensitive ((GtkWidget*) app->toolbar.create_metalink, sensitive);
}

void  ugtk_app_set_other_setting (UgtkApp* app, UgtkSetting* setting)
{
	// clipboard & commandline
	ugtk_clipboard_set_pattern (&app->clipboard,
			setting->clipboard.pattern);
	// global speed limit
	uget_task_set_speed (&app->task,
			setting->bandwidth.normal.download * 1024,
			setting->bandwidth.normal.upload   * 1024);
}

void  ugtk_app_set_menu_setting (UgtkApp* app, UgtkSetting* setting)
{
	// ----------------------------------------------------
	// UgtkEditMenu
	gtk_check_menu_item_set_active (
			(GtkCheckMenuItem*) app->menubar.edit.clipboard_monitor,
			setting->clipboard.monitor);
	gtk_check_menu_item_set_active (
			(GtkCheckMenuItem*) app->menubar.edit.clipboard_quiet,
			setting->clipboard.quiet);
	gtk_check_menu_item_set_active (
			(GtkCheckMenuItem*) app->menubar.edit.commandline_quiet,
			setting->commandline.quiet);
	gtk_check_menu_item_set_active (
			(GtkCheckMenuItem*) app->menubar.edit.skip_existing,
			setting->ui.skip_existing);

	switch (app->setting.completion.action) {
	default:
	case 0:
		gtk_check_menu_item_set_active (
				(GtkCheckMenuItem*)app->menubar.edit.completion.disable, TRUE);
		break;

	case 1:
		gtk_check_menu_item_set_active (
				(GtkCheckMenuItem*)app->menubar.edit.completion.hibernate, TRUE);
		break;

	case 2:
		gtk_check_menu_item_set_active (
				(GtkCheckMenuItem*)app->menubar.edit.completion.suspend, TRUE);
		break;

	case 3:
		gtk_check_menu_item_set_active (
				(GtkCheckMenuItem*)app->menubar.edit.completion.shutdown, TRUE);
		break;

	case 4:
		gtk_check_menu_item_set_active (
				(GtkCheckMenuItem*)app->menubar.edit.completion.reboot, TRUE);
		break;

	case 5:
		gtk_check_menu_item_set_active (
				(GtkCheckMenuItem*)app->menubar.edit.completion.custom, TRUE);
		break;
	}
	// ----------------------------------------------------
	// UgtkViewMenu
	gtk_check_menu_item_set_active (
			(GtkCheckMenuItem*) app->menubar.view.toolbar,
			setting->window.toolbar);
	gtk_check_menu_item_set_active (
			(GtkCheckMenuItem*) app->menubar.view.statusbar,
			setting->window.statusbar);
	gtk_check_menu_item_set_active (
			(GtkCheckMenuItem*) app->menubar.view.category,
			setting->window.category);
	gtk_check_menu_item_set_active (
			(GtkCheckMenuItem*) app->menubar.view.summary,
			setting->window.summary);
	// summary items
	gtk_check_menu_item_set_active (
			(GtkCheckMenuItem*) app->menubar.view.summary_items.name,
			setting->summary.name);
	gtk_check_menu_item_set_active (
			(GtkCheckMenuItem*) app->menubar.view.summary_items.folder,
			setting->summary.folder);
	gtk_check_menu_item_set_active (
			(GtkCheckMenuItem*) app->menubar.view.summary_items.category,
			setting->summary.category);
	gtk_check_menu_item_set_active (
			(GtkCheckMenuItem*) app->menubar.view.summary_items.uri,
			setting->summary.uri);
	gtk_check_menu_item_set_active (
			(GtkCheckMenuItem*) app->menubar.view.summary_items.message,
			setting->summary.message);
	// download column
	gtk_check_menu_item_set_active (
			(GtkCheckMenuItem*) app->menubar.view.columns.completed,
			setting->download_column.completed);
	gtk_check_menu_item_set_active (
			(GtkCheckMenuItem*) app->menubar.view.columns.total,
			setting->download_column.total);
	gtk_check_menu_item_set_active (
			(GtkCheckMenuItem*) app->menubar.view.columns.percent,
			setting->download_column.percent);
	gtk_check_menu_item_set_active (
			(GtkCheckMenuItem*) app->menubar.view.columns.elapsed,
			setting->download_column.elapsed);
	gtk_check_menu_item_set_active (
			(GtkCheckMenuItem*) app->menubar.view.columns.left,
			setting->download_column.left);
	gtk_check_menu_item_set_active (
			(GtkCheckMenuItem*) app->menubar.view.columns.speed,
			setting->download_column.speed);
	gtk_check_menu_item_set_active (
			(GtkCheckMenuItem*) app->menubar.view.columns.upload_speed,
			setting->download_column.upload_speed);
	gtk_check_menu_item_set_active (
			(GtkCheckMenuItem*) app->menubar.view.columns.uploaded,
			setting->download_column.uploaded);
	gtk_check_menu_item_set_active (
			(GtkCheckMenuItem*) app->menubar.view.columns.ratio,
			setting->download_column.ratio);
	gtk_check_menu_item_set_active (
			(GtkCheckMenuItem*) app->menubar.view.columns.retry,
			setting->download_column.retry);
	gtk_check_menu_item_set_active (
			(GtkCheckMenuItem*) app->menubar.view.columns.category,
			setting->download_column.category);
	gtk_check_menu_item_set_active (
			(GtkCheckMenuItem*) app->menubar.view.columns.uri,
			setting->download_column.uri);
	gtk_check_menu_item_set_active (
			(GtkCheckMenuItem*) app->menubar.view.columns.added_on,
			setting->download_column.added_on);
	gtk_check_menu_item_set_active (
			(GtkCheckMenuItem*) app->menubar.view.columns.completed_on,
			setting->download_column.completed_on);
}

void  ugtk_app_set_ui_setting (UgtkApp* app, UgtkSetting* setting)
{
#ifdef HAVE_APP_INDICATOR
	// AppIndicator
	ugtk_tray_icon_use_indicator (&app->trayicon, setting->ui.app_indicator);
#endif

	ugtk_tray_icon_set_visible (&app->trayicon, setting->ui.show_trayicon);
}

// decide sensitive for menu, toolbar
void  ugtk_app_decide_download_sensitive (UgtkApp* app)
{
	GtkTreeSelection*  selection;
	gboolean           sensitive;
	static gboolean    sensitive_last = TRUE;
	gint               n_selected;

	selection = gtk_tree_view_get_selection (app->traveler.download.view);
	n_selected = gtk_tree_selection_count_selected_rows (selection);
	if (n_selected > 0)
		sensitive = TRUE;
	else
		sensitive = FALSE;

	// change sensitive after select/unselect
	if (sensitive_last != sensitive) {
		sensitive_last  = sensitive;
		gtk_widget_set_sensitive (app->toolbar.runnable, sensitive);
		gtk_widget_set_sensitive (app->toolbar.pause, sensitive);
		gtk_widget_set_sensitive (app->toolbar.properties, sensitive);
		gtk_widget_set_sensitive (app->menubar.download.open, sensitive);
		gtk_widget_set_sensitive (app->menubar.download.open_folder, sensitive);
		gtk_widget_set_sensitive (app->menubar.download.delete, sensitive);
		gtk_widget_set_sensitive (app->menubar.download.delete_file, sensitive);
		gtk_widget_set_sensitive (app->menubar.download.force_start, sensitive);
		gtk_widget_set_sensitive (app->menubar.download.runnable, sensitive);
		gtk_widget_set_sensitive (app->menubar.download.pause, sensitive);
		gtk_widget_set_sensitive (app->menubar.download.move_to.item, sensitive);
		gtk_widget_set_sensitive (app->menubar.download.prioriy.item, sensitive);
	}
	// properties
	if (n_selected > 1) {
		gtk_widget_set_sensitive (app->toolbar.properties, FALSE);
		gtk_widget_set_sensitive (app->menubar.download.properties, FALSE);
	}
	else {
		gtk_widget_set_sensitive (app->toolbar.properties, sensitive);
		gtk_widget_set_sensitive (app->menubar.download.properties, sensitive);
	}

	// Move Up/Down/Top/Bottom functions need reset sensitive when selection changed.
	// These need by  on_move_download_xxx()  series.
	// "All Category" and "All Status" can't move download position if they were sorted.
	if (app->traveler.category.cursor.pos == 0 ||
	    app->traveler.state.cursor.pos    == 0)
	{
		if (n_selected > 0) {
			if (app->setting.download_column.sort.nth == UGTK_NODE_COLUMN_STATE)
				sensitive = TRUE;
			else
				sensitive = FALSE;
		}
	}
	// move up/down
	gtk_widget_set_sensitive (app->toolbar.move_up, sensitive);
	gtk_widget_set_sensitive (app->toolbar.move_down, sensitive);
	gtk_widget_set_sensitive (app->toolbar.move_top, sensitive);
	gtk_widget_set_sensitive (app->toolbar.move_bottom, sensitive);
	gtk_widget_set_sensitive (app->menubar.download.move_up, sensitive);
	gtk_widget_set_sensitive (app->menubar.download.move_down, sensitive);
	gtk_widget_set_sensitive (app->menubar.download.move_top, sensitive);
	gtk_widget_set_sensitive (app->menubar.download.move_bottom, sensitive);
}

// decide sensitive for menu, toolbar
void  ugtk_app_decide_category_sensitive (UgtkApp* app)
{
	static gboolean  sensitive_last = TRUE;
	gboolean         sensitive;

	if (app->traveler.category.cursor.node)
		sensitive = TRUE;
	else
		sensitive = FALSE;

	if (sensitive_last != sensitive) {
		sensitive_last  = sensitive;
		gtk_widget_set_sensitive (app->menubar.category.properties, sensitive);
		gtk_widget_set_sensitive (app->menubar.view.columns.self, sensitive);
	}
	// cursor at "All Category"
	if (app->traveler.category.cursor.pos == 0) {
		gtk_widget_set_sensitive (app->menubar.file.save_category, FALSE);
		gtk_widget_set_sensitive (app->menubar.category.delete, FALSE);
	}
	else {
		gtk_widget_set_sensitive (app->menubar.file.save_category, sensitive);
		gtk_widget_set_sensitive (app->menubar.category.delete, sensitive);
	}
	// Move Up
	if (app->traveler.category.cursor.pos <= 1)
		gtk_widget_set_sensitive (app->menubar.category.move_up, FALSE);
	else
		gtk_widget_set_sensitive (app->menubar.category.move_up, TRUE);
	// Move Down
	if (app->traveler.category.cursor.pos == 0 ||
	    app->traveler.category.cursor.node->next == NULL)
	{
		gtk_widget_set_sensitive (app->menubar.category.move_down, FALSE);
	}
	else
		gtk_widget_set_sensitive (app->menubar.category.move_down, TRUE);

	ugtk_app_decide_download_sensitive (app);
}

void  ugtk_app_decide_trayicon_visible (UgtkApp* app)
{
	gboolean  visible;

	if (app->setting.ui.show_trayicon)
		visible = TRUE;
	else {
		if (gtk_widget_get_visible ((GtkWidget*) app->window.self))
			visible = FALSE;
		else
			visible = TRUE;
	}
	ugtk_tray_icon_set_visible (&app->trayicon, visible);
}

void  ugtk_app_decide_to_quit (UgtkApp* app)
{
	UgtkConfirmDialog*  cdialog;

	if (app->setting.ui.exit_confirmation == FALSE)
		ugtk_app_quit (app);
	else if (app->dialogs.exit_confirmation)
		gtk_widget_show (app->dialogs.exit_confirmation);
	else {
		cdialog = ugtk_confirm_dialog_new (UGTK_CONFIRM_DIALOG_EXIT, app);
		ugtk_confirm_dialog_run (cdialog);
	}
}

// ------------------------------------
// create node by UI

void  ugtk_app_create_category (UgtkApp* app)
{
	UgtkNodeDialog*  ndialog;
	UgetNode*  cnode_src;
	UgetNode*  cnode;
	gchar*     title;

	title = g_strconcat (UGTK_APP_NAME, " - ", _("New Category"), NULL);
	ndialog = ugtk_node_dialog_new (title, app, TRUE);
	g_free (title);
	ugtk_download_form_set_folders (&ndialog->download, &app->setting);

	// category list
	cnode_src = app->traveler.category.cursor.node;
	if (cnode_src->parent != &app->real)
		cnode_src = app->real.children;
//	cnode_src = cnode_src->data;
	cnode = uget_node_new (NULL);
	cnode->name = ug_strdup_printf ("%s%s", _("Copy - "), cnode_src->name);
	ug_info_assign (&cnode->info, &cnode_src->info, NULL);

	ugtk_node_dialog_set (ndialog, cnode);
	ugtk_node_dialog_run (ndialog, UGTK_NODE_DIALOG_NEW_CATEGORY, cnode);
}

void  ugtk_app_create_download (UgtkApp* app, const char* sub_title, const char* uri)
{
	UgtkNodeDialog*  ndialog;
	UgetNode*  cnode;
	gchar*     title;
	GList*     list;

	if (sub_title)
		title = g_strconcat (UGTK_APP_NAME, " - ", sub_title, NULL);
	else
		title = g_strconcat (UGTK_APP_NAME, " - ", _("New Download"), NULL);
	ndialog = ugtk_node_dialog_new (title, app, FALSE);
	g_free (title);
	ugtk_download_form_set_folders (&ndialog->download, &app->setting);

	// category list
	cnode = app->traveler.category.cursor.node;
//	cnode = cnode->data;
	if (cnode->parent != &app->real)
		cnode = app->real.children;
	ugtk_node_dialog_set_category (ndialog, cnode->data);

	list = NULL;
	if (uri)
		gtk_entry_set_text (GTK_ENTRY (ndialog->download.uri_entry), uri);
	else
		list = ugtk_clipboard_get_uris (&app->clipboard);
	// use first URI from clipboard to set URL entry
	if (list) {
		ndialog->download.changed.uri = TRUE;
		gtk_entry_set_text ((GtkEntry*) ndialog->download.uri_entry, list->data);
		g_list_free_full (list, g_free);
		ugtk_download_form_complete_entry (&ndialog->download);
	}

	ugtk_node_dialog_run (ndialog, UGTK_NODE_DIALOG_NEW_DOWNLOAD, NULL);
}

// ------------------------------------
// delete selected node

void  ugtk_app_delete_category (UgtkApp* app)
{
	UgetNode*    cnode;
	int          pos;

	cnode = app->traveler.category.cursor.node;
	pos   = app->traveler.category.cursor.pos;
	// move cursor
	if (pos <= 0)
		return;

	if (cnode->next)
		ugtk_traveler_select_category (&app->traveler, pos + 1, -1);
	else {
		if (app->real.n_children > 1)
			ugtk_traveler_select_category (&app->traveler, pos - 1, -1);
		else {
			// The last category will be deleted.
			ugtk_app_add_default_category (app);
			ugtk_traveler_select_category (&app->traveler, pos + 1, -1);
		}
	}
	// remove category
	uget_app_delete_category ((UgetApp*) app, cnode);
	// sync UgtkMenubar.download.move_to
	ugtk_menubar_sync_category (&app->menubar, app, TRUE);
}

void  ugtk_app_delete_download (UgtkApp* app, gboolean delete_files)
{
	UgetNode* cursor;
	UgetNode* node;
	GList*    link;
	GList*    list = NULL;
	// check shift key status
	GdkWindow*       gdk_win;
	GdkDevice*       dev_pointer;
	GdkModifierType  mask;

	// check shift key status
	gdk_win = gtk_widget_get_parent_window ((GtkWidget*) app->traveler.download.view);
	dev_pointer = gdk_device_manager_get_client_pointer (
			gdk_display_get_device_manager (gdk_window_get_display (gdk_win)));
	gdk_window_get_device_position (gdk_win, dev_pointer, NULL, NULL, &mask);

	cursor = app->traveler.download.cursor.node->data;
	list = ugtk_traveler_get_selected (&app->traveler);
	for (link = list;  link;  link = link->next) {
		node = link->data;
		node = node->data;
		link->data = node;
		if (delete_files || mask & GDK_SHIFT_MASK)
			uget_app_delete_download ((UgetApp*) app, node, delete_files);
		else {
			if (uget_app_recycle_download ((UgetApp*) app, node))
				continue;
		}
		// if current node has been deleted
		if (cursor == node)
			cursor = NULL;
		link->data = NULL;
	}
	if (delete_files == FALSE && (mask & GDK_SHIFT_MASK) == 0) {
		ugtk_traveler_set_cursor (&app->traveler, cursor);
		ugtk_traveler_set_selected (&app->traveler, list);
	}
	g_list_free (list);
	// refresh
	gtk_widget_queue_draw ((GtkWidget*) app->traveler.category.view);
	gtk_widget_queue_draw ((GtkWidget*) app->traveler.state.view);

	app->user_action = TRUE;
}

// ------------------------------------
// edit selected node

void  ugtk_app_edit_category (UgtkApp* app)
{
	UgtkNodeDialog* ndialog;
	UgetNode*       node;
	gchar*          title;

	if (app->traveler.category.cursor.pos == 0)
		node = app->real.children;
	else
		node = app->traveler.category.cursor.node;
	if (node == NULL)
		return;

	title = g_strconcat (UGTK_APP_NAME " - ", _("Category Properties"), NULL);
	ndialog = ugtk_node_dialog_new (title, app, TRUE);
	g_free (title);
	ugtk_download_form_set_folders (&ndialog->download, &app->setting);
	ugtk_node_dialog_set (ndialog, node->data);
	ugtk_node_dialog_run (ndialog, UGTK_NODE_DIALOG_EDIT_CATEGORY, node->data);
}

void  ugtk_app_edit_download (UgtkApp* app)
{
	UgtkNodeDialog* ndialog;
	UgetNode*       node;
	gchar*          title;

	title = g_strconcat (UGTK_APP_NAME " - ", _("Download Properties"), NULL);
	ndialog = ugtk_node_dialog_new (title, app, FALSE);
	g_free (title);
	ugtk_download_form_set_folders (&ndialog->download, &app->setting);

	node = app->traveler.download.cursor.node;
	ugtk_node_dialog_set (ndialog, node->data);
	ugtk_node_dialog_run (ndialog, UGTK_NODE_DIALOG_EDIT_DOWNLOAD, node->data);
}

// ------------------------------------
// queue/pause

void  ugtk_app_queue_download (UgtkApp* app)
{
	UgetNode*  node;
	UgetNode*  cursor;
	GList*     list;
	GList*     link;

	list = ugtk_traveler_get_selected (&app->traveler);
	cursor = app->traveler.download.cursor.node->data;
	for (link = list;  link;  link = link->next) {
		node = link->data;
		node = node->data;
		link->data = node;
		uget_app_queue_download ((UgetApp*) app, node);
	}
	if (app->traveler.state.cursor.pos == 0) {
		ugtk_traveler_set_cursor (&app->traveler, cursor);
		ugtk_traveler_set_selected (&app->traveler, list);
	}
	g_list_free (list);
	// refresh other data & status
	gtk_widget_queue_draw ((GtkWidget*) app->traveler.category.view);
	gtk_widget_queue_draw ((GtkWidget*) app->traveler.state.view);
//	ugtk_summary_show (&app->summary, app->traveler.download.cursor.node);
}

void  ugtk_app_pause_download (UgtkApp* app)
{
	UgetNode*  node;
	UgetNode*  cursor;
	GList*     list;
	GList*     link;

	list = ugtk_traveler_get_selected (&app->traveler);
	cursor = app->traveler.download.cursor.node->data;
	for (link = list;  link;  link = link->next) {
		node = link->data;
		node = node->data;
		link->data = node;
		uget_app_pause_download ((UgetApp*) app, node);
	}
	if (app->traveler.state.cursor.pos == 0) {
		ugtk_traveler_set_cursor (&app->traveler, cursor);
		ugtk_traveler_set_selected (&app->traveler, list);
	}
	g_list_free (list);
	// refresh other data & status
	gtk_widget_queue_draw ((GtkWidget*) app->traveler.category.view);
	gtk_widget_queue_draw ((GtkWidget*) app->traveler.state.view);
//	ugtk_summary_show (&app->summary, app->traveler.download.cursor.node);

	app->user_action = TRUE;
}

void  ugtk_app_switch_download_state (UgtkApp* app)
{
	UgetNode*  node;
	UgetNode*  cursor;
	GList*     list;
	GList*     link;

	list = ugtk_traveler_get_selected (&app->traveler);
	cursor = app->traveler.download.cursor.node->data;
	for (link = list;  link;  link = link->next) {
		node = link->data;
		node = node->data;
		link->data = node;
		if (node->state & UGET_STATE_PAUSED)
			uget_app_queue_download ((UgetApp*) app, node);
		else if (node->state & UGET_STATE_ACTIVE)
			uget_app_pause_download ((UgetApp*) app, node);
	}
	if (app->traveler.state.cursor.pos == 0) {
		ugtk_traveler_set_cursor (&app->traveler, cursor);
		ugtk_traveler_set_selected (&app->traveler, list);
	}
	g_list_free (list);
	// refresh other data & status
	gtk_widget_queue_draw ((GtkWidget*) app->traveler.category.view);
	gtk_widget_queue_draw ((GtkWidget*) app->traveler.state.view);
//	ugtk_summary_show (&app->summary, app->traveler.download.cursor.node);

	app->user_action = TRUE;
}

// ------------------------------------
// move selected node

void  ugtk_app_move_download_up (UgtkApp* app)
{
	if (ugtk_traveler_move_selected_up (&app->traveler) > 0) {
		gtk_widget_set_sensitive (app->toolbar.move_down, TRUE);
		gtk_widget_set_sensitive (app->toolbar.move_bottom, TRUE);
		gtk_widget_set_sensitive (app->menubar.download.move_down, TRUE);
		gtk_widget_set_sensitive (app->menubar.download.move_bottom, TRUE);
	}
	else {
		gtk_widget_set_sensitive (app->toolbar.move_up, FALSE);
		gtk_widget_set_sensitive (app->toolbar.move_top, FALSE);
		gtk_widget_set_sensitive (app->menubar.download.move_up, FALSE);
		gtk_widget_set_sensitive (app->menubar.download.move_top, FALSE);
	}
}

void  ugtk_app_move_download_down (UgtkApp* app)
{
	if (ugtk_traveler_move_selected_down (&app->traveler) > 0) {
		gtk_widget_set_sensitive (app->toolbar.move_up, TRUE);
		gtk_widget_set_sensitive (app->toolbar.move_top, TRUE);
		gtk_widget_set_sensitive (app->menubar.download.move_up, TRUE);
		gtk_widget_set_sensitive (app->menubar.download.move_top, TRUE);
	}
	else {
		gtk_widget_set_sensitive (app->toolbar.move_down, FALSE);
		gtk_widget_set_sensitive (app->toolbar.move_bottom, FALSE);
		gtk_widget_set_sensitive (app->menubar.download.move_down, FALSE);
		gtk_widget_set_sensitive (app->menubar.download.move_bottom, FALSE);
	}
}

void  ugtk_app_move_download_top (UgtkApp* app)
{
	if (ugtk_traveler_move_selected_top (&app->traveler) > 0) {
		gtk_widget_set_sensitive (app->toolbar.move_down, TRUE);
		gtk_widget_set_sensitive (app->toolbar.move_bottom, TRUE);
		gtk_widget_set_sensitive (app->menubar.download.move_down, TRUE);
		gtk_widget_set_sensitive (app->menubar.download.move_bottom, TRUE);
	}
	gtk_widget_set_sensitive (app->toolbar.move_up, FALSE);
	gtk_widget_set_sensitive (app->toolbar.move_top, FALSE);
	gtk_widget_set_sensitive (app->menubar.download.move_up, FALSE);
	gtk_widget_set_sensitive (app->menubar.download.move_top, FALSE);
}

void  ugtk_app_move_download_bottom (UgtkApp* app)
{
	if (ugtk_traveler_move_selected_bottom (&app->traveler) > 0) {
		gtk_widget_set_sensitive (app->toolbar.move_up, TRUE);
		gtk_widget_set_sensitive (app->toolbar.move_top, TRUE);
		gtk_widget_set_sensitive (app->menubar.download.move_up, TRUE);
		gtk_widget_set_sensitive (app->menubar.download.move_top, TRUE);
	}
	gtk_widget_set_sensitive (app->toolbar.move_down, FALSE);
	gtk_widget_set_sensitive (app->toolbar.move_bottom, FALSE);
	gtk_widget_set_sensitive (app->menubar.download.move_down, FALSE);
	gtk_widget_set_sensitive (app->menubar.download.move_bottom, FALSE);
}

void  ugtk_app_move_download_to (UgtkApp* app, UgetNode* cnode)
{
	UgetNode* node;
	GList*    link;
	GList*    list = NULL;

	if (cnode == app->traveler.category.cursor.node)
		return;

	cnode = cnode->data;
	list = ugtk_traveler_get_selected (&app->traveler);
	for (link = list;  link;  link = link->next) {
		node = link->data;
		node = node->data;
		if (node->parent == cnode)
			continue;
		uget_node_remove (node->parent, node);
		uget_node_unref_fake (node);
		uget_node_append (cnode, node);
	}
	g_list_free (list);
	// refresh
	gtk_widget_queue_draw ((GtkWidget*) app->traveler.category.view);
	gtk_widget_queue_draw ((GtkWidget*) app->traveler.state.view);
}

// ------------------------------------
// torrent & metalink

static GtkWidget*  create_file_chooser (GtkWindow* parent,
                                        GtkFileChooserAction action,
                                        const gchar* title,
                                        const gchar* filter_name,
                                        const gchar* mine_type)
{
	GtkWidget*      dialog;
	GtkFileFilter*  filter;

	dialog = gtk_file_chooser_dialog_new (title,
			parent,
			action,
			GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
			GTK_STOCK_OK,     GTK_RESPONSE_OK,
			NULL);
	gtk_window_set_destroy_with_parent ((GtkWindow*) dialog, TRUE);

	if (filter_name) {
		filter = gtk_file_filter_new ();
		gtk_file_filter_set_name (filter, filter_name);
		gtk_file_filter_add_mime_type (filter, mine_type);
		gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (dialog), filter);
	}
	return dialog;
}

static void	on_create_torrent_response (GtkWidget* dialog, gint response, UgtkApp* app)
{
	gchar*  file;

	if (response != GTK_RESPONSE_OK ) {
		gtk_widget_destroy (dialog);
		return;
	}
	// get filename
	file = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));
//	file = gtk_file_chooser_get_uri (GTK_FILE_CHOOSER (dialog));
	gtk_widget_destroy (dialog);
	ugtk_app_create_download (app, _("New Torrent"), file);
	g_free (file);
}

static void	on_create_metalink_response (GtkWidget* dialog, gint response, UgtkApp* app)
{
	gchar*  file;

	if (response != GTK_RESPONSE_OK) {
		gtk_widget_destroy (dialog);
		return;
	}
	// get filename
	file = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));
//	file = gtk_file_chooser_get_uri (GTK_FILE_CHOOSER (dialog));
	gtk_widget_destroy (dialog);
	ugtk_app_create_download (app, _("New Metalink"), file);
	g_free (file);
}

void  ugtk_app_create_torrent (UgtkApp* app)
{
	GtkWidget*  dialog;
	gchar*      title;

	title = g_strconcat (UGTK_APP_NAME " - ", _("Open Torrent file"), NULL);
	dialog = create_file_chooser (app->window.self,
			GTK_FILE_CHOOSER_ACTION_OPEN,
			title, "Torrent file (*.torrent)", "application/x-bittorrent");
	g_free (title);
	g_signal_connect (dialog, "response",
			G_CALLBACK (on_create_torrent_response), app);
	gtk_widget_show (dialog);
}

void  ugtk_app_create_metalink (UgtkApp* app)
{
	GtkFileFilter*  filter;
	GtkWidget*      dialog;
	gchar*          title;

	title = g_strconcat (UGTK_APP_NAME " - ", _("Open Metalink file"), NULL);
	dialog = create_file_chooser (app->window.self,
			GTK_FILE_CHOOSER_ACTION_OPEN,
			title, NULL, NULL);
	g_free (title);

	filter = gtk_file_filter_new ();
	gtk_file_filter_set_name (filter, "Metalink file (*.metalink, *.meta4)");
	gtk_file_filter_add_pattern (filter, "*.metalink");
	gtk_file_filter_add_pattern (filter, "*.meta4");
//	gtk_file_filter_add_mime_type (filter, "application/metalink+xml");
//	gtk_file_filter_add_mime_type (filter, "application/metalink4+xml");
	gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (dialog), filter);
	g_signal_connect (dialog, "response",
			G_CALLBACK (on_create_metalink_response), app);
	gtk_widget_show (dialog);
}

// ------------------------------------
// import/export

static void	on_save_category_response (GtkWidget* dialog, gint response, UgtkApp* app)
{
	UgetNode* cnode;
	gchar*    file;

	gtk_widget_set_sensitive ((GtkWidget*) app->window.self, TRUE);
	if (response != GTK_RESPONSE_OK) {
		gtk_widget_destroy (dialog);
		return;
	}
	if (app->traveler.category.cursor.pos == 0)
		return;

	// get filename
	file = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));
//	file = gtk_file_chooser_get_uri (GTK_FILE_CHOOSER (dialog));
	gtk_widget_destroy (dialog);
	cnode = app->traveler.category.cursor.node;
	if (uget_app_save_category ((UgetApp*) app, cnode, file) == FALSE)
		ugtk_app_show_message (app, GTK_MESSAGE_ERROR, _("Failed to save category file."));
	g_free (file);
}

static void	on_load_category_response (GtkWidget* dialog, gint response, UgtkApp* app)
{
	gchar*  file;

	gtk_widget_set_sensitive ((GtkWidget*) app->window.self, TRUE);
	if (response != GTK_RESPONSE_OK) {
		gtk_widget_destroy (dialog);
		return;
	}
	// get filename
	file = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));
//	file = gtk_file_chooser_get_uri (GTK_FILE_CHOOSER (dialog));
	gtk_widget_destroy (dialog);
	if (uget_app_load_category ((UgetApp*) app, file))
		ugtk_menubar_sync_category (&app->menubar, app, TRUE);
	else
		ugtk_app_show_message (app, GTK_MESSAGE_ERROR, _("Failed to load category file."));
	g_free (file);
}

void  ugtk_app_save_category (UgtkApp* app)
{
	GtkWidget*      dialog;
	gchar*          title;

	gtk_widget_set_sensitive ((GtkWidget*) app->window.self, FALSE);

	title = g_strconcat (UGTK_APP_NAME " - ", _("Save Category file"), NULL);
	dialog = create_file_chooser (app->window.self,
			GTK_FILE_CHOOSER_ACTION_SAVE,
			title, NULL, NULL);
	g_free (title);

	g_signal_connect (dialog, "response",
			G_CALLBACK (on_save_category_response), app);
	gtk_widget_show (dialog);
}

void  ugtk_app_load_category (UgtkApp* app)
{
	GtkWidget*      dialog;
	gchar*          title;

	gtk_widget_set_sensitive ((GtkWidget*) app->window.self, FALSE);

	title = g_strconcat (UGTK_APP_NAME " - ", _("Open Category file"), NULL);
	dialog = create_file_chooser (app->window.self,
			GTK_FILE_CHOOSER_ACTION_OPEN,
			title, "JSON file (*.json)", "application/json");
	g_free (title);

	g_signal_connect (dialog, "response",
			G_CALLBACK (on_load_category_response), app);
	gtk_widget_show (dialog);
}

static void	on_import_html_file_response (GtkWidget* dialog, gint response, UgtkApp* app)
{
	UgHtmlFilter*     filter;
	UgHtmlFilterTag*  tag_a;
	UgHtmlFilterTag*  tag_img;
	UgtkBatchDialog*  bdialog;
	UgtkSelectorPage* page;
	UgetNode*  cnode;
	gchar*  string;
	gchar*  file;

	if (response != GTK_RESPONSE_OK ) {
		gtk_widget_destroy (dialog);
		return;
	}
	// read URLs from html file
	string = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));
	gtk_widget_destroy (dialog);
	file = g_filename_to_utf8 (string, -1, NULL, NULL, NULL);
	g_free (string);
	string = NULL;
	// parse html
	filter = ug_html_filter_new ();
	tag_a = ug_html_filter_tag_new ("A", "HREF");		// <A HREF="Link">
	ug_html_filter_add_tag (filter, tag_a);
	tag_img = ug_html_filter_tag_new ("IMG", "SRC");		// <IMG SRC="Link">
	ug_html_filter_add_tag (filter, tag_img);
	ug_html_filter_parse_file (filter, file);
	g_free (file);
	if (filter->base_href)
		string = g_strdup (filter->base_href);
	ug_html_filter_free (filter);
	// UgtkBatchDialog
	bdialog = ugtk_batch_dialog_new (
			gtk_window_get_title ((GtkWindow*) dialog), app);
	ugtk_download_form_set_folders (&bdialog->download, &app->setting);
	ugtk_batch_dialog_use_selector (bdialog);
	// category
	cnode = app->traveler.category.cursor.node;
	if (cnode->parent != &app->real)
		cnode = app->real.children;
	ugtk_batch_dialog_set_category (bdialog, cnode);
	// set <base href>
	if (string) {
		gtk_entry_set_text (bdialog->selector.href_entry, string);
		g_free (string);
	}
	// add link
	page = ugtk_selector_add_page (&bdialog->selector, _("Link <A>"));
	ugtk_selector_page_add_uris (page, (GList*)tag_a->attr_values.head);
	ug_list_clear (&tag_a->attr_values, TRUE);
	ug_html_filter_tag_unref (tag_a);
	// add image
	page = ugtk_selector_add_page (&bdialog->selector, _("Image <IMG>"));
	ugtk_selector_page_add_uris (page, (GList*)tag_img->attr_values.head);
	ug_list_clear (&tag_img->attr_values, TRUE);
	ug_html_filter_tag_unref (tag_img);

	ugtk_batch_dialog_run (bdialog);
}

static void	on_import_text_file_response (GtkWidget* dialog, gint response, UgtkApp* app)
{
	UgtkBatchDialog*   bdialog;
	UgtkSelectorPage*  page;
	UgetNode* cnode;
	gchar*    string;
	gchar*    file;
	GList*    list;
	GError*   error = NULL;

	if (response != GTK_RESPONSE_OK ) {
		gtk_widget_destroy (dialog);
		return;
	}
	// read URLs from text file
	string = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));
	gtk_widget_destroy (dialog);
	file = g_filename_to_utf8 (string, -1, NULL, NULL, NULL);
	g_free (string);
	list = ugtk_text_file_get_uris (file, &error);
	g_free (file);
	if (error) {
		ugtk_app_show_message (app, GTK_MESSAGE_ERROR, error->message);
		g_error_free (error);
		return;
	}
	// UgtkBatchDialog
	bdialog = ugtk_batch_dialog_new (
			gtk_window_get_title ((GtkWindow*) dialog), app);
	ugtk_batch_dialog_use_selector (bdialog);
	ugtk_download_form_set_folders (&bdialog->download, &app->setting);
	// category
	cnode = app->traveler.category.cursor.node;
	if (cnode->parent != &app->real)
		cnode = app->real.children;
	ugtk_batch_dialog_set_category (bdialog, cnode);

	page = ugtk_selector_add_page (&bdialog->selector, _("Text File"));
	ugtk_selector_hide_href (&bdialog->selector);
	ugtk_selector_page_add_uris (page, list);
	g_list_free (list);

	ugtk_batch_dialog_run (bdialog);
}

static void	on_export_text_file_response (GtkWidget* dialog, gint response, UgtkApp* app)
{
	GIOChannel*  channel;
	UgetCommon*  common;
	UgetNode*    node;
	gchar*       fname;

	if (response != GTK_RESPONSE_OK ) {
		gtk_widget_destroy (dialog);
		return;
	}
	// write all URLs to text file
	fname = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));
	gtk_widget_destroy (dialog);
	channel = g_io_channel_new_file (fname, "w", NULL);
	g_free (fname);

	node = app->traveler.category.cursor.node;
	for (node = node->children;  node;  node = node->next) {
		common = ug_info_get (&node->data->info, UgetCommonInfo);
		if (common == NULL)
			continue;
		if (common->uri) {
			g_io_channel_write_chars (channel, common->uri, -1, NULL, NULL);
#ifdef _WIN32
			g_io_channel_write_chars (channel, "\r\n", 2, NULL, NULL);
#else
			g_io_channel_write_chars (channel, "\n", 1, NULL, NULL);
#endif
		}
	}
	g_io_channel_unref (channel);
}

void  ugtk_app_import_html_file (UgtkApp* app)
{
	GtkWidget*  dialog;
	gchar*      title;

	title = g_strconcat (UGTK_APP_NAME " - ", _("Import URLs from HTML file"), NULL);
	dialog = create_file_chooser (app->window.self,
			GTK_FILE_CHOOSER_ACTION_OPEN,
			title, "HTML file (*.htm, *.html)", "text/html");
	g_free (title);
	g_signal_connect (dialog, "response",
			G_CALLBACK (on_import_html_file_response), app);
	gtk_widget_show (dialog);
}

void  ugtk_app_import_text_file (UgtkApp* app)
{
	GtkWidget*  dialog;
	gchar*      title;

	title = g_strconcat (UGTK_APP_NAME " - ", _("Import URLs from text file"), NULL);
	dialog = create_file_chooser (app->window.self,
			GTK_FILE_CHOOSER_ACTION_OPEN,
			title, "Plain text file", "text/plain");
	g_free (title);
	g_signal_connect (dialog, "response",
			G_CALLBACK (on_import_text_file_response), app);
	gtk_widget_show (dialog);
}

void  ugtk_app_export_text_file (UgtkApp* app)
{
	GtkWidget*  dialog;
	gchar*      title;

	title = g_strconcat (UGTK_APP_NAME " - ", _("Export URLs to text file"), NULL);
	dialog = create_file_chooser (app->window.self,
			GTK_FILE_CHOOSER_ACTION_SAVE,
			title, NULL, NULL);
	g_free (title);
	g_signal_connect (dialog, "response",
			G_CALLBACK (on_export_text_file_response), app);
	gtk_widget_show (dialog);
}

// ------------------------------------
// batch

void  ugtk_app_sequence_batch (UgtkApp* app)
{
	UgtkBatchDialog* bdialog;
	UgetNode*  cnode;
	gchar*     title;

	title = g_strconcat (UGTK_APP_NAME " - ", _("URL Sequence batch"), NULL);
	bdialog = ugtk_batch_dialog_new (title, app);
	g_free (title);
	ugtk_batch_dialog_use_sequencer (bdialog);
	ugtk_download_form_set_folders (&bdialog->download, &app->setting);

	// category list
	cnode = app->traveler.category.cursor.node;
//	cnode = cnode->data;
	if (cnode->parent != &app->real)
		cnode = app->real.children;
	ugtk_batch_dialog_set_category (bdialog, cnode->data);

	ugtk_batch_dialog_run (bdialog);
}

void  ugtk_app_clipboard_batch (UgtkApp* app)
{
	UgtkBatchDialog*  bdialog;
	UgtkSelectorPage* page;
	UgetNode*  cnode;
	GList*     list;
	gchar*     title;

	list = ugtk_clipboard_get_uris (&app->clipboard);
	if (list == NULL) {
		ugtk_app_show_message (app, GTK_MESSAGE_ERROR,
				_("No URLs found in clipboard."));
		return;
	}
	// filter existing
	if (app->setting.ui.skip_existing) {
		if (ugtk_app_filter_existing (app, list) == 0) {
//			g_list_foreach (list, (GFunc) g_free, NULL);
			g_list_free (list);
			ugtk_app_show_message (app, GTK_MESSAGE_INFO,
					_("All URLs had existed."));
			return;
		}
	}

	title = g_strconcat (UGTK_APP_NAME " - ", _("Clipboard batch"), NULL);
	bdialog = ugtk_batch_dialog_new (title, app);
	g_free (title);
	ugtk_download_form_set_folders (&bdialog->download, &app->setting);
	ugtk_batch_dialog_use_selector (bdialog);
	// selector
	ugtk_selector_hide_href (&bdialog->selector);
	page = ugtk_selector_add_page (&bdialog->selector, _("Clipboard"));
	ugtk_selector_page_add_uris (page, list);
	g_list_free (list);

	// category list
	cnode = app->traveler.category.cursor.node;
//	cnode = cnode->data;
	if (cnode->parent != &app->real)
		cnode = app->real.children;
	ugtk_batch_dialog_set_category (bdialog, cnode->data);

	ugtk_batch_dialog_run (bdialog);
}

int   ugtk_app_filter_existing (UgtkApp* app, GList* uris)
{
	int  counts;

	for (counts = 0;  uris;  uris = uris->next) {
		if (uris->data == NULL)
			continue;
		if (uget_uri_hash_find (app->uri_hash, (char*) uris->data) == FALSE)
			counts++;
		else {
			g_free (uris->data);
			uris->data = NULL;
		}
	}
	return counts;
}

// ------------------------------------
// emit signal "row-changed" for UgtkNodeTree and UgtkNodeList

void  ugtk_app_download_changed (UgtkApp* app, UgetNode* dnode)
{
	GtkTreeModel* model;
	GtkTreePath*  path;
	GtkTreeIter   iter;

	if (ugtk_traveler_get_iter (&app->traveler, &iter, dnode)) {
		model = GTK_TREE_MODEL (app->traveler.download.model);
		path = gtk_tree_model_get_path (model, &iter);
		if (path) {
			gtk_tree_model_row_changed (model, path, &iter);
			gtk_tree_path_free (path);
		}
	}
}

void  ugtk_app_category_changed (UgtkApp* app, UgetNode* cnode)
{
	GtkTreeIter   iter;
	GtkTreePath*  path;
	GtkTreeModel* model;

	model = GTK_TREE_MODEL (app->traveler.category.model);
	if (app->traveler.category.cursor.pos > 0) {
		iter.stamp = app->traveler.category.model->stamp;
		iter.user_data = cnode;
		// update category
		path = gtk_tree_model_get_path (model, &iter);
		gtk_tree_model_row_changed (model, path, &iter);
		gtk_tree_path_free (path);
	}
	// update "All Category"
	path = gtk_tree_path_new_first ();
	gtk_tree_model_get_iter_first (model, &iter);
	gtk_tree_model_row_changed (model, path, &iter);
	gtk_tree_path_free (path);

	// refresh status list
	gtk_widget_queue_draw ((GtkWidget*) app->traveler.state.view);
}

void  ugtk_app_add_default_category (UgtkApp* app)
{
	UgetNode*     cnode;
	UgetCommon*   common;
	UgetCategory* category;
	static int    counts = 0;

	cnode = uget_node_new (NULL);
	cnode->name = ug_strdup_printf ("%s %d", _("New"), counts++);
	common = ug_info_realloc (&cnode->info, UgetCommonInfo);
	common->folder = ug_strdup (g_get_home_dir ());
	category = ug_info_realloc (&cnode->info, UgetCategoryInfo);
	*(char**)ug_array_alloc (&category->schemes, 1) = ug_strdup ("http");
	*(char**)ug_array_alloc (&category->schemes, 1) = ug_strdup ("ftp");
	*(char**)ug_array_alloc (&category->hosts, 1) = ug_strdup (".com");
	*(char**)ug_array_alloc (&category->hosts, 1) = ug_strdup (".org");
	*(char**)ug_array_alloc (&category->file_exts, 1) = ug_strdup ("torrent");
	*(char**)ug_array_alloc (&category->file_exts, 1) = ug_strdup ("metalink");

	uget_app_add_category ((UgetApp*) app, cnode);
}

void  ugtk_app_clear_attachment (UgtkApp* app)
{
	UgetNode*   dnode;
	UgetHttp*   http;
	GDir*       gdir;
	void*       hash;
	const char* name;
	GString*    gstr;
	int         folder_len;

	hash = uget_uri_hash_new ();
	// add attachment
	for (dnode = app->mix.children->children;  dnode;  dnode = dnode->next) {
		if ((http = ug_info_get (&dnode->data->info, UgetHttpInfo)) == NULL)
			continue;
		if (http->cookie_file)
			uget_uri_hash_add (hash, http->cookie_file);
		if (http->post_file)
			uget_uri_hash_add (hash, http->post_file);
	}

	gstr = g_string_new (app->config_dir);
	g_string_append_c (gstr, G_DIR_SEPARATOR);
	g_string_append (gstr, "attachment");

	gdir = g_dir_open (gstr->str, 0, NULL);
	if (gdir == NULL)
		ug_create_dir_all (gstr->str, -1);
	else {
		g_string_append_c (gstr, G_DIR_SEPARATOR);
		folder_len = gstr->len;

		while ((name = g_dir_read_name (gdir)) != NULL) {
			g_string_append (gstr, name);
			if (uget_uri_hash_find (hash, gstr->str) == FALSE)
				ug_unlink (gstr->str);
			g_string_truncate (gstr, folder_len);
		}
		g_dir_close (gdir);
	}

	g_string_free (gstr, TRUE);
	uget_uri_hash_free (hash);
}

void  ugtk_app_backup_attachment (UgtkApp* app, UgetNode* dnode)
{
	UgetHttp*  http;
	GString*   gstr;
	int        base_len;

	http = ug_info_get (&dnode->info, UgetHttpInfo);
	if (http == NULL && (http->cookie_file == NULL || http->post_file == NULL))
		return;

	gstr = g_string_new (app->config_dir);
	g_string_append_c (gstr, G_DIR_SEPARATOR);
	g_string_append (gstr, "attachment" G_DIR_SEPARATOR_S);
	g_string_append_printf (gstr, "%X", (unsigned int) g_random_int ());
	base_len = gstr->len;

	if (http->cookie_file) {
		g_string_append (gstr, ".cookie");
		ugtk_copy_file (http->cookie_file, gstr->str);
		ug_free (http->cookie_file);
		http->cookie_file = ug_strdup (gstr->str);
		g_string_truncate (gstr, base_len);
	}
	if (http->post_file) {
		g_string_append (gstr, ".post");
		ugtk_copy_file (http->post_file, gstr->str);
		ug_free (http->post_file);
		http->post_file = ug_strdup (gstr->str);
	}

	g_string_free (gstr, FALSE);
}

// ------------------------------------
// others

static void	on_message_response (GtkWidget* dialog, gint response, GtkWidget** value)
{
	gtk_widget_destroy (dialog);
	*value = NULL;
}

void  ugtk_app_show_message (UgtkApp* app, GtkMessageType type,
                             const gchar* message)
{
	GtkWidget*		dialog;
	GtkWidget**		value;
	gchar*			title;

	dialog = gtk_message_dialog_new (app->window.self,
			GTK_DIALOG_DESTROY_WITH_PARENT,
			type, GTK_BUTTONS_OK,
			"%s", message);
	// set title
	switch (type) {
	case GTK_MESSAGE_ERROR:
		if (app->dialogs.error)
			gtk_widget_destroy (app->dialogs.error);
		app->dialogs.error = dialog;
		value = &app->dialogs.error;
		title = g_strconcat (UGTK_APP_NAME " - ", _("Error"), NULL);
		break;

	default:
		if (app->dialogs.message)
			gtk_widget_destroy (app->dialogs.message);
		app->dialogs.message = dialog;
		value = &app->dialogs.message;
		title = g_strconcat (UGTK_APP_NAME " - ", _("Message"), NULL);
		break;
	}
	gtk_window_set_title ((GtkWindow*) dialog, title);
	g_free (title);
	// signal handler
	g_signal_connect (dialog, "response",
			G_CALLBACK (on_message_response), value);
	gtk_widget_show (dialog);
}

// -------------------------------------------------------
// UgtkClipboard

void  ugtk_clipboard_init (struct UgtkClipboard* clipboard, const gchar* pattern)
{
	clipboard->self  = gtk_clipboard_get (GDK_SELECTION_CLIPBOARD);
	clipboard->text  = NULL;
	clipboard->regex = g_regex_new (pattern, G_REGEX_CASELESS, 0, NULL);
}

void  ugtk_clipboard_set_pattern (struct UgtkClipboard* clipboard, const gchar* pattern)
{
	if (clipboard->regex)
		g_regex_unref (clipboard->regex);
	clipboard->regex = g_regex_new (pattern, G_REGEX_CASELESS, 0, NULL);
}

void  ugtk_clipboard_set_text (struct UgtkClipboard* clipboard, gchar* text)
{
	g_free (clipboard->text);
	clipboard->text = text;
	gtk_clipboard_set_text (clipboard->self, text, -1);
}

GList* ugtk_clipboard_get_uris (struct UgtkClipboard* clipboard)
{
	GList*		list;
	gchar*		text;

	if (gtk_clipboard_wait_is_text_available (clipboard->self) == FALSE)
		return NULL;
	text = gtk_clipboard_wait_for_text (clipboard->self);
	if (text == NULL)
		return NULL;
	// get URIs that scheme is not "file" from text
	list = ugtk_text_get_uris (text);
	list = ugtk_uri_list_remove_scheme (list, "file");
	g_free (text);

	return list;
}

GList* ugtk_clipboard_get_matched (struct UgtkClipboard* clipboard, const gchar* text)
{
	GList*		link;
	GList*		list;
	gchar*		temp;

	if (text == NULL) {
		g_free (clipboard->text);
		clipboard->text = NULL;
		return NULL;
	}
	// compare
	temp = (clipboard->text) ? clipboard->text : "";
	if (g_ascii_strcasecmp (text, temp) == 0)
		return NULL;
	// replace text
	g_free (clipboard->text);
	clipboard->text = g_strdup (text);
	// get and filter list
	list = ugtk_text_get_uris (text);
	list = ugtk_uri_list_remove_scheme (list, "file");
	// filter by filename extension
	for (link = list;  link;  link = link->next) {
		temp = ug_filename_from_uri (link->data);
		// get filename extension
		if (temp)
			text = strrchr (temp, '.');
		else
			text = NULL;
		// free URIs if not matched
		if (text == NULL || g_regex_match (clipboard->regex, text+1, 0, NULL) == FALSE) {
			g_free (link->data);
			link->data = NULL;
		}
		ug_free (temp);
	}
	list = g_list_remove_all (list, NULL);
	return list;
}

// -------------------------------------------------------
// UgtkStatusbar

void  ugtk_statusbar_set_info (struct UgtkStatusbar* statusbar, gint n_selected)
{
	static guint	context_id = 0;
	gchar*			string;

	if (context_id == 0)
		context_id = gtk_statusbar_get_context_id (statusbar->self, "selected");
	gtk_statusbar_pop  (statusbar->self, context_id);

	if (n_selected > 0) {
		string = g_strdup_printf (_("Selected %d items"), n_selected);
		gtk_statusbar_push (statusbar->self, context_id, string);
		g_free (string);
	}
}

void  ugtk_statusbar_set_speed (struct UgtkStatusbar* statusbar, gint64 down_speed, gint64 up_speed)
{
	char*		string;

	string = ug_str_from_int_unit (down_speed, "/s");
	gtk_label_set_text (statusbar->down_speed, string);
	ug_free (string);

	string = ug_str_from_int_unit (up_speed, "/s");
	gtk_label_set_text (statusbar->up_speed, string);
	ug_free (string);
}

