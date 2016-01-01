/*
 *
 *   Copyright (C) 2011-2016 by C.H. Huang
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

// Aria2 Control File for uGet
#ifndef UGET_A2CF_H
#define UGET_A2CF_H

#include <stdint.h>
#include <UgList.h>

#ifdef __cplusplus
extern "C" {
#endif

// A2cf = Aria2 Control File
typedef struct UgetA2cf         UgetA2cf;
typedef struct UgetA2cfPiece    UgetA2cfPiece;

struct UgetA2cfPiece
{
	UG_LINK_MEMBERS (UgetA2cfPiece, UgetA2cfPiece, self);
//	UgetA2cfPiece*     self;
//	UgetA2cfPiece*     next;
//	UgetA2cfPiece*     prev;

	uint32_t    index;
	uint32_t    length;
	uint32_t    bitfield_len;
	uint8_t     bitfield[1];
};

struct UgetA2cf
{
	uint16_t     ver;
	uint32_t     ext;
	uint32_t     info_hash_len;
	uint8_t*     info_hash;

	uint32_t     piece_len;
	uint64_t     total_len;
	uint64_t     upload_len;
	uint32_t     bitfield_len;
	uint8_t*     bitfield;

	// aria2 control file has this field.
//	uint32_t     n_pieces;

	// piece
	struct {
		UgList   list;
		uint32_t index_end;
	} piece;
};

void  uget_a2cf_init (UgetA2cf* a2cf, uint64_t total_size);
void  uget_a2cf_clear (UgetA2cf* a2cf);
// return TRUE if successful.
int   uget_a2cf_load (UgetA2cf* a2cf, const char* filename);
int   uget_a2cf_save (UgetA2cf* a2cf, const char* filename);

// beg [in, out]: pass search position and return new begin position
// end [out]    : return end position
int       uget_a2cf_lack (UgetA2cf* a2cf, uint64_t* beg, uint64_t* end);
uint64_t  uget_a2cf_fill (UgetA2cf* a2cf, uint64_t  beg, uint64_t  end);

uint64_t  uget_a2cf_completed (UgetA2cf* a2cf);

void            uget_a2cf_insert (UgetA2cf* a2cf, UgetA2cfPiece* piece);
UgetA2cfPiece*  uget_a2cf_find (UgetA2cf* a2cf, uint32_t piece_index);
UgetA2cfPiece*  uget_a2cf_realloc (UgetA2cf* a2cf, uint32_t piece_index);

#ifdef __cplusplus
}
#endif  // __cplusplus

#endif  // End of UGET_A2CF_H
