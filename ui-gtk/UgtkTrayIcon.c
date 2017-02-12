/*
 *
 *   Copyright (C) 2005-2017 by C.H. Huang
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

#include <UgString.h>
#include <UgtkTrayIcon.h>
#include <UgtkAboutDialog.h>
#include <UgtkApp.h>

#include <glib/gi18n.h>

#define UGTK_TRAY_ICON_NAME         "uget-tray-default"
#define UGTK_TRAY_ICON_ERROR_NAME   "uget-tray-error"
#define UGTK_TRAY_ICON_ACTIVE_NAME  "uget-tray-downloading"

void ugtk_tray_icon_init (UgtkTrayIcon* trayicon)
{
	GtkWidget*  image;
	GtkWidget*  menu;
	GtkWidget*  menu_item;
	gchar*      icon_name;
	gchar*      file_name;

	// UgTrayIcon.menu
	menu = gtk_menu_new ();
	// New Download
	menu_item = gtk_image_menu_item_new_with_mnemonic (_("New _Download..."));
#if GTK_MAJOR_VERSION >= 3 && GTK_MINOR_VERSION >= 10
	image = gtk_image_new_from_icon_name ("document-new", GTK_ICON_SIZE_MENU);
#else
	image = gtk_image_new_from_stock (GTK_STOCK_NEW, GTK_ICON_SIZE_MENU);
#endif
	gtk_image_menu_item_set_image ((GtkImageMenuItem*)menu_item, image);
	gtk_menu_shell_append ((GtkMenuShell*)menu, menu_item);
	trayicon->menu.create_download = menu_item;

	// New Clipboard batch
	menu_item = gtk_image_menu_item_new_with_mnemonic (_("New Clipboard _batch..."));
#if GTK_MAJOR_VERSION >= 3 && GTK_MINOR_VERSION >= 10
	image = gtk_image_new_from_icon_name ("edit-paste", GTK_ICON_SIZE_MENU);
#else
	image = gtk_image_new_from_stock (GTK_STOCK_PASTE, GTK_ICON_SIZE_MENU);
#endif
	gtk_image_menu_item_set_image ((GtkImageMenuItem*)menu_item, image);
	gtk_menu_shell_append ((GtkMenuShell*)menu, menu_item);
	trayicon->menu.create_clipboard = menu_item;

	gtk_menu_shell_append ((GtkMenuShell*)menu, gtk_separator_menu_item_new() );

	// New Torrent
	menu_item = gtk_image_menu_item_new_with_mnemonic (_("New Torrent..."));
//	image = gtk_image_new_from_stock (GTK_STOCK_NEW, GTK_ICON_SIZE_MENU);
//	gtk_image_menu_item_set_image ((GtkImageMenuItem*)menu_item, image);
	gtk_menu_shell_append ((GtkMenuShell*)menu, menu_item);
	trayicon->menu.create_torrent = menu_item;

	// New Metalink
	menu_item = gtk_image_menu_item_new_with_mnemonic (_("New Metalink..."));
//	image = gtk_image_new_from_stock (GTK_STOCK_NEW, GTK_ICON_SIZE_MENU);
//	gtk_image_menu_item_set_image ((GtkImageMenuItem*)menu_item, image);
	gtk_menu_shell_append ((GtkMenuShell*)menu, menu_item);
	trayicon->menu.create_metalink = menu_item;

	gtk_menu_shell_append ((GtkMenuShell*)menu, gtk_separator_menu_item_new() );

	// Settings shortcut
	menu_item = gtk_check_menu_item_new_with_mnemonic (_("Clipboard _Monitor"));
	gtk_menu_shell_append ((GtkMenuShell*)menu, menu_item);
	trayicon->menu.clipboard_monitor = menu_item;

	menu_item = gtk_check_menu_item_new_with_mnemonic (_("Clipboard works quietly"));
	gtk_menu_shell_append ((GtkMenuShell*)menu, menu_item);
	trayicon->menu.clipboard_quiet = menu_item;

	menu_item = gtk_check_menu_item_new_with_mnemonic (_("Command-line works quietly"));
	gtk_menu_shell_append ((GtkMenuShell*)menu, menu_item);
	trayicon->menu.commandline_quiet = menu_item;

	menu_item = gtk_check_menu_item_new_with_mnemonic (_("Skip existing URI"));
	gtk_menu_shell_append ((GtkMenuShell*)menu, menu_item);
	trayicon->menu.skip_existing = menu_item;

	menu_item = gtk_check_menu_item_new_with_mnemonic (_("Apply recent download settings"));
	gtk_menu_shell_append ((GtkMenuShell*)menu, menu_item);
	trayicon->menu.apply_recent = menu_item;

	gtk_menu_shell_append ((GtkMenuShell*)menu, gtk_separator_menu_item_new() );

	// Settings
	menu_item = gtk_image_menu_item_new_with_mnemonic (_("_Settings..."));
#if GTK_MAJOR_VERSION >= 3 && GTK_MINOR_VERSION >= 10
	image = gtk_image_new_from_icon_name ("document-properties", GTK_ICON_SIZE_MENU);
#else
	image = gtk_image_new_from_stock (GTK_STOCK_PROPERTIES, GTK_ICON_SIZE_MENU);
#endif
	gtk_image_menu_item_set_image ((GtkImageMenuItem*)menu_item, image);
	gtk_menu_shell_append ((GtkMenuShell*)menu, menu_item);
	trayicon->menu.settings = menu_item;

	// About
	menu_item = gtk_image_menu_item_new_from_stock (GTK_STOCK_ABOUT, NULL);
	gtk_menu_shell_append ((GtkMenuShell*)menu, menu_item);
	trayicon->menu.about = menu_item;

	gtk_menu_shell_append ((GtkMenuShell*)menu, gtk_separator_menu_item_new() );

#ifdef HAVE_APP_INDICATOR
	// Show window
	menu_item = gtk_menu_item_new_with_mnemonic (_("Show window"));
	gtk_menu_shell_append ((GtkMenuShell*)menu, menu_item);
	trayicon->menu.show_window = menu_item;
#endif

	// Offline mode
	menu_item = gtk_check_menu_item_new_with_mnemonic (_("_Offline Mode"));
	gtk_menu_shell_append ((GtkMenuShell*)menu, menu_item);
	trayicon->menu.offline_mode = menu_item;

	// Quit
	menu_item = gtk_image_menu_item_new_from_stock (GTK_STOCK_QUIT, NULL);
	gtk_menu_shell_append ((GtkMenuShell*)menu, menu_item);
	trayicon->menu.quit = menu_item;

	gtk_widget_show_all (menu);
	trayicon->menu.self = menu;

	// decide tray icon
	file_name = g_build_filename (UG_DATADIR, "icons",
	                         "hicolor", "16x16", "apps",
	                         "uget-icon.png", NULL);
	if (g_file_test (file_name, G_FILE_TEST_IS_REGULAR))
		icon_name = UGTK_TRAY_ICON_NAME;
	else
		icon_name = GTK_STOCK_GO_DOWN;
	g_free (file_name);
#ifdef HAVE_APP_INDICATOR
	trayicon->indicator = app_indicator_new ("uget-gtk", icon_name,
			APP_INDICATOR_CATEGORY_APPLICATION_STATUS);
	trayicon->indicator_temp = trayicon->indicator;
	if (trayicon->indicator) {
		app_indicator_set_menu (trayicon->indicator, GTK_MENU (trayicon->menu.self));
//		app_indicator_set_attention_icon_full (trayicon->indicator,
//				UGTK_TRAY_ICON_ACTIVE_NAME, NULL);
		app_indicator_set_attention_icon (trayicon->indicator,
				UGTK_TRAY_ICON_ACTIVE_NAME);
		app_indicator_set_status (trayicon->indicator,
				APP_INDICATOR_STATUS_PASSIVE);
	}
#endif	// HAVE_APP_INDICATOR
	trayicon->self = gtk_status_icon_new_from_icon_name (icon_name);
	gtk_status_icon_set_visible (trayicon->self, FALSE);
}

void ugtk_tray_icon_set_info (UgtkTrayIcon* trayicon, guint n_active, gint64 down_speed, gint64 up_speed)
{
	gchar*  string;
	char*   string_down_speed;
	char*   string_up_speed;
	guint   current_state;

	// change tray icon
	if (trayicon->error_occurred) {
		string = UGTK_TRAY_ICON_ERROR_NAME;
		current_state = UGTK_TRAY_ICON_STATE_ERROR;
	}
	else if (n_active > 0) {
		string = UGTK_TRAY_ICON_ACTIVE_NAME;
		current_state = UGTK_TRAY_ICON_STATE_RUNNING;
	}
	else {
		string = UGTK_TRAY_ICON_NAME;
		current_state = UGTK_TRAY_ICON_STATE_NORMAL;
	}

	if (trayicon->state != current_state) {
		trayicon->state  = current_state;
#ifdef HAVE_APP_INDICATOR
		if (trayicon->indicator) {
			trayicon->error_occurred = FALSE;
			if (app_indicator_get_status (trayicon->indicator) != APP_INDICATOR_STATUS_PASSIVE) {
				if (current_state == UGTK_TRAY_ICON_STATE_NORMAL) {
					app_indicator_set_status (trayicon->indicator,
							APP_INDICATOR_STATUS_ACTIVE);
				}
				else {
					app_indicator_set_attention_icon (trayicon->indicator, string);
	//				app_indicator_set_attention_icon_full (trayicon->indicator,
	//						string, "attention");
					app_indicator_set_status (trayicon->indicator,
							APP_INDICATOR_STATUS_ATTENTION);
				}
			}
		}
		else
#endif	// HAVE_APP_INDICATOR
		gtk_status_icon_set_from_icon_name (trayicon->self, string);
	}

	// change tooltip
	string_down_speed = ug_str_from_int_unit (down_speed, "/s");
	string_up_speed   = ug_str_from_int_unit (up_speed, "/s");
	string = g_strdup_printf (
			UGTK_APP_NAME " " PACKAGE_VERSION "\n"
			"%u %s" "\n"
			"\xE2\x86\x93 %s"     // "↓"
			" , "
			"\xE2\x86\x91 %s",    // "↑"
			n_active, _("tasks"),
			string_down_speed,
			string_up_speed);
#ifdef HAVE_APP_INDICATOR
	if (trayicon->indicator) {
//		g_object_set (trayicon->indicator, "icon-desc", string, NULL);
//		g_object_set (trayicon->indicator, "attention-icon-desc", string, NULL);
	}
	else
#endif	// HAVE_APP_INDICATOR
	gtk_status_icon_set_tooltip_text (trayicon->self, string);

	ug_free (string_down_speed);
	ug_free (string_up_speed);
	g_free (string);
}

void  ugtk_tray_icon_set_visible (UgtkTrayIcon* trayicon, gboolean visible)
{
#ifdef HAVE_APP_INDICATOR
	if (trayicon->indicator) {
		if (visible)
			app_indicator_set_status (trayicon->indicator,
					APP_INDICATOR_STATUS_ACTIVE);
		else
			app_indicator_set_status (trayicon->indicator,
					APP_INDICATOR_STATUS_PASSIVE);
	}
	else
#endif	// HAVE_APP_INDICATOR
	gtk_status_icon_set_visible (trayicon->self, visible);
	trayicon->visible = visible;
}

#ifdef HAVE_APP_INDICATOR

void  ugtk_tray_icon_use_indicator (UgtkTrayIcon* trayicon, gboolean enable)
{
	ugtk_tray_icon_set_visible (trayicon, FALSE);
	if (enable)
		trayicon->indicator = trayicon->indicator_temp;
	else
		trayicon->indicator = NULL;
	ugtk_tray_icon_set_visible (trayicon, trayicon->visible);
}

#endif // HAVE_APP_INDICATOR

// ----------------------------------------------------------------------------
// Callback

static void	on_trayicon_activate (GtkStatusIcon* status_icon, UgtkApp* app)
{
	gint  x, y;

	if (gtk_widget_get_visible ((GtkWidget*) app->window.self) == TRUE) {
		// get position and size
		gtk_window_get_position (app->window.self, &x, &y);
		gtk_window_get_size (app->window.self,
				&app->setting.window.width, &app->setting.window.height);
		// gtk_window_get_position() may return: x == -32000, y == -32000
		if (x + app->setting.window.width > 0)
			app->setting.window.x = x;
		if (y + app->setting.window.height > 0)
			app->setting.window.y = y;
		// hide window
#if defined _WIN32 || defined _WIN64
//		gtk_window_iconify (app->window.self);
#endif
		gtk_widget_hide ((GtkWidget*) app->window.self);
	}
	else {
#if defined _WIN32 || defined _WIN64
//		gtk_window_deiconify (app->window.self);
#endif
		gtk_widget_show ((GtkWidget*) app->window.self);
		gtk_window_present (app->window.self);
		ugtk_app_decide_trayicon_visible (app);
		// set position and size
		gtk_window_move (app->window.self,
				app->setting.window.x, app->setting.window.y);
		gtk_window_resize (app->window.self,
				app->setting.window.width, app->setting.window.height);
	}
	// clear error status
	if (app->trayicon.error_occurred) {
		app->trayicon.error_occurred = FALSE;
		app->trayicon.state = UGTK_TRAY_ICON_STATE_NORMAL;
		gtk_status_icon_set_from_icon_name (status_icon, UGTK_TRAY_ICON_NAME);
	}
}

static void	on_trayicon_popup_menu (GtkStatusIcon* status_icon, guint button,
                                    guint activate_time, UgtkTrayIcon* trayicon)
{
	gtk_menu_set_screen ((GtkMenu*) trayicon->menu.self,
			gtk_status_icon_get_screen (status_icon));
#if defined _WIN32 || defined _WIN64
	gtk_menu_popup ((GtkMenu*) trayicon->menu.self,
			NULL, NULL,
			NULL, NULL,
			button, activate_time);
#else
	gtk_menu_popup ((GtkMenu*) trayicon->menu.self,
			NULL, NULL,
			gtk_status_icon_position_menu, status_icon,
			button, activate_time);
#endif
}

#if defined _WIN32 || defined _WIN64
static gboolean	tray_menu_timeout (GtkMenu* menu)
{
	gtk_menu_popdown (menu);
	// return FALSE if the source should be removed.
	return FALSE;
}

static gboolean	tray_menu_leave_enter (GtkWidget* menu, GdkEventCrossing* event, gpointer data)
{
	static guint	tray_menu_timer = 0;

	if (event->type == GDK_LEAVE_NOTIFY &&
		(event->detail == GDK_NOTIFY_ANCESTOR || event->detail == GDK_NOTIFY_UNKNOWN))
	{
		if (tray_menu_timer == 0) {
			tray_menu_timer = g_timeout_add (500,
					(GSourceFunc) tray_menu_timeout, menu);
		}
	}
	else if (event->type == GDK_ENTER_NOTIFY && event->detail == GDK_NOTIFY_ANCESTOR)
	{
		if (tray_menu_timer != 0) {
			g_source_remove (tray_menu_timer);
			tray_menu_timer = 0;
		}
	}
	return FALSE;
}
#endif // _WIN32 || _WIN64

static void	on_create_download (GtkWidget* widget, UgtkApp* app)
{
	ugtk_app_create_download (app, NULL, NULL);
}

static void on_clipboard_monitor (GtkWidget* widget, UgtkApp* app)
{
	if (app->trayicon.menu.emission)
		g_signal_emit_by_name (app->menubar.edit.clipboard_monitor, "activate");
}

static void on_clipboard_quiet (GtkWidget* widget, UgtkApp* app)
{
	if (app->trayicon.menu.emission)
		g_signal_emit_by_name (app->menubar.edit.clipboard_quiet, "activate");
}

static void on_commandline_quiet (GtkWidget* widget, UgtkApp* app)
{
	if (app->trayicon.menu.emission)
		g_signal_emit_by_name (app->menubar.edit.commandline_quiet, "activate");
}

static void on_skip_existing (GtkWidget* widget, UgtkApp* app)
{
	if (app->trayicon.menu.emission)
		g_signal_emit_by_name (app->menubar.edit.skip_existing, "activate");
}

static void on_apply_recent (GtkWidget* widget, UgtkApp* app)
{
	if (app->trayicon.menu.emission)
		g_signal_emit_by_name (app->menubar.edit.apply_recent, "activate");
}

static void  on_config_settings (GtkWidget* widget, UgtkApp* app)
{
	g_signal_emit_by_name (app->menubar.edit.settings, "activate");
}

static void  on_offline_mode (GtkWidget* widget, UgtkApp* app)
{
	gboolean  offline;

	offline = gtk_check_menu_item_get_active ((GtkCheckMenuItem*) widget);
	app->setting.offline_mode = offline;
	// file menu
	offline = gtk_check_menu_item_get_active (
			(GtkCheckMenuItem*) app->menubar.file.offline_mode);
	if (offline != app->setting.offline_mode) {
		gtk_check_menu_item_set_active (
				(GtkCheckMenuItem*) app->menubar.file.offline_mode,
				app->setting.offline_mode);
	}
}

static void  on_about (GtkWidget* widget, UgtkApp* app)
{
	g_signal_emit_by_name (app->menubar.help.about_uget, "activate");
}

#ifdef HAVE_APP_INDICATOR
static void	on_trayicon_show_window (GtkWidget* widget, UgtkApp* app)
{
	if (gtk_widget_get_visible ((GtkWidget*) app->window.self))
		gtk_window_present (app->window.self);
	else {
		gtk_widget_show ((GtkWidget*) app->window.self);
//		gtk_window_deiconify (app->window.self);
		gtk_window_present (app->window.self);
		ugtk_app_decide_trayicon_visible (app);
	}
}
#endif // HAVE_APP_INDICATOR

void  ugtk_trayicon_init_callback (struct UgtkTrayIcon* trayicon, UgtkApp* app)
{
	g_signal_connect (trayicon->self, "activate",
			G_CALLBACK (on_trayicon_activate), app);
	g_signal_connect (trayicon->self, "popup-menu",
			G_CALLBACK (on_trayicon_popup_menu), trayicon);

#if defined _WIN32 || defined _WIN64
	g_signal_connect (trayicon->menu.self, "leave-notify-event",
			G_CALLBACK (tray_menu_leave_enter), NULL);
	g_signal_connect (trayicon->menu.self, "enter-notify-event",
			G_CALLBACK (tray_menu_leave_enter), NULL);
#endif
	g_signal_connect (trayicon->menu.create_download, "activate",
			G_CALLBACK (on_create_download), app);
	g_signal_connect_swapped (trayicon->menu.create_clipboard, "activate",
			G_CALLBACK (ugtk_app_clipboard_batch), app);
	g_signal_connect_swapped (trayicon->menu.create_torrent, "activate",
			G_CALLBACK (ugtk_app_create_torrent), app);
	g_signal_connect_swapped (trayicon->menu.create_metalink, "activate",
			G_CALLBACK (ugtk_app_create_metalink), app);

	trayicon->menu.emission = TRUE;
	g_signal_connect (trayicon->menu.clipboard_monitor, "activate",
			G_CALLBACK (on_clipboard_monitor), app);
	g_signal_connect (trayicon->menu.clipboard_quiet, "activate",
			G_CALLBACK (on_clipboard_quiet), app);
	g_signal_connect (trayicon->menu.commandline_quiet, "activate",
			G_CALLBACK (on_commandline_quiet), app);
	g_signal_connect (trayicon->menu.skip_existing, "activate",
			G_CALLBACK (on_skip_existing), app);
	g_signal_connect (trayicon->menu.apply_recent, "activate",
			G_CALLBACK (on_apply_recent), app);

	g_signal_connect (trayicon->menu.settings, "activate",
			G_CALLBACK (on_config_settings), app);
	g_signal_connect (trayicon->menu.offline_mode, "toggled",
			G_CALLBACK (on_offline_mode), app);
	g_signal_connect (trayicon->menu.about, "activate",
			G_CALLBACK (on_about), app);
	g_signal_connect_swapped (trayicon->menu.quit, "activate",
			G_CALLBACK (ugtk_app_decide_to_quit), app);

#ifdef HAVE_APP_INDICATOR
	g_signal_connect (trayicon->menu.show_window, "activate",
			G_CALLBACK (on_trayicon_show_window), app);
#endif // HAVE_APP_INDICATOR
}

