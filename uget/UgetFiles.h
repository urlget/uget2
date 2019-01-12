/*
 *
 *   Copyright (C) 2018-2019 by C.H. Huang
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

#ifndef UGET_FILES_H
#define UGET_FILES_H

#include <stdint.h>
#include <UgList.h>
#include <UgData.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct  UgetFile       UgetFile;
typedef struct  UgetFiles      UgetFiles;

extern  const   UgDataInfo*    UgetFilesInfo;

enum UgetFileType
{
	UGET_FILE_REGULAR,
	UGET_FILE_FOLDER,
	UGET_FILE_ATTACHMENT,  // torrent, metalink, or HTTP POST file
	UGET_FILE_TEMPORARY,   // temporary file.

	UGET_FILE_ALL,
};

enum UgetFileState
{
	// state for torrent or metalink
//	UGET_FILE_STATE_IGNORE       = 0x0001,
//	UGET_FILE_STATE_SOURCE       = 0x0002,

	// state for output (actually write into storage device)
	UGET_FILE_STATE_DELETED      = 0x0004,  // this file was deleted/renamed
	UGET_FILE_STATE_COMPLETED    = 0x0008,

	UGET_FILE_STATE_ALL          = 0x00FF,
};

// ----------------------------------------------------------------------------
// UgetFile functions

UgetFile*  uget_file_new(void);
void       uget_file_free(UgetFile* file);

/* ----------------------------------------------------------------------------
   UgetFiles: It derived from UgData and store in UgInfo.

   UgType
   |
   `-- UgData
       |
       `-- UgetFiles
 */

struct UgetFiles
{
	UG_DATA_MEMBERS;
//	const UgDataInfo*  info;    // UgData(UgType) member

	UgList  list;

	int     sync_count;
};

int   uget_files_assign(UgetFiles* files, UgetFiles* src);

void  uget_files_clear(UgetFiles* files);

// sync elements from 'src' to 'files.
// 1. all elements in 'src' will insert/replace into 'files'.
// 2. remove deleted (state == UGET_FILE_STATE_DELETED) elements in 'src'.
// return TRUE if 'files' have added or removed elements.
int   uget_files_sync(UgetFiles* files, UgetFiles* src);

UgetFile* uget_files_find(UgetFiles* files, const char* path,
                          UgetFile** sibling);

// realloc struct UgetFile by 'path' in array.
UgetFile* uget_files_realloc(UgetFiles* files, const char* path);

UgetFile* uget_files_replace(UgetFiles* files, const char* path,
                             int type, int state);

// apply state to element if type is matched.
void  uget_files_apply(UgetFiles* files, int type, int state);
// erase element by state if type is matched.
void  uget_files_erase(UgetFiles* files, int type, int state);

#define uget_files_apply_deleted(files)  \
		uget_files_apply(files, UGET_FILE_ALL, UGET_FILE_STATE_DELETED)
#define uget_files_erase_deleted(files)  \
		uget_files_erase(files, UGET_FILE_ALL, UGET_FILE_STATE_DELETED)

#ifdef __cplusplus
}
#endif

// ----------------------------------------------------------------------------
// UgetFile structure: file information with list link

struct UgetFile
{
	UG_LINK_MEMBERS(UgetFile, char, path);
/*	// ------ UgLink members ------
	char*       path;    // absolute file path
	UgetFile*   next;
	UgetFile*   prev;
 */

	int16_t type;    // UgetFileType
	int16_t state;   // UgetFileState

	// save original index in torrent and metalink file.
//	int32_t order;

	// progress
	int64_t total;
	int64_t complete;

#ifdef __cplusplus
	inline void* operator new(size_t size)
		{ return uget_file_new(); }
	inline void  operator delete(void* p)
		{ uget_file_free((UgetFile*)p); }
#endif  // __cplusplus
};

// ----------------------------------------------------------------------------
// C++11 standard-layout

#ifdef __cplusplus

namespace Uget
{

const UgDataInfo* const FilesInfo = UgetFilesInfo;

// These are for directly use only. You can NOT derived it.
typedef struct UgetFile    File;

struct Files : Ug::DataMethod<Files>, UgetFiles
{
	inline void* operator new(size_t size)
		{ return ug_data_new(UgetFilesInfo); }

	inline int    sync(UgetFiles* src)
		{ return uget_files_sync(this, src); }
	inline UgetFile* find(const char* path, UgetFile** sibling)
		{ return uget_files_find(this, path, sibling); }

	inline UgetFile* realloc(const char* path)
		{ return uget_files_realloc(this, path); }
	inline UgetFile* replace(const char* path,int type, int state)
		{ return uget_files_replace(this, path, type, state); }

	inline void   apply(int type, int state)
		{ uget_files_apply(this, type, state); }
	inline void   erase(int type, int state)
		{ uget_files_erase(this, type, state); }

	inline void   apply_deleted(void)
		{ uget_files_apply(this, UGET_FILE_ALL, UGET_FILE_STATE_DELETED); }
	inline void   erase_deleted(void)
		{ uget_files_erase(this, UGET_FILE_ALL, UGET_FILE_STATE_DELETED); }
};

};  // namespace Uget

#endif  // __cplusplus


#endif  // End of UGET_FILES_H

