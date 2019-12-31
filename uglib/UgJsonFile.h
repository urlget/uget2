/*
 *
 *   Copyright (C) 2012-2020 by C.H. Huang
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

#ifndef UG_JSON_FILE_H
#define UG_JSON_FILE_H

#include <UgJson.h>
#include <UgBuffer.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct UgJsonFile            UgJsonFile;

// ----------------------------------------------------------------------------
// UgJsonFile: JSON file loader and writer

struct UgJsonFile
{
	UgJson    json;
	UgBuffer  buffer;
	int       fd;
	int       n_bytes;
	char      bytes[1];
};

UgJsonFile*  ug_json_file_new (int buffer_size);
void         ug_json_file_free (UgJsonFile* jfile);

// return TRUE or FALSE
int   ug_json_file_begin_parse (UgJsonFile* jfile, const char* filename);
int   ug_json_file_begin_write (UgJsonFile* jfile, const char* filename, UgJsonFormat format);
int   ug_json_file_begin_parse_fd (UgJsonFile* jfile, int fd);
int   ug_json_file_begin_write_fd (UgJsonFile* jfile, int fd, UgJsonFormat format);

UgJsonError  ug_json_file_end_parse (UgJsonFile* jfile);
void         ug_json_file_end_write (UgJsonFile* jfile);

#ifdef __cplusplus
}
#endif

#endif  // UG_JSON_FILE_H


