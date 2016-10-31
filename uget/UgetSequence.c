/*
 *
 *   Copyright (C) 2016 by C.H. Huang
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
#include <string.h>
#include <UgUtil.h>
#include <UgetSequence.h>

typedef struct UgLinkString    UgLinkString;

struct UgLinkString
{
	UG_LINK_MEMBERS (UgLinkString, char, data);
//	char*         data;
//	UgLinkString* next;
//	UgLinkString* prev;

	char        string[1];
};

static UgLink* ug_link_string_new (const char* string, int length)
{
	UgLinkString*   link;

	link = ug_malloc (sizeof (UgLinkString) + length);
	link->data = link->string;
	strcpy (link->string, string);
	return (UgLink*) link;
}

// ----------------------------------------------------------------------------
// UgetSeqRange
static void uget_seq_range_to_first (UgetSeqRange* range)
{
    range->cur = range->first;
}

static void uget_seq_range_to_last (UgetSeqRange* range)
{
    range->cur = range->last;
}

// ----------------------------------------------------------------------------

void uget_sequence_init (UgetSequence* useq)
{
	ug_array_init (useq, sizeof (UgetSeqRange), 8);
	ug_buffer_init (&useq->buf, 128);
}

void uget_sequence_final (UgetSequence* useq)
{
	ug_array_clear (useq);
	ug_buffer_clear (&useq->buf, TRUE);
}

void uget_sequence_add (UgetSequence* useq, uint32_t first, uint32_t last, int digits)
{
	UgetSeqRange*  range;

	range = ug_array_alloc (useq, 1);
	range->digits = digits;

	if (first < last) {
		range->first = first;
		range->last  = last;
	}
	else {
		range->first = last;
		range->last  = first;
	}
	range->cur  = range->first;
}

void uget_sequence_clear (UgetSequence* useq)
{
	useq->length = 0;
}

int  uget_sequence_count (UgetSequence* useq, const char* pattern)
{
	UgetSeqRange*   range;
	UgetSeqRange*   range_end;
	const char* pcur;
	int         pcur_len;
	int         count;

	count = 0;
	range     = useq->at;
	range_end = useq->at + useq->length;

	for (pcur = pattern;  pcur[0] && range < range_end;  pcur++, range++) {
		pcur_len = strcspn (pcur, "*");
		if (pcur[pcur_len] != '*')
			break;

		if (count == 0)
			count = range->last - range->first + 1;
		else
			count = count * (range->last - range->first + 1);

		pcur += pcur_len;    // to next '*'
	}

	return count;
}

// generate string by pattern
static char* uget_sequence_generate1 (UgetSequence* useq, const char* pattern)
{
	UgetSeqRange*  range;
	char*       utf8;
	int         length;
	const char* pcur;
	int         pcur_len;

	range = useq->at;
	useq->buf.cur = useq->buf.beg;

	for (pcur = pattern;  pcur[0];  pcur++) {
		pcur_len = strcspn (pcur, "*");
		ug_buffer_write (&useq->buf, pcur, pcur_len);
		if (pcur[pcur_len] != '*')
			break;

		if (range->digits == 0) {
			// ASCII or Unicode
			if (range->cur < 0x80)
				ug_buffer_write_char (&useq->buf, range->cur);
			else {
				utf8 = ug_ucs4_to_utf8 (&range->cur, 1, &length);
				ug_buffer_write (&useq->buf, utf8, length);
				ug_free (utf8);
			}
		}
		else {
			// digits, 0 - 9
#ifdef _MSC_VER		// for MS C only
			length = _scprintf ("%.*u", range->digits, range->cur);
#else				// for C99 standard
			length = snprintf (NULL, 0, "%.*u", range->digits, range->cur);
#endif
			sprintf (ug_buffer_alloc (&useq->buf, length + 1),
			          "%.*u", range->digits, range->cur);
			useq->buf.cur--;    // remove null character in tail
		}

		if (++range > useq->range_last)
			range = useq->at;
		pcur += pcur_len;    // to next '*'
	}

	ug_buffer_write_char (&useq->buf, 0);
	return useq->buf.beg;
}

static int  uget_sequence_generate (UgetSequence* useq, const char* pattern, UgetSeqRange* range, UgList* result)
{
	UgLink*         link;
	int             count;

	for (count = 0;  range->cur <= range->last;  range->cur++, count++) {
		if (range+1 <= useq->range_last)
			count += uget_sequence_generate (useq, pattern, range+1, result);
		else {
			uget_sequence_generate1 (useq, pattern);
			link = ug_link_string_new (useq->buf.beg, ug_buffer_length (&useq->buf));
			ug_list_append (result, link);
		}
	}

	range->cur = range->first;
	return count;
}

static UgetSeqRange*  uget_sequence_decide_range_last (UgetSequence* useq, const char* pattern)
{
	const char* wildcard;
	int         count;

	if (useq->length == 0)
		return NULL;

	// count wildcard character (*) to decide the last UgetSeqRange
	for (count = 0, wildcard = pattern;  wildcard[0];  wildcard++) {
		if (wildcard[0] == '*')
			count++;
	}
	if (count < useq->length)
		useq->range_last = useq->at + count -1;
	else
		useq->range_last = useq->at + useq->length -1;

	return useq->range_last;
}

int  uget_sequence_get_list (UgetSequence* useq, const char* pattern, UgList* result)
{
	if (uget_sequence_decide_range_last(useq, pattern) == NULL)
		return 0;

	// reset range
	ug_array_foreach (useq, (UgForeachFunc)uget_seq_range_to_first, NULL);
	// generate list
	return uget_sequence_generate (useq, pattern, useq->at, result);
}

int  uget_sequence_get_preview (UgetSequence* useq, const char* pattern, UgList* result)
{
	UgetSeqRange* range_last;
	UgetSeqRange* range_prev;
	UgLink*       link;
	int           count;

	// use uget_sequence_get_list() to generate preview if possible.
	count = uget_sequence_count (useq, pattern);
	if (count < 6)
		return uget_sequence_get_list (useq, pattern, result);

	range_last = uget_sequence_decide_range_last(useq, pattern);
	// decide previous UgetSeqRange by the last UgetSeqRange
	for (range_prev = range_last;  range_prev != useq->at;  range_prev--) {
		if (range_prev->last - range_prev->first + 1 > 1)
			break;
	}

	// reset range to first
	ug_array_foreach (useq, (UgForeachFunc)uget_seq_range_to_first, NULL);

	// create 1st string
	uget_sequence_generate1 (useq, pattern);
	link = ug_link_string_new (useq->buf.beg, ug_buffer_length (&useq->buf));
	ug_list_append (result, link);
	// create 2nd string
	range_prev->cur++;
	uget_sequence_generate1 (useq, pattern);
	link = ug_link_string_new (useq->buf.beg, ug_buffer_length (&useq->buf));
	ug_list_append (result, link);

	// create 3rd " ..."
	link = ug_link_string_new (" ...", 4);
	ug_list_append (result, link);

	// reset range to last
	ug_array_foreach (useq, (UgForeachFunc)uget_seq_range_to_last, NULL);

	// create 4th string
	range_prev->cur = range_prev->last - 1;
	uget_sequence_generate1 (useq, pattern);
	link = ug_link_string_new (useq->buf.beg, ug_buffer_length (&useq->buf));
	ug_list_append (result, link);
	// create 5th string
	range_prev->cur = range_prev->last;
	uget_sequence_generate1 (useq, pattern);
	link = ug_link_string_new (useq->buf.beg, ug_buffer_length (&useq->buf));
	ug_list_append (result, link);

	return 5;
}

