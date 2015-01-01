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

#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <stdio.h>  // vsnprintf
#include <stdlib.h>
#include <stdarg.h>
#include <UgString.h>

// ----------------------------------------------------------------------------
// String

#if !(defined HAVE_GLIB)
char* ug_strdup_printf (const char* format, ...)
{
	va_list arg_list;
	char*   string;
	int     string_len;

	va_start (arg_list, format);
#ifdef _MSC_VER
	/* for M$ C only */
	string_len = _vscprintf (format, arg_list);
#else
	/* for C99 standard */
	string_len = vsnprintf (NULL, 0, format, arg_list);
#endif
	va_end (arg_list);

	string = ug_malloc (string_len + 1);

	va_start (arg_list, format);
	vsprintf (string, format, arg_list);
	va_end (arg_list);

	return string;
}

char*  ug_strndup (const char* string, size_t length)
{
	char*  result;

	result = ug_malloc (length + 1);
	result[length] = 0;
	strncpy (result, string, length);
	return result;
}
#endif // ! (HAVE_GLIB)

/*
 * convert double to string
 * If value large than 1024, it will append unit string like "KiB",
 * "MiB", "GiB", "TiB", or "PiB"  to string.
 */
char*  ug_str_from_int_unit (int64_t value, const char* tail)
{
	static const char* unit_array[] = {"", " KiB", " MiB", " GiB", " TiB", " PiB"};
	static const int   unit_array_len = sizeof (unit_array) / sizeof (char*);
	int  index;

	for (index=0;  index < unit_array_len -1;  index++) {
//		if (value < 1024)
		if (value < 10000)
			break;
//		value /= 1024;
		value >>= 10;
	}

	return ug_strdup_printf ("%d%s%s",
			(int)value, unit_array[index], (tail) ? tail : "");
}

/*
 * convert seconds to string (hh:mm:ss)
 */
char*  ug_str_from_seconds (int seconds, int limit_99_99_99)
{
	int  hour, minute;
	int  day, year;

	minute  = seconds / 60;
	hour    = minute / 60;
	minute  = minute % 60;
	seconds = seconds % 60;

	if (hour < 100)
		return ug_strdup_printf ("%.2d:%.2d:%.2d", hour, minute, seconds);
	else if (limit_99_99_99)
		return ug_strdup ("99:99:99");
	else {
		day    = hour / 24;
		year   = day / 365;
		if (year)
			return ug_strdup ("> 1 year");
		else
			return ug_strdup_printf ("%u days", day);
	}
}

/*
 * convert time_t to string
 */
char*  ug_str_from_time (time_t ptt, int date_only)
{
	struct tm*  timem;

	timem = localtime (&ptt);
	if (timem == NULL)
		return NULL;
	if (date_only) {
		return ug_strdup_printf ("%.4d-%.2d-%.2d",
				timem->tm_year + 1900,
				timem->tm_mon  + 1,
				timem->tm_mday);
	}
	else {
		return ug_strdup_printf ("%.4d-%.2d-%.2d" " %.2d:%.2d:%.2d",
				timem->tm_year + 1900,
				timem->tm_mon  + 1,
				timem->tm_mday,
				timem->tm_hour,
				timem->tm_min,
				timem->tm_sec);
	}
}

static const char* month_string[] =
{
	"Jan", "Feb", "Mar", "Apr",
	"May", "Jun", "Jul", "Aug",
	"Sep", "Oct", "Nov", "Dec"
};

time_t  ug_str_rfc822_to_time (const char* string)
{
	struct tm   timem;
	const char* cur;
	const char* end;
	int         idx;

	end = string + strlen (string);

	// day
	if ((cur = string + 5) >= end)
		return -1;
	timem.tm_mday = strtoul (cur, NULL, 10);

	// month
	if ((cur += 3) >= end)
		return -1;
	for (idx = 0;  idx < 12;  idx++) {
		if (strncmp (cur, month_string[idx], 3) == 0)
			break;
	}
	if (idx == 12)
		return -1;
	timem.tm_mon = idx;

	// year
	if ((cur += 4) >= end)
		return -1;
	timem.tm_year = strtoul (cur, NULL, 10);
//	if (timem.tm_year < 50)
//		timem.tm_year += 100;
//	else if (timem.tm_year > 1000)
		timem.tm_year -= 1900;

	if ((cur = strchr (cur, ' ')) == NULL)
		return -1;
	cur++;

	// hour
	timem.tm_hour = strtoul (cur, NULL, 10);

	// minute
	if ((cur += 3) >= end)
		return -1;
	timem.tm_min = strtoul (cur, NULL, 10);

	// second
	if ((cur += 3) >= end)
		return -1;
	timem.tm_sec = strtoul (cur, NULL, 10);

	// zone
//	if ((cur += 3) >= end)
//		return -1;

	// other
	timem.tm_wday = 0;
	timem.tm_yday = 0;
	timem.tm_isdst = -1;

	return mktime (&timem);
}

time_t  ug_str_rfc3339_to_time (const char* string)
{
	struct tm   timem;
	const char* cur;
	const char* end;

	end = string + strlen (string);

	// year
	timem.tm_year = strtoul (string, NULL, 10);
	timem.tm_year -= 1900;

	if ((cur = strchr (string, '-')) == NULL)
		return -1;
	cur++;

	// month
	timem.tm_mon = strtoul (cur, NULL, 10);
	if (timem.tm_mon > 0)
		timem.tm_mon -= 1;

	// day
	if ((cur += 3) >= end)
		return -1;
	timem.tm_mday = strtoul (cur, NULL, 10);

	// hour
	if ((cur += 3) >= end)
		return -1;
	timem.tm_hour = strtoul (cur, NULL, 10);

	// minute
	if ((cur += 3) >= end)
		return -1;
	timem.tm_min = strtoul (cur, NULL, 10);

	// second
	if ((cur += 3) >= end)
		return -1;
	timem.tm_sec = strtoul (cur, NULL, 10);

	// zone
	if ((cur = strpbrk (cur, "+-")) == NULL)
		return -1;

	// other
	timem.tm_wday = 0;
	timem.tm_yday = 0;
	timem.tm_isdst = -1;

	return mktime (&timem);
}

// ------------------------------------
// command-line

char** ug_argv_from_cmd (const char* cmd, int* argc, int reserve_len)
{
	int    argn;
	char** argv;
	char*  buf;
	char*  beg;
	char*  cur;
	char*  end;
	char*  quote = NULL;   // pointer to '\"'

	for (argn = 0, cur = (char*)cmd;  ;  argn++) {
		if (cur[0] == 0) {
			end = cur;
			break;
		}
		while (cur[0] != ' ' && cur[0])
			cur++;
		while (cur[0] == ' ')
			cur++;
	}

	// malloc size include buffer and null-terminated
	argv = ug_malloc (sizeof (char*) * (argn+2+reserve_len));
	buf  = ug_strdup (cmd);
	end  = buf + (end - cmd);
	// argv[-1] == buf
	*argv++ = buf;

	for (argn = reserve_len, beg = buf, cur = buf;  cur[0];  ) {
		switch (cur[0]) {
		default:
			cur++;
			break;

		case '\"':
			// overwrite '\"', must include null-terminated
			memmove (cur, cur+1, end - cur);
			if (quote == NULL)
				quote = cur;
			else
				quote = NULL;
			break;

		case ' ':
			if (quote == NULL) {
				argv[argn++] = beg;
				while (cur[0] == ' ')
					*cur++ = 0;
				beg = cur;
			}
			break;
		}
	}

	if (beg[0])
		argv[argn++] = beg;
	if (argc)
		argc[0] = argn;
	argv[argn++] = NULL;  // null-termainate
	return argv;
}

void   ug_argv_free (char** ug_argv)
{
	ug_free (ug_argv[-1]);
	ug_free (ug_argv-1);
}
