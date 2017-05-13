/*
 *
 *   Copyright (C) 2012-2017 by C.H. Huang
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

#ifndef UG_FILE_UTIL_H
#define UG_FILE_UTIL_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef HAVE_GLIB
#include <glib.h>
#endif

#if !(defined _WIN32 || defined _WIN64)
#include <sys/types.h>
#include <dirent.h>      // opendir(), closedir(), readdir()
#endif

#include <time.h>
#include <UgList.h>

#ifdef __cplusplus
extern "C" {
#endif

// ----------------------------------------------------------------------------
// UgDir

#if defined HAVE_GLIB
typedef        GDir    UgDir;
#  define   ug_dir_open(p)  g_dir_open(p, 0, NULL)
#  define   ug_dir_close    g_dir_close
#  define   ug_dir_rewind   g_dir_rewind
#  define   ug_dir_read     g_dir_read_name
#elif defined _WIN32 || defined _WIN64
typedef struct UgDir   UgDir;
UgDir*      ug_dir_open (const char* path_utf8);
void        ug_dir_close (UgDir* udir);
void        ug_dir_rewind (UgDir* udir);
const char* ug_dir_read (UgDir* udir);
#else
typedef        DIR     UgDir;
#  define   ug_dir_open     opendir
#  define   ug_dir_close    closedir
#  define   ug_dir_rewind   rewinddir
const char* ug_dir_read (UgDir* udir);
#endif


// ----------------------------------------------------------------------------
// Time

// Change the modified time of file
int   ug_modify_file_time (const char *file_utf8, time_t mod_time);

// ----------------------------------------------------------------------------
// file & directory functions

#if defined _WIN32 || defined _WIN64 || defined HAVE_GLIB
int   ug_create_dir (const char *dir_utf8);
int   ug_delete_dir (const char *dir_utf8);
#else
#  define ug_create_dir(dir)    mkdir(dir,0755)
#  define ug_delete_dir         rmdir
#endif

int   ug_file_is_exist (const char* file_utf8);
int   ug_file_is_dir (const char* file_utf8);
int   ug_create_dir_all (const char* dir_utf8, int len);
//int ug_delete_dir_all (const char* dir_utf8, int len);

// ----------------------------------------------------------------------------
// File I/O

// return -1 if error
int   ug_file_copy (const char *src_file_utf8, const char *dest_file_utf8);
// return number of lines
int   ug_file_get_lines (const char* filename_utf8, UgList* list);

#ifdef __cplusplus
}
#endif

#endif // End of UG_FILE_UTIL_H

