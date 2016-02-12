/*
 *
 *   Copyright (C) 2005-2016 by C.H. Huang
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

#ifndef UG_URI_H
#define UG_URI_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Request for Comments: 3986  (RFC 3986)
 * Uniform Resource Identifier (URI):
 *
 *         uri://example.org:8080/road/where?name=ferret#nose
 *         \_/   \______________/\_________/ \_________/ \__/
 *          |           |            |            |        |
 *       scheme     authority       path        query   fragment
 *          |   _____________________|__
 *         / \ /                        \
 *         urn:example:animal:ferret:nose
 *
 *         uri://user:password@host:port/path/file.ext?query#fragment
 *         urn:path
 */

typedef struct UgUri              UgUri;

#define	UG_URI_MEMBERS  \
	const char*  uri;       \
	int16_t  scheme_len;    \
	int16_t  authority;     \
	int16_t  host;          \
	int16_t  port;          \
	int16_t  path;          \
	int16_t  file;          \
	int16_t  query;         \
	int16_t  fragment

struct UgUri
{
	UG_URI_MEMBERS;
//	const char*  uri;
//	int16_t  scheme_len;
//	int16_t  authority;
//	int16_t  host;
//	int16_t  port;
//	int16_t  path;
//	int16_t  file;
//	int16_t  query;
//	int16_t  fragment;
};

int  ug_uri_init (UgUri* uuri, const char* uri);
int  ug_uri_part_scheme   (UgUri* uuri, const char** scheme);
int  ug_uri_part_file     (UgUri* uuri, const char** file);
int  ug_uri_part_file_ext (UgUri* uuri, const char** ext);
int  ug_uri_part_query    (UgUri* uuri, const char** query);
int  ug_uri_part_fragment (UgUri* uuri, const char** fragment);
int  ug_uri_part_referrer (UgUri* uuri, const char** referrer);
int  ug_uri_part_user     (UgUri* uuri, const char** user);
int  ug_uri_part_password (UgUri* uuri, const char** password);
int  ug_uri_part_host     (UgUri* uuri, const char** host);
int  ug_uri_part_port     (UgUri* uuri, const char** port);

#define  ug_uri_scheme        ug_uri_part_scheme
#define  ug_uri_file          ug_uri_part_file
#define  ug_uri_file_ext      ug_uri_part_file_ext
#define  ug_uri_query         ug_uri_part_query
#define  ug_uri_fragment      ug_uri_part_fragment
#define  ug_uri_referrer      ug_uri_part_referrer
#define  ug_uri_user          ug_uri_part_user
#define  ug_uri_password      ug_uri_part_password
#define  ug_uri_host          ug_uri_part_host
#define  ug_uri_port          ug_uri_part_port

int   ug_uri_get_port (UgUri* uuri);
char* ug_uri_get_file (UgUri* uuri);

// ------------------------------------
// match uri functions

// return -1 if it can't match, return index if matched.
// second parameter must be null-terminated. Below are sample:
// hosts   = {".twodot.edu", "your.com", ".org", NULL};
// schemes = {"http", "ftp", NULL};
// exts    = {"torrent", "meta", NULL};
int  ug_uri_match_hosts   (UgUri* uuri, char** hosts);
int  ug_uri_match_schemes (UgUri* uuri, char** schemes);
int  ug_uri_match_file_exts (UgUri* uuri, char** exts);

// ------------------------------------
// UgUriQuery

// field1=value1&field2=value2&field3=value3,next_value

typedef struct UgUriQuery         UgUriQuery;

struct UgUriQuery {
	int    field_len;
	char*  value;
	int    value_len;

	char*  field_next;
	char*  value_next;
};

// param query_field can be NULL
// return 0: no field (end of query)
// return 1: field only
// return 2: field & value
//
// while (ug_uri_query_part (&uuquery, field) > 0) {
//	// your code here
//	field = uuquery->next;
// }
//
int  ug_uri_query_part (UgUriQuery* uuquery, const char* query_field);

// ------------------------------------
// Other URI functions

// return length of decoded uri. param dest can be param uri or NULL.
int   ug_decode_uri (const char* uri, int uri_length, char* dest);

char* ug_filename_from_uri (const char* uri);

#ifdef __cplusplus
}
#endif

// ----------------------------------------------------------------------------
// C++11 standard-layout

#ifdef __cplusplus

namespace Ug
{
// This one is for derived use only. No data members here.
// Your derived struct/class must be C++11 standard-layout
struct UriMethod
{
	inline void  init (const char* uri)
		{ ug_uri_init ((UgUri*) this, uri); }
//	inline void  final (void)
//		{}

	inline int  file (const char** file)
		{ return ug_uri_part_file ((UgUri*) this, file); }
	inline int  fileExt (const char** ext)
		{ return ug_uri_part_file_ext ((UgUri*) this, ext); }
	inline int  query (const char** query)
		{ return ug_uri_part_query ((UgUri*) this, query); }
	inline int  fragment (const char** fragment)
		{ return ug_uri_part_fragment ((UgUri*) this, fragment); }
	inline int  referrer (const char** referrer)
		{ return ug_uri_part_referrer ((UgUri*) this, referrer); }
	inline int  user (const char** user)
		{ return ug_uri_part_user ((UgUri*) this, user); }
	inline int  password (const char** password)
		{ return ug_uri_part_password ((UgUri*) this, password); }
	inline int  host (const char** host)
		{ return ug_uri_part_host ((UgUri*) this, host); }
	inline int  port (const char** port)
		{ return ug_uri_part_port ((UgUri*) this, port); }
	inline int  port (void)
		{ return ug_uri_get_port ((UgUri*) this); }

	// return -1 if it can't match, return index if matched.
	// parameter must be null-terminated. Below are sample:
	// hosts   = {".twodot.edu", "your.com", ".org", NULL};
	// schemes = {"http", "ftp", NULL};
	// exts    = {"torrent", "meta", NULL};
	inline int  matchHosts (char** hosts)
		{ return ug_uri_match_hosts ((UgUri*) this, hosts); }
	inline int  matchSchemes (char** schemes)
		{ return ug_uri_match_schemes ((UgUri*) this, schemes); }
	inline int  matchFileExts (char** exts)
		{ return ug_uri_match_file_exts ((UgUri*) this, exts); }
};

// This one is for directly use only. You can NOT derived it.
struct Uri : UriMethod, UgUri {};

};  // namespace Ug

#endif  // __cplusplus

#endif	// UG_URI_H


