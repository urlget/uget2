/*
 *
 *   Copyright (C) 2005-2020 by C.H. Huang
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

#ifndef UGTK_CONFIRM_DIALOG_H
#define UGTK_CONFIRM_DIALOG_H

#include <gtk/gtk.h>
#include <UgtkApp.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
	UGTK_CONFIRM_DIALOG_EXIT,
	UGTK_CONFIRM_DIALOG_DELETE,
	UGTK_CONFIRM_DIALOG_DELETE_CATEGORY,
} UgtkConfirmDialogMode;

typedef struct UgtkConfirmDialog        UgtkConfirmDialog;

struct UgtkConfirmDialog
{
	GtkDialog*     self;
	UgtkApp*       app;

	GtkToggleButton* confirmation;
};

UgtkConfirmDialog*  ugtk_confirm_dialog_new (UgtkConfirmDialogMode mode, UgtkApp* app);

void  ugtk_confirm_dialog_run (UgtkConfirmDialog* cdialog);


#ifdef __cplusplus
}
#endif

#endif // UGTK_CONFIRM_DIALOG_H
