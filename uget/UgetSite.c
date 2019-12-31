/*
 *
 *   Copyright (C) 2017-2020 by C.H. Huang
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

#include <string.h>
#include <UgDefine.h>
#include <UgetSite.h>

int  uget_site_get_id (const char* url)
{
	UgUri  uuri;

	if (ug_uri_init (&uuri, url) == 0)
		return UGET_SITE_UNKNOWN;

	if (uuri.scheme_len == 5 && strncmp (url, "https", 5) == 0) {
		if (uget_site_is_youtube (&uuri) == TRUE)
			return UGET_SITE_YOUTUBE;
		if (uget_site_is_mega (&uuri) == TRUE)
			return UGET_SITE_MEGA;
	}

	return UGET_SITE_UNKNOWN;
}

// youtube.com
// https://youtube.com/watch?=xxxxxxxxxxx
// https://youtu.be/xxxxxxxxxxx
int  uget_site_is_youtube (UgUri* uuri)
{
	int         length;
	const char* string;

	length = ug_uri_host (uuri, &string);
	if (length >= 11 && strncmp (string + length - 11, "youtube.com", 11) == 0)
	{
		if (strncmp (uuri->uri + uuri->file , "watch?", 6) == 0)
			return TRUE;
	}
	else if (length >= 8 && strncmp (string + length - 8, "youtu.be", 8) == 0)
	{
		if (uuri->file != -1)
			return TRUE;
	}
	return FALSE;
}

// mega.co.nz
// https://mega.co.nz/#!xxxxxxxx!xxxxxxxxxxxxxxxxxxxxxx
// https://mega.nz/#!xxxxxxxx!xxxxxxxxxxxxxxxxxxxxxx

int  uget_site_is_mega (UgUri* uuri)
{
	int         length;
	const char* string;

	length = ug_uri_host (uuri, &string);
	if (length >= 10 && strncmp (string + length - 10, "mega.co.nz", 10) == 0)
	{
		if (uuri->path > 0 && strchr (uuri->uri + uuri->path , '!') != NULL)
			return TRUE;
	}
	else if (length >= 7 && strncmp (string + length - 7, "mega.nz", 7) == 0)
	{
		if (uuri->path > 0 && strchr (uuri->uri + uuri->path , '!') != NULL)
			return TRUE;
	}
	return FALSE;
}
