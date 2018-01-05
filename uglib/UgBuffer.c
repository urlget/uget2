/*
 *
 *   Copyright (C) 2012-2018 by C.H. Huang
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
#include <stdlib.h>
#include <UgDefine.h>
#include <UgBuffer.h>

void  ug_buffer_init_external (UgBuffer* buffer, char* exbuf, int length)
{
	buffer->beg = exbuf;
	buffer->cur = exbuf;
	buffer->end = exbuf + length;
	buffer->more = ug_buffer_restart;
}

void  ug_buffer_init (UgBuffer* buffer, int length)
{
	buffer->beg = ug_malloc (length);
	buffer->cur = buffer->beg;
	buffer->end = buffer->beg + length;
	buffer->more = ug_buffer_expand;
}

void  ug_buffer_clear (UgBuffer* buffer, int free_buffer)
{
	if (free_buffer)
		ug_free (buffer->beg);
	buffer->beg = NULL;
	buffer->cur = NULL;
	buffer->end = NULL;
}

void  ug_buffer_set_size (UgBuffer* buffer, int length)
{
	char*  oldbeg;

	oldbeg = buffer->beg;
	buffer->beg = ug_realloc (buffer->beg, length);
	buffer->cur = buffer->beg + (buffer->cur - oldbeg);
	buffer->end = buffer->beg + length;
}

char* ug_buffer_alloc (UgBuffer* buffer, int length)
{
	char*  result;
	int    buffer_len;

	if (buffer->end < buffer->cur + length) {
		buffer_len = buffer->end - buffer->beg;
		if (buffer_len < length)
			buffer_len = length * 2;
		else
			buffer_len *= 2;
		ug_buffer_set_size (buffer, buffer_len);
	}
	result = buffer->cur;
	buffer->cur += length;
	return result;
}

// UgBuffer.more() default function for external buffer.
int   ug_buffer_restart (UgBuffer* buffer)
{
	buffer->cur = buffer->beg;
	return 1;
}

// UgBuffer.more() default function for internal buffer.
int   ug_buffer_expand (UgBuffer* buffer)
{
	int    length;

	length = (buffer->end - buffer->beg) * 2;
	if (length < 1024)
		length = 1024;
	ug_buffer_set_size (buffer, length);
	return 1;
}

void  ug_buffer_fill (UgBuffer* buffer, char ch, int count)
{
	while (count--) {
		if (buffer->cur == buffer->end)
			buffer->more (buffer);
		*buffer->cur++ = ch;
	}
}

int  ug_buffer_write (UgBuffer* buffer, const char* string, int length)
{
	const char*  end;

	if (length == -1)
		length = strlen (string);
	end = string + length;
	while (string < end && string[0]) {
		if (buffer->cur == buffer->end)
			buffer->more (buffer);
		*buffer->cur++ = *(uint8_t*)string++;
	}
	return length;
}

void  ug_buffer_write_data (UgBuffer* buffer, const char* binary, int length)
{
	const char*  end;

	end = binary + length;
	while (binary < end) {
		if (buffer->cur == buffer->end)
			buffer->more (buffer);
		*buffer->cur++ = *binary++;
	}
}

#if defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L)
// C99
// inline function in UgBuffer.h
#else
void  ug_buffer_write_char (UgBuffer* buffer, char ch)
{
	if ((buffer)->cur >= (buffer)->end)
		(buffer)->more (buffer);
	*(buffer)->cur++ = (char)(ch);
}
#endif  // __STDC_VERSION__
