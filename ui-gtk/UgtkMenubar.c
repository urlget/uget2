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

#if defined _WIN32 || defined _WIN64
#include <windows.h>
#endif

#include <UgtkMenubar.h>
#include <UgtkSettingDialog.h>
#include <UgtkConfirmDialog.h>
#include <UgtkAboutDialog.h>
#include <UgtkConfig.h>
#include <UgtkUtil.h>
#include <UgtkApp.h>

#include <glib/gi18n.h>

// ----------------------------------------------------------------------------
// UgtkFileMenu
static void	on_create_download (GtkWidget* widget, UgtkApp* app)
{
	ugtk_app_create_download (app, NULL, NULL);
}

static void	on_offline_mode (GtkWidget* widget, UgtkApp* app)
{
	UgetNode* cnode;

	app->setting.offline_mode = gtk_check_menu_item_get_active (
			(GtkCheckMenuItem*) app->menubar.file.offline_mode);

	if (app->setting.offline_mode) {
		for (cnode = app->real.children;  cnode;  cnode = cnode->next)
			uget_app_stop_category ((UgetApp*)app, cnode);
	}
}

// ----------------------------------------------------------------------------
// UgtkEditMenu
static void on_clipboard_monitor (GtkWidget* widget, UgtkApp* app)
{
	gboolean	active;

	active = gtk_check_menu_item_get_active ((GtkCheckMenuItem*) widget);
	app->setting.clipboard.monitor = active;
}

static void on_clipboard_quiet (GtkWidget* widget, UgtkApp* app)
{
	gboolean	active;

	active = gtk_check_menu_item_get_active ((GtkCheckMenuItem*) widget);
	app->setting.clipboard.quiet = active;
}

static void on_commandline_quiet (GtkWidget* widget, UgtkApp* app)
{
	gboolean	active;

	active = gtk_check_menu_item_get_active ((GtkCheckMenuItem*) widget);
	app->setting.commandline.quiet = active;
}

static void on_skip_existing (GtkWidget* widget, UgtkApp* app)
{
	gboolean	active;

	active = gtk_check_menu_item_get_active ((GtkCheckMenuItem*) widget);
	app->setting.ui.skip_existing = active;
}

static void on_config_completion (GtkWidget* widget, UgtkApp* app)
{
	if (widget == app->menubar.edit.completion.disable)
		app->setting.completion.action = 0;
	else if (widget == app->menubar.edit.completion.hibernate)
		app->setting.completion.action = 1;
	else if (widget == app->menubar.edit.completion.suspend)
		app->setting.completion.action = 2;
	else if (widget == app->menubar.edit.completion.shutdown)
		app->setting.completion.action = 3;
	else if (widget == app->menubar.edit.completion.reboot)
		app->setting.completion.action = 4;
	else if (widget == app->menubar.edit.completion.custom)
		app->setting.completion.action = 5;
}

static void on_config_completion_help (GtkWidget* widget, UgtkApp* app)
{
	ugtk_launch_uri ("http://ugetdm.com/documentation/on-complete");
}

static void	on_config_settings (GtkWidget* widget, UgtkApp* app)
{
	UgtkSettingDialog*  sdialog;
	gchar*              title;

	if (app->dialogs.setting) {
		gtk_window_present ((GtkWindow*) app->dialogs.setting);
		return;
	}
	title = g_strconcat (UGTK_APP_NAME " - ", _("Settings"), NULL);
	sdialog = ugtk_setting_dialog_new (title, app->window.self);
	g_free (title);
	ugtk_setting_dialog_set (sdialog, &app->setting);
	app->dialogs.setting = (GtkWidget*) sdialog->self;
	// set page
//	ugtk_setting_dialog_set_page (sdialog, UGTK_SETTING_PAGE_UI);
	// show settings dialog
	ugtk_setting_dialog_run (sdialog, app);
}

// ----------------------------------------------------------------------------
// UgtkViewMenu
//
static void  on_change_visible_widget (GtkWidget* widget, UgtkApp* app)
{
	struct UgtkWindowSetting* setting;
	GtkWidget* temp;
	gboolean   visible;

	setting = &app->setting.window;
	visible = gtk_check_menu_item_get_active ((GtkCheckMenuItem*) widget);
	// Toolbar
	if (widget == app->menubar.view.toolbar) {
		setting->toolbar = visible;
		gtk_widget_set_visible (app->toolbar.self, visible);
		return;
	}
	// Statusbar
	if (widget == app->menubar.view.statusbar) {
		setting->statusbar = visible;
		gtk_widget_set_visible ((GtkWidget*) app->statusbar.self, visible);
		return;
	}
	// Category
	if (widget == app->menubar.view.category) {
		setting->category = visible;
		temp = gtk_paned_get_child1 (app->window.hpaned);
		gtk_widget_set_visible (temp, visible);
		return;
	}
	// Summary
	if (widget == app->menubar.view.summary) {
		setting->summary = visible;
		gtk_widget_set_visible (app->summary.self, visible);
		return;
	}
}

static void  on_change_visible_summary (GtkWidget* widget, UgtkApp* app)
{
	struct UgtkSummarySetting*  setting;
	gboolean  visible;

	setting = &app->setting.summary;
	visible = gtk_check_menu_item_get_active ((GtkCheckMenuItem*) widget);
	// which widget
	if (widget == app->menubar.view.summary_items.name) {
		setting->name = visible;
		app->summary.visible.name = visible;
	}
	else if (widget == app->menubar.view.summary_items.folder) {
		setting->folder = visible;
		app->summary.visible.folder = visible;
	}
	else if (widget == app->menubar.view.summary_items.category) {
		setting->category = visible;
		app->summary.visible.category = visible;
	}
	else if (widget == app->menubar.view.summary_items.uri) {
		setting->uri = visible;
		app->summary.visible.uri = visible;
	}
	else if (widget == app->menubar.view.summary_items.message) {
		setting->message = visible;
		app->summary.visible.message = visible;
	}

	ugtk_summary_show (&app->summary, app->traveler.download.cursor.node);
}

static void  on_change_visible_column (GtkWidget* widget, UgtkApp* app)
{
	struct UgtkDownloadColumnSetting*  setting;
	UgtkTraveler*       traveler;
	GtkTreeViewColumn*  column;
	gboolean  visible;
	gint      column_index;

	setting  = &app->setting.download_column;
	traveler = &app->traveler;
	visible  = gtk_check_menu_item_get_active ((GtkCheckMenuItem*) widget);
	// which widget
	if (widget == app->menubar.view.columns.completed) {
		column_index = UGTK_NODE_COLUMN_COMPLETE;
		setting->completed = visible;
	}
	else if (widget == app->menubar.view.columns.total) {
		column_index = UGTK_NODE_COLUMN_SIZE;
		setting->total = visible;
	}
	else if (widget == app->menubar.view.columns.percent) {
		column_index = UGTK_NODE_COLUMN_PERCENT;
		setting->percent = visible;
	}
	else if (widget == app->menubar.view.columns.elapsed) {
		column_index = UGTK_NODE_COLUMN_ELAPSED;
		setting->elapsed = visible;
	}
	else if (widget == app->menubar.view.columns.left) {
		column_index = UGTK_NODE_COLUMN_LEFT;
		setting->left = visible;
	}
	else if (widget == app->menubar.view.columns.speed) {
		column_index = UGTK_NODE_COLUMN_SPEED;
		setting->speed = visible;
	}
	else if (widget == app->menubar.view.columns.upload_speed) {
		column_index = UGTK_NODE_COLUMN_UPLOAD_SPEED;
		setting->upload_speed = visible;
	}
	else if (widget == app->menubar.view.columns.uploaded) {
		column_index = UGTK_NODE_COLUMN_UPLOADED;
		setting->uploaded = visible;
	}
	else if (widget == app->menubar.view.columns.ratio) {
		column_index = UGTK_NODE_COLUMN_RATIO;
		setting->ratio = visible;
	}
	else if (widget == app->menubar.view.columns.retry) {
		column_index = UGTK_NODE_COLUMN_RETRY;
		setting->retry = visible;
	}
	else if (widget == app->menubar.view.columns.category) {
		column_index = UGTK_NODE_COLUMN_CATEGORY;
		setting->category = visible;
	}
	else if (widget == app->menubar.view.columns.uri) {
		column_index = UGTK_NODE_COLUMN_URI;
		setting->uri = visible;
	}
	else if (widget == app->menubar.view.columns.added_on) {
		column_index = UGTK_NODE_COLUMN_ADDED_ON;
		setting->added_on = visible;
	}
	else if (widget == app->menubar.view.columns.completed_on) {
		column_index = UGTK_NODE_COLUMN_COMPLETED_ON;
		setting->completed_on = visible;
	}
	else
		return;

	column = gtk_tree_view_get_column (traveler->download.view, column_index);
	gtk_tree_view_column_set_visible (column, visible);
}

// ----------------------------------------------------------------------------
// UgtkCategoryMenu

static void	on_move_category_up (GtkWidget* widget, UgtkApp* app)
{
	UgetNode* cnode;

	cnode = app->traveler.category.cursor.node;
	if (cnode == NULL || cnode->prev == NULL)
		return;

	uget_app_move_category ((UgetApp*) app, cnode, cnode->prev);
	ugtk_traveler_select_category (&app->traveler,
			app->traveler.category.cursor.pos -1, -1);
	gtk_widget_queue_draw ((GtkWidget*) app->traveler.category.view);
}

static void	on_move_category_down (GtkWidget* widget, UgtkApp* app)
{
	UgetNode* cnode;

	cnode = app->traveler.category.cursor.node;
	if (cnode == NULL || cnode->next == NULL)
		return;

	uget_app_move_category ((UgetApp*) app, cnode, cnode->next->next);
	ugtk_traveler_select_category (&app->traveler,
			app->traveler.category.cursor.pos +1, -1);
	gtk_widget_queue_draw ((GtkWidget*) app->traveler.category.view);
}

// ----------------------------------------------------------------------------
// UgtkDownloadMenu

static void	on_delete_download(GtkWidget* widget, UgtkApp* app)
{
	ugtk_app_delete_download (app, FALSE);
}

static void	on_delete_download_file (GtkWidget* widget, UgtkApp* app)
{
	UgtkConfirmDialog*  cdialog;

	if (app->setting.ui.delete_confirmation == FALSE)
		ugtk_app_delete_download (app, TRUE);
	else {
		// confirm to delete
		cdialog = ugtk_confirm_dialog_new(UGTK_CONFIRM_DIALOG_DELETE, app);
		ugtk_confirm_dialog_run (cdialog);
	}
}

static void	on_open_download_file (GtkWidget* widget, UgtkApp* app)
{
	UgetCommon*   common;
	UgetNode*     node;
	GtkWidget*    dialog;
	gchar*        string;

	node = app->traveler.download.cursor.node;
	if (node == NULL)
		return;
	node = node->data;
	common = ug_info_get (&node->info, UgetCommonInfo);
	if (common == NULL || common->folder == NULL || common->file == NULL)
		return;

	if (ugtk_launch_default_app (common->folder, common->file) == FALSE) {
		string = g_strdup_printf (_("Can't launch default application for file '%s'."), common->file);
		dialog = gtk_message_dialog_new (app->window.self,
				GTK_DIALOG_DESTROY_WITH_PARENT,
				GTK_MESSAGE_ERROR, GTK_BUTTONS_OK,
				"%s", string);
		g_free (string);
		string = g_strconcat (UGTK_APP_NAME " - ", _("Error"), NULL);
		gtk_window_set_title ((GtkWindow*) dialog, string);
		g_free (string);
		g_signal_connect (dialog, "response", G_CALLBACK (gtk_widget_destroy), NULL);
		gtk_widget_show ((GtkWidget*) dialog);
	}
}

static void	on_open_download_folder (GtkWidget* widget, UgtkApp* app)
{
	UgetCommon*  common;
	UgetNode*    node;
	GtkWidget*   dialog;
	gchar*       string;

	node = app->traveler.download.cursor.node;
	if (node == NULL)
		return;
	node = node->data;
	common = ug_info_get (&node->info, UgetCommonInfo);
	if (common == NULL || common->folder == NULL)
		return;

	string = g_filename_from_utf8 (common->folder, -1, NULL, NULL, NULL);
	if (g_file_test (string, G_FILE_TEST_EXISTS) == FALSE) {
		g_free (string);
		string = g_strdup_printf (_("'%s' - This folder does not exist."), common->folder);
		dialog = gtk_message_dialog_new (app->window.self,
				GTK_DIALOG_DESTROY_WITH_PARENT,
				GTK_MESSAGE_ERROR, GTK_BUTTONS_OK,
				"%s", string);
		g_free (string);
		string = g_strconcat (UGTK_APP_NAME " - ", _("Error"), NULL);
		gtk_window_set_title ((GtkWindow*) dialog, string);
		g_free (string);
		g_signal_connect (dialog, "response", G_CALLBACK (gtk_widget_destroy), NULL);
		gtk_widget_show ((GtkWidget*) dialog);
		return;
	}
	g_free (string);

#ifdef _WIN32
	{
		UgUri       uuri;
		gchar*		path;
		gchar*		argument;
		gunichar2*	argument_os;

		if (common->file == NULL && common->uri) {
			ug_uri_init (&uuri, common->uri);
			if (uuri.file == -1)
				argument = NULL;
			else {
				argument = g_strndup (uuri.uri + uuri.file,
				                      ug_uri_file (&uuri, NULL));
			}
			path = g_build_filename (common->folder, argument, NULL);
			g_free (argument);
		}
		else
			path = g_build_filename (common->folder, common->file, NULL);

		if (g_file_test (path, G_FILE_TEST_EXISTS))
			argument = g_strconcat ("/e,/select,\"", path, "\"", NULL);
		else
			argument = g_strconcat ("/e,\"", common->folder, "\"", NULL);
		g_free (path);
		argument_os = g_utf8_to_utf16 (argument, -1, NULL, NULL, NULL);
		g_free (argument);
		ShellExecuteW (NULL, NULL, L"explorer", argument_os, NULL, SW_SHOW);
		g_free (argument_os);
	}
#else
	{
		GError*	error = NULL;
		GFile*	gfile;
		gchar*	uri;

		gfile = g_file_new_for_path (common->folder);
		uri = g_file_get_uri (gfile);
		g_object_unref (gfile);
		g_app_info_launch_default_for_uri (uri, NULL, &error);
		g_free (uri);

		if (error)
			g_error_free (error);
	}
#endif
}

static void	on_set_download_force_start (GtkWidget* widget, UgtkApp* app)
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
		uget_app_activate_download ((UgetApp*) app, node->data);
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

// UgtkMenubar.download.move_to
static void	on_move_download (GtkWidget* widget, UgtkApp* app)
{
	GPtrArray*  array;
	UgetNode*   cnode;
	UgetNode*   dnode;
	GList*      list;
	GList*      link;
	guint       index;

	array = app->menubar.download.move_to.array;
	cnode = NULL;
	for (index = 0;  index < array->len;  index += 2) {
		if (widget == g_ptr_array_index (array, index)) {
			cnode = g_ptr_array_index (array, index + 1);
			break;
		}
	}

	if (cnode == NULL || cnode == app->traveler.category.cursor.node)
		return;

	// if current category is "All"
	if (app->traveler.category.cursor.pos == 0)
		ugtk_traveler_reserve_selection (&app->traveler);

	list = ugtk_traveler_get_selected (&app->traveler);
	for (link = list;  link;  link = link->next) {
		dnode = link->data;
		dnode = dnode->data;
		uget_app_move_download_to ((UgetApp*) app, dnode, cnode);
	}
	g_list_free (list);
	// refresh
	gtk_widget_queue_draw ((GtkWidget*) app->traveler.category.view);
	gtk_widget_queue_draw ((GtkWidget*) app->traveler.state.view);

	// if current category is "All"
	if (app->traveler.category.cursor.pos == 0)
		ugtk_traveler_restore_selection (&app->traveler);
}

// UgtkMenubar.download.prioriy
static void	on_set_download_prioriy (GtkWidget* widget, UgtkApp* app)
{
	UgetPriority   priority;
	UgetRelation*  relation;
	UgetNode*      node;
	GList*         list;
	GList*         link;

	if (widget == app->menubar.download.prioriy.high)
		priority = UGET_PRIORITY_HIGH;
	else if (widget == app->menubar.download.prioriy.normal)
		priority = UGET_PRIORITY_NORMAL;
	else
		priority = UGET_PRIORITY_LOW;

	list = ugtk_traveler_get_selected (&app->traveler);
	for (link = list;  link;  link = link->next) {
		node = link->data;
		node = node->data;
		relation = ug_info_realloc (&node->info, UgetRelationInfo);
		relation->task.priority = priority;
	}
	g_list_free (list);
}

// ----------------------------------------------------------------------------
// UgtkHelpMenu
//
static void  on_help_online (GtkWidget* widget, UgtkApp* app)
{
	ugtk_launch_uri ("http://ugetdm.com/help");
}

static void  on_documentation (GtkWidget* widget, UgtkApp* app)
{
	ugtk_launch_uri ("http://ugetdm.com/documentation");
}

static void  on_support_forum (GtkWidget* widget, UgtkApp* app)
{
	ugtk_launch_uri ("http://ugetdm.com/forum/");
}

static void  on_submit_feedback (GtkWidget* widget, UgtkApp* app)
{
	ugtk_launch_uri ("http://ugetdm.com/feedback");
}

static void  on_report_bug (GtkWidget* widget, UgtkApp* app)
{
	ugtk_launch_uri ("http://ugetdm.com/reportbug");
}

static void  on_keyboard_shortcuts (GtkWidget* widget, UgtkApp* app)
{
	ugtk_launch_uri ("http://ugetdm.com/keyboard-shortcuts");
}

static void  on_check_updates (GtkWidget* widget, UgtkApp* app)
{
	ugtk_launch_uri ("http://ugetdm.com/versioncheck?v=" PACKAGE_VERSION);
}

static void  on_about (GtkWidget* widget, UgtkApp* app)
{
	UgtkAboutDialog*  adialog;

	adialog = ugtk_about_dialog_new (app->window.self);
//	if (app->update_info.ready)
//		ugtk_about_dialog_set_info (adialog, app->update_info.text->str);
	ugtk_about_dialog_run (adialog);
}

// ----------------------------------------------------------------------------
// UgtkMenubar

void ugtk_menubar_init_callback (UgtkMenubar* menubar, UgtkApp* app)
{
	// ----------------------------------------------------
	// UgtkFileMenu
	g_signal_connect (menubar->file.create_download, "activate",
			G_CALLBACK (on_create_download), app);
	g_signal_connect_swapped (menubar->file.create_category, "activate",
			G_CALLBACK (ugtk_app_create_category), app);
	g_signal_connect_swapped (menubar->file.create_torrent, "activate",
			G_CALLBACK (ugtk_app_create_torrent), app);
	g_signal_connect_swapped (menubar->file.create_metalink, "activate",
			G_CALLBACK (ugtk_app_create_metalink), app);
	g_signal_connect_swapped (menubar->file.batch.clipboard, "activate",
			G_CALLBACK (ugtk_app_clipboard_batch), app);
	g_signal_connect_swapped (menubar->file.batch.sequence, "activate",
			G_CALLBACK (ugtk_app_sequence_batch), app);
	g_signal_connect_swapped (menubar->file.batch.text_import, "activate",
			G_CALLBACK (ugtk_app_import_text_file), app);
	g_signal_connect_swapped (menubar->file.batch.html_import, "activate",
			G_CALLBACK (ugtk_app_import_html_file), app);
	g_signal_connect_swapped (menubar->file.batch.text_export, "activate",
			G_CALLBACK (ugtk_app_export_text_file), app);
	g_signal_connect_swapped (menubar->file.open_category, "activate",
			G_CALLBACK (ugtk_app_load_category), app);
	g_signal_connect_swapped (menubar->file.save_category, "activate",
			G_CALLBACK (ugtk_app_save_category), app);
	g_signal_connect_swapped (menubar->file.save, "activate",
			G_CALLBACK (ugtk_app_save), app);
	g_signal_connect (menubar->file.offline_mode, "toggled",
			G_CALLBACK (on_offline_mode), app);
	g_signal_connect_swapped (menubar->file.quit, "activate",
			G_CALLBACK (ugtk_app_decide_to_quit), app);

	// ----------------------------------------------------
	// UgtkEditMenu
	g_signal_connect (menubar->edit.clipboard_monitor, "activate",
			G_CALLBACK (on_clipboard_monitor), app);
	g_signal_connect (menubar->edit.clipboard_quiet, "activate",
			G_CALLBACK (on_clipboard_quiet), app);
	g_signal_connect (menubar->edit.commandline_quiet, "activate",
			G_CALLBACK (on_commandline_quiet), app);
	g_signal_connect (menubar->edit.skip_existing, "activate",
			G_CALLBACK (on_skip_existing), app);
	g_signal_connect (menubar->edit.completion.disable, "activate",
			G_CALLBACK (on_config_completion), app);
	g_signal_connect (menubar->edit.completion.hibernate, "activate",
			G_CALLBACK (on_config_completion), app);
	g_signal_connect (menubar->edit.completion.suspend, "activate",
			G_CALLBACK (on_config_completion), app);
	g_signal_connect (menubar->edit.completion.shutdown, "activate",
			G_CALLBACK (on_config_completion), app);
	g_signal_connect (menubar->edit.completion.reboot, "activate",
			G_CALLBACK (on_config_completion), app);
	g_signal_connect (menubar->edit.completion.custom, "activate",
			G_CALLBACK (on_config_completion), app);
	g_signal_connect (menubar->edit.completion.help, "activate",
			G_CALLBACK (on_config_completion_help), app);
	g_signal_connect (menubar->edit.settings, "activate",
			G_CALLBACK (on_config_settings), app);

	// ----------------------------------------------------
	// UgtkViewMenu
	g_signal_connect (menubar->view.toolbar, "toggled",
			G_CALLBACK (on_change_visible_widget), app);
	g_signal_connect (menubar->view.statusbar, "toggled",
			G_CALLBACK (on_change_visible_widget), app);
	g_signal_connect (menubar->view.category, "toggled",
			G_CALLBACK (on_change_visible_widget), app);
	g_signal_connect (menubar->view.summary, "toggled",
			G_CALLBACK (on_change_visible_widget), app);
	// summary items
	g_signal_connect (menubar->view.summary_items.name, "toggled",
			G_CALLBACK (on_change_visible_summary), app);
	g_signal_connect (menubar->view.summary_items.folder, "toggled",
			G_CALLBACK (on_change_visible_summary), app);
	g_signal_connect (menubar->view.summary_items.category, "toggled",
			G_CALLBACK (on_change_visible_summary), app);
	g_signal_connect (menubar->view.summary_items.uri, "toggled",
			G_CALLBACK (on_change_visible_summary), app);
	g_signal_connect (menubar->view.summary_items.message, "toggled",
			G_CALLBACK (on_change_visible_summary), app);
	// download columns
	g_signal_connect (menubar->view.columns.completed, "toggled",
			G_CALLBACK (on_change_visible_column), app);
	g_signal_connect (menubar->view.columns.total, "toggled",
			G_CALLBACK (on_change_visible_column), app);
	g_signal_connect (menubar->view.columns.percent, "toggled",
			G_CALLBACK (on_change_visible_column), app);
	g_signal_connect (menubar->view.columns.elapsed, "toggled",
			G_CALLBACK (on_change_visible_column), app);
	g_signal_connect (menubar->view.columns.left, "toggled",
			G_CALLBACK (on_change_visible_column), app);
	g_signal_connect (menubar->view.columns.speed, "toggled",
			G_CALLBACK (on_change_visible_column), app);
	g_signal_connect (menubar->view.columns.upload_speed, "toggled",
			G_CALLBACK (on_change_visible_column), app);
	g_signal_connect (menubar->view.columns.uploaded, "toggled",
			G_CALLBACK (on_change_visible_column), app);
	g_signal_connect (menubar->view.columns.ratio, "toggled",
			G_CALLBACK (on_change_visible_column), app);
	g_signal_connect (menubar->view.columns.retry, "toggled",
			G_CALLBACK (on_change_visible_column), app);
	g_signal_connect (menubar->view.columns.category, "toggled",
			G_CALLBACK (on_change_visible_column), app);
	g_signal_connect (menubar->view.columns.uri, "toggled",
			G_CALLBACK (on_change_visible_column), app);
	g_signal_connect (menubar->view.columns.added_on, "toggled",
			G_CALLBACK (on_change_visible_column), app);
	g_signal_connect (menubar->view.columns.completed_on, "toggled",
			G_CALLBACK (on_change_visible_column), app);

	// ----------------------------------------------------
	// UgtkCategoryMenu
	g_signal_connect_swapped (menubar->category.create, "activate",
			G_CALLBACK (ugtk_app_create_category), app);
	g_signal_connect_swapped (menubar->category.delete, "activate",
			G_CALLBACK (ugtk_app_delete_category), app);
	g_signal_connect_swapped (menubar->category.properties, "activate",
			G_CALLBACK (ugtk_app_edit_category), app);
	g_signal_connect (menubar->category.move_up, "activate",
			G_CALLBACK (on_move_category_up), app);
	g_signal_connect (menubar->category.move_down, "activate",
			G_CALLBACK (on_move_category_down), app);

	// ----------------------------------------------------
	// UgtkDownloadMenu
	g_signal_connect (menubar->download.create, "activate",
			G_CALLBACK (on_create_download), app);
	g_signal_connect (menubar->download.delete, "activate",
			G_CALLBACK (on_delete_download), app);
	// file & folder
	g_signal_connect (menubar->download.delete_file, "activate",
			G_CALLBACK (on_delete_download_file), app);
	g_signal_connect (menubar->download.open, "activate",
			G_CALLBACK (on_open_download_file), app);
	g_signal_connect (menubar->download.open_folder, "activate",
			G_CALLBACK (on_open_download_folder), app);
	// change status
	g_signal_connect (menubar->download.force_start, "activate",
			G_CALLBACK (on_set_download_force_start), app);
	g_signal_connect_swapped (menubar->download.runnable, "activate",
			G_CALLBACK (ugtk_app_queue_download), app);
	g_signal_connect_swapped (menubar->download.pause, "activate",
			G_CALLBACK (ugtk_app_pause_download), app);
	// move
	g_signal_connect_swapped (menubar->download.move_up, "activate",
			G_CALLBACK (ugtk_app_move_download_up), app);
	g_signal_connect_swapped (menubar->download.move_down, "activate",
			G_CALLBACK (ugtk_app_move_download_down), app);
	g_signal_connect_swapped (menubar->download.move_top, "activate",
			G_CALLBACK (ugtk_app_move_download_top), app);
	g_signal_connect_swapped (menubar->download.move_bottom, "activate",
			G_CALLBACK (ugtk_app_move_download_bottom), app);
	// prioriy
	g_signal_connect (menubar->download.prioriy.high, "activate",
			G_CALLBACK (on_set_download_prioriy), app);
	g_signal_connect (menubar->download.prioriy.normal, "activate",
			G_CALLBACK (on_set_download_prioriy), app);
	g_signal_connect (menubar->download.prioriy.low, "activate",
			G_CALLBACK (on_set_download_prioriy), app);
	// properties
	g_signal_connect_swapped (menubar->download.properties, "activate",
			G_CALLBACK (ugtk_app_edit_download), app);

	// ----------------------------------------------------
	// UgtkHelpMenu
	g_signal_connect (menubar->help.help_online, "activate",
			G_CALLBACK (on_help_online), app);
	g_signal_connect (menubar->help.documentation, "activate",
			G_CALLBACK (on_documentation), app);
	g_signal_connect (menubar->help.support_forum, "activate",
			G_CALLBACK (on_support_forum), app);
	g_signal_connect (menubar->help.submit_feedback, "activate",
			G_CALLBACK (on_submit_feedback), app);
	g_signal_connect (menubar->help.report_bug, "activate",
			G_CALLBACK (on_report_bug), app);
	g_signal_connect (menubar->help.keyboard_shortcuts, "activate",
			G_CALLBACK (on_keyboard_shortcuts), app);
	g_signal_connect (menubar->help.check_updates, "activate",
			G_CALLBACK (on_check_updates), app);
	g_signal_connect (menubar->help.about_uget, "activate",
			G_CALLBACK (on_about), app);
}

void  ugtk_menubar_sync_category (UgtkMenubar* menubar, UgtkApp* app, gboolean reset)
{
	UgetNode*     cnode;
	GtkWidget*    menu_item;
	GtkWidget*    image;
	GPtrArray*    array;
	guint         index;

	array = menubar->download.move_to.array;

	if (reset) {
		// remove all item
		for (index = 0;  index < array->len;  index += 2) {
			menu_item = g_ptr_array_index (array, index);
			gtk_container_remove (
					(GtkContainer*) menubar->download.move_to.self, menu_item);
		}
		g_ptr_array_set_size (array, 0);
		// add new item
		for (cnode = app->real.children;  cnode;  cnode = cnode->next) {
			// create menu item
			menu_item = gtk_image_menu_item_new_with_label (cnode->name);
			image = gtk_image_new_from_stock (GTK_STOCK_DND_MULTIPLE,
			                                  GTK_ICON_SIZE_MENU);
			gtk_image_menu_item_set_image ((GtkImageMenuItem*) menu_item,
			                               image);
			gtk_menu_shell_append (
					GTK_MENU_SHELL (app->menubar.download.move_to.self),
					menu_item);
			g_signal_connect (menu_item, "activate",
					G_CALLBACK (on_move_download), app);
			gtk_widget_show (menu_item);
			g_ptr_array_add (array, menu_item);
			g_ptr_array_add (array, cnode);
		}
	}

	// set sensitive
	for (index = 0;  index < array->len;  index += 2) {
		menu_item = g_ptr_array_index (array, index);
		cnode     = g_ptr_array_index (array, index +1);
		if (cnode == app->traveler.category.cursor.node)
			gtk_widget_set_sensitive (menu_item, FALSE);
		else
			gtk_widget_set_sensitive (menu_item, TRUE);
	}
}

