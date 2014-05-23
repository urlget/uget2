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

#include <UgtkSequencer.h>
#include <UgUri.h>

#include <glib/gi18n.h>

// static functions
static void ugtk_sequencer_preview_init (struct UgtkSequencerPreview* preview);
static void ugtk_sequencer_preview_show (struct UgtkSequencerPreview* preview, const gchar* message);
// signal handlers
static void on_radio1_clicked (GtkWidget* widget, UgtkSequencer* seqer);
static void on_radio2_clicked (GtkWidget* widget, UgtkSequencer* seqer);

void  ugtk_sequencer_init (UgtkSequencer* seqer)
{
	GtkGrid*		grid;
	GtkWidget*		label;
	GtkWidget*		entry;
	GtkWidget*		widget;
	GtkAdjustment*	spin_adj;

	// top widget
	seqer->self = gtk_grid_new ();
	grid = (GtkGrid*) seqer->self;
	gtk_grid_set_row_homogeneous (grid, FALSE);
	// URI entry
	entry = gtk_entry_new ();
	label = gtk_label_new_with_mnemonic (_("_URI:"));
	seqer->entry = GTK_ENTRY (entry);
	gtk_label_set_mnemonic_widget(GTK_LABEL (label), entry);
	gtk_entry_set_activates_default (seqer->entry, TRUE);
	g_object_set (label, "margin", 3, NULL);
	g_object_set (entry, "margin", 3, "hexpand", TRUE, NULL);
	gtk_grid_attach (grid, label, 0, 0, 1, 1);
	gtk_grid_attach (grid, entry, 1, 0, 5, 1);
	g_signal_connect_swapped (GTK_EDITABLE (entry), "changed",
			G_CALLBACK (ugtk_sequencer_update_preview), seqer);
	// e.g.
	label = gtk_label_new (_("e.g."));
	g_object_set (label, "margin", 3, NULL);
	gtk_grid_attach (grid, label, 0, 1, 1, 1);
	label = gtk_label_new ("http://for.example/path/pre*.jpg");
	g_object_set (label, "margin", 3, NULL);
	gtk_grid_attach (grid, label, 1, 1, 5, 1);
	widget = gtk_separator_new (GTK_ORIENTATION_HORIZONTAL);
	gtk_grid_attach (grid, widget, 0, 2, 6, 1);

	// -------------------------------------------------------
	// radio "From"
	seqer->radio = gtk_radio_button_new_with_mnemonic (NULL, _("_From:"));
	g_object_set (seqer->radio, "margin", 3, NULL);
	gtk_grid_attach (grid, seqer->radio, 0, 3, 1, 1);
	g_signal_connect (seqer->radio, "clicked",
			G_CALLBACK (on_radio1_clicked), seqer);
	// spin "From"
	spin_adj = (GtkAdjustment *) gtk_adjustment_new (0.0, 0.0,
				99999.0, 1.0, 5.0, 0.0);
	seqer->spin_from = gtk_spin_button_new (spin_adj, 1.0, 0);
	g_object_set (seqer->spin_from, "margin", 3, "hexpand", TRUE, NULL);
	gtk_grid_attach (grid, seqer->spin_from, 1, 3, 1, 1);
	g_signal_connect_swapped (seqer->spin_from, "value-changed",
			G_CALLBACK (ugtk_sequencer_update_preview), seqer);

	// spin "To"
	spin_adj = (GtkAdjustment *) gtk_adjustment_new (10.0, 1.0,
				99999.0, 1.0, 5.0, 0.0);
	seqer->spin_to = gtk_spin_button_new (spin_adj, 1.0, 0);
	g_object_set (seqer->spin_to, "margin", 3, "hexpand", TRUE, NULL);
	gtk_grid_attach (grid, seqer->spin_to, 3, 3, 1, 1);
	g_signal_connect_swapped (seqer->spin_to, "value-changed",
			G_CALLBACK (ugtk_sequencer_update_preview), seqer);
	// label "To"
	label = gtk_label_new_with_mnemonic (_("To:"));
	g_object_set (label, "margin", 3, NULL);
	gtk_grid_attach (grid, label, 2, 3, 1, 1);
	gtk_label_set_mnemonic_widget (GTK_LABEL (label), seqer->spin_to);
	seqer->label_to = label;

	// spin "digits"
	spin_adj = (GtkAdjustment *) gtk_adjustment_new (2.0, 1.0,
			20.0, 1.0, 5.0, 0.0);
	seqer->spin_digits = gtk_spin_button_new (spin_adj, 1.0, 0);
	g_object_set (seqer->spin_digits, "margin", 3, "hexpand", TRUE, NULL);
	gtk_grid_attach (grid, seqer->spin_digits, 5, 3, 1, 1);
	g_signal_connect_swapped (seqer->spin_digits, "value-changed",
			G_CALLBACK (ugtk_sequencer_update_preview), seqer);
	// label "digits"
	label = gtk_label_new_with_mnemonic (_("digits:"));
	g_object_set (label, "margin", 3, NULL);
	gtk_grid_attach (grid, label, 4, 3, 1, 1);
	gtk_label_set_mnemonic_widget (GTK_LABEL (label), seqer->spin_digits);
	seqer->label_digits = label;

	// -------------------------------------------------------
	// radio "From"
	seqer->radio = gtk_radio_button_new_with_mnemonic_from_widget (
			GTK_RADIO_BUTTON (seqer->radio), _("F_rom:"));
	g_object_set (seqer->radio, "margin", 3, NULL);
	gtk_grid_attach (grid, seqer->radio, 0, 4, 1, 1);
	g_signal_connect (seqer->radio, "clicked",
			G_CALLBACK (on_radio2_clicked), seqer);
	// entry "From"
	entry = gtk_entry_new ();
	seqer->entry_from = GTK_ENTRY (entry);
	gtk_entry_set_text (seqer->entry_from, "a");
	gtk_entry_set_max_length (seqer->entry_from, 1);
	gtk_entry_set_width_chars (seqer->entry_from, 2);
	gtk_widget_set_sensitive (entry, FALSE);
	g_object_set (entry, "margin", 3, "hexpand", TRUE, NULL);
	gtk_grid_attach (grid, entry, 1, 4, 1, 1);
	g_signal_connect_swapped (GTK_EDITABLE (entry), "changed",
			G_CALLBACK (ugtk_sequencer_update_preview), seqer);

	// entry "To"
	entry = gtk_entry_new ();
	seqer->entry_to = GTK_ENTRY (entry);
	gtk_entry_set_text (seqer->entry_to, "z");
	gtk_entry_set_max_length (seqer->entry_to, 1);
	gtk_entry_set_width_chars (seqer->entry_to, 2);
	gtk_widget_set_sensitive (entry, FALSE);
	g_object_set (entry, "margin", 3, "hexpand", TRUE, NULL);
	gtk_grid_attach (grid, entry, 3, 4, 1, 1);
	g_signal_connect_swapped (GTK_EDITABLE (seqer->entry_to), "changed",
			G_CALLBACK(ugtk_sequencer_update_preview), seqer);

	// label case-sensitive
	label = gtk_label_new (_("case-sensitive"));
	gtk_widget_set_sensitive (label, FALSE);
	g_object_set (label, "margin", 3, NULL);
	gtk_grid_attach (grid, label, 4, 4, 2, 1);
	seqer->label_case = label;

	// -------------------------------------------------------
	// preview
	ugtk_sequencer_preview_init (&seqer->preview);
	g_object_set (seqer->preview.self, "margin", 3, "expand", TRUE, NULL);
	gtk_grid_attach (grid, seqer->preview.self, 0, 7, 6, 1);

	ugtk_sequencer_update_preview (seqer);
	gtk_widget_show_all (seqer->self);
}

void  ugtk_sequencer_update_preview (UgtkSequencer* seqer)
{
	GtkTreeIter		iter;
	GList*			list;
	GList*			link;

	list = ugtk_sequencer_get_list (seqer, TRUE);
	if (list == NULL) {
		if (seqer->preview.status == 1) {
			ugtk_sequencer_preview_show (&seqer->preview,
					_("No wildcard(*) character in URL entry."));
		}
		else if (seqer->preview.status == 2) {
			ugtk_sequencer_preview_show (&seqer->preview,
					_("URL is not valid."));
		}
		else if (seqer->preview.status == 3) {
			ugtk_sequencer_preview_show (&seqer->preview,
					_("No character in 'From' or 'To' entry."));
		}
		// notify
		if (seqer->notify.func)
			seqer->notify.func (seqer->notify.data, FALSE);
		return;
	}

	gtk_list_store_clear (seqer->preview.store);
	for (link = list;  link;  link = link->next) {
		gtk_list_store_append (seqer->preview.store, &iter);
		gtk_list_store_set (seqer->preview.store, &iter, 0, link->data, -1);
	}
	g_list_foreach (list, (GFunc) g_free, NULL);
	g_list_free (list);
	// notify
	if (seqer->notify.func)
		seqer->notify.func (seqer->notify.data, TRUE);
	return;
}

GList*  ugtk_sequencer_get_list (UgtkSequencer* seqer, gboolean preview)
{
	GString*     gstr;
	GList*       list;
	gint         from, to, cur, digits;
	gboolean     char_mode;
	const gchar* string;
	gint         offset;

	string = gtk_entry_get_text (seqer->entry);
	offset = strcspn (string, "*");
	if (string [offset] == 0) {
		seqer->preview.status = 1;
		return NULL;
	}
	if (ug_uri_init (NULL, string) == 0) {
		seqer->preview.status = 2;
		return NULL;
	}
	// char or digit
	char_mode = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (seqer->radio));
	if (char_mode) {
		// char
		from   = *gtk_entry_get_text (seqer->entry_from);
		to     = *gtk_entry_get_text (seqer->entry_to);
		digits = 1;
		if (from == 0 || to == 0) {
			seqer->preview.status = 3;
			return NULL;
		}
	}
	else {
		// digit
		from   = gtk_spin_button_get_value_as_int ((GtkSpinButton*) seqer->spin_from);
		to     = gtk_spin_button_get_value_as_int ((GtkSpinButton*) seqer->spin_to);
		digits = gtk_spin_button_get_value_as_int ((GtkSpinButton*) seqer->spin_digits);
	}
	// swap from & to
	if (from > to) {
		cur = from;
		from = to;
		to = cur;
	}
	// create URI list
	list = NULL;
	gstr = g_string_sized_new (80);
	g_string_append_len (gstr, string, offset);
	for (cur = to;  cur >= from;  cur--) {
		if (preview  &&  from < to -4  &&  cur == to -2) {
			list = g_list_prepend (list, g_strdup (" ..."));
			cur = from + 1;
		}
		if (char_mode)
			g_string_append_printf (gstr, "%c", cur);
		else
			g_string_append_printf (gstr, "%.*d", digits, cur);
		g_string_append (gstr, string + offset + 1);
		list = g_list_prepend (list, g_strdup (gstr->str));
		g_string_truncate (gstr, offset);
	}
	g_string_free (gstr, TRUE);
	seqer->preview.status = 0;
	return list;
}


// ----------------------------------------------------------------------------
//	static functions
//
static void ugtk_sequencer_preview_init (struct UgtkSequencerPreview* preview)
{
	GtkScrolledWindow*	scrolled;
	GtkCellRenderer*	renderer;
	GtkTreeViewColumn*	column;
	GtkTreeSelection*	selection;

	preview->view  = (GtkTreeView*) gtk_tree_view_new ();
	preview->store = gtk_list_store_new (1, G_TYPE_STRING);
	gtk_tree_view_set_model (preview->view, (GtkTreeModel*) preview->store);
//	gtk_tree_view_set_fixed_height_mode (preview->view, TRUE);
	gtk_widget_set_size_request ((GtkWidget*) preview->view, 140, 140);
	selection = gtk_tree_view_get_selection (preview->view);
	gtk_tree_selection_set_mode (selection, GTK_SELECTION_NONE);
	// It will free UgtkSequencer.preview_store when UgtkSequencer.preview_view destroy.
	g_object_unref (preview->store);

	renderer = gtk_cell_renderer_text_new ();
	column   = gtk_tree_view_column_new_with_attributes (
			_("Preview"), renderer, "text", 0, NULL);
//	gtk_tree_view_column_set_sizing (column, GTK_TREE_VIEW_COLUMN_FIXED);
	gtk_tree_view_append_column (preview->view, column);

	preview->self = gtk_scrolled_window_new (NULL, NULL);
	scrolled = GTK_SCROLLED_WINDOW (preview->self);
	gtk_scrolled_window_set_shadow_type (scrolled, GTK_SHADOW_IN);
	gtk_scrolled_window_set_policy (scrolled,
			GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_container_add (GTK_CONTAINER (scrolled), GTK_WIDGET (preview->view));
}

static void ugtk_sequencer_preview_show (struct UgtkSequencerPreview* preview, const gchar* message)
{
	GtkTreeIter		iter;

	gtk_list_store_clear (preview->store);
	// skip first row
	gtk_list_store_append (preview->store, &iter);
	// show message in second row
	gtk_list_store_append (preview->store, &iter);
	gtk_list_store_set (preview->store, &iter, 0, message, -1);
}

// ----------------------------------------------------------------------------
//	signal handler
//
static void on_radio1_clicked (GtkWidget* widget, UgtkSequencer* seqer)
{
	// digit
	gtk_widget_set_sensitive (seqer->spin_from, TRUE);
	gtk_widget_set_sensitive (seqer->spin_to, TRUE);
	gtk_widget_set_sensitive (seqer->spin_digits, TRUE);
	gtk_widget_set_sensitive (seqer->label_to, TRUE);
	gtk_widget_set_sensitive (seqer->label_digits, TRUE);
	// character
	gtk_widget_set_sensitive (GTK_WIDGET (seqer->entry_from), FALSE);
	gtk_widget_set_sensitive (GTK_WIDGET (seqer->entry_to), FALSE);
	gtk_widget_set_sensitive (seqer->label_case, FALSE);
	ugtk_sequencer_update_preview (seqer);
}

static void on_radio2_clicked (GtkWidget* widget, UgtkSequencer* seqer)
{
	// digit
	gtk_widget_set_sensitive (seqer->spin_from, FALSE);
	gtk_widget_set_sensitive (seqer->spin_to, FALSE);
	gtk_widget_set_sensitive (seqer->spin_digits, FALSE);
	gtk_widget_set_sensitive (seqer->label_to, FALSE);
	gtk_widget_set_sensitive (seqer->label_digits, FALSE);
	// character
	gtk_widget_set_sensitive (GTK_WIDGET (seqer->entry_from), TRUE);
	gtk_widget_set_sensitive (GTK_WIDGET (seqer->entry_to), TRUE);
	gtk_widget_set_sensitive (seqer->label_case, TRUE);
	ugtk_sequencer_update_preview (seqer);
}

