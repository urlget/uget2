/*
 *
 *   Copyright (C) 2012-2014 by C.H. Huang
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

#ifndef UG_HTML_H
#define UG_HTML_H

#include <UgArray.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef	struct  UgHtml         UgHtml;
typedef	struct  UgHtmlParser   UgHtmlParser;

typedef enum {
	UG_HTML_ERROR_NONE  = 0,

	UG_HTML_ERROR_BAD_ELEMENT    = -1,
	UG_HTML_ERROR_BAD_ENTITY     = -2,
	UG_HTML_ERROR_BAD_ATTRIBUTE  = -3,

	UG_HTML_ERROR_UNKNOWN_ENTITY = 1,
} UgHtmlError;

// ----------------------------------------------------------------------------
// UgHtml : simple and uncompleted parser.

struct	UgHtml
{
	// buffer is used by parser & writer
	struct {
		char*  at;
		int    length;
		int    allocated;
	} buf;

	// stack of parser
	// index 0, 2, 4, 6... : UgHtmlParser*  parser
	// index 1, 3, 5, 7... : void*         user_data
	UgArrayPtr   stack;

	// attribute name & value for callback
	UgArrayPtr   attr_names;
	UgArrayPtr   attr_values;

	const char*  element_name;
	int          name_beg_pos;
	int          name_end_pos;
	int          entity;

	uint8_t  error;
	uint8_t  state;
	uint8_t  tag_state;
	uint8_t  equ;
};

UgHtml* ug_html_new  (void);
void    ug_html_free (UgHtml* uhtml);

void  ug_html_init (UgHtml* uhtml);
void  ug_html_final (UgHtml* uhtml);

void  ug_html_push (UgHtml* uhtml, const UgHtmlParser* parser, void* dest, void* data);
void  ug_html_pop  (UgHtml* uhtml);

void         ug_html_begin_parse (UgHtml* uhtml);
UgHtmlError  ug_html_end_parse   (UgHtml* uhtml);
UgHtmlError  ug_html_parse (UgHtml* uhtml, const char* buffer, int buffer_len);

int  ug_html_parse_file (UgHtml* uhtml, const char* file_utf8);

// ----------------------------------------------------------------------------
// UgHtmlParser

typedef void (*UgHtmlParserStartElementFunc) (UgHtml*        uhtml,
                                              const char*    element_name,
                                              const char**   attribute_names,
                                              const char**   attribute_values,
                                              void*          dest,
                                              void*          data);

// This one is similar to GMarkupParser
struct UgHtmlParser
{
	/* Called for open tags <foo bar="baz"> */
	UgHtmlParserStartElementFunc    start_element;
//	void  (*start_element) (UgHtml*        uhtml,
//	                        const char*    element_name,
//	                        const char**   attribute_names,
//	                        const char**   attribute_values,
//	                        void*          dest,
//	                        void*          data);

	/* Called for close tags </foo> */
	void  (*end_element)   (UgHtml*        uhtml,
	                        const char*    element_name,
	                        void*          dest,
	                        void*          data);

	/* Called for character data */
	/* text is not null-terminated */
	void  (*text)          (UgHtml*        uhtml,
	                        const char*    text,
	                        int            text_len,
	                        void*          dest,
	                        void*          data);
};


#ifdef __cplusplus
}
#endif


#endif  // UG_HTML_H
