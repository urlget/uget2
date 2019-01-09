/*
 *
 *   Copyright (C) 2005-2019 by C.H. Huang
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

// utilities for g_markup_xxx

#ifndef UG_MARKUP_H
#define UG_MARKUP_H

#include <stdio.h>
#include <glib.h>

#ifdef __cplusplus
extern "C" {
#endif


typedef	struct	UgMarkup				UgMarkup;

//	If you want to skip next element_start()
//	g_markup_parse_context_push (context, &ug_markup_skip_parser, NULL);
extern	GMarkupParser		ug_markup_skip_parser;


// ----------------------------------------------------------------------------
// UgMarkup : write markup to file, parse markup file by GMarkupParseContext
//
UgMarkup*	ug_markup_new	(void);
void		ug_markup_free	(UgMarkup* markup);

// write markup to file
gboolean	ug_markup_write_start	(UgMarkup* markup, const gchar* filename, gboolean formating);
void		ug_markup_write_end		(UgMarkup* markup);

// write text. If text_len == -1, text is null-terminated.
void		ug_markup_write_text	(UgMarkup* markup, const gchar* text, gint text_len);

// write element
void		ug_markup_write_element_start	(UgMarkup* markup, const gchar* printf_format, ...);
void		ug_markup_write_element_end		(UgMarkup* markup, const gchar* element_name);

// parse
gboolean	ug_markup_parse	(const gchar* filename, const GMarkupParser* parser, gpointer data);


#ifdef __cplusplus
}
#endif

#endif  // UG_MARKUP_H
