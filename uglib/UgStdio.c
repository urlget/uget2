/*
 *
 *   Copyright (C) 2005-2018 by C.H. Huang
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

#if defined _WIN32 || defined _WIN64
#define  _CRT_SECURE_NO_DEPRECATE    // avoid some warning (MS VC 2005)
#include <windows.h>
#include <wchar.h>    // _wmkdir(), _wrmdir()
#endif  // _WIN32 || _WIN64

#include <errno.h>
#include <UgDefine.h>
#include <UgUtil.h>
#include <UgStdio.h>

#if defined _WIN32 || defined _WIN64
int  ug_open (const char* filename_utf8, int flags, int mode)
{
	int retval;
	int save_errno;

	wchar_t *wfilename = ug_utf8_to_utf16 (filename_utf8, -1, NULL);

	if (wfilename == NULL) {
		errno = EINVAL;
		return -1;
	}

	retval = _wopen (wfilename, flags, mode);
	save_errno = errno;
	ug_free (wfilename);

	errno = save_errno;
	return retval;
}

int  ug_creat (const char* filename_utf8, int mode)
{
	int retval;
	int save_errno;

	wchar_t *wfilename = ug_utf8_to_utf16 (filename_utf8, -1, NULL);

	if (wfilename == NULL) {
		errno = EINVAL;
		return -1;
	}

	retval = _wcreat (wfilename, mode);
	save_errno = errno;
	ug_free (wfilename);

	errno = save_errno;
	return retval;
}

int  ug_truncate (int fd, int64_t length)
{
	HANDLE h_file;

	if (ug_seek (fd, length, SEEK_SET) == 0) {
		h_file = (HANDLE)_get_osfhandle(fd);
		if (SetEndOfFile (h_file))
			return 0;
	}
	return -1;
}

FILE* ug_fopen (const char *filename, const char *mode)
{
	FILE *retval;
	int save_errno;
	wchar_t *wfilename = ug_utf8_to_utf16 (filename, -1, NULL);
	wchar_t *wmode;

	if (wfilename == NULL) {
		errno = EINVAL;
		return NULL;
	}

	wmode = ug_utf8_to_utf16 (mode, -1, NULL);

	if (wmode == NULL) {
		ug_free (wfilename);
		errno = EINVAL;
		return NULL;
	}

	retval = _wfopen (wfilename, wmode);
	save_errno = errno;
	ug_free (wfilename);
	ug_free (wmode);

	errno = save_errno;
	return retval;
}

// ----------------------------------------------------------------------------
// UNIX
#else

#if defined HAVE_GLIB
int  ug_open (const char* filename_utf8, int flags, int mode)
{
	if (g_get_filename_charsets (NULL))
		return g_open (filename_utf8, flags, mode);
	else {
		gchar *cp_filename = g_filename_from_utf8 (filename_utf8, -1, NULL, NULL, NULL);
		int retval;
		int save_errno;

		if (cp_filename == NULL) {
			errno = EINVAL;
			return -1;
		}

		retval = g_open (cp_filename, flags, mode);
		save_errno = errno;

		g_free (cp_filename);

		errno = save_errno;
		return retval;
	}
}

int  ug_creat (const char* filename_utf8, int mode)
{
	if (g_get_filename_charsets (NULL))
		return g_open (filename_utf8, mode);
	else {
		gchar *cp_filename = g_filename_from_utf8 (filename_utf8, -1, NULL, NULL, NULL);
		int retval;
		int save_errno;

		if (cp_filename == NULL) {
			errno = EINVAL;
			return -1;
		}

		retval = g_creat (cp_filename, mode);
		save_errno = errno;

		g_free (cp_filename);

		errno = save_errno;
		return retval;
	}
}

FILE* ug_fopen (const char *filename, const char *mode)
{
	if (g_get_filename_charsets (NULL))
		return g_fopen (filename, mode);
	else {
		gchar *cp_filename = g_filename_from_utf8 (filename, -1, NULL, NULL, NULL);
		FILE *retval;
		int save_errno;

		if (cp_filename == NULL) {
			errno = EINVAL;
			return NULL;
		}

		retval = g_fopen (cp_filename, mode);
		save_errno = errno;

		g_free (cp_filename);

		errno = save_errno;
		return retval;
	}
}
#endif // HAVE_GLIB

#endif // _WIN32 || _WIN64

#ifdef __ANDROID__
//#include <sys/linux-syscalls.h>

int  fseek_64 (FILE *stream, int64_t offset, int origin)
{
	int fd;

	if (feof (stream)) {
		rewind (stream);
	}
	else {
		setbuf (stream, NULL);  // clear buffer of fread()
	}

	fd = fileno (stream);
	if (lseek64 (fd, offset, origin) == -1) {
		return errno;
	}
	return 0;
}

int64_t ftell_64 (FILE *stream)
{
	int fd;

	fd = fileno (stream);
	return lseek64 (fd, 0L, SEEK_CUR);
}
#endif  // __ANDROID__

int  ug_ftruncate (FILE* file, int64_t size)
{
	int    fd;

	ug_fflush (file);
	ug_fseek (file, size, SEEK_SET);
	fd = ug_fileno (file);
	return ug_truncate (fd, size);
}

// ----------------------------------------------------------------------------
// file & directory functions

#if defined _WIN32 || defined _WIN64

int  ug_rename (const char *old_filename, const char *new_filename)
{
	wchar_t *wold_filename = ug_utf8_to_utf16 (old_filename, -1, NULL);
	wchar_t *wnew_filename = ug_utf8_to_utf16 (new_filename, -1, NULL);
	int save_errno;
	int retval;

	if (wold_filename == NULL || wnew_filename == NULL) {
		ug_free (wold_filename);
		ug_free (wnew_filename);
		errno = EINVAL;
		return -1;
	}

	retval = _wrename (wold_filename, wnew_filename);
	save_errno = errno;

	ug_free (wold_filename);
	ug_free (wnew_filename);

	errno = save_errno;
	return retval;
}

int  ug_remove (const char *filename)
{
	wchar_t *wfilename = ug_utf8_to_utf16 (filename, -1, NULL);
	int save_errno;
	int retval;

	if (wfilename == NULL) {
		errno = EINVAL;
		return -1;
	}

	retval = _wremove (wfilename);
	save_errno = errno;

	ug_free (wfilename);

	errno = save_errno;
	return retval;
}

int  ug_unlink (const char *filename)
{
	wchar_t *wfilename = ug_utf8_to_utf16 (filename, -1, NULL);
	int save_errno;
	int retval;

	if (wfilename == NULL) {
		errno = EINVAL;
		return -1;
	}

	retval = _wunlink (wfilename);
	save_errno = errno;

	ug_free (wfilename);

	errno = save_errno;
	return retval;
}

// ----------------------------------------------------------------------------
// UNIX
#else

#if defined HAVE_GLIB
int  ug_rename (const char *old_filename, const char *new_filename)
{
	if (g_get_filename_charsets (NULL))
		return g_rename (old_filename, new_filename);
	else {
		gchar *cp_old_filename = g_filename_from_utf8 (old_filename, -1, NULL, NULL, NULL);
		gchar *cp_new_filename = g_filename_from_utf8 (new_filename, -1, NULL, NULL, NULL);
		int save_errno;
		int retval;

		if (cp_old_filename == NULL || cp_new_filename == NULL) {
			g_free (cp_old_filename);
			g_free (cp_new_filename);
			errno = EINVAL;
			return -1;
		}

		retval = g_rename (cp_old_filename, cp_new_filename);
		save_errno = errno;

		g_free (cp_old_filename);
		g_free (cp_new_filename);

		errno = save_errno;
		return retval;
	}
}

int  ug_remove (const char *filename)
{
	if (g_get_filename_charsets (NULL))
		return g_remove (filename);
	else {
		gchar *cp_filename = g_filename_from_utf8 (filename, -1, NULL, NULL, NULL);
		int save_errno;
		int retval;

		if (cp_filename == NULL) {
			errno = EINVAL;
			return -1;
		}

		retval = g_remove (cp_filename);
		save_errno = errno;

		g_free (cp_filename);

		errno = save_errno;
		return retval;
	}
}

int  ug_unlink (const gchar *filename)
{
	if (g_get_filename_charsets (NULL))
		return g_unlink (filename);
	else {
		gchar *cp_filename = g_filename_from_utf8 (filename, -1, NULL, NULL, NULL);
		int save_errno;
		int retval;

		if (cp_filename == NULL) {
			errno = EINVAL;
			return -1;
		}

		retval = g_unlink (cp_filename);
		save_errno = errno;

		g_free (cp_filename);

		errno = save_errno;
		return retval;
	}
}

#endif // HAVE_GLIB

#endif // _WIN32 || _WIN64

