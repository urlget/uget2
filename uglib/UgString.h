/*
 *
 *   Copyright (C) 2012-2019 by C.H. Huang
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

#ifndef UG_STRING_H
#define UG_STRING_H

#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#ifndef strcasecmp
#define strcasecmp   stricmp
#define strncasecmp  strnicmp
#endif // strcasecmp
#endif // _MSC_VER

#include <time.h>        // time_t
#include <string.h>
#include <stdint.h>
#include <UgDefine.h>

#ifdef __cplusplus
extern "C" {
#endif

// ----------------------------------------------------------------------------
// String

#ifdef HAVE_GLIB
#define ug_strdup_printf   g_strdup_printf
#define ug_strndup         g_strndup
#define ug_strdup          g_strdup
#else
char*   ug_strdup_printf (const char* format, ...);
char*   ug_strndup (const char* string, size_t length);
#define ug_strdup          strdup
#endif

// return length of new string.
// param dest can be param src or NULL.
int    ug_str_remove_crlf (const char* src, char* dest);
int    ug_str_remove_chars (const char* src, char* dest, const char* chars);

// return number of characters was replaced by to_char
int    ug_str_replace_chars (char* str, const char* from_chars, int to_char);

/*
 * convert double to string
 * If value large than 1024, it will append unit string like "KiB",
 * "MiB", "GiB", "TiB", or "PiB"  to string.
 */
char*  ug_str_from_int_unit (int64_t value, const char* tail);

/*
 * convert seconds to string (hh:mm:ss)
 */
char*  ug_str_from_seconds (int seconds, int limit_99_99_99);

/*
 * convert time_t to string
 */
char*  ug_str_from_time (time_t ptt, int date_only);


// RFC3339: 2013-09-12T22:50:20+08:00
// RFC822:  Sat, 07 Sep 2002 00:00:01 GMT
// If the calendar time cannot be represented, a value of -1 is returned.
time_t  ug_str_rfc822_to_time  (const char* rfc822_string);
time_t  ug_str_rfc3339_to_time (const char* rfc3339_string);

// ------------------------------------
// command-line
char** ug_argv_from_cmd (const char* commandline, int* argc, int reserve_len);
void   ug_argv_free (char** ug_argv);

#ifdef __cplusplus
}
#endif

#endif // End of UG_STRING_H

