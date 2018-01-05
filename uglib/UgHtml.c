/*
 *
 *   Copyright (C) 2012-2018 by C.H. Huang
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

#include <UgStdio.h>
#include <UgString.h>
#include <UgHtml.h>

enum UgHtmlState
{
	UG_HTML_NULL,
	UG_HTML_TEXT,
	UG_HTML_ENTITY,
	UG_HTML_TAG,
//	UG_HTML_TAG_PASS,   // <!--pass-->
};

enum UgHtmlTagState
{
	UG_HTML_TAG_ERROR,  // (0 << 0)
	UG_HTML_TAG_START,  // (1 << 0)
	UG_HTML_TAG_END,    // (1 << 1)
};

UgHtml*  ug_html_new (void)
{
	UgHtml*  uhtml;

	uhtml = ug_malloc (sizeof (UgHtml));
	ug_html_init (uhtml);
	return uhtml;
}

void  ug_html_free (UgHtml* uhtml)
{
	ug_html_final (uhtml);
	ug_free (uhtml);
}

void  ug_html_init (UgHtml* uhtml)
{
	// initialize buffer
	uhtml->buf.allocated = 4096;
	uhtml->buf.length = 0;
	uhtml->buf.at = ug_malloc (uhtml->buf.allocated);
	//
	ug_array_init (&uhtml->stack, sizeof (void*), 16 * 3);
	ug_array_init (&uhtml->attr_names, sizeof (void*), 16);
	ug_array_init (&uhtml->attr_values, sizeof (void*), 16);
}

void  ug_html_final (UgHtml* uhtml)
{
	ug_free (uhtml->buf.at);
	ug_array_clear (&uhtml->stack);
	ug_array_clear (&uhtml->attr_names);
	ug_array_clear (&uhtml->attr_values);
}

// ----------------------------------------------------------------------------

enum {
	PARSER_STACK_BASE,      // 0
	PARSER_STACK_DATA2,     // 1
	PARSER_STACK_DATA1,     // 2
	PARSER_STACK_TABLE,     // 3
	PARSER_STACK_UNIT = PARSER_STACK_TABLE,  // equal with STACK_MULTIPLY_VALUE
};

static const UgHtmlParser  ug_html_parser_null = {NULL, NULL, NULL};

void  ug_html_push (UgHtml* uhtml, const UgHtmlParser* parser, void* dest, void* data)
{
	void** addr;

	addr = (void**) ug_array_alloc (&uhtml->stack, PARSER_STACK_UNIT);
	*addr++ = (void*) parser;
	*addr++ = dest;
	*addr++ = data;
}

void  ug_html_pop (UgHtml* uhtml)
{
	if (uhtml->stack.length >= PARSER_STACK_UNIT)
		uhtml->stack.length -= PARSER_STACK_UNIT;
	if (uhtml->stack.length == 0)
		ug_html_push (uhtml, &ug_html_parser_null, NULL, NULL);
}

void  ug_html_begin_parse (UgHtml* uhtml)
{
	uhtml->buf.at[0] = 0;
	uhtml->buf.length = 1;
	uhtml->name_beg_pos = 0;
	uhtml->name_end_pos = 0;
	uhtml->error = 0;
	uhtml->state = UG_HTML_TEXT;
}

UgHtmlError  ug_html_end_parse (UgHtml* uhtml)
{
	uhtml->stack.length = 0;

	if (uhtml->state == UG_HTML_ENTITY)
		return UG_HTML_ERROR_BAD_ENTITY;
	if (uhtml->state == UG_HTML_TAG)
		return UG_HTML_ERROR_BAD_ELEMENT;
	return (UgHtmlError) uhtml->error;
}

// parse context->tag, separate attribute names and values.
// output: uhtml->name_beg_pos, uhtml->name_end_pos,
//         uhtml->attr_names, uhtml->attr_values,
//         uhtml->tag_state
static int  ug_html_parse_tag (UgHtml* uhtml)
{
	char*   attr_name  = NULL;
	char*   attr_value = NULL;
	char    quote_chr = 0;
	int     quote_level = 0;
	char*   cur;
	char*   end;

	uhtml->name_beg_pos = 0;
	uhtml->name_end_pos = 0;
	uhtml->attr_names.length = 0;
	uhtml->attr_values.length = 0;
	uhtml->tag_state = UG_HTML_TAG_START;
	uhtml->equ = FALSE;

	end = uhtml->buf.at + uhtml->buf.length;
	for (cur = uhtml->buf.at;  cur < end;  cur++) {
		switch (cur[0]) {
		case ' ':
			cur[0] = 0;
			if (cur == uhtml->buf.at + uhtml->name_beg_pos)
				uhtml->name_beg_pos++;
			else {
				if (uhtml->name_end_pos == 0)
					uhtml->name_end_pos = cur - uhtml->buf.at;
				else if (uhtml->equ) {
					uhtml->equ = FALSE;
					*(void**) ug_array_alloc (&uhtml->attr_names, 1)  = attr_name;
					*(void**) ug_array_alloc (&uhtml->attr_values, 1) = attr_value;
					attr_value = NULL;
				}
				attr_name = cur + 1;
			}
			continue;

		case '=':
			cur[0] = 0;
			if (uhtml->equ || attr_name == NULL) {
				uhtml->error = UG_HTML_ERROR_BAD_ATTRIBUTE;
//				uhtml->tag_state = 0;
//				return FALSE;
			}
			else {
				uhtml->equ = TRUE;
				attr_value = cur + 1;
			}
			continue;

		case '/':
			if (quote_level == 0) {
				cur[0] = 0;
				if (uhtml->tag_state & UG_HTML_TAG_END)
					uhtml->error = UG_HTML_ERROR_BAD_ELEMENT;
				if (cur == uhtml->buf.at + uhtml->name_beg_pos) {
					uhtml->tag_state =  UG_HTML_TAG_END;
					uhtml->name_beg_pos++;
				}
				else {
					uhtml->tag_state |= UG_HTML_TAG_END;
				}
			}
			continue;

		case '\'':
		case '\"':
			// handle <tag attr='"'"value"'"'>
			if (quote_chr == cur[0]) {
				quote_chr = (quote_chr=='"') ? '\'' : '"';
				quote_level--;
			}
			else {
				if (quote_level == 0)
					attr_value = cur + 1;	// ignore first character
				quote_chr = cur[0];
				quote_level++;
			}

			if (quote_level == 0) {
				quote_chr = 0;
				cur[0] = 0;	   // null-terminated
			}
			continue;

		default:
			continue;
		}
	}

	if (uhtml->equ) {
		*(void**) ug_array_alloc (&uhtml->attr_names, 1)  = attr_name;
		*(void**) ug_array_alloc (&uhtml->attr_values, 1) = attr_value;
	}
	if (uhtml->name_end_pos == 0)
		uhtml->name_end_pos = cur - uhtml->buf.at - 1;

	// null-terminated
	*(void**) ug_array_alloc (&uhtml->attr_names, 1)  = NULL;
	*(void**) ug_array_alloc (&uhtml->attr_values, 1) = NULL;

	return TRUE;
}

UgHtmlError  ug_html_parse (UgHtml* uhtml, const char* buffer, int buffer_len)
{
	const UgHtmlParser* parser;
	const char*   buffer_cur;
	const char*   buffer_end;
	char          vchar;
	int           entity_len;

	if (buffer_len == -1)
		buffer_len = strlen (buffer);
	buffer_end = buffer + buffer_len;

	for (buffer_cur = buffer;  buffer_cur < buffer_end;  buffer_cur++) {
		if (uhtml->buf.allocated == uhtml->buf.length) {
			uhtml->buf.allocated *= 2;
			uhtml->buf.at = ug_realloc (uhtml->buf.at,
					uhtml->buf.allocated * sizeof (char));
		}
		vchar = *buffer_cur;

TopLevelSwitch:
		switch (uhtml->state) {
		case UG_HTML_TAG:
			if (vchar != '>') {
				uhtml->buf.at[uhtml->buf.length++] = vchar;
				continue;
			}
			// End of TAG
			uhtml->buf.at[uhtml->buf.length] = 0;	 // null-terminated
			ug_html_parse_tag (uhtml);
			uhtml->element_name = uhtml->buf.at + uhtml->name_beg_pos;
			if (uhtml->tag_state & UG_HTML_TAG_START) {
				parser = uhtml->stack.at[uhtml->stack.length - PARSER_STACK_TABLE];
				if (parser->start_element) {
					parser->start_element (uhtml, uhtml->element_name,
							(const char**) uhtml->attr_names.at,
							(const char**) uhtml->attr_values.at,
							uhtml->stack.at[uhtml->stack.length - PARSER_STACK_DATA1],
							uhtml->stack.at[uhtml->stack.length - PARSER_STACK_DATA2]);
				}
				// clear buffer but reserve element name
				uhtml->buf.length = uhtml->name_end_pos + 1;
			}
			if (uhtml->tag_state & UG_HTML_TAG_END) {
				parser = uhtml->stack.at[uhtml->stack.length - PARSER_STACK_TABLE];
				if (parser->end_element) {
					parser->end_element (uhtml, uhtml->element_name,
							uhtml->stack.at[uhtml->stack.length - PARSER_STACK_DATA1],
							uhtml->stack.at[uhtml->stack.length - PARSER_STACK_DATA2]);
				}
				// clear buffer and element name
				uhtml->buf.at[0] = 0;  // null-terminated
				uhtml->buf.length = 1;
				uhtml->name_beg_pos = 0;
				uhtml->name_end_pos = 0;
			}
			uhtml->state = UG_HTML_TEXT;
			uhtml->entity = 0;
			continue;
//			break;

		case UG_HTML_TEXT:
			parser = uhtml->stack.at[uhtml->stack.length - PARSER_STACK_TABLE];

			switch (vchar) {
			case '<':
				uhtml->element_name = uhtml->buf.at + uhtml->name_beg_pos;
				uhtml->buf.at[uhtml->buf.length] = 0;  // null-terminated
				if (parser->text && uhtml->buf.length > uhtml->name_end_pos +1) {
					parser->text (uhtml,
							uhtml->buf.at + uhtml->name_end_pos +1,
							uhtml->buf.length - uhtml->name_end_pos -1,
							uhtml->stack.at[uhtml->stack.length - PARSER_STACK_DATA1],
							uhtml->stack.at[uhtml->stack.length - PARSER_STACK_DATA2]);
				}
				uhtml->state = UG_HTML_TAG;
				// clear buffer
				uhtml->buf.length = 0;
				continue;
//				break;

			case '&':
				uhtml->state = UG_HTML_ENTITY;
				uhtml->entity = uhtml->buf.length;
				uhtml->buf.at[uhtml->buf.length++] = vchar;
				continue;
//				break;

			case '\r':
			case '\n':
				// skip
				continue;
//				break;

			default:
				// bypass text if parser doesn't care text.
				if (parser->text)
					uhtml->buf.at[uhtml->buf.length++] = vchar;
				continue;
//				break;
			}
			continue;
//			break;

		case UG_HTML_ENTITY:
			switch (vchar) {
			case '<':
				uhtml->error = UG_HTML_ERROR_BAD_ENTITY;
				uhtml->state = UG_HTML_TEXT;
				goto TopLevelSwitch;

			case ';':
				entity_len = uhtml->buf.length - uhtml->entity;
				// UgHtml doesn't support all entity
				if (entity_len > 5 || entity_len < 3)
					uhtml->buf.at[uhtml->buf.length++] = vchar;
				else {
					switch (uhtml->buf.at[uhtml->entity +1]) {
					// Entity Number
//					case '#':
//						break;

					case 'l':    // &lt;
						uhtml->buf.at[uhtml->entity] = '<';
						break;

					case 'g':    // &gt;
						uhtml->buf.at[uhtml->entity] = '>';
						break;

					case 'a':    // &amp;  or  &apos;
						vchar = uhtml->buf.at[uhtml->entity +2];
						if (vchar == 'm')         // &amp;
							uhtml->buf.at[uhtml->entity] = '&';
						else if (vchar == 'p')    // &apos;
							uhtml->buf.at[uhtml->entity] = '\'';
						else {
							uhtml->buf.at[uhtml->buf.length++] = vchar;
							uhtml->error = UG_HTML_ERROR_UNKNOWN_ENTITY;
							uhtml->state = UG_HTML_TEXT;
							continue;
						}
						break;

					case 'q':    // &quot;
						uhtml->buf.at[uhtml->entity] = '\"';
						break;

					default:
						uhtml->buf.at[uhtml->buf.length++] = vchar;
						uhtml->error = UG_HTML_ERROR_UNKNOWN_ENTITY;
						uhtml->state = UG_HTML_TEXT;
						continue;
					}
					uhtml->buf.length = uhtml->entity + 1;
				}
				uhtml->state = UG_HTML_TEXT;
				continue;
//				break;

			default:
				uhtml->buf.at[uhtml->buf.length++] = vchar;
				continue;
//				break;
			}
			// case UG_HTML_ENTITY:
//			break;
		}
	}

	return (UgHtmlError) uhtml->error;
}

int  ug_html_parse_file (UgHtml* uhtml, const char* file_utf8)
{
	char* buf;
	int   buf_len;
	int   fd;

//	fd = open (file_utf8, O_RDONLY | O_TEXT, S_IREAD);
	fd = ug_open (file_utf8, UG_O_READONLY | UG_O_TEXT, UG_S_IREAD);
	if (fd == -1)
		return FALSE;
	ug_html_begin_parse (uhtml);

	buf = ug_malloc (4096);
	for (;;) {
//		buf_len = read (fd, buf, 4096);
		buf_len = ug_read (fd, buf, 4096);
		if (buf_len <= 0)
			break;
		ug_html_parse (uhtml, buf, buf_len);
	}
	ug_free (buf);

	ug_html_end_parse (uhtml);
//	close (fd);
	ug_close (fd);

	return TRUE;
}

