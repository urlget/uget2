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

#ifndef UG_HTML_FILTER_H
#define UG_HTML_FILTER_H

#include <UgHtml.h>
#include <UgList.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef	struct  UgHtmlFilter       UgHtmlFilter;
typedef	struct  UgHtmlFilterTag    UgHtmlFilterTag;

typedef void  (*UgHtmlFilterTagFunc) (UgHtmlFilterTag* tag, const char* value, void* user_data);

// ----------------------------------------------------------------------------
// UgHtmlFilter
struct	UgHtmlFilter
{
	UgHtml html;

	char*  base_href;
	char*  charset;

	const char*  element_name;

	UgList tags;
};

UgHtmlFilter*  ug_html_filter_new  (void);
void           ug_html_filter_free (UgHtmlFilter* filter);

void  ug_html_filter_add_tag  (UgHtmlFilter* filter, UgHtmlFilterTag* tag);
int   ug_html_filter_parse_file (UgHtmlFilter* filter, const char* file_utf8);

// ----------------------------------------------------------------------------
// UgHtmlFilterTag

struct UgHtmlFilterTag
{
	UG_LINK_MEMBERS (UgHtmlFilterTag, UgHtmlFilterTag, self);
//	UgHtmlFilterTag*    self;
//	UgHtmlFilterTag*    next;
//	UgHtmlFilterTag*    prev;

	int     ref_count;

	char*   tag_name;		// element name
	char*   attr_name;		// attribute name
	UgList  attr_values;	// attribute values, string list

	UgHtmlFilter*     filter;

	// callback
	UgHtmlFilterTagFunc  callback;
	void*                callback_data;
};

UgHtmlFilterTag*  ug_html_filter_tag_new (char* element_name, char* attr_name);

void  ug_html_filter_tag_ref   (UgHtmlFilterTag* tag);
void  ug_html_filter_tag_unref (UgHtmlFilterTag* tag);

#ifdef __cplusplus
}
#endif


#endif  // UG_HTML_FILTER_H
