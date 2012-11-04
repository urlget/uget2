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

#ifndef UG_OPTION_H
#define UG_OPTION_H

#include <stdint.h>
#include <UgArray.h>
#include <UgEntry.h>
#include <UgInfo.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct UgOption         UgOption;
typedef struct UgOptionEntry    UgOptionEntry;

// return TRUE if accepted by parse function.
typedef int  (*UgOptionParseFunc) (UgOption* option,
                                   const char* name,
                                   const char* value,
                                   void* dest, void* data);

// type = UG_ENTRY_OBJECT
// UgOptionEntry.data pointer to UgEntry

// type = UG_ENTRY_ARRAY
// UgOptionEntry.data pointer to UgOptionParseFunc

// type = UG_ENTRY_CUSTOM
// UgOptionEntry.data pointer to UgOptionParseFunc

struct UgOptionEntry
{
	char*  long_name;
	char*  short_name;
	int    offset;
	int    type;
	char*  description;
	char*  arg_description;
	void*  data;
};

void ug_option_entry_print_help (UgOptionEntry* entry,
                                 const char* prog_name,
                                 const char* parameter_string,
                                 const char* summary_string);

// ----------------------------------------------------------------------------

struct UgOption
{
	UgArrayStr others;      // begin character is not '-'
	UgArrayStr unaccepted;  // Not accepted by parse function

	uint8_t    short_name;
	uint8_t    discard_unaccepted;

	struct {
		UgOptionParseFunc  func;
		void*              dest;
		void*              data;
	} parse;
};

void  ug_option_init (UgOption* option);
void  ug_option_final (UgOption* option);

void  ug_option_clear (UgOption* option);

void  ug_option_set_parser (UgOption* option, UgOptionParseFunc func,
                            void* dest, void* data);

int   ug_option_parse (UgOption* option, const char* string, int length);

// UgOptionEntry* entry
int   ug_option_parse_entry (UgOption* option,
                             const char* name,
                             const char* value,
                             void* dest, void* entry);

#ifdef __cplusplus
}
#endif

#endif // UG_OPTION_H
