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

#include <UgtkMenubar.h>
#include <UgtkApp.h>

#include <glib/gi18n.h>

// UgtkFileMenu
static void ugtk_menubar_file_init (UgtkMenubar* menubar, GtkAccelGroup* accel_group)
{
	GtkWidget*  image;
	GtkWidget*  menu;
	GtkWidget*  submenu;
	GtkWidget*  menu_item;

	menu = gtk_menu_new ();
	menu_item = gtk_menu_item_new_with_mnemonic (_("_File"));
	gtk_menu_item_set_submenu ((GtkMenuItem*)menu_item, menu);
	gtk_menu_shell_append ((GtkMenuShell*)menubar->self, menu_item);
	gtk_menu_set_accel_group ((GtkMenu*)menu, accel_group);
//	gtk_menu_shell_append ((GtkMenuShell*)menu, gtk_tearoff_menu_item_new() );

	// New Download
	menu_item = gtk_image_menu_item_new_with_mnemonic (_("New _Download..."));
	gtk_menu_item_set_accel_path ((GtkMenuItem*) menu_item, UGTK_APP_ACCEL_PATH_NEW);
#if GTK_MAJOR_VERSION >= 3 && GTK_MINOR_VERSION >= 10
	image = gtk_image_new_from_icon_name ("document-new", GTK_ICON_SIZE_MENU);
#else
	image = gtk_image_new_from_stock (GTK_STOCK_NEW, GTK_ICON_SIZE_MENU);
#endif
	gtk_image_menu_item_set_image ((GtkImageMenuItem*)menu_item, image);
	gtk_menu_shell_append ((GtkMenuShell*)menu, menu_item);
	menubar->file.create_download = menu_item;
	// New Category
	menu_item = gtk_image_menu_item_new_with_mnemonic (_("New _Category..."));
#if GTK_MAJOR_VERSION >= 3 && GTK_MINOR_VERSION >= 10
	image = gtk_image_new_from_icon_name ("gtk-dnd-multiple", GTK_ICON_SIZE_MENU);
#else
	image = gtk_image_new_from_stock (GTK_STOCK_DND_MULTIPLE, GTK_ICON_SIZE_MENU);
#endif
	gtk_image_menu_item_set_image ((GtkImageMenuItem*)menu_item, image);
	gtk_menu_shell_append ((GtkMenuShell*)menu, menu_item);
	menubar->file.create_category = menu_item;
	// separator
//	gtk_menu_shell_append ((GtkMenuShell*)submenu, gtk_separator_menu_item_new() );
	// New Torrent
	menu_item = gtk_image_menu_item_new_with_mnemonic (_("New Torrent..."));
//	image = gtk_image_new_from_stock (GTK_STOCK_FILE, GTK_ICON_SIZE_MENU);
//	gtk_image_menu_item_set_image ((GtkImageMenuItem*)menu_item, image);
	gtk_menu_shell_append ((GtkMenuShell*)menu, menu_item);
	menubar->file.create_torrent = menu_item;
	// New Metalink
	menu_item = gtk_image_menu_item_new_with_mnemonic (_("New Metalink..."));
//	image = gtk_image_new_from_stock (GTK_STOCK_FILE, GTK_ICON_SIZE_MENU);
//	gtk_image_menu_item_set_image ((GtkImageMenuItem*)menu_item, image);
	gtk_menu_shell_append ((GtkMenuShell*)menu, menu_item);
	menubar->file.create_metalink = menu_item;

	// Batch Downloads --- start ---
	menu_item = gtk_image_menu_item_new_with_mnemonic (_("_Batch Downloads"));
	submenu = gtk_menu_new ();
	gtk_menu_set_accel_group ((GtkMenu*)submenu, accel_group);
	gtk_menu_item_set_submenu ((GtkMenuItem*)menu_item, submenu);
	gtk_menu_shell_append ((GtkMenuShell*)menu, menu_item);
	// Batch downloads - Clipboard batch
	menu_item = gtk_image_menu_item_new_with_mnemonic (_("_Clipboard batch..."));
#if GTK_MAJOR_VERSION >= 3 && GTK_MINOR_VERSION >= 10
	image = gtk_image_new_from_icon_name ("edit-paste", GTK_ICON_SIZE_MENU);
#else
	image = gtk_image_new_from_stock (GTK_STOCK_PASTE, GTK_ICON_SIZE_MENU);
#endif
	gtk_image_menu_item_set_image ((GtkImageMenuItem*)menu_item, image);
	gtk_menu_shell_append ((GtkMenuShell*)submenu, menu_item);
	menubar->file.batch.clipboard = menu_item;
	// Batch downloads - URL Sequence batch
	menu_item = gtk_image_menu_item_new_with_mnemonic (_("_URL Sequence batch..."));
#if GTK_MAJOR_VERSION >= 3 && GTK_MINOR_VERSION >= 10
	image = gtk_image_new_from_icon_name ("view-sort-ascending", GTK_ICON_SIZE_MENU);
#else
	image = gtk_image_new_from_stock (GTK_STOCK_SORT_ASCENDING, GTK_ICON_SIZE_MENU);
#endif
	gtk_image_menu_item_set_image ((GtkImageMenuItem*)menu_item, image);
	gtk_menu_shell_append ((GtkMenuShell*)submenu, menu_item);
	menubar->file.batch.sequence = menu_item;
	// Batch downloads - Text file import (.txt)
	menu_item = gtk_image_menu_item_new_with_mnemonic (_("_Text file import (.txt)..."));
#if GTK_MAJOR_VERSION >= 3 && GTK_MINOR_VERSION >= 10
	image = gtk_image_new_from_icon_name ("go-next", GTK_ICON_SIZE_MENU);
#else
	image = gtk_image_new_from_stock (GTK_STOCK_GO_FORWARD, GTK_ICON_SIZE_MENU);
#endif
	gtk_image_menu_item_set_image ((GtkImageMenuItem*)menu_item, image);
	gtk_menu_shell_append ((GtkMenuShell*)submenu, menu_item);
	menubar->file.batch.text_import = menu_item;
	// Batch downloads - HTML file import (.html)
	menu_item = gtk_image_menu_item_new_with_mnemonic (_("_HTML file import (.html)..."));
#if GTK_MAJOR_VERSION >= 3 && GTK_MINOR_VERSION >= 10
	image = gtk_image_new_from_icon_name ("gtk-convert", GTK_ICON_SIZE_MENU);
#else
	image = gtk_image_new_from_stock (GTK_STOCK_CONVERT, GTK_ICON_SIZE_MENU);
#endif
	gtk_image_menu_item_set_image ((GtkImageMenuItem*)menu_item, image);
	gtk_menu_shell_append ((GtkMenuShell*)submenu, menu_item);
	menubar->file.batch.html_import = menu_item;
	// Batch downloads - separator
	gtk_menu_shell_append ((GtkMenuShell*)submenu, gtk_separator_menu_item_new() );
	// Batch downloads - Export to Text file (.txt)
	menu_item = gtk_image_menu_item_new_with_mnemonic (_("_Export to Text file (.txt)..."));
#if GTK_MAJOR_VERSION >= 3 && GTK_MINOR_VERSION >= 10
	image = gtk_image_new_from_icon_name ("go-previous", GTK_ICON_SIZE_MENU);
#else
	image = gtk_image_new_from_stock (GTK_STOCK_GO_BACK, GTK_ICON_SIZE_MENU);
#endif
	gtk_image_menu_item_set_image ((GtkImageMenuItem*)menu_item, image);
	gtk_menu_shell_append ((GtkMenuShell*)submenu, menu_item);
	menubar->file.batch.text_export = menu_item;
	// Batch downloads --- end ---

	gtk_menu_shell_append ((GtkMenuShell*)menu, gtk_separator_menu_item_new() );

	// Open Category
	menu_item = gtk_image_menu_item_new_with_mnemonic (_("_Open category..."));
	gtk_menu_item_set_accel_path ((GtkMenuItem*) menu_item, UGTK_APP_ACCEL_PATH_LOAD);
#if GTK_MAJOR_VERSION >= 3 && GTK_MINOR_VERSION >= 10
	image = gtk_image_new_from_icon_name ("document-open", GTK_ICON_SIZE_MENU);
#else
	image = gtk_image_new_from_stock (GTK_STOCK_OPEN, GTK_ICON_SIZE_MENU);
#endif
	gtk_image_menu_item_set_image ((GtkImageMenuItem*)menu_item, image);
	gtk_menu_shell_append ((GtkMenuShell*)menu, menu_item);
	menubar->file.open_category = menu_item;
	// Save Category
	menu_item = gtk_image_menu_item_new_with_mnemonic (_("_Save category as..."));
	gtk_menu_item_set_accel_path ((GtkMenuItem*) menu_item, UGTK_APP_ACCEL_PATH_SAVE);
#if GTK_MAJOR_VERSION >= 3 && GTK_MINOR_VERSION >= 10
	image = gtk_image_new_from_icon_name ("document-save", GTK_ICON_SIZE_MENU);
#else
	image = gtk_image_new_from_stock (GTK_STOCK_SAVE, GTK_ICON_SIZE_MENU);
#endif
	gtk_image_menu_item_set_image ((GtkImageMenuItem*)menu_item, image);
	gtk_menu_shell_append ((GtkMenuShell*)menu, menu_item);
	menubar->file.save_category = menu_item;
	// Save All
	menu_item = gtk_image_menu_item_new_with_mnemonic (_("Save _all settings"));
	gtk_menu_item_set_accel_path ((GtkMenuItem*) menu_item, UGTK_APP_ACCEL_PATH_SAVE_ALL);
#if GTK_MAJOR_VERSION >= 3 && GTK_MINOR_VERSION >= 10
	image = gtk_image_new_from_icon_name ("document-save", GTK_ICON_SIZE_MENU);
#else
	image = gtk_image_new_from_stock (GTK_STOCK_SAVE, GTK_ICON_SIZE_MENU);
#endif
	gtk_image_menu_item_set_image ((GtkImageMenuItem*)menu_item, image);
	gtk_menu_shell_append ((GtkMenuShell*)menu, menu_item);
	menubar->file.save = menu_item;

	gtk_menu_shell_append ((GtkMenuShell*)menu, gtk_separator_menu_item_new() );

	// Offline mode
	menu_item = gtk_check_menu_item_new_with_mnemonic (_("Offline Mode"));
	gtk_menu_shell_append ((GtkMenuShell*)menu, menu_item);
	menubar->file.offline_mode = menu_item;

	menu_item = gtk_image_menu_item_new_from_stock (GTK_STOCK_QUIT, accel_group);
	gtk_menu_shell_append ((GtkMenuShell*)menu, menu_item);
	menubar->file.quit = menu_item;
}

// UgtkEditMenu
static void ugtk_menubar_edit_init (UgtkMenubar* menubar)
{
	GtkWidget*  image;
	GtkWidget*  menu;
	GtkWidget*  submenu;
	GtkWidget*  menu_item;

	menu = gtk_menu_new ();
	menu_item = gtk_menu_item_new_with_mnemonic (_("_Edit"));
	gtk_menu_item_set_submenu ((GtkMenuItem*)menu_item, menu);
	gtk_menu_shell_append ((GtkMenuShell*)menubar->self, menu_item);
//	menu.gtk_menu_shell_append((GtkMenuShell*)menu, gtk_tearoff_menu_item_new() );

	menu_item = gtk_check_menu_item_new_with_mnemonic (_("Clipboard _Monitor"));
	gtk_menu_shell_append ((GtkMenuShell*)menu, menu_item);
	menubar->edit.clipboard_monitor = menu_item;

	menu_item = gtk_check_menu_item_new_with_mnemonic (_("Clipboard works quietly"));
	gtk_menu_shell_append ((GtkMenuShell*)menu, menu_item);
	menubar->edit.clipboard_quiet = menu_item;

	menu_item = gtk_check_menu_item_new_with_mnemonic (_("Command-line works quietly"));
	gtk_menu_shell_append ((GtkMenuShell*)menu, menu_item);
	menubar->edit.commandline_quiet = menu_item;

	menu_item = gtk_check_menu_item_new_with_mnemonic (_("Skip existing URI"));
	gtk_menu_shell_append ((GtkMenuShell*)menu, menu_item);
	menubar->edit.skip_existing = menu_item;

	menu_item = gtk_check_menu_item_new_with_mnemonic (_("Apply recent download settings"));
	gtk_menu_shell_append ((GtkMenuShell*)menu, menu_item);
	menubar->edit.apply_recent = menu_item;

	gtk_menu_shell_append ((GtkMenuShell*)menu, gtk_separator_menu_item_new());

	// --- Completion Auto-Actions --- start ---
	menu_item = gtk_image_menu_item_new_with_mnemonic (_("Completion _Auto-Actions"));
	submenu = gtk_menu_new ();
//	gtk_menu_set_accel_group ((GtkMenu*)submenu, accel_group);
	gtk_menu_item_set_submenu ((GtkMenuItem*)menu_item, submenu);
	gtk_menu_shell_append ((GtkMenuShell*)menu, menu_item);
	// Completion Auto-Actions - Disable
	menu_item = gtk_radio_menu_item_new_with_mnemonic (NULL, _("_Disable"));
	gtk_menu_shell_append ((GtkMenuShell*)submenu, menu_item);
	menubar->edit.completion.disable = menu_item;
	// Completion Auto-Actions - Hibernate
	menu_item = gtk_radio_menu_item_new_with_mnemonic_from_widget (
			(GtkRadioMenuItem*) menu_item, _("Hibernate"));
	gtk_menu_shell_append ((GtkMenuShell*)submenu, menu_item);
	menubar->edit.completion.hibernate = menu_item;
	// Completion Auto-Actions - Suspend
	menu_item = gtk_radio_menu_item_new_with_mnemonic_from_widget (
			(GtkRadioMenuItem*) menu_item, _("Suspend"));
	gtk_menu_shell_append ((GtkMenuShell*)submenu, menu_item);
	menubar->edit.completion.suspend = menu_item;
	// Completion Auto-Actions - Shutdown
	menu_item = gtk_radio_menu_item_new_with_mnemonic_from_widget (
			(GtkRadioMenuItem*) menu_item, _("Shutdown"));
	gtk_menu_shell_append ((GtkMenuShell*)submenu, menu_item);
	menubar->edit.completion.shutdown = menu_item;
	// Completion Auto-Actions - Reboot
	menu_item = gtk_radio_menu_item_new_with_mnemonic_from_widget (
			(GtkRadioMenuItem*) menu_item, _("Reboot"));
	gtk_menu_shell_append ((GtkMenuShell*)submenu, menu_item);
	menubar->edit.completion.reboot = menu_item;
	// Completion Auto-Actions - Custom
	menu_item = gtk_radio_menu_item_new_with_mnemonic_from_widget (
			(GtkRadioMenuItem*) menu_item, _("Custom"));
	gtk_menu_shell_append ((GtkMenuShell*)submenu, menu_item);
	menubar->edit.completion.custom = menu_item;
	// separator
	gtk_menu_shell_append ((GtkMenuShell*)submenu, gtk_separator_menu_item_new());
	// Completion Auto-Actions - Remember
	menu_item = gtk_check_menu_item_new_with_mnemonic (_("Remember setting"));
	gtk_menu_shell_append ((GtkMenuShell*)submenu, menu_item);
	menubar->edit.completion.remember = menu_item;
	// Completion Auto-Actions - Help
	menu_item = gtk_menu_item_new_with_mnemonic (_("_Help"));
	gtk_menu_shell_append ((GtkMenuShell*)submenu, menu_item);
	menubar->edit.completion.help = menu_item;
	// --- Completion Auto-Actions --- end ---

//	menu_item = gtk_menu_item_new_with_mnemonic (_("_Settings..."));
	menu_item = gtk_image_menu_item_new_with_mnemonic (_("_Settings..."));
#if GTK_MAJOR_VERSION >= 3 && GTK_MINOR_VERSION >= 10
	image = gtk_image_new_from_icon_name ("document-properties", GTK_ICON_SIZE_MENU);
#else
	image = gtk_image_new_from_stock (GTK_STOCK_PROPERTIES, GTK_ICON_SIZE_MENU);
#endif
	gtk_image_menu_item_set_image ((GtkImageMenuItem*)menu_item, image);
	gtk_menu_shell_append ((GtkMenuShell*)menu, menu_item);
	menubar->edit.settings = menu_item;
}

// UgtkViewMenu
static void ugtk_menubar_view_init (UgtkMenubar* menubar)
{
	GtkWidget*  menu;
	GtkWidget*  submenu;
	GtkWidget*  menu_item;

	menu = gtk_menu_new ();
	menu_item = gtk_menu_item_new_with_mnemonic (_("_View"));
	gtk_menu_item_set_submenu ((GtkMenuItem*) menu_item, menu);
	gtk_menu_shell_append ((GtkMenuShell*) menubar->self, menu_item);

	menu_item = gtk_check_menu_item_new_with_mnemonic (_("_Toolbar"));
	gtk_check_menu_item_set_active ((GtkCheckMenuItem*) menu_item, TRUE);
	gtk_menu_shell_append ((GtkMenuShell*)menu, menu_item);
	menubar->view.toolbar = menu_item;

	menu_item = gtk_check_menu_item_new_with_mnemonic (_("Statusbar"));
	gtk_check_menu_item_set_active ((GtkCheckMenuItem*) menu_item, TRUE);
	gtk_menu_shell_append ((GtkMenuShell*)menu, menu_item);
	menubar->view.statusbar = menu_item;

	menu_item = gtk_check_menu_item_new_with_mnemonic (_("Category"));
	gtk_check_menu_item_set_active ((GtkCheckMenuItem*) menu_item, TRUE);
	gtk_menu_shell_append ((GtkMenuShell*)menu, menu_item);
	menubar->view.category = menu_item;

	menu_item = gtk_check_menu_item_new_with_mnemonic (_("_Summary"));
	gtk_check_menu_item_set_active ((GtkCheckMenuItem*) menu_item, TRUE);
	gtk_menu_shell_append ((GtkMenuShell*)menu, menu_item);
	menubar->view.summary = menu_item;

	gtk_menu_shell_append ((GtkMenuShell*)menu, gtk_separator_menu_item_new() );

	// Summary Items --- start ---
	menu_item = gtk_menu_item_new_with_mnemonic (_("Summary _Items"));
	submenu  = gtk_menu_new ();
	gtk_menu_item_set_submenu ((GtkMenuItem*) menu_item, submenu);
	gtk_menu_shell_append ((GtkMenuShell*)menu, menu_item);
	// Summary Items - Name
	menu_item = gtk_check_menu_item_new_with_mnemonic (_("_Name"));
	gtk_check_menu_item_set_active ((GtkCheckMenuItem*) menu_item, TRUE);
	gtk_menu_shell_append ((GtkMenuShell*) submenu, menu_item);
	menubar->view.summary_items.name = menu_item;
	// Summary Items - Folder
	menu_item = gtk_check_menu_item_new_with_mnemonic (_("_Folder"));
	gtk_check_menu_item_set_active ((GtkCheckMenuItem*) menu_item, TRUE);
	gtk_menu_shell_append ((GtkMenuShell*) submenu, menu_item);
	menubar->view.summary_items.folder = menu_item;
	// Summary Items - Category
	menu_item = gtk_check_menu_item_new_with_mnemonic (_("Category"));
	gtk_check_menu_item_set_active ((GtkCheckMenuItem*) menu_item, TRUE);
	gtk_menu_shell_append ((GtkMenuShell*) submenu, menu_item);
	menubar->view.summary_items.category = menu_item;
	// Summary Items - Elapsed
//	menu_item = gtk_check_menu_item_new_with_mnemonic (_("_Elapsed"));
//	gtk_check_menu_item_set_active ((GtkCheckMenuItem*) menu_item, TRUE);
//	gtk_menu_shell_append ((GtkMenuShell*) submenu, menu_item);
//	menubar->view.summary_items.elapsed = menu_item;
	// Summary Items - URL
	menu_item = gtk_check_menu_item_new_with_mnemonic (_("_URL"));
	gtk_check_menu_item_set_active ((GtkCheckMenuItem*) menu_item, TRUE);
	gtk_menu_shell_append ((GtkMenuShell*) submenu, menu_item);
	menubar->view.summary_items.uri = menu_item;
	// Summary Items - Message
	menu_item = gtk_check_menu_item_new_with_mnemonic (_("_Message"));
	gtk_check_menu_item_set_active ((GtkCheckMenuItem*) menu_item, TRUE);
	gtk_menu_shell_append ((GtkMenuShell*) submenu, menu_item);
	menubar->view.summary_items.message = menu_item;
	// Summary Items --- end ---

//	gtk_menu_shell_append ((GtkMenuShell*)menu, gtk_separator_menu_item_new() );

//	menu_item = gtk_check_menu_item_new_with_mnemonic (_("Download _Rules Hint"));
//	gtk_menu_shell_append ((GtkMenuShell*)menu, menu_item);
//	gtk_check_menu_item_set_active ((GtkCheckMenuItem*) menu_item, TRUE);
//	menubar->view.rules_hint = menu_item;

	// Download Columns --- start ---
	submenu  = gtk_menu_new ();
	menu_item = gtk_menu_item_new_with_mnemonic (_("Download _Columns"));
	gtk_menu_item_set_submenu ((GtkMenuItem*) menu_item, submenu);
	gtk_menu_shell_append ((GtkMenuShell*)menu, menu_item);
	menubar->view.columns.self = submenu;
	// Download Columns - Complete
	menu_item = gtk_check_menu_item_new_with_mnemonic (_("_Complete"));
	gtk_menu_shell_append ((GtkMenuShell*) submenu, menu_item);
	gtk_check_menu_item_set_active ((GtkCheckMenuItem*) menu_item, TRUE);
	menubar->view.columns.complete = menu_item;
	// Download Columns - Total
	menu_item = gtk_check_menu_item_new_with_mnemonic (_("_Size"));
	gtk_menu_shell_append ((GtkMenuShell*) submenu, menu_item);
	gtk_check_menu_item_set_active ((GtkCheckMenuItem*) menu_item, TRUE);
	menubar->view.columns.total = menu_item;
	// Download Columns - Percent (%)
	menu_item = gtk_check_menu_item_new_with_mnemonic (_("_Percent '%'"));
	gtk_menu_shell_append ((GtkMenuShell*) submenu, menu_item);
	gtk_check_menu_item_set_active ((GtkCheckMenuItem*) menu_item, TRUE);
	menubar->view.columns.percent = menu_item;
	// Download Columns - Elapsed
	menu_item = gtk_check_menu_item_new_with_mnemonic (_("_Elapsed"));
	gtk_menu_shell_append ((GtkMenuShell*) submenu, menu_item);
	gtk_check_menu_item_set_active ((GtkCheckMenuItem*) menu_item, TRUE);
	menubar->view.columns.elapsed = menu_item;
	// Download Columns - Left
	menu_item = gtk_check_menu_item_new_with_mnemonic (_("_Left"));
	gtk_menu_shell_append ((GtkMenuShell*) submenu, menu_item);
	gtk_check_menu_item_set_active ((GtkCheckMenuItem*) menu_item, TRUE);
	menubar->view.columns.left = menu_item;
	// Download Columns - Speed
	menu_item = gtk_check_menu_item_new_with_mnemonic (_("Speed"));
	gtk_menu_shell_append ((GtkMenuShell*) submenu, menu_item);
	gtk_check_menu_item_set_active ((GtkCheckMenuItem*) menu_item, TRUE);
	menubar->view.columns.speed = menu_item;
	// Download Columns - Up Speed
	menu_item = gtk_check_menu_item_new_with_mnemonic (_("Up Speed"));
	gtk_menu_shell_append ((GtkMenuShell*) submenu, menu_item);
	gtk_check_menu_item_set_active ((GtkCheckMenuItem*) menu_item, TRUE);
	menubar->view.columns.upload_speed = menu_item;
	// Download Columns - Uploaded
	menu_item = gtk_check_menu_item_new_with_mnemonic (_("Uploaded"));
	gtk_menu_shell_append ((GtkMenuShell*) submenu, menu_item);
	gtk_check_menu_item_set_active ((GtkCheckMenuItem*) menu_item, TRUE);
	menubar->view.columns.uploaded = menu_item;
	// Download Columns - Ratio
	menu_item = gtk_check_menu_item_new_with_mnemonic (_("Ratio"));
	gtk_menu_shell_append ((GtkMenuShell*) submenu, menu_item);
	gtk_check_menu_item_set_active ((GtkCheckMenuItem*) menu_item, TRUE);
	menubar->view.columns.ratio = menu_item;
	// Download Columns - Retry
	menu_item = gtk_check_menu_item_new_with_mnemonic (_("_Retry"));
	gtk_menu_shell_append ((GtkMenuShell*) submenu, menu_item);
	gtk_check_menu_item_set_active ((GtkCheckMenuItem*) menu_item, TRUE);
	menubar->view.columns.retry = menu_item;
	// Download Columns - Category
	menu_item = gtk_check_menu_item_new_with_mnemonic (_("Category"));
	gtk_menu_shell_append ((GtkMenuShell*) submenu, menu_item);
	gtk_check_menu_item_set_active ((GtkCheckMenuItem*) menu_item, TRUE);
	menubar->view.columns.category = menu_item;
	// Download Columns - URL
	menu_item = gtk_check_menu_item_new_with_mnemonic (_("_URL"));
	gtk_menu_shell_append ((GtkMenuShell*) submenu, menu_item);
	gtk_check_menu_item_set_active ((GtkCheckMenuItem*) menu_item, TRUE);
	menubar->view.columns.uri = menu_item;
	// Download Columns - Added On
	menu_item = gtk_check_menu_item_new_with_mnemonic (_("Added On"));
	gtk_menu_shell_append ((GtkMenuShell*) submenu, menu_item);
	gtk_check_menu_item_set_active ((GtkCheckMenuItem*) menu_item, TRUE);
	menubar->view.columns.added_on = menu_item;
	// Download Columns - Completed On
	menu_item = gtk_check_menu_item_new_with_mnemonic (_("Completed On"));
	gtk_menu_shell_append ((GtkMenuShell*) submenu, menu_item);
	gtk_check_menu_item_set_active ((GtkCheckMenuItem*) menu_item, TRUE);
	menubar->view.columns.completed_on = menu_item;
	// Download Columns --- end ---
}

// UgtkCategoryMenu
static void ugtk_menubar_category_init (UgtkMenubar* menubar)
{
	GtkWidget*  image;
	GtkWidget*  menu;
	GtkWidget*  menu_item;

	menu = gtk_menu_new ();
	menu_item = gtk_menu_item_new_with_mnemonic (_("_Category"));
	gtk_menu_item_set_submenu ((GtkMenuItem*)menu_item, menu);
	gtk_menu_shell_append ((GtkMenuShell*)menubar->self, menu_item);
	menubar->category.self = menu;
	// New Category
	menu_item = gtk_image_menu_item_new_with_mnemonic(_("_New Category..."));
#if GTK_MAJOR_VERSION >= 3 && GTK_MINOR_VERSION >= 10
	image = gtk_image_new_from_icon_name ("gtk-dnd-multiple", GTK_ICON_SIZE_MENU);
#else
	image = gtk_image_new_from_stock (GTK_STOCK_DND_MULTIPLE, GTK_ICON_SIZE_MENU);
#endif
	gtk_image_menu_item_set_image ((GtkImageMenuItem*)menu_item, image);
	gtk_menu_shell_append ((GtkMenuShell*)menu, menu_item);
	menubar->category.create = menu_item;
	// Delete Category
	menu_item = gtk_image_menu_item_new_with_mnemonic(_("_Delete Category"));
#if GTK_MAJOR_VERSION >= 3 && GTK_MINOR_VERSION >= 10
	image = gtk_image_new_from_icon_name ("window-close", GTK_ICON_SIZE_MENU);
#else
	image = gtk_image_new_from_stock (GTK_STOCK_CLOSE, GTK_ICON_SIZE_MENU);
#endif
	gtk_image_menu_item_set_image ((GtkImageMenuItem*)menu_item, image);
	gtk_menu_shell_append ((GtkMenuShell*)menu, menu_item);
	menubar->category.delete = menu_item;
	// Properties
	menu_item = gtk_image_menu_item_new_from_stock (GTK_STOCK_PROPERTIES, NULL);
	gtk_menu_shell_append ((GtkMenuShell*)menu, menu_item);
	menubar->category.properties = menu_item;

	gtk_menu_shell_append ((GtkMenuShell*)menu, gtk_separator_menu_item_new() );
	// Move Up
	menu_item = gtk_image_menu_item_new_from_stock (GTK_STOCK_GO_UP, NULL);
//	menu_item = gtk_image_menu_item_new_with_mnemonic(_("Move _Up"));
//	image = gtk_image_new_from_stock (GTK_STOCK_GO_UP, GTK_ICON_SIZE_MENU);
//	gtk_image_menu_item_set_image ((GtkImageMenuItem*)menu_item, image);
	gtk_menu_shell_append ((GtkMenuShell*)menu, menu_item);
	menubar->category.move_up = menu_item;
	// Move Down
	menu_item = gtk_image_menu_item_new_from_stock (GTK_STOCK_GO_DOWN, NULL);
//	menu_item = gtk_image_menu_item_new_with_mnemonic(_("Move _Down"));
//	image = gtk_image_new_from_stock (GTK_STOCK_GO_DOWN, GTK_ICON_SIZE_MENU);
//	gtk_image_menu_item_set_image ((GtkImageMenuItem*)menu_item, image);
	gtk_menu_shell_append ((GtkMenuShell*)menu, menu_item);
	menubar->category.move_down = menu_item;
}

// UgtkDownloadMenu
static void ugtk_menubar_download_init (UgtkMenubar* menubar, GtkAccelGroup* accel_group)
{
	GtkWidget*  image;
	GtkWidget*  menu;
	GtkWidget*  submenu;
	GtkWidget*  menu_item;

	menu = gtk_menu_new ();
	gtk_menu_set_accel_group ((GtkMenu*)menu, accel_group);
	menu_item = gtk_menu_item_new_with_mnemonic (_("_Download"));
	gtk_menu_item_set_submenu ((GtkMenuItem*)menu_item, menu);
	gtk_menu_shell_append ((GtkMenuShell*)menubar->self, menu_item);
	menubar->download.self = menu;

//	gtk_menu_shell_append((GtkMenuShell*)menu, gtk_tearoff_menu_item_new() );

	menu_item = gtk_image_menu_item_new_from_stock (GTK_STOCK_NEW, accel_group);
	gtk_menu_shell_append ((GtkMenuShell*)menu, menu_item);
	menubar->download.create = menu_item;

//	menu_item = gtk_image_menu_item_new_from_stock (GTK_STOCK_DELETE, accel_group);
	menu_item = gtk_image_menu_item_new_with_mnemonic (_("_Delete Entry"));
	gtk_menu_item_set_accel_path ((GtkMenuItem*) menu_item, UGTK_APP_ACCEL_PATH_DELETE);
#if GTK_MAJOR_VERSION >= 3 && GTK_MINOR_VERSION >= 10
	image = gtk_image_new_from_icon_name ("edit-delete", GTK_ICON_SIZE_MENU);
#else
	image = gtk_image_new_from_stock (GTK_STOCK_DELETE, GTK_ICON_SIZE_MENU);
#endif
	gtk_image_menu_item_set_image ((GtkImageMenuItem*)menu_item, image);
	gtk_menu_shell_append ((GtkMenuShell*)menu, menu_item);
	menubar->download.delete = menu_item;

	menu_item = gtk_image_menu_item_new_with_mnemonic (_("Delete Entry and _File"));
	gtk_menu_item_set_accel_path ((GtkMenuItem*) menu_item, UGTK_APP_ACCEL_PATH_DELETE_F);
//	image = gtk_image_new_from_stock (GTK_STOCK_DELETE, GTK_ICON_SIZE_MENU);
//	gtk_image_menu_item_set_image ((GtkImageMenuItem*)menu_item, image);
	gtk_menu_shell_append ((GtkMenuShell*)menu, menu_item);
	menubar->download.delete_file = menu_item;

	gtk_menu_shell_append ((GtkMenuShell*)menu, gtk_separator_menu_item_new() );

	menu_item = gtk_image_menu_item_new_from_stock (GTK_STOCK_OPEN, NULL);
	gtk_menu_item_set_accel_path ((GtkMenuItem*) menu_item, UGTK_APP_ACCEL_PATH_OPEN);
	gtk_menu_shell_append ((GtkMenuShell*)menu, menu_item);
	menubar->download.open = menu_item;
	gtk_widget_hide (menu_item);

	menu_item = gtk_image_menu_item_new_with_mnemonic(_("Open _Containing folder"));
	gtk_menu_item_set_accel_path ((GtkMenuItem*) menu_item, UGTK_APP_ACCEL_PATH_OPEN_F);
#if GTK_MAJOR_VERSION >= 3 && GTK_MINOR_VERSION >= 10
	image = gtk_image_new_from_icon_name ("folder", GTK_ICON_SIZE_MENU);
#else
	image = gtk_image_new_from_stock (GTK_STOCK_DIRECTORY, GTK_ICON_SIZE_MENU);
#endif
	gtk_image_menu_item_set_image ((GtkImageMenuItem*)menu_item, image);
	gtk_menu_shell_append ((GtkMenuShell*)menu, menu_item);
	menubar->download.open_folder = menu_item;

	gtk_menu_shell_append ((GtkMenuShell*)menu, gtk_separator_menu_item_new() );

	menu_item = gtk_image_menu_item_new_with_mnemonic(_("Force Start"));
//	image = gtk_image_new_from_stock (GTK_STOCK_MEDIA_PLAY, GTK_ICON_SIZE_MENU);
//	gtk_image_menu_item_set_image ((GtkImageMenuItem*)menu_item, image);
	gtk_menu_shell_append ((GtkMenuShell*)menu, menu_item);
	menubar->download.force_start = menu_item;

	menu_item = gtk_image_menu_item_new_with_mnemonic(_("_Runnable"));
	gtk_menu_item_set_accel_path ((GtkMenuItem*) menu_item, UGTK_APP_ACCEL_PATH_SWITCH);
#if GTK_MAJOR_VERSION >= 3 && GTK_MINOR_VERSION >= 10
	image = gtk_image_new_from_icon_name ("media-playback-start", GTK_ICON_SIZE_MENU);
#else
	image = gtk_image_new_from_stock (GTK_STOCK_MEDIA_PLAY, GTK_ICON_SIZE_MENU);
#endif
	gtk_image_menu_item_set_image ((GtkImageMenuItem*)menu_item, image);
	gtk_menu_shell_append ((GtkMenuShell*)menu, menu_item);
	menubar->download.runnable = menu_item;

	menu_item = gtk_image_menu_item_new_from_stock (GTK_STOCK_MEDIA_PAUSE, NULL);
	gtk_menu_item_set_accel_path ((GtkMenuItem*) menu_item, UGTK_APP_ACCEL_PATH_SWITCH);
//	menu_item = gtk_image_menu_item_new_with_mnemonic(_("P_ause"));
//	image = gtk_image_new_from_stock (GTK_STOCK_MEDIA_PAUSE, GTK_ICON_SIZE_MENU);
//	gtk_image_menu_item_set_image ((GtkImageMenuItem*)menu_item, image);
	gtk_menu_shell_append ((GtkMenuShell*)menu, menu_item);
	menubar->download.pause = menu_item;

	gtk_menu_shell_append ((GtkMenuShell*)menu, gtk_separator_menu_item_new() );

	// Move to --- start ---
	menu_item = gtk_image_menu_item_new_with_mnemonic(_("_Move To"));
#if GTK_MAJOR_VERSION >= 3 && GTK_MINOR_VERSION >= 10
	image = gtk_image_new_from_icon_name ("gtk-dnd-multiple", GTK_ICON_SIZE_MENU);
#else
	image = gtk_image_new_from_stock (GTK_STOCK_DND_MULTIPLE, GTK_ICON_SIZE_MENU);
#endif
	gtk_image_menu_item_set_image ((GtkImageMenuItem*)menu_item, image);
	gtk_menu_shell_append ((GtkMenuShell*)menu, menu_item);
	menubar->download.move_to.item = menu_item;
	// Move to - submenu
	submenu = gtk_menu_new ();
	gtk_menu_item_set_submenu ((GtkMenuItem*)menu_item, submenu);
	menubar->download.move_to.self = submenu;
	menubar->download.move_to.array = g_ptr_array_sized_new (16*2);
	// Move to --- end ---

	gtk_menu_shell_append ((GtkMenuShell*)menu, gtk_separator_menu_item_new() );

	menu_item = gtk_image_menu_item_new_from_stock (GTK_STOCK_GO_UP, NULL);
//	menu_item = gtk_image_menu_item_new_with_mnemonic(_("Move _Up"));
//	image = gtk_image_new_from_stock (GTK_STOCK_GO_UP, GTK_ICON_SIZE_MENU);
//	gtk_image_menu_item_set_image ((GtkImageMenuItem*)menu_item, image);
	gtk_menu_shell_append ((GtkMenuShell*)menu, menu_item);
	menubar->download.move_up = menu_item;

	menu_item = gtk_image_menu_item_new_from_stock (GTK_STOCK_GO_DOWN, NULL);
//	menu_item = gtk_image_menu_item_new_with_mnemonic(_("Move _Down"));
//	image = gtk_image_new_from_stock (GTK_STOCK_GO_DOWN, GTK_ICON_SIZE_MENU);
//	gtk_image_menu_item_set_image ((GtkImageMenuItem*)menu_item, image);
	gtk_menu_shell_append ((GtkMenuShell*)menu, menu_item);
	menubar->download.move_down = menu_item;

	menu_item = gtk_image_menu_item_new_from_stock (GTK_STOCK_GOTO_TOP, NULL);
//	menu_item = gtk_image_menu_item_new_with_mnemonic(_("Move _Top"));
//	image = gtk_image_new_from_stock (GTK_STOCK_GOTO_TOP, GTK_ICON_SIZE_MENU);
//	gtk_image_menu_item_set_image ((GtkImageMenuItem*)menu_item, image);
	gtk_menu_shell_append ((GtkMenuShell*)menu, menu_item);
	menubar->download.move_top = menu_item;

	menu_item = gtk_image_menu_item_new_from_stock (GTK_STOCK_GOTO_BOTTOM, NULL);
//	menu_item = gtk_image_menu_item_new_with_mnemonic(_("Move _Bottom"));
//	image = gtk_image_new_from_stock (GTK_STOCK_GOTO_BOTTOM, GTK_ICON_SIZE_MENU);
//	gtk_image_menu_item_set_image ((GtkImageMenuItem*)menu_item, image);
	gtk_menu_shell_append ((GtkMenuShell*)menu, menu_item);
	menubar->download.move_bottom = menu_item;

	gtk_menu_shell_append ((GtkMenuShell*)menu, gtk_separator_menu_item_new() );

	// Priority --- start ---
	menu_item = gtk_image_menu_item_new_with_mnemonic(_("Priority"));
	gtk_menu_shell_append ((GtkMenuShell*)menu, menu_item);
	menubar->download.prioriy.item = menu_item;
	// Priority - submenu
	submenu = gtk_menu_new ();
	gtk_menu_item_set_submenu ((GtkMenuItem*)menu_item, submenu);
	menubar->download.prioriy.self = submenu;
	// Priority - High
	menu_item = gtk_radio_menu_item_new_with_mnemonic (
			NULL, _("_High"));
	gtk_menu_shell_append ((GtkMenuShell*)submenu, menu_item);
	menubar->download.prioriy.high = menu_item;
	// Priority - Normal
	menu_item = gtk_radio_menu_item_new_with_mnemonic_from_widget (
			(GtkRadioMenuItem*) menu_item, _("_Normal"));
	gtk_menu_shell_append ((GtkMenuShell*)submenu, menu_item);
	menubar->download.prioriy.normal = menu_item;
	// Priority - Low
	menu_item = gtk_radio_menu_item_new_with_mnemonic_from_widget (
			(GtkRadioMenuItem*) menu_item, _("_Low"));
	gtk_menu_shell_append ((GtkMenuShell*)submenu, menu_item);
	menubar->download.prioriy.low = menu_item;
	// Priority --- end ---

	menu_item = gtk_image_menu_item_new_from_stock (GTK_STOCK_PROPERTIES, NULL);
	gtk_menu_shell_append ((GtkMenuShell*)menu, menu_item);
	menubar->download.properties = menu_item;
}

// UgtkHelpMenu
static void ugtk_menubar_help_init (UgtkMenubar* menubar)
{
	GtkWidget*  image;
	GtkWidget*  menu;
	GtkWidget*  menu_item;

	menu = gtk_menu_new ();
	menu_item = gtk_menu_item_new_with_mnemonic(_("_Help"));
	gtk_menu_item_set_submenu ((GtkMenuItem*)menu_item, menu);
	gtk_menu_shell_append ((GtkMenuShell*)menubar->self, menu_item);

	// Get Help Online
	menu_item = gtk_image_menu_item_new_with_mnemonic(_("Get Help Online"));
#if GTK_MAJOR_VERSION >= 3 && GTK_MINOR_VERSION >= 10
	image = gtk_image_new_from_icon_name ("help-browser", GTK_ICON_SIZE_MENU);
#else
	image = gtk_image_new_from_stock (GTK_STOCK_HELP, GTK_ICON_SIZE_MENU);
#endif
	gtk_image_menu_item_set_image ((GtkImageMenuItem*)menu_item, image);
	gtk_menu_shell_append ((GtkMenuShell*)menu, menu_item);
	menubar->help.help_online = menu_item;

	// Documentation
	menu_item = gtk_image_menu_item_new_with_mnemonic(_("Documentation"));
#if GTK_MAJOR_VERSION >= 3 && GTK_MINOR_VERSION >= 10
	image = gtk_image_new_from_icon_name ("text-x-generic", GTK_ICON_SIZE_MENU);
#else
	image = gtk_image_new_from_stock (GTK_STOCK_FILE, GTK_ICON_SIZE_MENU);
#endif
	gtk_image_menu_item_set_image ((GtkImageMenuItem*)menu_item, image);
	gtk_menu_shell_append ((GtkMenuShell*)menu, menu_item);
	menubar->help.documentation = menu_item;

	// Support Forum
	menu_item = gtk_image_menu_item_new_with_mnemonic(_("Support Forum"));
#if GTK_MAJOR_VERSION >= 3 && GTK_MINOR_VERSION >= 10
	image = gtk_image_new_from_icon_name ("dialog-question", GTK_ICON_SIZE_MENU);
#else
	image = gtk_image_new_from_stock (GTK_STOCK_DIALOG_QUESTION, GTK_ICON_SIZE_MENU);
#endif
	gtk_image_menu_item_set_image ((GtkImageMenuItem*)menu_item, image);
	gtk_menu_shell_append ((GtkMenuShell*)menu, menu_item);
	menubar->help.support_forum = menu_item;

	// Submit Feedback
	menu_item = gtk_image_menu_item_new_with_mnemonic(_("Submit Feedback"));
#if GTK_MAJOR_VERSION >= 3 && GTK_MINOR_VERSION >= 10
	image = gtk_image_new_from_icon_name ("list-add", GTK_ICON_SIZE_MENU);
#else
	image = gtk_image_new_from_stock (GTK_STOCK_ADD, GTK_ICON_SIZE_MENU);
#endif
	gtk_image_menu_item_set_image ((GtkImageMenuItem*)menu_item, image);
	gtk_menu_shell_append ((GtkMenuShell*)menu, menu_item);
	menubar->help.submit_feedback = menu_item;

	// Report a Bug
	menu_item = gtk_image_menu_item_new_with_mnemonic(_("Report a Bug"));
#if GTK_MAJOR_VERSION >= 3 && GTK_MINOR_VERSION >= 10
	image = gtk_image_new_from_icon_name ("dialog-warning-symbolic", GTK_ICON_SIZE_MENU);
#else
	image = gtk_image_new_from_stock (GTK_STOCK_CAPS_LOCK_WARNING, GTK_ICON_SIZE_MENU);
#endif
	gtk_image_menu_item_set_image ((GtkImageMenuItem*)menu_item, image);
	gtk_menu_shell_append ((GtkMenuShell*)menu, menu_item);
	menubar->help.report_bug = menu_item;

	// Keyboard Shortcuts
	menu_item = gtk_image_menu_item_new_with_mnemonic(_("Keyboard Shortcuts"));
//	image = gtk_image_new_from_stock (GTK_STOCK_DND_MULTIPLE, GTK_ICON_SIZE_MENU);
//	gtk_image_menu_item_set_image ((GtkImageMenuItem*)menu_item, image);
	gtk_menu_shell_append ((GtkMenuShell*)menu, menu_item);
	menubar->help.keyboard_shortcuts = menu_item;

	// Check for Updates
	menu_item = gtk_image_menu_item_new_with_mnemonic(_("Check for Updates"));
//	image = gtk_image_new_from_stock (GTK_STOCK_DND_MULTIPLE, GTK_ICON_SIZE_MENU);
//	gtk_image_menu_item_set_image ((GtkImageMenuItem*)menu_item, image);
	gtk_menu_shell_append ((GtkMenuShell*)menu, menu_item);
	menubar->help.check_updates = menu_item;

	// About Uget
	menu_item = gtk_image_menu_item_new_from_stock (GTK_STOCK_ABOUT, NULL);
	gtk_menu_shell_append ((GtkMenuShell*)menu, menu_item);
	menubar->help.about_uget = menu_item;
}

void ugtk_menubar_init_ui (UgtkMenubar* menubar, GtkAccelGroup* accel_group)
{
	menubar->self = gtk_menu_bar_new ();
	ugtk_menubar_file_init (menubar, accel_group);
	ugtk_menubar_edit_init (menubar);
	ugtk_menubar_view_init (menubar);
	ugtk_menubar_category_init (menubar);
	ugtk_menubar_download_init (menubar, accel_group);
	ugtk_menubar_help_init (menubar);
}

