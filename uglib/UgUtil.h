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

#ifndef UG_UTIL_H
#define UG_UTIL_H

#include <time.h>
#include <stdint.h>
#include <UgList.h>

#ifdef __cplusplus
extern "C" {
#endif

// ----------------------------------------------------------------------------
// Time

uint64_t   ug_get_time_count (void);

// ----------------------------------------------------------------------------
// Unicode

int        ug_utf8_get_invalid (const char* input, char* ch);

uint16_t*  ug_utf8_to_utf16 (const char* string, int stringLength,
                             int* utf16len);

char*      ug_utf16_to_utf8 (const uint16_t* string, int stringLength,
                             int* utf8len);

uint32_t*  ug_utf8_to_ucs4 (const char* string, int stringLength,
                            int* ucs4len);

char*      ug_ucs4_to_utf8 (const uint32_t* string, int stringLength,
                            int* utf8len);

// ----------------------------------------------------------------------------
// Base64

char*  ug_base64_encode (const unsigned char* data,
                         int  input_length,
                         int* output_length);

unsigned char* ug_base64_decode (const char* data,
                                 int  input_length,
                                 int* output_length);

// ----------------------------------------------------------------------------
// filename & path functions

char*  ug_build_filename (const char* first_element, ...);

// ----------------------------------------------------------------------------
// Power Management

// Suspend does not turn off your computer. It puts the computer and all peripherals on a low power consumption mode.
// Hibernate saves the state of your computer to the hard disk and completely powers off.

void  ug_reboot (void);
void  ug_shutdown (void);
void  ug_suspend (void);
void  ug_hibernate (void);

// ----------------------------------------------------------------------------
// Others

char* ug_sys_release (void);

#ifdef __cplusplus
}
#endif

#endif // End of UG_UTIL_H

