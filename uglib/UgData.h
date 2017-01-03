/*
 *
 *   Copyright (C) 2005-2017 by C.H. Huang
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

#ifndef UG_DATA_H
#define UG_DATA_H

#include <stdint.h>     // uintptr_t
#include <UgEntry.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct	UgData           UgData;
typedef struct	UgDataInfo       UgDataInfo;

typedef void  (*UgAssignFunc) (void* instance, void* src);

// ----------------------------------------------------------------------------
// UgDataInfo

#define	UG_DATA_INFO_MEMBERS  \
	const char*     name;    \
	uintptr_t       size;    \
	const UgEntry*  entry;   \
	UgInitFunc      init;    \
	UgFinalFunc     final;   \
	UgAssignFunc    assign

struct UgDataInfo
{
	UG_DATA_INFO_MEMBERS;
//	const char*     name;
//	uintptr_t       size;
//	const UgEntry*	entry;
//	UgInitFunc      init;
//	UgFinalFunc     final;
//	UgAssignFunc    assign;
};

// ----------------------------------------------------------------------------
// UgData
#define	UG_DATA_MEMBERS  \
	const UgDataInfo*  info

struct UgData
{
	UG_DATA_MEMBERS;
//	const UgDataInfo*  info;
};

// UgData* ug_data_new (const UgDataInfo* dinfo);
// void    ug_data_free (UgData* data);
void*      ug_data_new (const UgDataInfo* dinfo);
void       ug_data_free (void* data);

void       ug_data_init (void* data);
void       ug_data_final (void* data);

// UgData* ug_data_copy (UgData* data);
//void     ug_data_assign (UgData* data, UgData* src);
void*      ug_data_copy (void* data);
void       ug_data_assign (void* data, void* src);

// UgJsonParseFunc for UgData, used by UgEntry with UG_ENTRY_CUSTOM
UgJsonError ug_json_parse_data (UgJson* json,
                                const char* name, const char* value,
                                void* data, void* none);
// write UgData, used by UgEntry with UG_ENTRY_CUSTOM
void        ug_json_write_data (UgJson* json, const UgData* data);


#ifdef __cplusplus
}
#endif

// ----------------------------------------------------------------------------
// C++11 standard-layout

#ifdef __cplusplus

namespace Ug
{
typedef struct UgDataInfo    DataInfo;

// This one is for derived use only. No data members here.
// Your derived struct/class must be C++11 standard-layout
struct DataMethod
{
	inline void init (const UgDataInfo* info) {
		*(UgDataInfo**)this = (UgDataInfo*)info;
		ug_data_init ((void*)this);
	}
	inline void init ()
		{ ug_data_init ((void*)this); }
	inline void final (void)
		{ ug_data_final ((void*)this); }

	inline void assign (DataMethod* src)
		{ ug_data_assign ((void*)this, (void*)src); }
	inline DataMethod* copy (void)
		{ return (DataMethod*) ug_data_copy ((void*)this); }
};

// This one is for directly use only. You can NOT derived it.
struct Data : DataMethod, UgData {};

};  // namespace Ug

#endif  // __cplusplus


#endif  // UG_DATA_H

