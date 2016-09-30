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
#include <UgtkNodeList.h>
#include <UgtkNodeTree.h>
#include <UgtkNodeView.h>
#include <gdk/gdk.h>        // GdkScreen

#include <glib/gi18n.h>

static void ugtk_statusbar_init_ui (struct UgtkStatusbar* app_statusbar);
static void ugtk_toolbar_init_ui   (struct UgtkToolbar* app_toolbar, GtkAccelGroup* accel_group);
static void ugtk_window_init_ui    (struct UgtkWindow* window, UgtkApp* app);
static void ugtk_app_init_size     (UgtkApp* app);
#if defined _WIN32 || defined _WIN64
static void ugtk_app_init_ui_win32 (UgtkApp* app, int screen_width);
#endif

void  ugtk_app_init_ui (UgtkApp* app)
{
	// Registers a new accelerator "Ctrl+N" with the global accelerator map.
	gtk_accel_map_add_entry (UGTK_APP_ACCEL_PATH_NEW,      GDK_KEY_n,      GDK_CONTROL_MASK);
	gtk_accel_map_add_entry (UGTK_APP_ACCEL_PATH_LOAD,     GDK_KEY_o,      GDK_CONTROL_MASK);
	gtk_accel_map_add_entry (UGTK_APP_ACCEL_PATH_SAVE,     GDK_KEY_s,      GDK_CONTROL_MASK);
	gtk_accel_map_add_entry (UGTK_APP_ACCEL_PATH_SAVE_ALL, GDK_KEY_s,      GDK_CONTROL_MASK | GDK_SHIFT_MASK);
	gtk_accel_map_add_entry (UGTK_APP_ACCEL_PATH_DELETE,   GDK_KEY_Delete, 0);
//	gtk_accel_map_add_entry (UGTK_APP_ACCEL_PATH_DELETE_F, GDK_KEY_Delete, GDK_SHIFT_MASK);
	gtk_accel_map_add_entry (UGTK_APP_ACCEL_PATH_DELETE_F, GDK_KEY_Delete, GDK_CONTROL_MASK);
	gtk_accel_map_add_entry (UGTK_APP_ACCEL_PATH_OPEN,     GDK_KEY_Return, 0);
	gtk_accel_map_add_entry (UGTK_APP_ACCEL_PATH_OPEN_F,   GDK_KEY_Return, GDK_SHIFT_MASK);
	// accelerators
	app->accel_group = gtk_accel_group_new ();
	// tray icon
	ugtk_tray_icon_init (&app->trayicon);
	// Main Window and it's widgets
	ugtk_banner_init (&app->banner);
	ugtk_menubar_init_ui (&app->menubar, app->accel_group);
	ugtk_summary_init (&app->summary, app->accel_group);
	ugtk_traveler_init (&app->traveler, app);
	ugtk_statusbar_init_ui (&app->statusbar);
	ugtk_toolbar_init_ui (&app->toolbar, app->accel_group);
	ugtk_window_init_ui (&app->window, app);
	ugtk_app_init_size (app);
}

// set default size
static void ugtk_app_init_size (UgtkApp* app)
{
	GdkScreen*  screen;
	gint        width, height;
	gint        paned_position;

	screen = gdk_screen_get_default ();
	if (screen) {
		width  = gdk_screen_get_width (screen);
		height = gdk_screen_get_height (screen);
	}
	else {
		width  = 0;
		height = 0;
	}

	// decide window & traveler size
	if (width <= 640) {
		width = 620;
		height = 430;
		paned_position = 165;
	}
	if (width <= 800) {
		width = width * 95 / 100;
		height = height * 9 / 10;
		paned_position = 180;
	}
	else if (width <= 1000) {
		width = width * 90 / 100;
		height = height * 8 / 10;
		paned_position = 200;
	}
	else {
		width = width * 85 / 100;
		height = height * 8 / 10;
		paned_position = width * 22 / 100;
	}

	gtk_window_resize (app->window.self, width, height);
	gtk_paned_set_position (app->window.hpaned, paned_position);

#if defined _WIN32 || defined _WIN64
	ugtk_app_init_ui_win32 (app, width);
#endif
}

#if defined _WIN32 || defined _WIN64
static void ugtk_app_init_ui_win32 (UgtkApp* app, int screen_width)
{
	GSettings*  gset;
	gint        sidebar_width;

#if 0
	// This will use icons\hicolor\index.theme
	GtkIconTheme*   icon_theme;
	gchar*          path;

	icon_theme = gtk_icon_theme_get_default ();
	path = g_build_filename (UG_DATADIR, "icons", NULL);
	gtk_icon_theme_append_search_path (icon_theme, path);
	g_free (path);
#endif

	if (screen_width <= 800)
		sidebar_width = 0;
	else if (screen_width <= 1200)
		sidebar_width = 180;
	else
		sidebar_width = 220;

	gset = g_settings_new ("org.gtk.Settings.FileChooser");
	g_settings_set_boolean (gset, "sort-directories-first", TRUE);

	// default of "sidebar-width" == 148
	if (sidebar_width > 0 && g_settings_get_int(gset, "sidebar-width") == 148)
		g_settings_set_int (gset, "sidebar-width", sidebar_width);
}
#endif  // _WIN32 || _WIN64

// ----------------------------------------------------------------------------
// UgtkWindow

static void ugtk_window_init_ui (struct UgtkWindow* window, UgtkApp* app)
{
	GtkBox*  vbox;
	GtkBox*  lbox;    // left side vbox
	GtkBox*  rbox;    // right side vbox

	window->self = (GtkWindow*) gtk_window_new (GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title (window->self, UGTK_APP_NAME);
//	gtk_window_resize (window->self, 640, 480);
	gtk_window_add_accel_group (window->self, app->accel_group);
	gtk_window_set_default_icon_name (UGTK_APP_ICON_NAME);
#if GTK_MAJOR_VERSION <= 3 && GTK_MINOR_VERSION < 14
	gtk_window_set_has_resize_grip (window->self, FALSE);
#endif

	// top container for Main Window
	vbox = (GtkBox*) gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
	gtk_container_add (GTK_CONTAINER (window->self), GTK_WIDGET (vbox));
	// banner + menubar
	gtk_box_pack_start (vbox, app->banner.self, FALSE, FALSE, 0);
	gtk_box_pack_start (vbox, app->menubar.self, FALSE, FALSE, 0);

	// hpaned
	window->hpaned = (GtkPaned*) gtk_paned_new (GTK_ORIENTATION_HORIZONTAL);
	gtk_box_pack_start (vbox, GTK_WIDGET (window->hpaned), TRUE, TRUE, 0);
	lbox = (GtkBox*) gtk_box_new (GTK_ORIENTATION_VERTICAL, 2);
	rbox = (GtkBox*) gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
	gtk_paned_pack1 (window->hpaned, GTK_WIDGET (lbox), FALSE, TRUE);
	gtk_paned_pack2 (window->hpaned, GTK_WIDGET (rbox), TRUE, FALSE);

	gtk_box_pack_start (lbox, gtk_label_new (_("Status")), FALSE, FALSE, 0);
	gtk_box_pack_start (lbox, app->traveler.state.self, FALSE, FALSE, 0);
	gtk_box_pack_start (lbox, gtk_label_new (_("Category")), FALSE, FALSE, 0);
	gtk_box_pack_start (lbox, app->traveler.category.self, TRUE, TRUE, 0);
	gtk_box_pack_start (rbox, app->toolbar.self, FALSE, FALSE, 0);

	// vpaned
	window->vpaned = (GtkPaned*) gtk_paned_new (GTK_ORIENTATION_VERTICAL);
	gtk_box_pack_start (rbox, (GtkWidget*) window->vpaned, TRUE, TRUE, 0);
	gtk_paned_pack1 (window->vpaned, app->traveler.download.self , TRUE, TRUE);
	gtk_paned_pack2 (window->vpaned, app->summary.self, FALSE, TRUE);

	gtk_box_pack_start (vbox, GTK_WIDGET (app->statusbar.self), FALSE, FALSE, 0);
	gtk_widget_show_all ((GtkWidget*) vbox);
}

// ----------------------------------------------------------------------------
// UgtkStatusbar
//
static void ugtk_statusbar_init_ui (struct UgtkStatusbar* sbar)
{
	GtkBox*    hbox;
	GtkWidget* widget;
	PangoContext*  context;
	PangoLayout*   layout;
	int            text_width;

	sbar->self = (GtkStatusbar*) gtk_statusbar_new ();
	hbox = GTK_BOX (sbar->self);

	// calculate text width
	context = gtk_widget_get_pango_context (GTK_WIDGET (sbar->self));
	layout = pango_layout_new (context);
	pango_layout_set_text (layout, "9999 MiB/s", -1);
	pango_layout_get_pixel_size (layout, &text_width, NULL);
	g_object_unref (layout);
	if (text_width < 100)
		text_width = 100;

	// upload speed label
	widget = gtk_label_new ("");
	sbar->up_speed = (GtkLabel*) widget;
	gtk_widget_set_size_request (widget, text_width, 0);
	gtk_box_pack_end (hbox, widget, FALSE, TRUE, 2);
//	gtk_label_set_width_chars (sbar->down_speed, 15);
	gtk_misc_set_alignment (GTK_MISC (widget), 0, 0.5);
	gtk_box_pack_end (hbox, gtk_label_new ("\xE2\x86\x91"), FALSE, TRUE, 2);    // "↑"

	gtk_box_pack_end (hbox, gtk_separator_new (GTK_ORIENTATION_VERTICAL), FALSE, TRUE, 8);

	// download speed label
	widget = gtk_label_new ("");
	sbar->down_speed = (GtkLabel*) widget;
	gtk_widget_set_size_request (widget, text_width, 0);
	gtk_box_pack_end (hbox, widget, FALSE, TRUE, 2);
//	gtk_label_set_width_chars (sbar->down_speed, 15);
	gtk_misc_set_alignment (GTK_MISC (widget), 0, 0.5);
	gtk_box_pack_end (hbox, gtk_label_new ("\xE2\x86\x93"), FALSE, TRUE, 2);    // "↓"
}

// ----------------------------------------------------------------------------
// UgtkToolbar
//
static void ugtk_toolbar_init_ui (struct UgtkToolbar* ugt, GtkAccelGroup* accel_group)
{
	GtkToolbar*   toolbar;
	GtkToolItem*  tool_item;
	GtkWidget*    image;
	GtkWidget*    menu;
	GtkWidget*    menu_item;

	// toolbar
	ugt->toolbar = gtk_toolbar_new ();
	toolbar = GTK_TOOLBAR (ugt->toolbar);
	gtk_toolbar_set_style (toolbar, GTK_TOOLBAR_ICONS);
	gtk_toolbar_set_icon_size (toolbar, GTK_ICON_SIZE_SMALL_TOOLBAR);
	ugt->self = ugt->toolbar;
	gtk_widget_show (ugt->self);

	// New button --- start ---
#if GTK_MAJOR_VERSION >= 3 && GTK_MINOR_VERSION >= 10
	image = gtk_image_new_from_icon_name ("document-new", GTK_ICON_SIZE_SMALL_TOOLBAR);
	tool_item = (GtkToolItem*) gtk_menu_tool_button_new (image, NULL);
#else
	tool_item = (GtkToolItem*) gtk_menu_tool_button_new_from_stock (GTK_STOCK_NEW);
#endif
	gtk_tool_item_set_tooltip_text (tool_item, _("Create new download"));
	gtk_menu_tool_button_set_arrow_tooltip_text ((GtkMenuToolButton*)tool_item, "Create new item");
	gtk_tool_item_set_homogeneous (tool_item, FALSE);
	gtk_toolbar_insert (toolbar, tool_item, -1);
	ugt->create = GTK_WIDGET (tool_item);
	// menu for tool button (accelerators)
	menu = gtk_menu_new ();
	gtk_menu_set_accel_group ((GtkMenu*) menu, accel_group);
	gtk_menu_tool_button_set_menu ((GtkMenuToolButton*)tool_item, menu);
	// New Download (accelerators)
	menu_item = gtk_image_menu_item_new_with_mnemonic (_("New _Download..."));
	gtk_menu_item_set_accel_path ((GtkMenuItem*) menu_item, UGTK_APP_ACCEL_PATH_NEW);
#if GTK_MAJOR_VERSION >= 3 && GTK_MINOR_VERSION >= 10
	image = gtk_image_new_from_icon_name ("document-new", GTK_ICON_SIZE_MENU);
#else
	image = gtk_image_new_from_stock (GTK_STOCK_NEW, GTK_ICON_SIZE_MENU);
#endif
	gtk_image_menu_item_set_image ((GtkImageMenuItem*)menu_item, image);
	gtk_menu_shell_append ((GtkMenuShell*)menu, menu_item);
	ugt->create_download = menu_item;
	// New Category
	menu_item = gtk_image_menu_item_new_with_mnemonic (_("New _Category..."));
#if GTK_MAJOR_VERSION >= 3 && GTK_MINOR_VERSION >= 10
	image = gtk_image_new_from_icon_name ("gtk-dnd-multiple", GTK_ICON_SIZE_MENU);
#else
	image = gtk_image_new_from_stock (GTK_STOCK_DND_MULTIPLE, GTK_ICON_SIZE_MENU);
#endif
	gtk_image_menu_item_set_image ((GtkImageMenuItem*)menu_item, image);
	gtk_menu_shell_append ((GtkMenuShell*)menu, menu_item);
	ugt->create_category = menu_item;
	// New Clipboard batch
	menu_item = gtk_image_menu_item_new_with_mnemonic (_("New Clipboard _batch..."));
#if GTK_MAJOR_VERSION >= 3 && GTK_MINOR_VERSION >= 10
	image = gtk_image_new_from_icon_name ("edit-paste", GTK_ICON_SIZE_MENU);
#else
	image = gtk_image_new_from_stock (GTK_STOCK_PASTE, GTK_ICON_SIZE_MENU);
#endif
	gtk_image_menu_item_set_image ((GtkImageMenuItem*)menu_item, image);
	gtk_menu_shell_append ((GtkMenuShell*)menu, menu_item);
	ugt->create_clipboard = menu_item;
	gtk_widget_show_all (menu);
	// New URL Sequence batch
	menu_item = gtk_image_menu_item_new_with_mnemonic (_("New _URL Sequence batch..."));
#if GTK_MAJOR_VERSION >= 3 && GTK_MINOR_VERSION >= 10
	image = gtk_image_new_from_icon_name ("view-sort-ascending", GTK_ICON_SIZE_MENU);
#else
	image = gtk_image_new_from_stock (GTK_STOCK_SORT_ASCENDING, GTK_ICON_SIZE_MENU);
#endif
	gtk_image_menu_item_set_image ((GtkImageMenuItem*)menu_item, image);
	gtk_menu_shell_append ((GtkMenuShell*)menu, menu_item);
	ugt->create_sequence = menu_item;
	gtk_widget_show_all (menu);

	gtk_menu_shell_append ((GtkMenuShell*)menu, gtk_separator_menu_item_new() );

	// New Torrent
	menu_item = gtk_image_menu_item_new_with_mnemonic (_("New Torrent..."));
//	image = gtk_image_new_from_stock (GTK_STOCK_FILE, GTK_ICON_SIZE_MENU);
//	gtk_image_menu_item_set_image ((GtkImageMenuItem*)menu_item, image);
	gtk_menu_shell_append ((GtkMenuShell*)menu, menu_item);
	ugt->create_torrent = menu_item;
	gtk_widget_show_all (menu);
	// New Metalink
	menu_item = gtk_image_menu_item_new_with_mnemonic (_("New Metalink..."));
//	image = gtk_image_new_from_stock (GTK_STOCK_FILE, GTK_ICON_SIZE_MENU);
//	gtk_image_menu_item_set_image ((GtkImageMenuItem*)menu_item, image);
	gtk_menu_shell_append ((GtkMenuShell*)menu, menu_item);
	ugt->create_metalink = menu_item;
	gtk_widget_show_all (menu);
	// New button --- end ---

#if GTK_MAJOR_VERSION >= 3 && GTK_MINOR_VERSION >= 10
	image = gtk_image_new_from_icon_name ("document-save", GTK_ICON_SIZE_SMALL_TOOLBAR);
	tool_item = (GtkToolItem*) gtk_tool_button_new (image, NULL);
#else
	tool_item = (GtkToolItem*) gtk_tool_button_new_from_stock (GTK_STOCK_SAVE);
#endif
	gtk_tool_item_set_tooltip_text (tool_item, _("Save all settings"));
	gtk_toolbar_insert (toolbar, tool_item, -1);
	ugt->save = GTK_WIDGET (tool_item);

	tool_item = gtk_separator_tool_item_new ();
	gtk_toolbar_insert (toolbar, tool_item, -1);

#if GTK_MAJOR_VERSION >= 3 && GTK_MINOR_VERSION >= 10
	image = gtk_image_new_from_icon_name ("media-playback-start", GTK_ICON_SIZE_SMALL_TOOLBAR);
	tool_item = (GtkToolItem*) gtk_tool_button_new (image, NULL);
#else
	tool_item = (GtkToolItem*) gtk_tool_button_new_from_stock (GTK_STOCK_MEDIA_PLAY);
#endif
	gtk_tool_item_set_tooltip_text (tool_item, _("Set selected download runnable"));
	gtk_toolbar_insert (toolbar, tool_item, -1);
	ugt->runnable = GTK_WIDGET (tool_item);

#if GTK_MAJOR_VERSION >= 3 && GTK_MINOR_VERSION >= 10
	image = gtk_image_new_from_icon_name ("media-playback-pause", GTK_ICON_SIZE_SMALL_TOOLBAR);
	tool_item = (GtkToolItem*) gtk_tool_button_new (image, NULL);
#else
	tool_item = (GtkToolItem*) gtk_tool_button_new_from_stock (GTK_STOCK_MEDIA_PAUSE);
#endif
	gtk_tool_item_set_tooltip_text (tool_item, _("Set selected download to pause"));
	gtk_toolbar_insert (toolbar, tool_item, -1);
	ugt->pause = GTK_WIDGET (tool_item);

#if GTK_MAJOR_VERSION >= 3 && GTK_MINOR_VERSION >= 10
	image = gtk_image_new_from_icon_name ("document-properties", GTK_ICON_SIZE_SMALL_TOOLBAR);
	tool_item = (GtkToolItem*) gtk_tool_button_new (image, NULL);
#else
	tool_item = (GtkToolItem*) gtk_tool_button_new_from_stock (GTK_STOCK_PROPERTIES);
#endif
	gtk_tool_item_set_tooltip_text (tool_item, _("Set selected download properties"));
	gtk_toolbar_insert (toolbar, tool_item, -1);
	ugt->properties = GTK_WIDGET (tool_item);

	tool_item = gtk_separator_tool_item_new ();
	gtk_toolbar_insert (toolbar, tool_item, -1);

#if GTK_MAJOR_VERSION >= 3 && GTK_MINOR_VERSION >= 10
	image = gtk_image_new_from_icon_name ("go-up", GTK_ICON_SIZE_SMALL_TOOLBAR);
	tool_item = (GtkToolItem*) gtk_tool_button_new (image, NULL);
#else
	tool_item = (GtkToolItem*) gtk_tool_button_new_from_stock (GTK_STOCK_GO_UP);
#endif
	gtk_tool_item_set_tooltip_text (tool_item, _("Move selected download up"));
	gtk_toolbar_insert (toolbar, tool_item, -1);
	ugt->move_up = GTK_WIDGET (tool_item);

#if GTK_MAJOR_VERSION >= 3 && GTK_MINOR_VERSION >= 10
	image = gtk_image_new_from_icon_name ("go-down", GTK_ICON_SIZE_SMALL_TOOLBAR);
	tool_item = (GtkToolItem*) gtk_tool_button_new (image, NULL);
#else
	tool_item = (GtkToolItem*) gtk_tool_button_new_from_stock (GTK_STOCK_GO_DOWN);
#endif
	gtk_tool_item_set_tooltip_text (tool_item, _("Move selected download down"));
	gtk_toolbar_insert (toolbar, tool_item, -1);
	ugt->move_down = GTK_WIDGET (tool_item);

#if GTK_MAJOR_VERSION >= 3 && GTK_MINOR_VERSION >= 10
	image = gtk_image_new_from_icon_name ("go-top", GTK_ICON_SIZE_SMALL_TOOLBAR);
	tool_item = (GtkToolItem*) gtk_tool_button_new (image, NULL);
#else
	tool_item = (GtkToolItem*) gtk_tool_button_new_from_stock (GTK_STOCK_GOTO_TOP);
#endif
	gtk_tool_item_set_tooltip_text (tool_item, _("Move selected download to top"));
	gtk_toolbar_insert (toolbar, tool_item, -1);
	ugt->move_top = GTK_WIDGET (tool_item);

#if GTK_MAJOR_VERSION >= 3 && GTK_MINOR_VERSION >= 10
	image = gtk_image_new_from_icon_name ("go-bottom", GTK_ICON_SIZE_SMALL_TOOLBAR);
	tool_item = (GtkToolItem*) gtk_tool_button_new (image, NULL);
#else
	tool_item = (GtkToolItem*) gtk_tool_button_new_from_stock (GTK_STOCK_GOTO_BOTTOM);
#endif
	gtk_tool_item_set_tooltip_text (tool_item, _("Move selected download to bottom"));
	gtk_toolbar_insert (toolbar, tool_item, -1);
	ugt->move_bottom = GTK_WIDGET (tool_item);

	gtk_widget_show_all ((GtkWidget*) toolbar);
}

