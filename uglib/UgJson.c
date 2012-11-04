/*
 *
 *   Copyright (C) 2012-2014 by C.H. Huang
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

// parser & writer
#include <stdint.h>     // uintptr_t
#include <stddef.h>     // offsetof ()
#include <string.h>     // strlen
// writer
#include <limits.h>     // INT_MAX
#include <stdarg.h>     // va_list, va_start, va_end
#include <stdio.h>      // vsnprintf
// uglib
#include <UgDefine.h>
#include <UgJson.h>

#define BUFFER_SIZE             128

void  ug_json_init (UgJson* json)
{
	// initialize buffer
	json->buf.allocated = BUFFER_SIZE;
	json->buf.length = 0;
	json->buf.at = ug_malloc (json->buf.allocated);
	// initialize stack
	json->stack.allocated = 16 * 4;  // 16 x PARSER_STACK_UNIT
	json->stack.length = 0;
	json->stack.at = ug_malloc (sizeof (void*) * json->stack.allocated);
}

void  ug_json_final (UgJson* json)
{
	ug_free (json->buf.at);
	ug_free (json->stack.at);
}

// ----------------------------------------------------------------------------
// JSON Parser functions

// UgJsonState (include UgJsonType) are used by UgJson.state
// for internal use only
enum UgJsonState {
	UG_JSON_CONTROL = UG_JSON_N_TYPE,
	UG_JSON_UNICODE,
	UG_JSON_END_OA,         // End of Object or Array

	UG_JSON_VALUE,          // start of value
	UG_JSON_SEPARATOR,      // ','
};

enum {
	PARSER_STACK_BASE,      // 0
	PARSER_STACK_TYPE,      // 1
	PARSER_STACK_DATA2,     // 2
	PARSER_STACK_DATA1,     // 3
	PARSER_STACK_FUNC,      // 4
	PARSER_STACK_UNIT = PARSER_STACK_FUNC,  // equal with STACK_MULTIPLY_VALUE
};

static void  ug_json_call_parser (UgJson* json);

void  ug_json_begin_parse (UgJson* json)
{
	// reset state
	json->count = 0;
	json->colon = 0;
	json->numberPoint = 0;
	json->numberPm = 0;
	json->numberEe = 0;
	json->index[0] = 0;
	json->index[1] = 0;
	// initialize state
	json->type = UG_JSON_VALUE;
	json->scope = 0;
	json->state = UG_JSON_VALUE;
	json->error = 0;
	// initialize buffer
	json->buf.at[0] = 0;
	json->buf.length = 1;
}

UgJsonError  ug_json_end_parse (UgJson* json)
{
	int  stack_length;

	// clear stack
	stack_length = json->stack.length;
	json->stack.length = 0;

	if (stack_length > PARSER_STACK_UNIT)
		return UG_JSON_ERROR_UNCOMPLETED;
	if (json->index[0])
		return UG_JSON_ERROR_UNCOMPLETED;
	if (json->index[1]) {
		if (json->type >= UG_JSON_N_TYPE)
			return UG_JSON_ERROR_UNCOMPLETED;
		// check uncompleted string.
		if (json->type == UG_JSON_STRING) {
			if (json->buf.at[json->buf.length-1] != 0)
				return UG_JSON_ERROR_UNCOMPLETED;
		}
		// null-terminated for UG_JSON_NUMBER.
		else if (json->type == UG_JSON_NUMBER) {
			if (json->stack.allocated == stack_length) {
				json->stack.allocated *= 2;
				json->stack.at = ug_realloc (json->stack.at,
						json->stack.allocated * sizeof (void*));
			}
			json->buf.at[json->buf.length++] = 0;
		}
		// parse remain value
		ug_json_call_parser (json);
		return (UgJsonError) json->error;
	}

	return UG_JSON_ERROR_NONE;
}

void  ug_json_set  (UgJson* json, UgJsonParseFunc func, void* dest, void* data)
{
	void** stack;

	stack   = json->stack.at + json->stack.length;
	*(stack - PARSER_STACK_FUNC) = func;
	*(stack - PARSER_STACK_DATA1) = dest;
	*(stack - PARSER_STACK_DATA2) = data;
}

void  ug_json_push (UgJson* json, UgJsonParseFunc func, void* dest, void* data)
{
	void** stack;

	if (json->stack.allocated <  json->stack.length + PARSER_STACK_UNIT) {
		json->stack.allocated *= 2;
		json->stack.at = ug_realloc (json->stack.at,
				json->stack.allocated * sizeof (void*));
	}

	 stack   = json->stack.at + json->stack.length;
	*stack++ = func;
	*stack++ = dest;
	*stack++ = data;
	*stack++ = (void*)(uintptr_t)json->scope;
	json->stack.length += PARSER_STACK_UNIT;
}

void  ug_json_pop (UgJson* json)
{
	void**    stack;
	uintptr_t scope;

	if (json->stack.length >= PARSER_STACK_UNIT) {
		stack = json->stack.at + json->stack.length;
		scope = (uintptr_t) *(stack - PARSER_STACK_TYPE);
		json->stack.length -= PARSER_STACK_UNIT;
		// End of Object or Array
		json->type = UG_JSON_END_OA;
		// restore state
		json->scope = scope;
		json->state = json->scope;
//		json->index[0] = 0;
//		json->index[1] = 0;
	}
}

UgJsonError  ug_json_parse (UgJson* json, const char* string, int len)
{
	const char* cur;
	const char* end;
	char        vchar;

	if (len == -1)
		len = strlen (string);

	for (cur = string, end = string + len;  cur < end;  cur++) {
		if (json->buf.allocated == json->buf.length) {
			json->buf.allocated *= 2;
			json->buf.at = ug_realloc (json->buf.at,
					json->buf.allocated * sizeof (char));
		}
		vchar = cur[0];
		// begin of top level switch - select JSON part by state
TopLevelSwitch:
		switch (json->state) {
		case UG_JSON_VALUE:
			if (json->index[1])
				return UG_JSON_ERROR_EXCESS_VALUE;

			switch (vchar) {
			case ' ':
			case '\t':
			case '\n':
			case '\r':
				continue;
//				break;

			case '[':
				json->type = UG_JSON_ARRAY;
				ug_json_call_parser (json);
				json->type = UG_JSON_VALUE;  // for ',' in array
				json->scope = UG_JSON_ARRAY;
				json->state = UG_JSON_VALUE;
				continue;
//				break;

			case '{':
				json->type = UG_JSON_OBJECT;
				ug_json_call_parser (json);
				json->type = UG_JSON_VALUE;  // for ',' in object
				json->scope = UG_JSON_OBJECT;
				json->state = UG_JSON_OBJECT;
				continue;
//				break;

			case '\"':
				json->type = UG_JSON_STRING;
				json->state = UG_JSON_STRING;
				json->index[1] = json->buf.length;
				continue;
//				break;

			default:
				switch (vchar) {
				case 't':	// true
					json->state = UG_JSON_TRUE;
					break;
				case 'f':	// false
					json->state = UG_JSON_FALSE;
					break;
				case 'n':	// null
					json->state = UG_JSON_NULL;
					break;
				default:	// number or not
					if (vchar == '-' || (vchar >= '0' && vchar <= '9')) {
						json->type = UG_JSON_NUMBER;
						json->state = UG_JSON_NUMBER;
					}
					else {
						json->state = json->scope;
						goto TopLevelSwitch;
					}
					break;
				}
				json->count = 1;
				json->index[1] = json->buf.length;
				json->buf.at[json->buf.length++] = vchar;
				continue;
//				break;
			}
			break;

		case UG_JSON_OBJECT:
			switch (vchar) {
			case '\"':
				if (json->index[0])
					return UG_JSON_ERROR_EXCESS_NAME;
				json->index[0] = json->buf.length;
				json->state = UG_JSON_STRING;
				continue;
//				break;

			case ':':
				if (json->colon)
					return UG_JSON_ERROR_EXCESS_COLON;
				if (json->index[0] == 0)
					return UG_JSON_ERROR_BEFORE_COLON;
				json->type = UG_JSON_VALUE;
				json->colon = 1;
				json->state = UG_JSON_VALUE;
				continue;
//				break;

			case ' ':
			case '\t':
			case '\n':
			case '\r':
				continue;
//				break;

			case ',':
				// if type is UG_JSON_VALUE or UG_JSON_SEPARATOR
				if (json->type >= UG_JSON_VALUE)
					return UG_JSON_ERROR_IN_SEPARATOR;
				ug_json_call_parser (json);
				json->type = UG_JSON_SEPARATOR;
				continue;
//				break;

			case '}':
				if (json->type == UG_JSON_SEPARATOR)
					return UG_JSON_ERROR_IN_OBJECT;
				ug_json_call_parser (json);
				ug_json_pop (json);
				continue;
//				break;

			default:
				return UG_JSON_ERROR_IN_OBJECT;
			}
			break;

		case UG_JSON_ARRAY:
			switch (vchar) {
			case ' ':
			case '\t':
			case '\n':
			case '\r':
				continue;
//				break;

			case ',':
				// if type is UG_JSON_VALUE or UG_JSON_SEPARATOR
				if (json->type >= UG_JSON_VALUE)
					return UG_JSON_ERROR_IN_SEPARATOR;
				ug_json_call_parser (json);
				json->type = UG_JSON_SEPARATOR;
				json->state = UG_JSON_VALUE;
				continue;
//				break;

			case ']':
				if (json->type == UG_JSON_SEPARATOR)
					return UG_JSON_ERROR_IN_ARRAY;
				ug_json_call_parser (json);
				ug_json_pop (json);
				continue;
//				break;

			default:
				return UG_JSON_ERROR_IN_ARRAY;
			}
			break;

		case UG_JSON_STRING:
			switch (vchar) {
			case '\"':
				// null-terminated
				json->buf.at[json->buf.length++] = 0;
				json->state = json->scope;
				continue;
//				break;

			case '\\':
				json->state = UG_JSON_CONTROL;
				continue;
//				break;

			default:
				json->buf.at[json->buf.length++] = vchar;
				continue;
//				break;
			}
			break;

		case UG_JSON_CONTROL:
			json->state = UG_JSON_STRING;
			switch (vchar) {
			case '\"':
				json->buf.at[json->buf.length++] = '\"';
				continue;
//				break;
			case '\\':
				json->buf.at[json->buf.length++] = '\\';
				continue;
//				break;
			case '/':
				json->buf.at[json->buf.length++] = '/';
				continue;
//				break;
			case 'b':
				json->buf.at[json->buf.length++] = '\b';
				continue;
//				break;
			case 'f':
				json->buf.at[json->buf.length++] = '\f';
				continue;
//				break;
			case 'n':
				json->buf.at[json->buf.length++] = '\n';
				continue;
//				break;
			case 'r':
				json->buf.at[json->buf.length++] = '\r';
				continue;
//				break;
			case 't':
				json->buf.at[json->buf.length++] = '\t';
				continue;
//				break;
			case 'u':
				json->state = UG_JSON_UNICODE;
				json->count = 0;
				continue;
//				break;
			default:
				return UG_JSON_ERROR_INVALID_CONTROL;
			}
			break;

		case UG_JSON_UNICODE:
			// convert unicode UTF-16 to UTF-8
			json->buf.at[json->buf.length++] = vchar;
			if (++json->count == 4) {
				const char* putf;
				uint32_t    value;

				json->state = UG_JSON_STRING;
				json->buf.length -= 4;
//				json->count = 0;		// json->count must reset to 0
				// get UTF-16 value from hex string
				putf = json->buf.at + json->buf.length;
				for (value = 0;  json->count > 0;  json->count--, putf++) {
					value <<= 4;
					vchar = putf[0];
					if (vchar >= '0' && vchar <= '9')
						value += vchar - '0';
					else if (vchar >= 'a' && vchar <= 'f')
						value += vchar - 'a' + 10;
					else if (vchar >= 'A' && vchar <= 'F')
						value += vchar - 'A' + 10;
					else
						return UG_JSON_ERROR_INVALID_UNICODE;
				}

				if (value >= 1 && value <= 0x7F)
					json->buf.at[json->buf.length++] = value;
				else if (value > 0x7FF) {
					json->buf.at[json->buf.length++] = 0xE0 | ((value >> 12) & 0x0F);
					json->buf.at[json->buf.length++] = 0x80 | ((value >>  6) & 0x3F);
					json->buf.at[json->buf.length++] = 0x80 | ((value >>  0) & 0x3F);
				}
				else {
					json->buf.at[json->buf.length++] = 0xC0 | ((value >> 12) & 0x1F);
					json->buf.at[json->buf.length++] = 0x80 | ((value >>  6) & 0x3F);
				}
			}
			break;

		case UG_JSON_TRUE:
			json->buf.at[json->buf.length++] = vchar;
			if (++json->count == 4) {
				if (strncmp ("true", json->buf.at + json->buf.length - 4, 4))
					return UG_JSON_ERROR_INVALID_VALUE;
				json->type = UG_JSON_TRUE;
				json->state = json->scope;
				json->buf.at[json->buf.length++] = 0;	// null-terminated
			}
			break;

		case UG_JSON_FALSE:
			json->buf.at[json->buf.length++] = vchar;
			if (++json->count == 5) {
				if (strncmp ("false", json->buf.at + json->buf.length - 5, 5))
					return UG_JSON_ERROR_INVALID_VALUE;
				json->type = UG_JSON_FALSE;
				json->state = json->scope;
				json->buf.at[json->buf.length++] = 0;	// null-terminated
			}
			break;

		case UG_JSON_NULL:
			json->buf.at[json->buf.length++] = vchar;
			if (++json->count == 4) {
				if (strncmp ("null", json->buf.at + json->buf.length - 4, 4))
					return UG_JSON_ERROR_INVALID_VALUE;
				json->type = UG_JSON_NULL;
				json->state = json->scope;
				json->buf.at[json->buf.length++] = 0;	// null-terminated
			}
			break;

		case UG_JSON_NUMBER:
			switch (vchar) {
			case '.':
				if (json->numberPoint || json->numberEe || json->numberPm)
					return UG_JSON_ERROR_INVALID_NUMBER;
				json->numberPoint = 1;
				break;

			case 'E':
			case 'e':
				if (json->numberEe || json->numberPm)
					return UG_JSON_ERROR_INVALID_NUMBER;
				json->numberEe = 1;
				break;

			case '+':
			case '-':
				if (json->numberPm || json->numberEe == 0)
					return UG_JSON_ERROR_INVALID_NUMBER;
				json->numberPm = 1;
				break;

			default:
				if (vchar < '0' || vchar > '9') {
					json->state = json->scope;
					json->buf.at[json->buf.length++] = 0;
					goto TopLevelSwitch;
				}
				break;
			}
			json->buf.at[json->buf.length++] = vchar;
			break;

		default:
			return UG_JSON_ERROR_UNKNOWN;
		}
		// end of top level switch
	}

	return json->error;
}

// UgJsonParseFunc for JSON that starting with array.
UgJsonError  ug_json_parse_array (UgJson* json,
                                  const char* name, const char* value,
                                  void* dest, void* data)
{
	if (json->type != UG_JSON_ARRAY) {
		// pop this and previous parser
		ug_json_pop (json);
		ug_json_pop (json);
		return UG_JSON_ERROR_TYPE_NOT_MATCH;
	}
	// pop this parser only, use previous parser
	ug_json_pop (json);
	return UG_JSON_ERROR_NONE;
}

// UgJsonParseFunc for JSON that starting with object.
UgJsonError  ug_json_parse_object (UgJson* json,
                                   const char* name, const char* value,
                                   void* dest, void* data)
{
	if (json->type != UG_JSON_OBJECT) {
		// pop this and previous parser
		ug_json_pop (json);
		ug_json_pop (json);
		return UG_JSON_ERROR_TYPE_NOT_MATCH;
	}
	// pop this parser only, use previous parser
	ug_json_pop (json);
	return UG_JSON_ERROR_NONE;
}

// UgJsonParseFunc for unknown object or array.
UgJsonError  ug_json_parse_unknown (UgJson* json,
                                    const char* name, const char* value,
                                    void* dest, void* data)
{
//	if (json->type >= UG_JSON_OBJECT)
//		ug_json_push (json, ug_json_parse_unknown, NULL, NULL);

	return UG_JSON_ERROR_NONE;
}

// static function
static void  ug_json_call_parser (UgJson* json)
{
	UgJsonParseFunc parser;
	void**          stack;
	char            error;
	int             stackLen;

	// get parser and data from stack
	if (json->stack.length && json->type < UG_JSON_N_TYPE) {
		stackLen = json->stack.length;
		stack  = json->stack.at + json->stack.length;
		parser = *(stack - PARSER_STACK_FUNC);
		error = parser (json,
				json->buf.at + json->index[0],
				json->buf.at + json->index[1],
				*(stack - PARSER_STACK_DATA1),
				*(stack - PARSER_STACK_DATA2));
		if (error)
			json->error = error;
		// It must push parser when getting UG_JSON_OBJECT or UG_JSON_ARRAY.
		// If callback function does NOT push any parser, push default one.
		if (json->type >= UG_JSON_OBJECT && stackLen == json->stack.length)
			ug_json_push (json, ug_json_parse_unknown, NULL, NULL);
	}

	// reset state
//	json->type = 0;    // UG_JSON_VALUE
	json->count = 0;
	json->colon = 0;
	json->numberPoint = 0;
	json->numberEe = 0;
	json->numberPm = 0;
	json->index[0] = 0;
	json->index[1] = 0;
	// reset buffer
	json->buf.at[0] = 0;
	json->buf.length = 1;
}

// ----------------------------------------------------------------------------
// JSON Writer functions

#define WRITER_STACK_BASE   1
#define WRITER_INDENT_LEN   1    // 2
// UgJson.state = UgJsonFormat
// UgJson.index[0] = level
// UgJson.stack.at[0] = UgBuffer

// UgJson Writer functions
void  ug_json_begin_write (UgJson* json, UgJsonFormat format, UgBuffer* buffer)
{
	json->index[0] = 0;
	json->stack.at[0] = buffer;
	json->stack.length = WRITER_STACK_BASE;
	json->state = format;
	json->error = 0;
	json->colon = 1;	// for calling ug_json_write_head() first time
	json->type = UG_JSON_N_TYPE;
	// reset buffer
	json->buf.length = 0;
}

void  ug_json_end_write (UgJson* json)
{
	UgBuffer*  buffer;

	// UgJson.stack.at[0] = UgBuffer
	buffer = json->stack.at[0];
	if (buffer->more != ug_buffer_expand)
		buffer->more (buffer);
	// clear stack
	json->stack.length = 0;
}

void  ug_json_write_head (UgJson* json, char ch)
{
	UgBuffer* buffer;

	// UgJson.stack.at[0] = UgBuffer
	buffer = json->stack.at[0];
	if (json->type < UG_JSON_N_TYPE)
		ug_buffer_write_char (buffer, ',');
	// UgJson.state = UgJsonFormat
	// UgJson.index[0] = level
	if ((json->state & UG_JSON_FORMAT_INDENT) && json->colon == 0) {
		ug_buffer_write_char (buffer, '\n');
//		ug_buffer_fill (buffer, ' ', json->index[0]);
		ug_buffer_fill (buffer, '\t', json->index[0]);
	}

	// push scope to stack
	if (json->stack.allocated == json->stack.length) {
		json->stack.allocated *= 2;
		json->stack.at = ug_realloc (json->stack.at,
				json->stack.allocated * sizeof (void*));
	}
	json->stack.at[json->stack.length++] = (void*)(uintptr_t) json->scope;

	ug_buffer_write_char (buffer, ch);
	json->scope = (ch == '{') ? UG_JSON_OBJECT : UG_JSON_ARRAY;
	json->colon = 0;
	json->type = UG_JSON_N_TYPE;
	json->index[0] += WRITER_INDENT_LEN;	//	json->index[0]++;
}

void  ug_json_write_tail (UgJson* json, char ch)
{
	UgBuffer* buffer;

	// UgJson.stack.at[0] = UgBuffer
	buffer = json->stack.at[0];
	// pop scope from stack
	if (json->stack.length > WRITER_STACK_BASE)
		json->scope = (uintptr_t)json->stack.at[--json->stack.length];

	// UgJson.state = UgJsonFormat
	// UgJson.index[0] = level
	if (json->index[0])
		json->index[0] -= WRITER_INDENT_LEN;	// json->index[0]--;
	if (json->state & UG_JSON_FORMAT_INDENT) {
		ug_buffer_write_char (buffer, '\n');
		// WRITER_INDENT_LEN
//		ug_buffer_fill (buffer, ' ', json->index[0]);
		ug_buffer_fill (buffer, '\t', json->index[0]);
	}

	ug_buffer_write_char (buffer, ch);
	json->type = json->scope;
}

void  ug_json_write_null (UgJson* json)
{
	UgBuffer* buffer;

	// UgJson.stack.at[0] = UgBuffer
	buffer = json->stack.at[0];

	if (json->type < UG_JSON_N_TYPE)
		ug_buffer_write_char (buffer, ',');
	// UgJson.state = UgJsonFormat
	// UgJson.index[0] = level
	if ((json->state & UG_JSON_FORMAT_INDENT) && json->colon == 0) {
		ug_buffer_write_char (buffer, '\n');
		// WRITER_INDENT_LEN
//		ug_buffer_fill (buffer, ' ', json->index[0]);
		ug_buffer_fill (buffer, '\t', json->index[0]);
	}

	ug_buffer_write (buffer, "null", 4);
	json->type = UG_JSON_NULL;
	json->colon = 0;
}

void  ug_json_write_bool (UgJson* json, int value)
{
	UgBuffer* buffer;

	// UgJson.stack.at[0] = UgBuffer
	buffer = json->stack.at[0];

	if (json->type < UG_JSON_N_TYPE)
		ug_buffer_write_char (buffer, ',');
	// UgJson.state = UgJsonFormat
	// UgJson.index[0] = level
	if ((json->state & UG_JSON_FORMAT_INDENT) && json->colon == 0) {
		ug_buffer_write_char (buffer, '\n');
		// WRITER_INDENT_LEN
//		ug_buffer_fill (buffer, ' ', json->index[0]);
		ug_buffer_fill (buffer, '\t', json->index[0]);
	}

	if (value == 0) {
		ug_buffer_write (buffer, "false", 5);
		json->type = UG_JSON_FALSE;
	}
	else {
		ug_buffer_write (buffer, "true", 4);
		json->type = UG_JSON_TRUE;
	}

	json->colon = 0;
}

void  ug_json_write_number (UgJson* json, const char* format, ...)
{
	va_list   arg_list;
	int       length;
	UgBuffer* buffer;

	// UgJson.stack.at[0] = UgBuffer
	buffer = json->stack.at[0];

	if (json->type < UG_JSON_N_TYPE)
		ug_buffer_write_char (buffer, ',');
	// UgJson.state = UgJsonFormat
	// UgJson.index[0] = level
	if ((json->state & UG_JSON_FORMAT_INDENT) && json->colon == 0) {
		ug_buffer_write_char (buffer, '\n');
		// WRITER_INDENT_LEN
//		ug_buffer_fill (buffer, ' ', json->index[0]);
		ug_buffer_fill (buffer, '\t', json->index[0]);
	}

	va_start (arg_list, format);
#ifdef _MSC_VER		// for M$ C only
	length = _vscprintf (format, arg_list) + 1;
#else				// for C99 standard
	length = vsnprintf (NULL, 0, format, arg_list) + 1;
#endif
	va_end (arg_list);

	if (length < buffer->end - buffer->cur) {
		va_start (arg_list, format);
		vsprintf ((char*) buffer->cur, format, arg_list);
		va_end (arg_list);
		buffer->cur += length - 1;
	}
	else {
		if (length > json->buf.allocated) {
			json->buf.allocated = (json->buf.allocated + length) * 2;
			json->buf.at = ug_realloc (json->buf.at, json->buf.allocated);
		}
		va_start (arg_list, format);
		vsprintf (json->buf.at, format, arg_list);
		va_end (arg_list);
		ug_buffer_write (buffer, json->buf.at, length - 1);
	}

	json->type = UG_JSON_NUMBER;
	json->colon = 0;
}

void  ug_json_write_string (UgJson* json, const char* string)
{
	static const uint8_t  utf8Limits[] = {0xC0, 0xE0, 0xF0, 0xF8, 0xFC};
	static const uint8_t  hexTable[]   = {"0123456789ABCDEF"};
	uint8_t               ch;
	int                   count;
	uint32_t              value;
	UgBuffer*             buffer;

	if (string == NULL) {
		ug_json_write_null (json);
		return;
	}

	// UgJson.stack.at[0] = UgBuffer
	buffer = json->stack.at[0];

	if (json->type < UG_JSON_N_TYPE)
		ug_buffer_write_char (buffer, ',');
	// UgJson.state = UgJsonFormat
	// UgJson.index[0] = level
	if ((json->state & UG_JSON_FORMAT_INDENT) && json->colon == 0) {
		ug_buffer_write_char (buffer, '\n');
		// WRITER_INDENT_LEN
//		ug_buffer_fill (buffer, ' ', json->index[0]);
		ug_buffer_fill (buffer, '\t', json->index[0]);
	}

	ug_buffer_write_char (buffer, '\"');

	while ((ch = string[0])) {
		string++;
		if (ch < 0x80 || (json->state & UG_JSON_FORMAT_UTF8)) {
			switch (ch) {
			case '\"':
				ug_buffer_write (buffer, "\\\"", 2);
				break;

			case '\\':
				ug_buffer_write (buffer, "\\\\", 2);
				break;

			case '/':
				ug_buffer_write (buffer, "\\/", 2);
				break;

			case '\b':
				ug_buffer_write (buffer, "\\b", 2);
				break;

			case '\f':
				ug_buffer_write (buffer, "\\f", 2);
				break;

			case '\n':
				ug_buffer_write (buffer, "\\n", 2);
				break;

			case '\r':
				ug_buffer_write (buffer, "\\r", 2);
				break;

			case '\t':
				ug_buffer_write (buffer, "\\t", 2);
				break;

			default:
				ug_buffer_write_char (buffer, ch);
				break;
			}
			continue;
		}

		// UTF-8 to UTF-16 "\uXXXX"
		// If first is less than 0xC0? skip it.
		if (ch < 0xC0)
			break;

		for(count = 1; count < 5; ++count)
			if(ch < utf8Limits[count])
				break;
		value = ch - utf8Limits[count - 1];

		do {
			uint8_t  ch2;

			ch2 = *string++;
			if (ch2 == 0)
				break;
			if (ch2 < 0x80 || ch2 >= 0xC0)
				break;
			value <<= 6;
			value |= (ch2 - 0x80);
		} while(--count != 0);

		if(value < 0x10000) {
			ug_buffer_write (buffer, "\\u", 2);
			for (count = 12;  count >= 0;  count -= 4)
				ug_buffer_write_char (buffer, hexTable[ (value >> count) & 0xF]);
		}
		else {
			value -= 0x10000;
			if(value >= 0x100000)
				break;

			ug_buffer_write (buffer, "\\u", 2);
			for (count = 12 + 10;  count >= 0 + 10;  count -= 4)
				ug_buffer_write_char (buffer, hexTable[ (value >> count) & 0xF]);

			ug_buffer_write (buffer, "\\u", 2);
			value &= 0x3FF;
			for (count = 12;  count >= 0;  count -= 4)
				ug_buffer_write_char (buffer, hexTable[ (value >> count) & 0xF]);
		}
		// End of UTF-8 to UTF-16
	}

	ug_buffer_write_char (buffer, '\"');

	if (json->scope == UG_JSON_OBJECT) {
		if (json->colon) {
			json->colon = 0;
			json->type = UG_JSON_STRING;
		}
		else {
			json->colon = 1;
			json->type = UG_JSON_N_TYPE;
			ug_buffer_write_char (buffer, ':');
			// json->state = UgJsonFormat
			if (json->state & UG_JSON_FORMAT_INDENT)
				ug_buffer_write_char (buffer, ' ');
		}
	}
	else
		json->type = UG_JSON_STRING;
}

