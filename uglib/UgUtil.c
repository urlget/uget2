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

#include <stdarg.h>
#include <stdlib.h>     // strtoul
#include <UgDefine.h>
#include <UgUtil.h>
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

uint64_t ug_get_time_count (void)
{
#if defined _WIN32 || defined _WIN64
	return (uint64_t) GetTickCount ();
#else
    struct timeval tv;

    gettimeofday(&tv, NULL);

    return (tv.tv_sec * 1000 + tv.tv_usec / 1000);
#endif
}

// ----------------------------------------------------------------------------
// URI

char*  ug_unescape_uri (const char *uri, int length)
{
	char*  res;
	char*  eptr;
	char*  str;
	const char*  end;

	if (length == -1)
		length = strlen (uri);
	end = uri + length;
	res = ug_malloc (length + 1);
	str = res;

	while (uri < end) {
		if (uri[0] == '%' && uri + 2 < end) {
			str[0] = uri[1];
			str[1] = uri[2];
			str[2] = 0;
			eptr = NULL;
			*(uint8_t*)str = (uint8_t) strtoul (str, &eptr, 16);
			if (eptr == str + 2) {
				str++;
				uri+=3;
				continue;
			}
		}
		*str++ = *uri++;
	}
	*str = 0;
	return res;
}

// ----------------------------------------------------------------------------
// Unicode

static const uint8_t  utf8Limits[] = {0xC0, 0xE0, 0xF0, 0xF8, 0xFC};

uint16_t*  ug_utf8_to_utf16 (const char* string, int count, int* utf16len)
{
	uint8_t     ch;
	uint16_t*   result;
	uint16_t*   dest;
	uint32_t    value;
	const char* end;

	if (count == -1)
		count  = strlen (string);
	end  = string + count;
	result = ug_malloc (sizeof (uint16_t) * (count+1) );
	dest   = result;

	while (string < end) {
		ch = *string++;

		if(ch < 0x80) { // 0-127, US-ASCII (single byte)
			*dest++ = (uint16_t)ch;
			continue;
		}

		if(ch < 0xC0) // The first octet for each code point should within 0-191
			break;

		for(count = 1;  count < 5;  count++)
			if(ch < utf8Limits[count])
				break;
		value = ch - utf8Limits[count - 1];

		do {
			uint8_t  ch2;

			if (string >= end)  //  || string[0] == 0
				break;
			ch2 = *string++;
			if (ch2 == 0)
				break;
			if(ch2 < 0x80 || ch2 >= 0xC0)
				break;
			value <<= 6;
			value |= (ch2 - 0x80);
		} while(--count != 0);

		if(value < 0x10000) {
			*dest++ = (uint16_t) value;
		}
		else {
			value -= 0x10000;
			if(value >= 0x100000)
				break;
			*dest++ = (uint16_t) (0xD800 + (value >> 10));
			*dest++ = (uint16_t) (0xDC00 + (value & 0x3FF));
		}
	}

	*dest++ = 0;
	if (utf16len)
		*utf16len = dest - result;
	return result;
}

int  ug_utf8_get_invalid (const uint8_t* input, uint8_t* ch)
{
	int            nb = 0, na;
	const uint8_t *c = input;

	for (c = input;  *c;  c += (nb + 1)) {
		if (!(*c & 0x80))
			nb = 0;
		else if ((*c & 0xc0) == 0x80) {
			if (ch)
				*ch = *c;
			return (intptr_t)c - (intptr_t)input;
		}
		else if ((*c & 0xe0) == 0xc0)
			nb = 1;
		else if ((*c & 0xf0) == 0xe0)
			nb = 2;
		else if ((*c & 0xf8) == 0xf0)
			nb = 3;
		else if ((*c & 0xfc) == 0xf8)
			nb = 4;
		else if ((*c & 0xfe) == 0xfc)
			nb = 5;

		na = nb;
		while (na-- > 0) {
			if ((*(c + nb) & 0xc0) != 0x80) {
				if (ch)
					*ch = *(c + nb);
				return (intptr_t)(c + nb) - (intptr_t)input;
			}
		}
	}
	return -1;
}

// ----------------------------------------------------------------------------
// Base64

static int mod_table[] = {0, 2, 1};
static char encoding_table[] = {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
                                'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
                                'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
                                'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
                                'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
                                'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
                                'w', 'x', 'y', 'z', '0', '1', '2', '3',
                                '4', '5', '6', '7', '8', '9', '+', '/'};

char* ug_base64_encode (const uint8_t* data, int input_length,
                        int* output_length)
{
	int   i, j;
	int   length;
	char* encoded_data;

	length = 4 * ((input_length + 2) / 3);
	encoded_data = (char*) ug_malloc (length + 1);  // + '\0'
	if (encoded_data == NULL)
		return NULL;

	for (i = 0, j = 0; i < input_length;) {
		uint32_t octet_a = i < input_length ? data[i++] : 0;
		uint32_t octet_b = i < input_length ? data[i++] : 0;
		uint32_t octet_c = i < input_length ? data[i++] : 0;

		uint32_t triple = (octet_a << 0x10) + (octet_b << 0x08) + octet_c;

		encoded_data[j++] = encoding_table[(triple >> 3 * 6) & 0x3F];
		encoded_data[j++] = encoding_table[(triple >> 2 * 6) & 0x3F];
		encoded_data[j++] = encoding_table[(triple >> 1 * 6) & 0x3F];
		encoded_data[j++] = encoding_table[(triple >> 0 * 6) & 0x3F];
	}

	for (i = 0; i < mod_table[input_length % 3]; i++)
		encoded_data[length - 1 - i] = '=';

	encoded_data[length] = '\0';
	if (output_length)
		*output_length = length;
	return encoded_data;
}

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

char* ug_build_filename (const char* first_element, ...)
{
	va_list     arg_list;
	int         total_len;
	const char* temp;
	char*       result;

	// count total length
	total_len = 0;
	temp = first_element;
	va_start (arg_list, first_element);
	do {
		total_len += strlen (temp) + 1;  // + '/'
		temp = va_arg (arg_list, char*);
	} while (temp);
	va_end (arg_list);

	// concat path element
	result = ug_malloc (total_len + 1);     // + '\0'
	result[0] = 0;
	temp = first_element;
	va_start (arg_list, first_element);
	do {
		if (temp != first_element) {
#if defined _WIN32 || defined _WIN64
			if (strchr (temp, '\\') == NULL)
				strcat (result, "\\");
#else
			if (strchr (temp, '/') == NULL)
				strcat (result, "/");
#endif // _WIN32 || _WIN64
		}
		strcat (result, temp);
		temp = va_arg (arg_list, char*);
	} while (temp);
	va_end (arg_list);

	return result;
}

// ----------------------------------------------------------------------------
// Power Management

#if defined _WIN32 || defined _WIN64

static int  ug_win32_get_shutdown_privilege ()
{
	HANDLE hToken;
	TOKEN_PRIVILEGES tkp;

	if (OpenProcessToken (GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES|TOKEN_QUERY, &hToken))
	{
		LookupPrivilegeValue (NULL, SE_SHUTDOWN_NAME, &tkp.Privileges[0].Luid);
		tkp.PrivilegeCount = 1;
		tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
		AdjustTokenPrivileges (hToken, FALSE, &tkp, 0, (PTOKEN_PRIVILEGES)NULL, 0);
		return TRUE;
	}
	return FALSE;
}

void  ug_reboot (void)
{
	ug_win32_get_shutdown_privilege ();
	ExitWindowsEx (EWX_SHUTDOWN | EWX_REBOOT, 0);
}

void  ug_shutdown (void)
{
	ug_win32_get_shutdown_privilege ();
	ExitWindowsEx (EWX_SHUTDOWN | EWX_POWEROFF, 0);
}

void  ug_suspend (void)
{
	system ("powercfg -hibernate off");
//	SetSuspendState (0, 1, 0);
	system ("rundll32 powrprof.dll,SetSuspendState 0,1,0");
	system ("powercfg -hibernate on");
}

void  ug_hibernate (void)
{
//	SetSuspendState (1, 0, 0);
	system ("rundll32 powrprof.dll,SetSuspendState 1,0,0");
}

#else

void  ug_reboot (void)
{
	system ("reboot");
//	system ("shutdown -r now");

	// old system
	system ("dbus-send --system --print-reply "
	        "--dest=\"org.freedesktop.ConsoleKit\" "
	        "/org/freedesktop/ConsoleKit/Manager "
	        "org.freedesktop.ConsoleKit.Manager.Restart");

	system ("dbus-send --system --print-reply "
	        "--dest=org.freedesktop.login1 "
	        "/org/freedesktop/login1 "
	        "org.freedesktop.login1.Manager.Reboot "
	        "boolean:true");
}

void  ug_shutdown (void)
{
	system ("poweroff");
//	system ("shutdown -h -P now");

	// old system
	system ("dbus-send --system --print-reply "
	        "--dest=\"org.freedesktop.ConsoleKit\" "
	        "/org/freedesktop/ConsoleKit/Manager "
	        "org.freedesktop.ConsoleKit.Manager.Stop");

	system ("dbus-send --system --print-reply "
	        "--dest=org.freedesktop.login1 "
	        "/org/freedesktop/login1 "
	        "org.freedesktop.login1.Manager.PowerOff "
	        "boolean:true");
}

void  ug_suspend (void)
{
	system ("pm-suspend");

	// old system
	system ("dbus-send --system --print-reply "
	        "--dest=\"org.freedesktop.UPower\" "
	        "/org/freedesktop/UPower "
	        "org.freedesktop.UPower.Suspend");

	system ("dbus-send --system --print-reply "
	        "--dest=org.freedesktop.login1 "
	        "/org/freedesktop/login1 "
	        "org.freedesktop.login1.Manager.Suspend "
	        "boolean:true");
}

void  ug_hibernate (void)
{
	system ("pm-hibernate");

	// old system
	system ("dbus-send --system --print-reply "
	        "--dest=\"org.freedesktop.UPower\" "
	        "/org/freedesktop/UPower "
	        "org.freedesktop.UPower.Hibernate");

	system ("dbus-send --system --print-reply "
	        "--dest=org.freedesktop.login1 "
	        "/org/freedesktop/login1 "
	        "org.freedesktop.login1.Manager.Hibernate "
	        "boolean:true");

	// if system can't hibernate, try to suspend
	ug_suspend ();
}

#endif // _WIN32 || _WIN64

// ----------------------------------------------------------------------------
// Others

#if defined _WIN32 || defined _WIN64

char*  ug_sys_release (void)
{
	OSVERSIONINFO  info;

	memset (&info, 0, sizeof (info));
	info.dwOSVersionInfoSize = sizeof (info);
	GetVersionEx (&info);
	return ug_strdup_printf ("Windows-%u.%u",
			(unsigned) info.dwMajorVersion,
			(unsigned) info.dwMinorVersion);
}

#else

// lsb_release -i
// Distributor ID:
// lsb_release -r
// Release:
char* ug_sys_release (void)
{
	FILE*  file;
	char*  buf;
	char*  dist = NULL;
	char*  release = NULL;

	file = popen ("lsb_release -a", "r");
	if (file == NULL)
		return ug_strdup ("Unknown");
	buf = ug_malloc (80);
	while (fgets (buf, 80, file) != NULL) {
		if (strncmp (buf, "Distributor ID:", 15) == 0) {
			for (dist = buf + 15;  dist[0] == '\t';  dist++);
			dist = ug_strndup (dist, strcspn (dist, "\n"));
		}
		else if (strncmp (buf, "Release:", 8) == 0) {
			for (release = buf + 8;  release[0] == '\t';  release++);
			release = ug_strndup (release, strcspn (release, "\n"));
		}
	}
	ug_free (buf);
	pclose (file);

	if (dist && release)
		buf = ug_strdup_printf ("%s-%s", dist, release);
	else
		buf = ug_strdup ("Unknown");
	ug_free (dist);
	ug_free (release);
	return buf;
}

#endif // _WIN32 || _WIN64
