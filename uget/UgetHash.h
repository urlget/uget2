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

#ifndef UGET_HASH_H
#define UGET_HASH_H

#include <UgetNode.h>

#ifdef __cplusplus
extern "C" {
#endif

// ----------------------------------------------------------------------------
// URI hash table

#ifdef NO_URI_HASH

#define uget_uri_hash_new()
#define uget_uri_hash_free(uuhash)

#define uget_uri_hash_find(uuhash, uri)
#define uget_uri_hash_add(uuhash, uri)      FALSE
#define uget_uri_hash_remove(uuhash, uri)

#define uget_uri_hash_add_download(uuhash, dnode)
#define uget_uri_hash_remove_download(uuhash, dnode)

#define uget_uri_hash_add_category(uuhash, cnode)
#define uget_uri_hash_remove_category(uuhash, cnode)

#else

void* uget_uri_hash_new (void);
void  uget_uri_hash_free (void* uuhash);

int   uget_uri_hash_find (void* uuhash, const char* uri);
void  uget_uri_hash_add (void* uuhash, const char* uri);
void  uget_uri_hash_remove (void* uuhash, const char* uri);

void  uget_uri_hash_add_download (void* uuhash, UgInfo* dnode_info);
void  uget_uri_hash_remove_download (void* uuhash, UgInfo* dnode_info);

void  uget_uri_hash_add_category (void* uuhash, UgetNode* cnode);
void  uget_uri_hash_remove_category (void* uuhash, UgetNode* cnode);

#endif  // End of NO_URI_HASH

#ifdef __cplusplus
}
#endif

#endif  // End of UGET_HASH_H
