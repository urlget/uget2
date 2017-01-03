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

#include <string.h>

#include <UgStdio.h>
#include "UgMarkup.h"


enum UgMarkupState
{
	UG_MARKUP_ELEMENT_START,	// 0 :  element is not ended.   eg. <element
	UG_MARKUP_ELEMENT_END,		// 1 :  element was ended.      eg. <element/> or </element>
	UG_MARKUP_ELEMENT_TEXT,		// 2 :  element follows text.   eg. <element>text
};

static gboolean  ug_markup_close_element (UgMarkup* markup, gboolean element_end);


// ----------------------------------------------------------------------------
// UgMarkup : write markup to file, parse markup file by GMarkupParseContext
//
struct UgMarkup
{
	guint		output_state;	// UgMarkupState

	gboolean	formating;
	guint		level;

	FILE*		file;
};

UgMarkup*	ug_markup_new (void)
{
	UgMarkup*	markup;

	markup = g_malloc0 (sizeof (UgMarkup));
	return markup;
}

void	ug_markup_free (UgMarkup* markup)
{
	if (markup->file)
		fclose (markup->file);

	g_free (markup);
}

// ----------------------------------------------
// write markup to file
gboolean ug_markup_write_start  (UgMarkup* markup, const gchar* filename, gboolean formating)
{
	markup->output_state = UG_MARKUP_ELEMENT_END;
	markup->formating = TRUE;

	markup->file = ug_fopen (filename, "w");
	if (markup->file) {
		fputs ("<?xml version='1.0' encoding='UTF-8'?>\n", markup->file);
		return TRUE;
	}
	return FALSE;
}

void ug_markup_write_end (UgMarkup* markup)
{
	if (markup->file) {
		fputc ('\n', markup->file);
		fclose (markup->file);
		markup->file = NULL;
	}
}

// ----------------------------------------------
// write text. If text_len == -1, text is null-terminated.
void ug_markup_write_text (UgMarkup* markup, const gchar* text, gint len)
{
	char*	esc_text;

	if (text) {
		if (len == -1)
			len = strlen (text);
		if (len == 0)
			return;

		ug_markup_close_element (markup, FALSE);

		esc_text = g_markup_escape_text (text, len);
		fputs (esc_text, markup->file);
		markup->output_state = UG_MARKUP_ELEMENT_TEXT;
		g_free (esc_text);
	}
}

// ----------------------------------------------
// write element
void ug_markup_write_element_start (UgMarkup* markup, const gchar* printf_format, ...)
{
	FILE*		file = markup->file;
	char*		esc_text;
	va_list		arg_list;

	ug_markup_close_element (markup, FALSE);

	if (markup->formating) {
	    if (markup->level > 0)
			fprintf (file, "\n%*c", markup->level * 2, ' ');
		markup->level++;
	}
	fputc ('<', file);

	va_start (arg_list, printf_format);
	esc_text = g_markup_vprintf_escaped (printf_format, arg_list);
	va_end (arg_list);

	fputs (esc_text, file);
	g_free (esc_text);

	markup->output_state = UG_MARKUP_ELEMENT_START;
}

void ug_markup_write_element_end   (UgMarkup* markup, const gchar* element_name)
{
	FILE*		file = markup->file;
	gboolean	element_end;

	element_end = ug_markup_close_element (markup, TRUE);

	if (markup->formating) {
		markup->level--;
		if (markup->output_state != UG_MARKUP_ELEMENT_TEXT && element_end == FALSE) {
			fputc ('\n', file);
			if (markup->level > 0)
				fprintf (file, "%*c", markup->level * 2, ' ');
		}
	}

	if (element_end == FALSE)
		fprintf (file, "</%s>", element_name);

	markup->output_state = UG_MARKUP_ELEMENT_END;
}

static gboolean  ug_markup_close_element (UgMarkup* markup, gboolean element_end)
{
    char*	string;

	if (markup->output_state == UG_MARKUP_ELEMENT_START) {
//		markup->output_state = UG_MARKUP_ELEMENT_END;
		string = (element_end) ? "/>" : ">";
		fputs (string, markup->file);
		return TRUE;
	}

	return FALSE;
}

// ----------------------------------------------------------------------------
// parse
gboolean ug_markup_parse (const gchar* filename, const GMarkupParser* parser, gpointer data)
{
	GMarkupParseContext*	context;
	gchar*		buffer;
	gint		fd;
	gint		size;
	gboolean	retval = FALSE;

//	fd = open (filename, O_RDONLY | O_TEXT, S_IREAD);
	fd = ug_open (filename, UG_O_READONLY | UG_O_TEXT, UG_S_IREAD);
	if (fd == -1)
		return FALSE;

	buffer = g_malloc (4096);
	context = g_markup_parse_context_new (parser, 0, data, NULL);

	do {
//		size = read (fd, buffer, 4096);
		size = ug_read (fd, buffer, 4096);
		if (size > 0)
			retval = g_markup_parse_context_parse (context, buffer, size, NULL);
	} while (size > 0 && retval);

	retval = g_markup_parse_context_end_parse (context, NULL);
	g_markup_parse_context_free (context);
	g_free (buffer);

//	close (fd);
	ug_close (fd);

	return retval;
}


// ----------------------------------------------
// ug_markup_skip_parser
//
static void ug_markup_skip_start_element (GMarkupParseContext*	context,
                                          const gchar*	element_name,
                                          const gchar**	attr_names,
                                          const gchar**	attr_values,
                                          gpointer		data,
                                          GError**		error)
{
	g_markup_parse_context_push (context, &ug_markup_skip_parser, NULL);
}

GMarkupParser	ug_markup_skip_parser =
{
	(gpointer) ug_markup_skip_start_element,
	(gpointer) g_markup_parse_context_pop,
	NULL, NULL, NULL
};


