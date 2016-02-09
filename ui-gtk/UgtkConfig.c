/*
 *
 *   Copyright (C) 2012-2016 by C.H. Huang
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

#include <glib.h>

// ----------------------------------------------------------------------------
// path

#if defined _WIN32 || defined _WIN64
#include <windows.h>

const char* ugtk_get_data_dir (void)
{
	static gchar* data_dir = NULL;

	if (data_dir == NULL) {
		gchar*		path;
		gunichar2*	path_wcs;
		HMODULE		hmod;

		hmod = GetModuleHandle (NULL);
		// UNICODE
		path_wcs = g_malloc (sizeof (wchar_t) * MAX_PATH);
		GetModuleFileNameW (hmod, path_wcs, MAX_PATH);
		path = g_utf16_to_utf8 (path_wcs, -1, NULL, NULL, NULL);
		g_free (path_wcs);
		data_dir = g_path_get_dirname (path);
		g_free (path);
	}
	return data_dir;
}

const char* ugtk_get_config_dir (void)
{
#ifdef UGET_PORTABLE
	static gchar* config_dir = NULL;

	if (config_dir == NULL) {
		config_dir = (char*) ugtk_get_data_dir ();
		config_dir = g_build_filename (ugtk_get_data_dir(), "..", "config", NULL);
	}
	return config_dir;
#endif

	return g_get_user_config_dir ();
}

#else

const char* ugtk_get_data_dir (void)
{
	return DATADIR;
}

const char* ugtk_get_config_dir (void)
{
	return g_get_user_config_dir ();
}

#endif // _WIN32 || _WIN64


const char* ugtk_get_locale_dir (void)
{
	static gchar* locale_dir = NULL;

	if (locale_dir == NULL)
		locale_dir = g_build_filename (ugtk_get_data_dir(), "locale", NULL);

	return locale_dir;
}
