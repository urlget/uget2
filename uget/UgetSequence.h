/*
 *
 *   Copyright (C) 2016-2018 by C.H. Huang
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

#ifndef UGET_SEQUENCE_H
#define UGET_SEQUENCE_H

#include <stdint.h>
#include <UgList.h>
#include <UgArray.h>
#include <UgBuffer.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct	UgetSequence    UgetSequence;
typedef struct	UgetSeqRange    UgetSeqRange;

struct UgetSeqRange
{
	// [first, last]
	// e.g. 0-9, A-Z, a-z, or Unicode
	uint32_t  first;
	uint32_t  last;
	uint32_t  cur;

	// if digits == 0, use ASCII or Unicode to generate string.
	int  digits;
};

struct UgetSequence
{
	UG_ARRAY_MEMBERS (UgetSeqRange);
//	UgetSeqRange*  at;
//	int    length;
//	int    allocated;
//	int    element_size;

	UgetSeqRange*  range_last;    // used by uget_sequence_get_list()
	UgBuffer       buf;
};

void uget_sequence_init (UgetSequence* useq);
void uget_sequence_final (UgetSequence* useq);

void uget_sequence_add (UgetSequence* useq, uint32_t first, uint32_t last, int digits);
void uget_sequence_clear (UgetSequence* useq);

int  uget_sequence_count (UgetSequence* useq, const char* pattern);

// call ug_list_foreach_link (result, (UgForeachFunc)ug_free, NULL) to free result list
int  uget_sequence_get_list (UgetSequence* useq, const char* pattern, UgList* result);
int  uget_sequence_get_preview (UgetSequence* useq, const char* pattern, UgList* result);

// param: pattern is a string that contain wildcard character *.
// e.g. Number-*.jpg
//      Number-0.jpg, Number-1.jpg ...etc
//      Number-a.jpg, Number-b.jpg ...etc
//      Number-A.jpg, Number-B.jpg ...etc


#ifdef __cplusplus
}
#endif

#endif  // UGET_SEQUENCE_H
