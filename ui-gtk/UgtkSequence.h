/*
 *
 *   Copyright (C) 2005-2015 by C.H. Huang
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

#ifndef UGTK_SEQUENCE_H
#define UGTK_SEQUENCE_H

#include <gtk/gtk.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct  UgtkSequence      UgtkSequence;
typedef	void  (*UgtkSequenceNotify) (gpointer user_data, gboolean completed);

struct UgtkSequence
{
	GtkWidget*  self;       // GtkGrid

	GtkEntry*   entry;      // URI, wildcard
	GtkWidget*  radio;      // GtkRadioButton

	// digit mode
	GtkWidget*  spin_from;
	GtkWidget*  spin_to;
	GtkWidget*  spin_digits;
	GtkWidget*  label_to;
	GtkWidget*  label_digits;

	// character mode
	GtkEntry*   entry_from;
	GtkEntry*   entry_to;
	GtkWidget*  label_case;

	// preview
	struct UgtkSequencePreview
	{
		GtkWidget*      self;    // GtkScrolledWindow
		GtkTreeView*    view;
		GtkListStore*   store;
		guint           status;
	} preview;

	// callback
	struct
	{
//		UgtkNotify  func1;
		UgtkSequenceNotify func;
		gpointer            data;
	} notify;
};

void   ugtk_sequence_init (UgtkSequence* seqer);
void   ugtk_sequence_update_preview (UgtkSequence* seqer);
GList* ugtk_sequence_get_list (UgtkSequence* seqer, gboolean preview);


#ifdef __cplusplus
}
#endif

#endif  // End of UGTK_SEQUENCE_H

