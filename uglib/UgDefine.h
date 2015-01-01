/*
 *
 *   Copyright (C) 2012-2015 by C.H. Huang
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

#ifndef UG_DEFINE_H
#define UG_DEFINE_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef HAVE_GLIB
#include <glib.h>
#define ug_malloc          g_malloc
#define ug_malloc0         g_malloc0
#define ug_realloc         g_realloc
#define ug_free            g_free
#else
#include <stdlib.h>    // malloc, calloc, free
#define ug_malloc          malloc
#define ug_malloc0(size)   calloc (1, size)
#define ug_realloc         realloc
#define ug_free            free
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef void  (*UgInitFunc)   (void* instance);
typedef void  (*UgFinalFunc)  (void* instance);

typedef void* (*UgCreateFunc) (void);
typedef void  (*UgDeleteFunc) (void* instance);

typedef void  (*UgNotifyFunc) (void* user_data);

typedef void  (*UgForeachFunc)(void* instance, void* data);
typedef int   (*UgCompareFunc)(const void* a, const void* b);

#if defined _WIN32 || defined _WIN64
#define UG_DIR_SEPARATOR    '\\'
#else
#define UG_DIR_SEPARATOR    '/'
#endif  // _WIN32 || _WIN64

#ifndef TRUE
#define TRUE    1
#endif

#ifndef FALSE
#define FALSE   0
#endif

#ifdef __cplusplus
}
#endif

#endif  // UG_DEFINE_H

