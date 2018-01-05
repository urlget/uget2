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

#ifndef UG_HTML_ENTRY_H
#define UG_HTML_ENTRY_H

#include <UgHtml.h>

#ifdef __cplusplus
extern "C" {
#endif

// ----------------------------------------------------------------------------

typedef struct UgHtmlEntry    UgHtmlEntry;

struct UgHtmlEntry
{
	char*               name;
	int                 offset;
	const UgHtmlParser* parser;
	void*               param1;
	void*               param2;
};

// parser for UgHtmlEntry

extern const UgHtmlParser  ug_html_parser_entry;

void  ug_html_start_element_entry (UgHtml*        uhtml,
                                   const char*    element_name,
                                   const char**   attribute_names,
                                   const char**   attribute_values,
                                   void*          dest,
                                   void*          entry);

// UgHtmlEntry.name   = element_name
// UgHtmlEntry.offset = offset of structure

// if UgHtmlEntry.parser == NULL, this entry is null-terminated.

// if UgHtmlEntry.name == NULL, it can match any name.
// it usually uses at first or last entry.

// if UgHtmlEntry.parser == ug_html_parser_custom
// UgEntry.param1 = start element function (UgHtmlParserStartElementFunc)
// UgEntry.param2 = custom writer function

// UgHtmlEntry.parser == ug_html_parser_int, ug_html_parser_string...etc
// UgHtmlEntry.param2 = UgHtmlEntry for attribute

// UgHtmlEntry.parser == ug_html_parser_string
// If you don't want to output anything when string value is NULL,
// set UgHtmlEntry.param1 != NULL.
// UgHtmlEntry.param2 = UgHtmlEntry for attribute

// UgHtmlEntry.parser == ug_html_parser_entry
// UgHtmlEntry.param1 = UgHtmlEntry
// UgHtmlEntry.param2 = UgHtmlEntry for attribute

// ------------------------------------
// parser for unknown element

extern const UgHtmlParser  ug_html_parser_unknown;

void ug_html_start_element_unknown (UgHtml*        uhtml,
                                    const char*    element_name,
                                    const char**   attribute_names,
                                    const char**   attribute_values,
                                    void*          dest,
                                    void*          data);

void ug_html_end_element_unknown (UgHtml*        uhtml,
                                  const char*    element_name,
                                  void*          dest,
                                  void*          data);

// ------------------------------------
// parser for C type

extern const UgHtmlParser  ug_html_parser_bool;
extern const UgHtmlParser  ug_html_parser_int;
extern const UgHtmlParser  ug_html_parser_uint;
extern const UgHtmlParser  ug_html_parser_int64;
extern const UgHtmlParser  ug_html_parser_uint64;
extern const UgHtmlParser  ug_html_parser_double;
extern const UgHtmlParser  ug_html_parser_string;
extern const UgHtmlParser  ug_html_parser_custom;

// ------------------------------------
// parser for time_t.
// RFC3339: 2012-05-23T18:30:00.000-05:00
//          2013-09-12T22:50:20+08:00
// RFC822:  Sat, 07 Sep 2002 00:00:01 GMT

extern const UgHtmlParser  ug_html_parser_time_rfc822;
extern const UgHtmlParser  ug_html_parser_time_rfc3339;

#ifdef __cplusplus
}
#endif


#endif  // UG_HTML_ENTRY_H
