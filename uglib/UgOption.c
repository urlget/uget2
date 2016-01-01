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

#include <stdio.h>
#include <stdlib.h>
#include <UgString.h>
#include <UgBuffer.h>
#include <UgOption.h>

#if defined(_MSC_VER)
#define strtoll     _strtoi64
#define strtoull    _strtoui64
#endif

static void  print_entry (UgBuffer* buffer, UgOptionEntry* entry, int  arg_max_length)
{
	int  length = 0;

	length += ug_buffer_write (buffer, "  ", 2);

	if (entry->short_name) {
		ug_buffer_write_char (buffer, '-');
		length++;
		length += ug_buffer_write (buffer, entry->short_name, -1);
		length += ug_buffer_write (buffer, ", ", 2);
	}
	if (entry->long_name) {
		length += ug_buffer_write (buffer, "--", 2);
		length += ug_buffer_write (buffer, entry->long_name, -1);
	}

	if (entry->arg_description) {
		ug_buffer_write_char (buffer, '=');
		length++;
#if 0 // defined HAVE_GLIB
		length += ug_buffer_write (buffer, gettext (entry->arg_description), -1);
#else
		length += ug_buffer_write (buffer, entry->arg_description, -1);
#endif
	}

	length = arg_max_length + 4 - length;
	ug_buffer_fill (buffer, ' ', length);

	if (entry->description) {
#if 0//defined HAVE_GLIB
		ug_buffer_write (buffer, gettext (entry->description), -1);
#else
		ug_buffer_write (buffer, entry->description, -1);
#endif
	}

	ug_buffer_write_char (buffer, '\n');
}

static int buffer_more (UgBuffer* buffer)
{
	int  length;

	length = buffer->cur - buffer->beg;
	if (length > 0) {
		printf ("%.*s", length, buffer->beg);
		buffer->cur = buffer->beg;
	}
	return length;
}

void ug_option_entry_print_help (UgOptionEntry* entry,
                                 const char* prog_name,
                                 const char* parameter_string,
                                 const char* summary_string)
{
	UgBuffer  buffer;

	ug_buffer_init (&buffer, 4096);
	buffer.more = buffer_more;

	ug_buffer_write (&buffer, "\n" "Usage:\n  ", -1);
	ug_buffer_write (&buffer, prog_name, -1);
	ug_buffer_write (&buffer, " [OPTION...] ", -1);
	ug_buffer_write (&buffer, parameter_string, -1);
	ug_buffer_write (&buffer, "\n\n", -1);

	if (summary_string) {
		ug_buffer_write (&buffer, summary_string, -1);
		ug_buffer_write (&buffer, "\n\n", -1);
	}

	for (;  entry->long_name;  entry++)
		print_entry (&buffer, entry, 28);
	buffer_more (&buffer);
	ug_buffer_write (&buffer, "\n\n", -1);

	ug_buffer_clear (&buffer, TRUE);
}

// ----------------------------------------------------------------------------

void ug_option_init (UgOption* option)
{
	ug_array_init (&option->others, sizeof (char*), 16);
	ug_array_init (&option->unaccepted, sizeof (char*), 16);
}

void ug_option_final (UgOption* option)
{
	ug_option_clear (option);
	ug_array_clear (&option->others);
	ug_array_clear (&option->unaccepted);
}

void ug_option_clear (UgOption* option)
{
	ug_array_foreach_ptr (&option->others, (UgForeachFunc) ug_free, NULL);
	ug_array_foreach_ptr (&option->unaccepted, (UgForeachFunc) ug_free, NULL);
	option->unaccepted.length = 0;
	option->others.length = 0;
}

void ug_option_set_parser (UgOption* option, UgOptionParseFunc func,
                           void* dest, void* data)
{
	option->parse.func  = func;
	option->parse.dest  = dest;
	option->parse.data  = data;
}

int  ug_option_parse (UgOption* option, const char* string, int length)
{
	char* beg;
	char* cur;
	char* end;

	if (string == NULL || length == 0)
		return FALSE;
	if (length == -1)
		length = strlen (string);
	string = ug_strndup (string, length);
	// not option
	if (string[0] != '-') {
		*(char**) ug_array_alloc (&option->others, 1) = (char*) string;
		return TRUE;
	}

	// option name
	end = (char*) string + length;
	for (beg = (char*) string;  beg < end;  beg++) {
		if (beg[0] != '-')
			break;
	}
	if (beg == end) {
		ug_free ((void*) string);
		return FALSE;
	}
	if (beg - string == 1)
		option->short_name = TRUE;
	else
		option->short_name = FALSE;
	// option value
	for (cur = beg;  cur < end;  cur++) {
		if (cur[0] == '=') {
			*cur++ = 0;
			break;
		}
	}
	if (cur == end)
		cur = NULL;

	if (option->parse.func (option, beg, cur,
	    option->parse.dest, option->parse.data) == FALSE)
	{
		if (option->discard_unaccepted == FALSE)
			*(char**) ug_array_alloc (&option->unaccepted, 1) = (char*) string;
	}
	ug_free ((void*) string);
	return TRUE;
}

int  ug_option_parse_entry (UgOption* option,
                            const char* name,
                            const char* value,
                            void* dest, void* data)
{
	UgOptionEntry*  entry = (UgOptionEntry*) data;

	for (;  entry->long_name;  entry++) {
		// compare option name
		if (entry->long_name[0] != 0) {
			if (option->short_name) {
				if (strcmp (entry->short_name, name) != 0)
					continue;
			}
			else {
				if (strcmp (entry->long_name, name) != 0)
					continue;
			}
		}
		// get destination
		dest = ((char*) dest) + entry->offset;

		// handle data by UgEntryType
		switch (entry->type) {
		case UG_ENTRY_BOOL:
			*(int*) dest = TRUE;
			if (value) {
				if (strcmp (value, "no") == 0 || strcmp (value, "false") == 0)
					*(int*) dest = FALSE;
			}
			return TRUE;

		case UG_ENTRY_INT:
			*(int*) dest = strtol (value, NULL, 10);
			return TRUE;

		case UG_ENTRY_UINT:
			*(unsigned int*) dest = (unsigned int) strtoul (value, NULL, 10);
			return TRUE;

		case UG_ENTRY_INT64:
			// C99 Standard
			*(int64_t*) dest = strtoll (value, NULL, 10);
			return TRUE;

		case UG_ENTRY_UINT64:
			// C99 Standard
			*(uint64_t*) dest = strtoull (value, NULL, 10);
			return TRUE;

		case UG_ENTRY_DOUBLE:
			*(double*) dest = strtod (value, NULL);
			return TRUE;

		case UG_ENTRY_STRING:
			*(char**) dest = ug_strdup (value);
			return TRUE;

		case UG_ENTRY_OBJECT:
			// UgOptionEntry.data pointer to UgEntry
			return ug_option_parse_entry (option, name, value, dest, entry->data);

		case UG_ENTRY_ARRAY:
			// UgOptionEntry.data pointer to UgOptionParseFunc
			return ((UgOptionParseFunc) entry->data) (option, name, value, dest, data);

		case UG_ENTRY_CUSTOM:
			// UgOptionEntry.data pointer to UgOptionParseFunc
			return ((UgOptionParseFunc) entry->data) (option, name, value, dest, data);

		default:
			return FALSE;
		}
	}

	return FALSE;
}

// ----------------------------------------------------------------------------
// command-line argument

// find -?, -h, --help, or --help- in command-line options and return it.
char*	ug_args_find_help (int argc, char** argv)
{
	char*	arg;
	int     arg_len;

	for (argc -= 1;  argc >= 0;  argc--) {
		arg = argv[argc];
		arg_len = strlen (arg);
#ifdef _WIN32
		// Check and remove some character (space,0x20) in tail of argument from command line.
		// This problem only happen in Windows platform.
//		ug_str_clear_tail_charset (arg, arg_len, " \n");
#endif
		// check short_name: -h or -?
		if (arg_len < 2 || arg[0] != '-')
			continue;
		if (arg_len == 2 && (arg[1] == 'h' || arg[1] == '?'))
			return arg;
		// check long name: --help
		if (arg_len < 6 || arg[1] != '-')
			continue;
		if (strncmp (arg+2, "help", 4) == 0) {
			if (arg_len == 6 || arg[6] == '-')
				return arg;
		}
	}
	return NULL;
}

// find -V, --version
char*	ug_args_find_version (int argc, char** argv)
{
	char*	arg;
	int     arg_len;

	for (argc -= 1;  argc >= 0;  argc--) {
		arg = argv[argc];
		arg_len = strlen (arg);
#ifdef _WIN32
		// Check and remove some character (space,0x20) in tail of argument from command line.
		// This problem only happen in Windows platform.
//		ug_str_clear_tail_charset (arg, arg_len, " \n");
#endif
		// check short_name: -V
		if (arg_len < 2 || arg[0] != '-')
			continue;
		if (arg_len == 2 && arg[1] == 'V')
			return arg;
		// check long name: --version
		if (arg_len != 9 || arg[1] != '-')
			continue;
		if (strncmp (arg+2, "version", 7) == 0)
			return arg;
	}
	return NULL;
}

