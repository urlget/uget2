/*
 *
 *   Copyright (C) 2005-2018 by C.H. Huang
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

#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <UgArray.h>
#include <UgString.h>
#include <UgtkCategoryForm.h>

#include <glib/gi18n.h>

static gchar* string_from_ug_array (UgArrayStr* uarray);
static void   string_to_ug_array (const gchar* string, UgArrayStr* uarray);

void  ugtk_category_form_init (UgtkCategoryForm* cform)
{
	GtkWidget*	label;
	GtkWidget*  entry;
	GtkGrid*	top_grid;
	GtkGrid*	grid;
	GtkWidget*  widget;

	cform->self = gtk_grid_new ();
	top_grid  = (GtkGrid*) cform->self;
	gtk_container_set_border_width (GTK_CONTAINER (top_grid), 2);

	label = gtk_label_new_with_mnemonic (_("Category _name:"));
	entry = gtk_entry_new ();
	gtk_entry_set_activates_default (GTK_ENTRY (entry), TRUE);
	gtk_label_set_mnemonic_widget (GTK_LABEL (label), entry);
	g_object_set (label, "margin", 2, NULL);
	g_object_set (entry, "margin", 2, "hexpand", TRUE, NULL);
	gtk_grid_attach (top_grid, label, 0, 0, 1, 1);
	gtk_grid_attach (top_grid, entry, 1, 0, 1, 1);
	cform->name_entry = entry;
	cform->name_label = label;

	// ------------------------------------------------------------------------
	// Capacity
	grid = (GtkGrid*) gtk_grid_new ();
	g_object_set (grid, "margin-top", 2, "margin-bottom", 2, NULL);
	gtk_grid_attach (top_grid, GTK_WIDGET (grid), 0, 1, 2, 1);

	cform->spin_active = gtk_spin_button_new_with_range (1.0, 20.0, 1.0);
	gtk_entry_set_activates_default (GTK_ENTRY (cform->spin_active), TRUE);
//	gtk_entry_set_width_chars (GTK_ENTRY(cform->spin_active), 5);
	label = gtk_label_new_with_mnemonic (_("Active _downloads:"));
	gtk_label_set_mnemonic_widget (GTK_LABEL(label), cform->spin_active);
	g_object_set (label, "margin", 2, NULL);
	g_object_set (cform->spin_active, "margin-top", 2, "margin-bottom", 2, NULL);
	gtk_grid_attach (grid, label, 0, 0, 1, 1);
	gtk_grid_attach (grid, cform->spin_active, 1, 0, 1, 1);

	cform->spin_finished = gtk_spin_button_new_with_range (0.0, 99999.0, 1.0);
	gtk_entry_set_activates_default (GTK_ENTRY (cform->spin_finished), TRUE);
//	gtk_entry_set_width_chars (GTK_ENTRY(cform->spin_finished), 5);
	label = gtk_label_new_with_mnemonic (_("Capacity of Finished:"));
	gtk_label_set_mnemonic_widget (GTK_LABEL(label), cform->spin_finished);
	g_object_set (label, "margin", 2, NULL);
	g_object_set (cform->spin_finished, "margin-top", 2, "margin-bottom", 2, NULL);
	gtk_grid_attach (grid, label, 0, 1, 1, 1);
	gtk_grid_attach (grid, cform->spin_finished, 1, 1, 1, 1);

	cform->spin_recycled = gtk_spin_button_new_with_range (0.0, 99999.0, 1.0);
	gtk_entry_set_activates_default (GTK_ENTRY (cform->spin_recycled), TRUE);
//	gtk_entry_set_width_chars (GTK_ENTRY(cform->spin_recycled), 5);
	label = gtk_label_new_with_mnemonic (_("Capacity of Recycled:"));
	gtk_label_set_mnemonic_widget (GTK_LABEL(label), cform->spin_recycled);
	g_object_set (label, "margin", 2, NULL);
	g_object_set (cform->spin_recycled, "margin-top", 2, "margin-bottom", 2, NULL);
	gtk_grid_attach (grid, label, 0, 2, 1, 1);
	gtk_grid_attach (grid, cform->spin_recycled, 1, 2, 1, 1);

	// ------------------------------------------------------------------------
	// URI Matching conditions
	widget = gtk_frame_new (_("URI Matching conditions"));
	g_object_set (grid, "margin-top", 4, "margin-bottom", 4, NULL);
	gtk_grid_attach (top_grid, widget, 0, 2, 2, 1);
	grid = (GtkGrid*) gtk_grid_new ();
	g_object_set (grid, "margin-top", 2, "margin-bottom", 2, NULL);
	gtk_container_add (GTK_CONTAINER (widget), (GtkWidget*) grid);

	label = gtk_label_new_with_mnemonic (_("Matched _Hosts:"));
	entry = gtk_entry_new ();
	gtk_entry_set_activates_default (GTK_ENTRY (entry), TRUE);
	gtk_label_set_mnemonic_widget (GTK_LABEL (label), entry);
	g_object_set (label, "margin", 2, NULL);
	g_object_set (entry, "margin", 2, "hexpand", TRUE, NULL);
	gtk_grid_attach (grid, label, 0, 0, 1, 1);
	gtk_grid_attach (grid, entry, 1, 0, 1, 1);
	cform->hosts_entry = entry;
	cform->hosts_label = label;

	label = gtk_label_new_with_mnemonic (_("Matched _Schemes:"));
	entry = gtk_entry_new ();
	gtk_entry_set_activates_default (GTK_ENTRY (entry), TRUE);
	gtk_label_set_mnemonic_widget (GTK_LABEL (label), entry);
	g_object_set (label, "margin", 2, NULL);
	g_object_set (entry, "margin", 2, "hexpand", TRUE, NULL);
	gtk_grid_attach (grid, label, 0, 1, 1, 1);
	gtk_grid_attach (grid, entry, 1, 1, 1, 1);
	cform->schemes_entry = entry;
	cform->schemes_label = label;

	label = gtk_label_new_with_mnemonic (_("Matched _Types:"));
	entry = gtk_entry_new ();
	gtk_entry_set_activates_default (GTK_ENTRY (entry), TRUE);
	gtk_label_set_mnemonic_widget (GTK_LABEL (label), entry);
	g_object_set (label, "margin", 2, NULL);
	g_object_set (entry, "margin", 2, "hexpand", TRUE, NULL);
	gtk_grid_attach (grid, label, 0, 2, 1, 1);
	gtk_grid_attach (grid, entry, 1, 2, 1, 1);
	cform->types_entry = entry;
	cform->types_label = label;

	gtk_widget_show_all (GTK_WIDGET (top_grid));
}

void  ugtk_category_form_get (UgtkCategoryForm* cform, UgetNode* cnode)
{
	UgetCategory*  category;
	const gchar*   text;

	category = ug_info_realloc (&cnode->info, UgetCategoryInfo);
	if (gtk_widget_is_sensitive (cform->name_entry) == TRUE) {
		text = gtk_entry_get_text ((GtkEntry*) cform->name_entry);
		ug_free (cnode->name);
		cnode->name = (*text) ? ug_strdup (text) : NULL;
	}
	category->active_limit = gtk_spin_button_get_value_as_int (
			(GtkSpinButton*) cform->spin_active);
	// Finished
	category->finished_limit = gtk_spin_button_get_value_as_int (
			(GtkSpinButton*) cform->spin_finished);
	// Recycled
	category->recycled_limit = gtk_spin_button_get_value_as_int (
			(GtkSpinButton*) cform->spin_recycled);

	// matching - clear
	ug_array_foreach_str (&category->hosts, (UgForeachFunc) ug_free, NULL);
	ug_array_foreach_str (&category->schemes, (UgForeachFunc) ug_free, NULL);
	ug_array_foreach_str (&category->file_exts, (UgForeachFunc) ug_free, NULL);
	category->hosts.length = 0;
	category->schemes.length = 0;
	category->file_exts.length = 0;
	// matching - set
	string_to_ug_array (gtk_entry_get_text ((GtkEntry*) cform->hosts_entry),
			&category->hosts);
	string_to_ug_array (gtk_entry_get_text ((GtkEntry*) cform->schemes_entry),
			&category->schemes);
	string_to_ug_array (gtk_entry_get_text ((GtkEntry*) cform->types_entry),
			&category->file_exts);
}

void  ugtk_category_form_set  (UgtkCategoryForm*  cform, UgetNode* cnode)
{
	UgetCategory*  category;
	gchar*  str;

	category = ug_info_realloc (&cnode->info, UgetCategoryInfo);
	if (gtk_widget_is_sensitive (cform->name_entry) == TRUE) {
		gtk_entry_set_text ((GtkEntry*) cform->name_entry,
				(cnode->name) ? cnode->name : "");
	}
	gtk_spin_button_set_value ((GtkSpinButton*) cform->spin_active,
			(gdouble) category->active_limit);
	// Finished
	gtk_spin_button_set_value ((GtkSpinButton*) cform->spin_finished,
			(gdouble) category->finished_limit);
	// Recycled
	gtk_spin_button_set_value ((GtkSpinButton*) cform->spin_recycled,
			(gdouble) category->recycled_limit);

	// matching
	str = string_from_ug_array (&category->hosts);
	gtk_entry_set_text ((GtkEntry*) cform->hosts_entry, str);
	g_free (str);
	str = string_from_ug_array (&category->schemes);
	gtk_entry_set_text ((GtkEntry*) cform->schemes_entry, str);
	g_free (str);
	str = string_from_ug_array (&category->file_exts);
	gtk_entry_set_text ((GtkEntry*) cform->types_entry, str);
	g_free (str);
}

void  ugtk_category_form_set_multiple (UgtkCategoryForm*  cform, gboolean multiple_mode)
{
	if (multiple_mode) {
		gtk_widget_hide (cform->name_entry);
		gtk_widget_hide (cform->name_label);
		gtk_widget_set_sensitive (cform->name_entry, FALSE);
		gtk_widget_set_sensitive (cform->name_label, FALSE);
	}
	else {
		gtk_widget_show (cform->name_entry);
		gtk_widget_show (cform->name_label);
		gtk_widget_set_sensitive (cform->name_entry, TRUE);
		gtk_widget_set_sensitive (cform->name_label, TRUE);
	}
}

// ----------------------------------------------------------------------------
// static function

static gchar* string_from_ug_array (UgArrayStr* uarray)
{
	int   index, length;
	char* buffer;

	for (length = 0, index = 0;  index < uarray->length;  index++)
		length += strlen(uarray->at[index]) + 1;  // + ';'

	buffer = g_malloc (length + 1);
	buffer[0] = 0;

	for (index = 0;  index < uarray->length;  index++) {
		strcat (buffer, uarray->at[index]);
		strcat (buffer, ";");
	}

	return buffer;
}

static void string_to_ug_array (const gchar* string, UgArrayStr* uarray)
{
	const gchar*  cur;
	const gchar*  prev;

	for (prev = string, cur = string; ;  cur++) {
		if ((cur[0] == ';' || cur[0] == 0) && cur != prev) {
			*(char**)ug_array_alloc (uarray, 1) = ug_strndup (prev, cur - prev);
			prev = cur + 1;
		}
		if (cur[0] == 0)
			break;
	}
}
