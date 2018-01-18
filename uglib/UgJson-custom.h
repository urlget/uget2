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

// JSON parser and writer for custom type
#ifndef UG_JSON_CUSTOM_H
#define UG_JSON_CUSTOM_H

#include <UgJson.h>
#include <UgEntry.h>
#include <UgValue.h>

#ifdef __cplusplus
extern "C" {
#endif

// ----------------------------------------------------------------------------
// parse and write JSON number as JSON string

// below 4 functions can parse JSON number and JSON string.
UgJsonError  ug_json_parse_int_string(UgJson* json,
                                      const char* name, const char* value,
                                      void* dest, void* data);
UgJsonError  ug_json_parse_uint_string(UgJson* json,
                                       const char* name, const char* value,
                                       void* dest, void* data);
UgJsonError  ug_json_parse_int64_string(UgJson* json,
                                        const char* name, const char* value,
                                        void* dest, void* data);
UgJsonError  ug_json_parse_double_string(UgJson* json,
                                         const char* name, const char* value,
                                         void* dest, void* data);

void  ug_json_write_int_string(UgJson* json, int* value);
void  ug_json_write_uint_string(UgJson* json, unsigned int* value);
void  ug_json_write_int64_string(UgJson* json, int64_t* value);
void  ug_json_write_double_string(UgJson* json, double* value);

// ----------------------------------------------------------------------------
// parse string "true" and "false" to integer (boolean).
// write integer (boolean) to string "true" and "false".
UgJsonError  ug_json_parse_bool_string(UgJson* json,
                                       const char* name, const char* value,
                                       void* dest, void* data);
void  ug_json_write_bool_string(UgJson* json, int* value);

// ----------------------------------------------------------------------------
// UgJsonParseFunc and UgJsonWriteFunc for time_t

UgJsonError ug_json_parse_time_t(UgJson* json,
                                 const char* name, const char* value,
                                 void* dest, void* none);
void  ug_json_write_time_t(UgJson* json, void* src);

// ----------------------------------------------------------------------------
// UgJson parse by UgValue: convert UgValue to UgEntry
// e.g.
// ug_json_begin_parse(json);
// ug_json_push(json, ug_json_parse_entry, dest, entry);
// ug_json_parse_by_value(json, value);
// ug_json_end_parse(json);

void  ug_json_parse_by_value(UgJson* json, UgValue* value);


#ifdef __cplusplus
}
#endif  // __cplusplus

#endif  // UG_JSON_CUSTOM_H

