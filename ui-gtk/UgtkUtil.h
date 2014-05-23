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

#ifndef UGTK_UTIL_H
#define UGTK_UTIL_H

#include <glib.h>
//#include <glib/gstdio.h>

#ifdef __cplusplus
extern "C" {
#endif

int  ugtk_copy_file (const gchar *src_file_utf8, const gchar *new_file_utf8);

// ------------------------------------------------------------------
// URI list functions
// To get URIs from text file, error is G_FILE_ERROR.
GList*  ugtk_text_file_get_uris (const gchar* file_utf8, GError** error);
// get URIs from text
GList*  ugtk_text_get_uris (const gchar* text);
// remove URIs from list by scheme
GList*  ugtk_uri_list_remove_scheme (GList* uris, const gchar* scheme);

// ------------------------------------------------------------------
// check BOM in file header and set it's encoding.
// return encoding string.
const char* ugtk_io_channel_decide_encoding (GIOChannel* channel);

// ------------------------------------------------------------------
// others
//
gboolean  ugtk_launch_uri (const gchar* uri);
gboolean  ugtk_launch_default_app (const gchar* folder, const gchar* file);

#ifdef __cplusplus
}
#endif

#endif  // UGTK_UTIL_H

