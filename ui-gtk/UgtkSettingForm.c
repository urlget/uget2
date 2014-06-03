/*
 *
 *   Copyright (C) 2005-2014 by C.H. Huang
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
#include <UgtkSettingForm.h>

#include <glib/gi18n.h>

static void remove_line_breaks (gchar* buffer, gint len);

// ----------------------------------------------------------------------------
// UgtkClipboardForm
//
void  ugtk_clipboard_form_init (struct UgtkClipboardForm* csform)
{
	GtkTextView* textview;
	GtkWidget*   widget;
	GtkBox*      vbox;
	GtkBox*      hbox;

	csform->self = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
	vbox = (GtkBox*) csform->self;
	// Monitor button
	widget = gtk_check_button_new_with_mnemonic (_("_Enable clipboard monitor"));
	gtk_box_pack_start (vbox, widget, FALSE, FALSE, 1);
	csform->monitor = (GtkToggleButton*) widget;

	// quiet mode
	widget = gtk_check_button_new_with_mnemonic (_("_Quiet mode"));
	gtk_box_pack_start (vbox, widget, FALSE, FALSE, 0);
	csform->quiet = (GtkToggleButton*) widget;
	// Nth category
	hbox = (GtkBox*) gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 2);
	gtk_box_pack_start (vbox, (GtkWidget*) hbox, FALSE, FALSE, 2);
	widget = gtk_label_new (_("Default category index"));
	gtk_box_pack_start (hbox, widget, FALSE, FALSE, 2);
	csform->nth_label = widget;
	widget = gtk_spin_button_new_with_range (0.0, 1000.0, 1.0);
	gtk_entry_set_activates_default (GTK_ENTRY (widget), TRUE);
	gtk_box_pack_start (hbox, widget, FALSE, FALSE, 2);
	csform->nth_spin = (GtkSpinButton*) widget;
	// hint
	hbox = (GtkBox*) gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 2);
	gtk_box_pack_start (vbox, (GtkWidget*) hbox, FALSE, FALSE, 2);
	widget = gtk_label_new (" - ");
	gtk_box_pack_start (hbox, widget, FALSE, FALSE, 0);
	widget = gtk_label_new (_("Adding to Nth category if no matched category."));
	gtk_box_pack_start (hbox, widget, FALSE, FALSE, 0);

	gtk_box_pack_start (vbox, gtk_label_new (""), FALSE, FALSE, 2);

	hbox = (GtkBox*) gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 2);
	gtk_box_pack_start (vbox, (GtkWidget*) hbox, FALSE, FALSE, 2);
	widget = gtk_label_new (_("Monitor clipboard for specified file types:"));
	gtk_box_pack_start (hbox, widget, FALSE, FALSE, 1);
	// file type pattern : TextView
	csform->buffer = gtk_text_buffer_new (NULL);
	csform->pattern = gtk_text_view_new_with_buffer (csform->buffer);
	g_object_unref (csform->buffer);
	gtk_widget_set_size_request (csform->pattern, 100, 100);
	textview = (GtkTextView*) csform->pattern;
	gtk_text_view_set_wrap_mode (textview, GTK_WRAP_WORD_CHAR);
	gtk_box_pack_start (vbox, csform->pattern, FALSE, FALSE, 2);

	// tips
	hbox = (GtkBox*) gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 2);
	gtk_box_pack_start (vbox, (GtkWidget*) hbox, FALSE, FALSE, 1);
	gtk_box_pack_end (hbox,
			gtk_label_new (_("Separate the types with character '|'.")),
			FALSE, FALSE, 2);
	hbox = (GtkBox*) gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 2);
	gtk_box_pack_start (vbox, (GtkWidget*) hbox, FALSE, FALSE, 1);
	gtk_box_pack_end (hbox,
			gtk_label_new (_("You can use regular expressions here.")),
			FALSE, FALSE, 2);
}

void  ugtk_clipboard_form_set (struct UgtkClipboardForm* csform, UgtkSetting* setting)
{
	if (setting->clipboard.pattern)
		gtk_text_buffer_set_text (csform->buffer, setting->clipboard.pattern, -1);
	gtk_toggle_button_set_active (csform->monitor, setting->clipboard.monitor);
	gtk_toggle_button_set_active (csform->quiet, setting->clipboard.quiet);
	gtk_spin_button_set_value (csform->nth_spin, setting->clipboard.nth_category);
	gtk_toggle_button_toggled (csform->monitor);
	gtk_toggle_button_toggled (csform->quiet);
//	on_clipboard_quiet_mode_toggled ((GtkWidget*) csform->quiet, csform);
}

void  ugtk_clipboard_form_get (struct UgtkClipboardForm* csform, UgtkSetting* setting)
{
	GtkTextIter  iter1;
	GtkTextIter  iter2;

	gtk_text_buffer_get_start_iter (csform->buffer, &iter1);
	gtk_text_buffer_get_end_iter (csform->buffer, &iter2);

	ug_free (setting->clipboard.pattern);
	setting->clipboard.pattern = gtk_text_buffer_get_text (csform->buffer, &iter1, &iter2, FALSE);
	setting->clipboard.monitor = gtk_toggle_button_get_active (csform->monitor);
	setting->clipboard.quiet = gtk_toggle_button_get_active (csform->quiet);
	setting->clipboard.nth_category = gtk_spin_button_get_value_as_int (csform->nth_spin);
	// remove line break
	remove_line_breaks (setting->clipboard.pattern, -1);
}

// ----------------------------------------------------------------------------
// UgtkUserInterfaceForm
//
void  ugtk_user_interface_form_init (struct UgtkUserInterfaceForm* uiform)
{
	GtkWidget*  widget;
	GtkBox*     vbox;

	uiform->self = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
	// Confirmation
	vbox = (GtkBox*) uiform->self;
	widget = gtk_frame_new (_("Confirmation"));
	gtk_box_pack_start (vbox, widget, FALSE, FALSE, 1);
	vbox = (GtkBox*) gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
	gtk_container_add (GTK_CONTAINER (widget), (GtkWidget*) vbox);
	gtk_container_set_border_width (GTK_CONTAINER (vbox), 2);
	// Confirmation check buttons
	widget = gtk_check_button_new_with_label (_("Show confirmation dialog on exit"));
	uiform->confirm_exit = (GtkToggleButton*) widget;
	gtk_box_pack_start (vbox, widget, FALSE, FALSE, 1);
	widget = gtk_check_button_new_with_label (_("Confirm when deleting files"));
	uiform->confirm_delete = (GtkToggleButton*) widget;
	gtk_box_pack_start (vbox, widget, FALSE, FALSE, 1);

	// System Tray
	vbox = (GtkBox*) uiform->self;
	widget = gtk_frame_new (_("System Tray"));
	gtk_box_pack_start (vbox, widget, FALSE, FALSE, 1);
	vbox = (GtkBox*) gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
	gtk_container_add (GTK_CONTAINER (widget), (GtkWidget*) vbox);
	gtk_container_set_border_width (GTK_CONTAINER (vbox), 2);
	// System Tray check buttons
	widget = gtk_check_button_new_with_label (_("Always show tray icon"));
	uiform->show_trayicon = (GtkToggleButton*) widget;
	gtk_box_pack_start (vbox, widget, FALSE, FALSE, 1);
	widget = gtk_check_button_new_with_label (_("Minimize to tray on startup"));
	uiform->start_in_tray = (GtkToggleButton*) widget;
	gtk_box_pack_start (vbox, widget, FALSE, FALSE, 1);
	widget = gtk_check_button_new_with_label (_("Close to tray on window close"));
	uiform->close_to_tray = (GtkToggleButton*) widget;
	gtk_box_pack_start (vbox, widget, FALSE, FALSE, 1);
#ifdef HAVE_APP_INDICATOR
	widget = gtk_check_button_new_with_label (_("Use Ubuntu's App Indicator"));
	uiform->app_indicator = (GtkToggleButton*) widget;
	gtk_box_pack_start (vbox, widget, FALSE, FALSE, 1);
#endif

	// Others
	vbox = (GtkBox*) uiform->self;
	widget = gtk_check_button_new_with_label (_("Enable offline mode on startup"));
	uiform->start_in_offline_mode = (GtkToggleButton*) widget;
	gtk_box_pack_start (vbox, widget, FALSE, FALSE, 1);
	widget = gtk_check_button_new_with_label (_("Download starting notification"));
	uiform->start_notification = (GtkToggleButton*) widget;
	gtk_box_pack_start (vbox, widget, FALSE, FALSE, 1);
	widget = gtk_check_button_new_with_label (_("Sound when download is finished"));
	uiform->sound_notification = (GtkToggleButton*) widget;
	gtk_box_pack_start (vbox, widget, FALSE, FALSE, 1);
	widget = gtk_check_button_new_with_label (_("Apply recently download settings"));
	uiform->apply_recently = (GtkToggleButton*) widget;
	gtk_box_pack_start (vbox, widget, FALSE, FALSE, 1);
//	widget = gtk_check_button_new_with_label (_("Skip existing URI from clipboard and command-line"));
//	uiform->skip_existing = (GtkToggleButton*) widget;
//	gtk_box_pack_start (vbox, widget, FALSE, FALSE, 1);
}

void  ugtk_user_interface_form_set (struct UgtkUserInterfaceForm* uiform, UgtkSetting* setting)
{
	gtk_toggle_button_set_active (uiform->close_to_tray,
			setting->ui.close_to_tray);
	gtk_toggle_button_set_active (uiform->confirm_exit,
			setting->ui.exit_confirmation);
	gtk_toggle_button_set_active (uiform->confirm_delete,
			setting->ui.delete_confirmation);
	gtk_toggle_button_set_active (uiform->show_trayicon,
			setting->ui.show_trayicon);
	gtk_toggle_button_set_active (uiform->start_in_tray,
			setting->ui.start_in_tray);
	gtk_toggle_button_set_active (uiform->start_in_offline_mode,
			setting->ui.start_in_offline_mode);
	gtk_toggle_button_set_active (uiform->start_notification,
			setting->ui.start_notification);
	gtk_toggle_button_set_active (uiform->sound_notification,
			setting->ui.sound_notification);
	gtk_toggle_button_set_active (uiform->apply_recently,
			setting->ui.apply_recently);
//	gtk_toggle_button_set_active (uiform->skip_existing,
//			setting->ui.skip_existing);
#ifdef HAVE_APP_INDICATOR
	gtk_toggle_button_set_active (uiform->app_indicator,
			setting->ui.app_indicator);
#endif
}

void  ugtk_user_interface_form_get (struct UgtkUserInterfaceForm* uiform, UgtkSetting* setting)
{
	setting->ui.close_to_tray = gtk_toggle_button_get_active (uiform->close_to_tray);
	setting->ui.exit_confirmation = gtk_toggle_button_get_active (uiform->confirm_exit);
	setting->ui.delete_confirmation = gtk_toggle_button_get_active (uiform->confirm_delete);
	setting->ui.show_trayicon = gtk_toggle_button_get_active (uiform->show_trayicon);
	setting->ui.start_in_tray = gtk_toggle_button_get_active (uiform->start_in_tray);
	setting->ui.start_in_offline_mode = gtk_toggle_button_get_active (uiform->start_in_offline_mode);
	setting->ui.start_notification = gtk_toggle_button_get_active (uiform->start_notification);
	setting->ui.sound_notification = gtk_toggle_button_get_active (uiform->sound_notification);
	setting->ui.apply_recently = gtk_toggle_button_get_active (uiform->apply_recently);
//	setting->ui.skip_existing = gtk_toggle_button_get_active (uiform->skip_existing);
#ifdef HAVE_APP_INDICATOR
	setting->ui.app_indicator = gtk_toggle_button_get_active (uiform->app_indicator);
#endif
}

// ----------------------------------------------------------------------------
// UgtkBandwidthForm
//

void  ugtk_bandwidth_form_init (struct UgtkBandwidthForm* bwform)
{
	GtkWidget*  widget;
	GtkBox*     box;
	GtkBox*     vbox;
	GtkBox*     hbox;

	box = (GtkBox*) gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
	bwform->self = (GtkWidget*) box;
	widget = gtk_label_new (_("These will affect all plug-ins."));
	gtk_box_pack_start (box, widget, FALSE, FALSE, 2);
	widget = gtk_label_new ("");
	gtk_box_pack_start (box, widget, FALSE, FALSE, 2);

	// Global speed limit
	widget = gtk_frame_new (_("Global speed limit"));
	vbox = (GtkBox*) gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
	gtk_container_add (GTK_CONTAINER (widget), (GtkWidget*) vbox);
	gtk_container_set_border_width (GTK_CONTAINER (vbox), 2);
	gtk_box_pack_start (box, widget, FALSE, FALSE, 2);
	// Max upload speed
	hbox = (GtkBox*) gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 2);
	gtk_box_pack_start (vbox, (GtkWidget*) hbox, FALSE, FALSE, 2);
	widget = gtk_label_new (_("Max upload speed"));
	gtk_box_pack_start (hbox, widget, FALSE, FALSE, 2);
	widget = gtk_label_new (_("KiB/s"));
	gtk_box_pack_end (hbox, widget, FALSE, FALSE, 2);
	widget = gtk_spin_button_new_with_range (0.0, 4000000000.0, 5.0);
	gtk_entry_set_width_chars (GTK_ENTRY (widget), 15);
	gtk_entry_set_activates_default (GTK_ENTRY (widget), TRUE);
	gtk_box_pack_end (hbox, widget, FALSE, FALSE, 2);
	bwform->upload = (GtkSpinButton*) widget;
	// Max download speed
	hbox = (GtkBox*) gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 2);
	gtk_box_pack_start (vbox, (GtkWidget*) hbox, FALSE, FALSE, 2);
	widget = gtk_label_new (_("Max download speed"));
	gtk_box_pack_start (hbox, widget, FALSE, FALSE, 2);
	widget = gtk_label_new (_("KiB/s"));
	gtk_box_pack_end (hbox, widget, FALSE, FALSE, 2);
	widget = gtk_spin_button_new_with_range (0.0, 4000000000.0, 5.0);
	gtk_entry_set_width_chars (GTK_ENTRY (widget), 15);
	gtk_entry_set_activates_default (GTK_ENTRY (widget), TRUE);
	gtk_box_pack_end (hbox, widget, FALSE, FALSE, 2);
	bwform->download = (GtkSpinButton*) widget;
}

void  ugtk_bandwidth_form_set (struct UgtkBandwidthForm* bwform, UgtkSetting* setting)
{
	gtk_spin_button_set_value (bwform->upload, setting->bandwidth.normal.upload);
	gtk_spin_button_set_value (bwform->download, setting->bandwidth.normal.download);
}

void  ugtk_bandwidth_form_get (struct UgtkBandwidthForm* bwform, UgtkSetting* setting)
{
	setting->bandwidth.normal.upload   = (guint) gtk_spin_button_get_value (bwform->upload);
	setting->bandwidth.normal.download = (guint) gtk_spin_button_get_value (bwform->download);
}

// ----------------------------------------------------------------------------
// UgtkCompletionForm
//
void  ugtk_completion_form_init (struct UgtkCompletionForm* csform)
{
	GtkWidget*  widget;
	GtkWidget*  entry;
	GtkBox*     vbox;
	GtkBox*     hbox;

	widget = gtk_frame_new (_("Completion Auto-Actions"));
	vbox = (GtkBox*) gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
	gtk_container_add (GTK_CONTAINER (widget), (GtkWidget*) vbox);
	gtk_container_set_border_width (GTK_CONTAINER (vbox), 2);
	csform->self = widget;

	hbox = (GtkBox*) gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
	gtk_box_pack_start (vbox, (GtkWidget*) hbox, FALSE, FALSE, 1);
	widget = gtk_label_new (_("Custom command:"));
	gtk_box_pack_start (hbox, widget, FALSE, FALSE, 0);

	entry = gtk_entry_new ();
	gtk_entry_set_activates_default (GTK_ENTRY (entry), TRUE);
	gtk_box_pack_start (vbox, entry, TRUE, TRUE, 2);
	csform->command = (GtkEntry*) entry;

	gtk_box_pack_start (vbox, gtk_label_new (""), FALSE, FALSE, 2);

	hbox = (GtkBox*) gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
	gtk_box_pack_start (vbox, (GtkWidget*) hbox, FALSE, FALSE, 1);
	widget = gtk_label_new (_("Custom command if error occured:"));
	gtk_box_pack_start (hbox, widget, FALSE, FALSE, 0);

	entry = gtk_entry_new ();
	gtk_entry_set_activates_default (GTK_ENTRY (entry), TRUE);
	gtk_box_pack_start (vbox, entry, TRUE, TRUE, 2);
	csform->on_error = (GtkEntry*) entry;
}

void  ugtk_completion_form_set (struct UgtkCompletionForm* csform, UgtkSetting* setting)
{
	if (setting->completion.command)
		gtk_entry_set_text (csform->command, setting->completion.command);
	if (setting->completion.on_error)
		gtk_entry_set_text (csform->command, setting->completion.on_error);
}

void  ugtk_completion_form_get (struct UgtkCompletionForm* csform, UgtkSetting* setting)
{
	ug_free (setting->completion.command);
	setting->completion.command = ug_strdup (gtk_entry_get_text (csform->command));
	ug_free (setting->completion.on_error);
	setting->completion.on_error = ug_strdup (gtk_entry_get_text (csform->on_error));
}

// ----------------------------------------------------------------------------
// UgtkAutoSaveForm
//
static void on_auto_save_toggled (GtkWidget* widget, struct UgtkAutoSaveForm* asform)
{
	gboolean  sensitive;

	sensitive = gtk_toggle_button_get_active (asform->enable);
	gtk_widget_set_sensitive (asform->interval_label, sensitive);
	gtk_widget_set_sensitive ((GtkWidget*) asform->interval_spin, sensitive);
	gtk_widget_set_sensitive (asform->minutes_label, sensitive);
}

void  ugtk_auto_save_form_init (struct UgtkAutoSaveForm* asform)
{
	GtkBox*    hbox;
	GtkWidget* widget;

	asform->self = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
	hbox = (GtkBox*) asform->self;
	widget = gtk_check_button_new_with_mnemonic (_("_Auto save"));
	gtk_box_pack_start (hbox, widget, FALSE, FALSE, 0);
	g_signal_connect (widget, "toggled",
			G_CALLBACK (on_auto_save_toggled), asform);
	asform->enable = (GtkToggleButton*) widget;
	// auto save spin & label (interval)
	widget = gtk_label_new_with_mnemonic (_("minutes"));
	gtk_box_pack_end (hbox, widget, FALSE, FALSE, 2);
	asform->minutes_label = widget;
	widget = gtk_spin_button_new_with_range (1.0, 120.0, 1.0);
	gtk_entry_set_activates_default (GTK_ENTRY (widget), TRUE);
	gtk_box_pack_end (hbox, widget, FALSE, FALSE, 2);
	asform->interval_spin = (GtkSpinButton*) widget;
	// auto save label
	widget = gtk_label_new_with_mnemonic (_("_Interval:"));
	gtk_box_pack_end (hbox, widget, FALSE, FALSE, 2);
	asform->interval_label = widget;
	gtk_label_set_mnemonic_widget (GTK_LABEL (asform->interval_label),
			(GtkWidget*) asform->interval_spin);
}

void  ugtk_auto_save_form_set (struct UgtkAutoSaveForm* asform, UgtkSetting* setting)
{
	gtk_toggle_button_set_active (asform->enable, setting->auto_save.enable);
	gtk_spin_button_set_value (asform->interval_spin, (gdouble) setting->auto_save.interval);
	gtk_toggle_button_toggled (asform->enable);
//	on_auto_save_toggled ((GtkWidget*) asform->enable, asform);
}

void  ugtk_auto_save_form_get (struct UgtkAutoSaveForm* asform, UgtkSetting* setting)
{
	setting->auto_save.enable = gtk_toggle_button_get_active (asform->enable);
	setting->auto_save.interval = gtk_spin_button_get_value_as_int (asform->interval_spin);
}

// ----------------------------------------------------------------------------
// UgtkCommandlineForm
//
void  ugtk_commandline_form_init (struct UgtkCommandlineForm* csform)
{
	GtkWidget*  widget;
	GtkBox*     vbox;
	GtkBox*     hbox;

	// Commandline Settings
	widget = gtk_frame_new (_("Commandline Settings"));
	csform->self = widget;
	vbox = (GtkBox*) gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
	gtk_container_add (GTK_CONTAINER (widget), (GtkWidget*) vbox);
	gtk_container_set_border_width (GTK_CONTAINER (vbox), 2);

	// --quiet
	widget = gtk_check_button_new_with_mnemonic (_("Use '--quiet' by default"));
	gtk_box_pack_start (vbox, widget, FALSE, FALSE, 0);
	csform->quiet = (GtkToggleButton*) widget;
	// --category-index
	hbox = (GtkBox*) gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 2);
	gtk_box_pack_start (vbox, (GtkWidget*) hbox, FALSE, FALSE, 2);
	widget = gtk_label_new (_("Default category index"));
	gtk_box_pack_start (hbox, widget, FALSE, FALSE, 2);
	csform->index_label = widget;
	widget = gtk_spin_button_new_with_range (0.0, 1000.0, 1.0);
	gtk_entry_set_activates_default (GTK_ENTRY (widget), TRUE);
	gtk_box_pack_start (hbox, widget, FALSE, FALSE, 2);
	csform->index_spin = (GtkSpinButton*) widget;
	// hint
	hbox = (GtkBox*) gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 2);
	gtk_box_pack_start (vbox, (GtkWidget*) hbox, FALSE, FALSE, 2);
	widget = gtk_label_new (" - ");
	gtk_box_pack_start (hbox, widget, FALSE, FALSE, 0);
	widget = gtk_label_new (_("Adding to Nth category if no matched category."));
	gtk_box_pack_start (hbox, widget, FALSE, FALSE, 0);
}

void  ugtk_commandline_form_set (struct UgtkCommandlineForm* csform, UgtkSetting* setting)
{
	gtk_toggle_button_set_active (csform->quiet, setting->commandline.quiet);
	gtk_spin_button_set_value (csform->index_spin, setting->commandline.nth_category);
}

void  ugtk_commandline_form_get (struct UgtkCommandlineForm* csform, UgtkSetting* setting)
{
	setting->commandline.quiet = gtk_toggle_button_get_active (csform->quiet);
	setting->commandline.nth_category = gtk_spin_button_get_value_as_int (csform->index_spin);
}

// ----------------------------------------------------------------------------
// UgtkPluginForm
//
static void	on_plugin_launch_toggled (GtkWidget* widget, struct UgtkPluginForm* psform)
{
	gboolean  sensitive;

	sensitive = gtk_toggle_button_get_active (psform->launch);
	gtk_widget_set_sensitive ((GtkWidget*) psform->local, sensitive);
}

static void on_order_changed (GtkComboBox* widget, struct UgtkPluginForm* psform)
{
	int  index;

	index = gtk_combo_box_get_active (widget);
	if (index >= UGTK_PLUGIN_ORDER_ARIA2)
		gtk_widget_set_sensitive (psform->aria2_opts, TRUE);
	else
		gtk_widget_set_sensitive (psform->aria2_opts, FALSE);
}

void  ugtk_plugin_form_init (struct UgtkPluginForm* psform)
{
	GtkBox*      vbox;
	GtkBox*      hbox;
	GtkBox*      box;
	GtkWidget*   widget;

	psform->self = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
	vbox = (GtkBox*) psform->self;

	hbox = (GtkBox*) gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
	gtk_box_pack_start (vbox, (GtkWidget*) hbox, FALSE, TRUE, 2);
	widget = gtk_label_new (_("Plug-in matching order:"));
	gtk_box_pack_start (hbox, widget, FALSE, FALSE, 0);
	widget = gtk_combo_box_text_new ();
	psform->order = (GtkComboBoxText*) widget;
	gtk_combo_box_text_insert_text (psform->order,
			UGTK_PLUGIN_ORDER_CURL, "curl");
	gtk_combo_box_text_insert_text (psform->order,
			UGTK_PLUGIN_ORDER_ARIA2, "aria2");
	gtk_combo_box_text_insert_text (psform->order,
			UGTK_PLUGIN_ORDER_CURL_ARIA2, "curl + aria2");
	gtk_combo_box_text_insert_text (psform->order,
			UGTK_PLUGIN_ORDER_ARIA2_CURL, "aria2 + curl");
	g_signal_connect (psform->order, "changed",
			G_CALLBACK (on_order_changed), psform);
	gtk_box_pack_start (hbox, widget, FALSE, FALSE, 4);

	widget = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
	gtk_box_pack_start (vbox, widget, TRUE, TRUE, 2);
	psform->aria2_opts = widget;
	vbox = (GtkBox*) widget;

	hbox = (GtkBox*) gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
	gtk_box_pack_start (vbox, (GtkWidget*) hbox, FALSE, TRUE, 2);
	widget = gtk_label_new (_("Aria2 plug-in options"));
	gtk_box_pack_start (hbox, widget, FALSE, FALSE, 0);
	gtk_box_pack_start (hbox, gtk_separator_new (GTK_ORIENTATION_HORIZONTAL), TRUE, TRUE, 4);

	// URI entry
	hbox = (GtkBox*) gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
	gtk_box_pack_start (vbox, (GtkWidget*) hbox, FALSE, TRUE, 2);
	widget = gtk_label_new (_("URI"));
	gtk_box_pack_start (hbox, widget, FALSE, FALSE, 2);
	widget = gtk_entry_new ();
	gtk_entry_set_activates_default (GTK_ENTRY (widget), TRUE);
	gtk_box_pack_start (hbox, widget, TRUE,  TRUE,  4);
	psform->uri = (GtkEntry*) widget;
	// Token entry
	hbox = (GtkBox*) gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
	gtk_box_pack_start (vbox, (GtkWidget*) hbox, FALSE, TRUE, 2);
	widget = gtk_label_new (_("RPC authorization secret token"));
	gtk_box_pack_start (hbox, widget, FALSE, FALSE, 2);
	widget = gtk_entry_new ();
	gtk_entry_set_activates_default (GTK_ENTRY (widget), TRUE);
	gtk_box_pack_start (hbox, widget, TRUE,  TRUE,  4);
	psform->token = (GtkEntry*) widget;

	// ------------------------------------------------------------------------
	// Speed Limits
	widget = gtk_frame_new (_("Global speed limit for aria2 only"));
	gtk_box_pack_start (vbox, widget, FALSE, FALSE, 4);
	box = (GtkBox*) gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
	gtk_container_add (GTK_CONTAINER (widget), (GtkWidget*) box);
	gtk_container_set_border_width (GTK_CONTAINER (box), 2);
	// Max upload speed
	hbox = (GtkBox*) gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 2);
	gtk_box_pack_start (box, (GtkWidget*) hbox, FALSE, FALSE, 2);
	widget = gtk_label_new (_("Max upload speed"));
	gtk_box_pack_start (hbox, widget, FALSE, FALSE, 2);
	widget = gtk_label_new (_("KiB/s"));
	gtk_box_pack_end (hbox, widget, FALSE, FALSE, 2);
	widget = gtk_spin_button_new_with_range (0.0, 4000000000.0, 5.0);
	gtk_entry_set_width_chars (GTK_ENTRY (widget), 15);
	gtk_entry_set_activates_default (GTK_ENTRY (widget), TRUE);
	gtk_box_pack_end (hbox, widget, FALSE, FALSE, 2);
	psform->upload = (GtkSpinButton*) widget;
	// Max download speed
	hbox = (GtkBox*) gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 2);
	gtk_box_pack_start (box, (GtkWidget*) hbox, FALSE, FALSE, 2);
	widget = gtk_label_new (_("Max download speed"));
	gtk_box_pack_start (hbox, widget, FALSE, FALSE, 2);
	widget = gtk_label_new (_("KiB/s"));
	gtk_box_pack_end (hbox, widget, FALSE, FALSE, 2);
	widget = gtk_spin_button_new_with_range (0.0, 4000000000.0, 5.0);
	gtk_entry_set_width_chars (GTK_ENTRY (widget), 15);
	gtk_entry_set_activates_default (GTK_ENTRY (widget), TRUE);
	gtk_box_pack_end (hbox, widget, FALSE, FALSE, 2);
	psform->download = (GtkSpinButton*) widget;

	// ------------------------------------------------------------------------
	// aria2 works on local device
	// launch
	widget = gtk_check_button_new_with_mnemonic (_("_Launch aria2 on startup"));
	gtk_box_pack_start (vbox, widget, FALSE, FALSE, 2);
	g_signal_connect (widget, "toggled",
			G_CALLBACK (on_plugin_launch_toggled), psform);
	psform->launch = (GtkToggleButton*) widget;
	// shutdown
	widget = gtk_check_button_new_with_mnemonic (_("_Shutdown aria2 on exit"));
	gtk_box_pack_start (vbox, widget, FALSE, FALSE, 2);
	psform->shutdown = (GtkToggleButton*) widget;

	// ------------------------------------------------------------------------
	// Local options
	widget = gtk_frame_new (_("Launch aria2 on local device"));
	gtk_box_pack_start (vbox, widget, FALSE, FALSE, 4);
	psform->local = widget;
	box = (GtkBox*) gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
	gtk_container_add (GTK_CONTAINER (widget), (GtkWidget*) box);
	gtk_container_set_border_width (GTK_CONTAINER (box), 2);
	// Path
	hbox = (GtkBox*) gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
	gtk_box_pack_start (box, (GtkWidget*) hbox, FALSE, TRUE, 4);
	widget = gtk_label_new (_("Path"));
	gtk_box_pack_start (hbox, widget, FALSE, FALSE, 2);
	widget = gtk_entry_new ();
	gtk_entry_set_activates_default (GTK_ENTRY (widget), TRUE);
	gtk_box_pack_start (hbox, widget, TRUE,  TRUE,  2);
	psform->path = (GtkEntry*) widget;
	//
	gtk_box_pack_start (box, gtk_label_new (""), FALSE, FALSE, 0);
	// Arguments
	hbox = (GtkBox*) gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
	gtk_box_pack_start (box, (GtkWidget*) hbox, FALSE, TRUE, 2);
	widget = gtk_label_new (_("Arguments"));
	gtk_box_pack_start (hbox, widget, FALSE, FALSE, 2);
	widget = gtk_label_new (" - ");
	gtk_box_pack_start (hbox, widget, FALSE, FALSE, 0);
	// Arguments - hint
	widget = gtk_label_new (_("You must restart uGet after modifying it."));
	gtk_box_pack_start (hbox, widget, FALSE, FALSE, 0);
	// Arguments - text view
	psform->args_buffer = gtk_text_buffer_new (NULL);
	psform->args = gtk_text_view_new_with_buffer (psform->args_buffer);
	g_object_unref (psform->args_buffer);
	gtk_widget_set_size_request (psform->args, 100, 50);
	gtk_text_view_set_wrap_mode ((GtkTextView*) psform->args, GTK_WRAP_WORD_CHAR);
	gtk_box_pack_start (box, psform->args, FALSE, TRUE, 2);

	// ------------------------------------------------------------------------
	on_plugin_launch_toggled ((GtkWidget*) psform->launch, psform);
	gtk_widget_show (psform->self);
}

void  ugtk_plugin_form_set (struct UgtkPluginForm* psform, UgtkSetting* setting)
{
	gtk_combo_box_set_active ((GtkComboBox*) psform->order, setting->plugin_order);

	gtk_toggle_button_set_active (psform->launch, setting->aria2.launch);
	gtk_toggle_button_set_active (psform->shutdown, setting->aria2.shutdown);

	if (setting->aria2.uri)
		gtk_entry_set_text (psform->uri,  setting->aria2.uri);
	if (setting->aria2.token)
		gtk_entry_set_text (psform->token,  setting->aria2.token);
	if (setting->aria2.path)
		gtk_entry_set_text (psform->path, setting->aria2.path);
//	if (setting->aria2.args)
//		gtk_entry_set_text (psform->args, setting->aria2.args);
	if (setting->aria2.args)
		gtk_text_buffer_set_text (psform->args_buffer, setting->aria2.args, -1);

	gtk_spin_button_set_value (psform->upload, setting->aria2.limit.upload);
	gtk_spin_button_set_value (psform->download, setting->aria2.limit.download);
}

void  ugtk_plugin_form_get (struct UgtkPluginForm* psform, UgtkSetting* setting)
{
	GtkTextIter  iter1;
	GtkTextIter  iter2;
	const char*  token;

	setting->plugin_order = gtk_combo_box_get_active ((GtkComboBox*) psform->order);

	setting->aria2.launch = gtk_toggle_button_get_active (psform->launch);
	setting->aria2.shutdown = gtk_toggle_button_get_active (psform->shutdown);

	ug_free (setting->aria2.uri);
	ug_free (setting->aria2.token);
	ug_free (setting->aria2.path);
	ug_free (setting->aria2.args);
	setting->aria2.uri = ug_strdup (gtk_entry_get_text (psform->uri));
	token = gtk_entry_get_text (psform->token);
	if (token[0] == 0)
		setting->aria2.token = NULL;
	else
		setting->aria2.token = ug_strdup (token);
	setting->aria2.path = ug_strdup (gtk_entry_get_text (psform->path));
//	setting->aria2.args = ug_strdup (gtk_entry_get_text (psform->args));
	gtk_text_buffer_get_start_iter (psform->args_buffer, &iter1);
	gtk_text_buffer_get_end_iter (psform->args_buffer, &iter2);
	setting->aria2.args = gtk_text_buffer_get_text (psform->args_buffer, &iter1, &iter2, FALSE);
	remove_line_breaks (setting->aria2.args, -1);

	setting->aria2.limit.upload   = (guint) gtk_spin_button_get_value (psform->upload);
	setting->aria2.limit.download = (guint) gtk_spin_button_get_value (psform->download);
}

// ----------------------------------------------------------------------------

static void remove_line_breaks (gchar* buffer, gint len)
{
	gchar*  cur;
	gchar*  end;

	if (len == -1)
		len = strlen (buffer);
	cur = buffer;
	end = buffer + len;
	while ( (cur = strchr (cur, '\n')) ) {
		memmove (cur, cur + 1, end - cur);
		end--;
	}
}
