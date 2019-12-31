/*
 *
 *   Copyright (C) 2005-2020 by C.H. Huang
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

typedef struct	UgType           UgType;
typedef struct  UgTypeInfo       UgTypeInfo;
typedef struct	UgData           UgData;
typedef struct	UgDataInfo       UgDataInfo;

typedef int   (*UgAssignFunc) (void* instance, void* src);

// ----------------------------------------------------------------------------
// UgTypeInfo: a base type info for UgDataInfo and UgetPluginInfo

#define	UG_TYPE_INFO_MEMBERS  \
	const char*     name;     \
	uintptr_t       size;     \
	UgInitFunc      init;     \
	UgFinalFunc     final

struct UgTypeInfo
{
	UG_TYPE_INFO_MEMBERS;
/*	// ------ UgTypeInfo members ------
	const char*     name;
	uintptr_t       size;
	UgInitFunc      init;
	UgFinalFunc     final;
 */
};

// ----------------------------------------------------------------------------
// UgType: a base type for UgData and UgetPlugin

#define	UG_TYPE_MEMBERS  \
	const UgTypeInfo*  info

struct UgType
{
	UG_TYPE_MEMBERS;
//	const UgTypeInfo*  info;    // UgType member
};

// void*   ug_type_new(const UgTypeInfo* typeinfo);
void*      ug_type_new(const void* typeinfo);
void       ug_type_free(void* type);
void       ug_type_init(void* type);
void       ug_type_final(void* type);

/* ----------------------------------------------------------------------------
   UgDataInfo

   UgTypeInfo
   |
   `-- UgDataInfo
 */

#define	UG_DATA_INFO_MEMBERS  \
	UG_TYPE_INFO_MEMBERS;     \
	UgAssignFunc    assign;   \
	const UgEntry*  entry

struct UgDataInfo
{
	UG_DATA_INFO_MEMBERS;
/*	// ------ UgTypeInfo members ------
	const char*     name;
	uintptr_t       size;
	UgInitFunc      init;
	UgFinalFunc     final;

	// ------ UgDataInfo members ------
	UgAssignFunc    assign;
	const UgEntry*	entry;
 */
};

/* ----------------------------------------------------------------------------
   UgData: a group of data that store in UgInfo.

   UgType
   |
   `-- UgData
 */

#define	UG_DATA_MEMBERS  \
	const UgDataInfo*  info

struct UgData
{
	UG_DATA_MEMBERS;
//	const UgDataInfo*  info;    // UgType member
};

// UgData* ug_data_new(const UgDataInfo* dinfo);
// void    ug_data_free(UgData* data);
#define    ug_data_new    ug_type_new
#define    ug_data_free   ug_type_free

// void    ug_data_init(void* data);
// void    ug_data_final(void* data);
#define    ug_data_init   ug_type_new
#define    ug_data_final  ug_type_final

// UgData* ug_data_copy(UgData* data);
// void    ug_data_assign(UgData* data, UgData* src);
void*      ug_data_copy(void* data);
int        ug_data_assign(void* data, void* src);

// UgJsonParseFunc for UgData, used by UgEntry with UG_ENTRY_CUSTOM
UgJsonError ug_json_parse_data(UgJson* json,
                               const char* name, const char* value,
                               void* data, void* none);
// write UgData, used by UgEntry with UG_ENTRY_CUSTOM
void        ug_json_write_data(UgJson* json, const UgData* data);


#ifdef __cplusplus
}
#endif

// ----------------------------------------------------------------------------
// C++11 standard-layout

#ifdef __cplusplus

namespace Ug
{
typedef struct UgTypeInfo    TypeInfo;
typedef struct UgDataInfo    DataInfo;

// This one is for derived use only. No data members here.
// Your derived struct/class must be C++11 standard-layout
template <class DataType>
struct DataMethod
{
	inline void* operator new(size_t size, const UgDataInfo* dinfo)
		{ return ug_data_new(dinfo); }
	inline void  operator delete(void* p)
		{ ug_data_free(p); }

	/*
	inline void init()
		{ ug_data_init((void*)this); }
	inline void final(void)
		{ ug_data_final((void*)this); }
	*/

	inline int  assign(DataType* src)
		{ return ug_data_assign((void*)this, (void*)src); }
	inline DataType* copy(void)
		{ return (DataType*)ug_data_copy((void*)this); }
};

// This one is for directly use only. You can NOT derived it.
struct Data : DataMethod<Data>, UgData {};

};  // namespace Ug

#endif  // __cplusplus


#endif  // UG_DATA_H

