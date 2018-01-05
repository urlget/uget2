/*
 *
 *   Copyright (C) 2015-2018 by C.H. Huang
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
#include <UgUri.h>
#include <UgString.h>
#include <UgetMedia.h>

#ifndef ABS
#define ABS(a)	   (((a) < 0) ? -(a) : (a))
#endif

// ----------------------------------------------------------------------------
// UgetMedia

int  uget_media_grab_youtube (UgetMedia* umedia, UgetProxy* proxy);

UgetMedia*  uget_media_new (const char* url, UgetSiteId site_id)
{
	UgetMedia* umedia;

	umedia = ug_malloc0 (sizeof (UgetMedia));
	umedia->url = ug_strdup (url);
	if (site_id == UGET_SITE_UNKNOWN)
		site_id = uget_site_get_id (url);
	umedia->site_id = site_id;

	return umedia;
}

void  uget_media_free (UgetMedia* umedia)
{
	uget_media_clear (umedia, TRUE);
	ug_free (umedia);
}

void  uget_media_clear (UgetMedia* umedia, int free_items)
{
	if (free_items== TRUE) {
		ug_list_foreach ((UgList*) umedia,
				(UgForeachFunc) uget_media_item_free, NULL);
		ug_list_clear ((UgList*) umedia, FALSE);
	}

	ug_free (umedia->url);
	umedia->url = NULL;

	ug_free (umedia->title);
	umedia->title = NULL;

	if (umedia->event) {
		uget_event_free (umedia->event);
		umedia->event = NULL;
	}
}

int  uget_media_grab_items (UgetMedia* umedia, UgetProxy* proxy)
{
	int    n_items = 0;

	switch (umedia->site_id) {
	case UGET_SITE_YOUTUBE:
		n_items = uget_media_grab_youtube (umedia, proxy);
		break;

	case UGET_SITE_UNKNOWN:
	default:
		break;
	}

	return n_items;
}

UgetMediaItem*  uget_media_match (UgetMedia*          umedia,
                                  UgetMediaMatchMode  mode,
                                  UgetMediaQuality    quality,
                                  UgetMediaType       type)
{
	UgetMediaItem*  cur;
	UgetMediaItem*  prev;
	UgetMediaItem*  result = NULL;
	int             abs_cur, abs_res;
	int             count_cur, count_res;

	if (mode == UGET_MEDIA_MATCH_0)
		return umedia->head;

	for (cur = umedia->tail;  cur;  cur = prev) {
		prev = cur->prev;
		count_cur = 0;
		if (cur->quality == quality)
			count_cur++;
		if (cur->type == type)
			count_cur++;

		if ((mode == UGET_MEDIA_MATCH_1 && count_cur >= 1) ||
		    (mode == UGET_MEDIA_MATCH_2 && count_cur >= 2))
		{
			// move matched items to tail of list
			ug_list_remove ((UgList*) umedia, (UgLink*) cur);
			ug_list_append ((UgList*) umedia, (UgLink*) cur);
			if (result == NULL)
				result = cur;
		}
		else if (mode == UGET_MEDIA_MATCH_NEAR) {
			if (result == NULL) {
				result = cur;
				count_res = count_cur;
				continue;
			}
			abs_res = ABS (quality - result->quality);
			abs_cur = ABS (quality - cur->quality);
			if (abs_res == abs_cur) {
				if (count_res < count_cur) {
					result = cur;
					count_res = count_cur;
				}
				// choose near (or less) quality media file.
				else if (result->quality > cur->quality) {
					result = cur;
					count_res = count_cur;
				}
			}
			else if (abs_res > abs_cur) {
				result = cur;
				count_res = count_cur;
			}
		}
	}

	if (mode == UGET_MEDIA_MATCH_NEAR && result) {
		// move matched items to tail of list
		ug_list_remove ((UgList*) umedia, (UgLink*) result);
		ug_list_append ((UgList*) umedia, (UgLink*) result);
	}

	return result;
}

// ----------------------------------------------------------------------------
// UgetMediaItem

UgetMediaItem*  uget_media_item_new (UgetMedia* umedia)
{
	UgetMediaItem* umitem;

	umitem = ug_malloc0 (sizeof (UgetMediaItem));
	umitem->self = umitem;
	ug_list_append ((UgList*) umedia, (UgLink*) umitem);
	return umitem;
}

void  uget_media_item_free (UgetMediaItem* umitem)
{
	ug_free (umitem->url);
	ug_free (umitem);
}

