/*
 *
 *   Copyright (C) 2012-2015 by C.H. Huang
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <UgDefine.h>
#include <UgUtil.h>
#include <UgFileUtil.h>
#include <UgStdio.h>    // ug_create_dir
#include <UgString.h>   // ug_strdup

#if defined _WIN32 || defined _WIN64
#include <windows.h>
#include <sys/utime.h>   // struct utimbuf
//#include <PowrProf.h>    // SetSuspendState()
#else
#include <unistd.h>
#include <utime.h>       // struct utimbuf
#include <sys/time.h>
#endif

// ----------------------------------------------------------------------------
// Time

#if defined _WIN32 || defined _WIN64
int  ug_modify_file_time (const char *file_utf8, time_t mod_time)
{
	struct _utimbuf utb;
	wchar_t*        file;
	int             retval;

	utb.actime = time (NULL);
	utb.modtime = mod_time;
	file = (wchar_t*) ug_utf8_to_utf16 (file_utf8, -1, NULL);
	retval = _wutime (file, &utb);
	ug_free (file);

	return retval;
}
#elif defined HAVE_GLIB
int  ug_modify_file_time (const char *file_utf8, time_t mod_time)
{
	struct utimbuf  utb;
	gchar*          file;
	int             retval;

	utb.actime = time (NULL);
	utb.modtime = mod_time;
	file = g_filename_from_utf8 (file_utf8, -1, NULL, NULL, NULL);
	retval = g_utime (file, &utb);
	g_free (file);

	return retval;
}
#else
int  ug_modify_file_time (const char *file_utf8, time_t mod_time)
{
	struct utimbuf  utb;
	int             retval;

	utb.actime = time (NULL);
	utb.modtime = mod_time;
	retval = utime (file_utf8, &utb);

	return retval;
}
#endif

// ----------------------------------------------------------------------------
// file and directory functions

#if defined _WIN32 || defined _WIN64
int   ug_file_is_exist (const char* filename)
{
	int      attributes;
	wchar_t *wfilename = ug_utf8_to_utf16 (filename, -1, NULL);

	if (wfilename == NULL)
		return FALSE;
	attributes = GetFileAttributesW (wfilename);
	ug_free (wfilename);
	if (attributes == INVALID_FILE_ATTRIBUTES)
		return FALSE;
	return TRUE;
}

int   ug_file_is_dir (const char* dir)
{
	int      attributes;
	wchar_t *wfilename = ug_utf8_to_utf16 (dir, -1, NULL);

	if (wfilename == NULL)
		return FALSE;
	attributes = GetFileAttributesW (wfilename);
	ug_free (wfilename);

	if ((attributes & FILE_ATTRIBUTE_DIRECTORY) != 0)
		return TRUE;
	return FALSE;
}

#elif defined HAVE_GLIB

int   ug_file_is_exist (const char* filename)
{
	gchar *name;
	int    result;

	name = g_filename_from_utf8 (filename, -1, NULL, NULL, NULL);
	result = access (name, F_OK);
	g_free (name);
	if (result == -1)
		return FALSE;
	return TRUE;
}

int   ug_file_is_dir (const char* dir)
{
	struct stat s;
	gchar *cp_dir;
	int    result;

	cp_dir = g_filename_from_utf8 (dir, -1, NULL, NULL, NULL);
	result = stat (cp_dir, &s) == 0;
	g_free (cp_dir);
	if (result == 0)
		return S_ISDIR (s.st_mode);
	return FALSE;
}

#else

int   ug_file_is_exist (const char* filename)
{
	if (access (filename, F_OK) == -1)
		return FALSE;
	return TRUE;
}

int   ug_file_is_dir (const char* dir)
{
	struct stat s;

	if (stat (dir, &s) == 0)
		return S_ISDIR (s.st_mode);
	return FALSE;
}

#endif // _WIN32 || _WIN64

// This function use complex way to handle directory because some locale encoding doesn't avoid '\\' or '/'.
int  ug_create_dir_all (const char* dir, int len)
{
	const char*   dir_end;
	const char*   element_end;	// path element
	char*         element_os;

	if (len == -1)
		len = strlen (dir);
	dir_end = dir + len;
	element_end = dir;

	for (;;) {
		// skip directory separator "\\\\" or "//"
		for (;  element_end < dir_end;  element_end++) {
			if (*element_end != UG_DIR_SEPARATOR)
				break;
		}
		if (element_end == dir_end)
			return 0;
		// get directory name [dir, element_end)
		for (;  element_end < dir_end;  element_end++) {
			if (*element_end == UG_DIR_SEPARATOR)
				break;
		}
		// create directory by locale encoding name.
		element_os = (char*) ug_malloc (element_end - dir + 1);
		element_os[element_end - dir] = 0;
		strncpy (element_os, dir, element_end - dir);

		if (element_os == NULL)
			break;
		if (ug_file_is_exist (element_os) == FALSE) {
			if (ug_create_dir (element_os) == -1) {
				ug_free (element_os);
				return -1;
			}
		}
		ug_free (element_os);
	}
	return -1;
}

// ----------------------------------------------------------------------------
// File I/O

#if defined _WIN32 || defined _WIN64
int  ug_file_copy (const char *src_file_utf8, const char *new_file_utf8)
{
	int	 retval;
	uint16_t*  src_file_wcs;
	uint16_t*  new_file_wcs;

	src_file_wcs = ug_utf8_to_utf16 (src_file_utf8, -1, NULL);
	new_file_wcs = ug_utf8_to_utf16 (new_file_utf8, -1, NULL);
	retval = CopyFileW (src_file_wcs, new_file_wcs, FALSE);
	ug_free (src_file_wcs);
	ug_free (new_file_wcs);
	if (retval == 0)
		return -1;
	return 0;
}
#else
int  ug_file_copy (const char *src_file_utf8, const char *new_file_utf8)
{
	int		src_fd;
	int		new_fd;
	char*	buf;
	int		buf_len;
	int		retval = 0;

//	new_fd = open (new_file_utf8,
//	               O_BINARY | O_WRONLY | O_CREAT,
//	               S_IREAD | S_IWRITE | S_IRGRP | S_IROTH);
	new_fd = ug_open (new_file_utf8,
	                  UG_O_BINARY | UG_O_WRONLY | UG_O_CREAT,
	                  UG_S_IREAD | UG_S_IWRITE | UG_S_IRGRP | UG_S_IROTH);
	if (new_fd == -1)
		return -1;

//	src_fd = open (src_file_utf8, O_BINARY | O_RDONLY, S_IREAD);
	src_fd = ug_open (src_file_utf8, UG_O_BINARY | UG_O_RDONLY, UG_S_IREAD);
	if (src_fd == -1) {
		ug_close (new_fd);
		return -1;
	}
	// read & write
	buf = ug_malloc (8192);
	for (;;) {
		buf_len = ug_read (src_fd, buf, 8192);
		if (buf_len <=0)
			break;
		if (ug_write (new_fd, buf, buf_len) != buf_len) {
			retval = -1;
			break;
		}
	}
	// clear
	ug_free (buf);
	ug_close (src_fd);
	ug_close (new_fd);
	return retval;
}
#endif	// _WIN32

int  ug_file_get_lines (const char* filename_utf8, UgList* list)
{
	UgLink* link;
	FILE*   file;
	char*   buf;
	int     len;
	int     count = 0;

	file = ug_fopen (filename_utf8, "r");
	if (file == NULL)
		return 0;
	buf = ug_malloc (8192);
	if (fread (buf, 1, 3, file) == 3) {
		if (buf[0] != 0xEF || buf[1] != 0xBB)
			rewind (file);
	}

	while (fgets (buf, 8192, file) != NULL) {
		count++;
		len = strlen (buf);
		if (len > 0 && buf[len-1] == '\n')
			buf[--len] = 0;
		if (list) {
			link = ug_link_new ();
			link->data = ug_strndup (buf, len);
			ug_list_append (list, link);
		}
	}

	ug_free (buf);
	fclose (file);
	return count;
}


