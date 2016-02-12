/*
 *
 *   Copyright (C) 2015-2016 by C.H. Huang
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
#include <UgetCurl.h>
#include <UgetMedia.h>

#ifdef HAVE_GLIB
#include <glib/gi18n.h>
#else
#define _(x)   x
#endif

#ifndef ABS
#define ABS(a)	   (((a) < 0) ? -(a) : (a))
#endif

// ----------------------------------------------------------------------------
// UgetMediaItem

static UgetMediaItem*  uget_media_item_new (UgetMedia* um)
{
	UgetMediaItem* umitem;

	umitem = ug_malloc0 (sizeof (UgetMediaItem));
	umitem->self = umitem;
	ug_list_append ((UgList*) um, (UgLink*) umitem);
	return umitem;
}

static void  uget_media_item_free (UgetMediaItem* umitem)
{
	ug_free (umitem->url);
	ug_free (umitem);
}

// ----------------------------------------------------------------------------
// UgetMedia

static int    grab_items_youtube (UgetMedia* um, CURL*);
static size_t curl_output_youtube (char *buffer, size_t size,
                                   size_t nmemb, void* data);

int  uget_media_get_site_id (const char* url)
{
	UgUri       uuri;
//	const char* str;
//	int         str_len;

	if (ug_uri_init (&uuri, url) == 0)
		return UGET_MEDIA_UNKNOWN;

	if (uuri.scheme_len >=4 && strncmp (url, "http", 4) == 0) {
		// youtube
		if (strncmp (url + uuri.host , "www.youtube.com", 15) == 0) {
			if (strncmp (url + uuri.file , "watch?", 6) != 0)
				return UGET_MEDIA_UNKNOWN;
			return UGET_MEDIA_YOUTUBE;
		}
	}

	return UGET_MEDIA_UNKNOWN;
}


UgetMedia*  uget_media_new (const char* url, UgetMediaSiteId site_id)
{
	UgetMedia* um;

	um = ug_malloc0 (sizeof (UgetMedia));
	um->url = ug_strdup (url);
	if (site_id == UGET_MEDIA_UNKNOWN)
		site_id = uget_media_get_site_id (url);
	um->site_id = site_id;

	return um;
}

void  uget_media_free (UgetMedia* um)
{
	ug_list_foreach ((UgList*)um, (UgForeachFunc)uget_media_item_free, NULL);
	ug_free (um->url);
	ug_free (um->title);
	if (um->event)
		uget_event_free (um->event);
	ug_free (um);
}

int  uget_media_grab_items (UgetMedia* um, UgetProxy* proxy)
{
	CURL*  curl;
	int    n_items = 0;

	curl = curl_easy_init ();
	curl_easy_setopt (curl, CURLOPT_URL, um->url);
	if (proxy)
		ug_curl_set_proxy (curl, proxy);

	switch (um->site_id) {
	case UGET_MEDIA_YOUTUBE:
		n_items = grab_items_youtube (um, curl);
		break;

	case UGET_MEDIA_UNKNOWN:
	default:
		break;
	}

	curl_easy_cleanup (curl);
	return n_items;
}

UgetMediaItem*  uget_media_match (UgetMedia*  um,
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
		return um->head;

	for (cur = um->tail;  cur;  cur = prev) {
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
			ug_list_remove ((UgList*) um, (UgLink*) cur);
			ug_list_append ((UgList*) um, (UgLink*) cur);
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
		ug_list_remove ((UgList*) um, (UgLink*) result);
		ug_list_append ((UgList*) um, (UgLink*) result);
	}

	return result;
}

// --------------------------------------------------------
// UgetMedia for YouTube

typedef struct UgetYouTube    UgetYouTube;

struct UgetYouTube
{
	UgBuffer    buffer;
	UgUriQuery  query;
};

static UgetYouTube* uget_youtube_new (void)
{
	UgetYouTube* uyt;

	uyt = ug_malloc0 (sizeof (UgetYouTube));
	ug_buffer_init (&uyt->buffer, 4096);
	return uyt;
}

static void  uget_youtube_free (UgetYouTube* uyt)
{
	ug_buffer_clear (&uyt->buffer, TRUE);
	ug_free (uyt);
}

// url_encoded_fmt_stream_map
static void  uget_youtube_parse_map (UgetYouTube* uyt, UgetMedia* um, const char* field)
{
	UgetMediaItem*  umitem = NULL;

	while (ug_uri_query_part (&uyt->query, field)) {
		// debug
//		printf ("    %.*s=%.*s\n",
//				uyt->query.field_len, field,
//				uyt->query.value_len, uyt->query.value);

		if (umitem == NULL)
			umitem = uget_media_item_new (um);

		if (strncmp ("url", field, uyt->query.field_len) == 0) {
			ug_decode_uri (uyt->query.value, uyt->query.value_len, uyt->query.value);
			umitem->url = ug_strdup (uyt->query.value);
		}
		else if (strncmp ("type", field, uyt->query.field_len) == 0) {
			ug_decode_uri (uyt->query.value, uyt->query.value_len, uyt->query.value);
			if (strncmp ("video/webm", uyt->query.value, 10) == 0)
				umitem->type = UGET_MEDIA_TYPE_WEBM;
			else if (strncmp ("video/mp4", uyt->query.value, 9) == 0)
				umitem->type = UGET_MEDIA_TYPE_MP4;
			else if (strncmp ("video/x-flv", uyt->query.value, 11) == 0)
				umitem->type = UGET_MEDIA_TYPE_FLV;
			else if (strncmp ("video/3gpp", uyt->query.value, 10) == 0)
				umitem->type = UGET_MEDIA_TYPE_3GPP;
			else
				umitem->type = UGET_MEDIA_TYPE_UNKNOWN;
		}
		else if (strncmp (field, "quality", uyt->query.field_len) == 0) {
//			ug_decode_uri (uyt->query.value, uyt->query.value_len, uyt->query.value);
			if (strncmp ("small", uyt->query.value, uyt->query.value_len) == 0)
				umitem->quality = UGET_MEDIA_QUALITY_240P;
			else if (strncmp ("medium", uyt->query.value, uyt->query.value_len) == 0)
				umitem->quality = UGET_MEDIA_QUALITY_360P;
			else if (strncmp ("large", uyt->query.value, uyt->query.value_len) == 0)
				umitem->quality = UGET_MEDIA_QUALITY_480P;
			else if (strncmp ("hd720", uyt->query.value, uyt->query.value_len) == 0)
				umitem->quality = UGET_MEDIA_QUALITY_720P;
			else if (strncmp ("hd1080", uyt->query.value, uyt->query.value_len) == 0)
				umitem->quality = UGET_MEDIA_QUALITY_1080P;
			else
				umitem->quality = UGET_MEDIA_QUALITY_UNKNOWN;
		}

		if (uyt->query.value_next) {
			field = uyt->query.value_next;
			umitem = NULL;
		}
		else
			field = uyt->query.field_next;
	}
}

static void  uget_youtube_parse_query (UgetYouTube* uyt, UgetMedia* um)
{
	if (ug_uri_query_part (&uyt->query, uyt->buffer.beg) == 0)
		return;

	// debug
//	printf ("%.*s\n", uyt->query.field_len, uyt->buffer.beg);

	if (uyt->query.field_len == 26 && strncmp (uyt->buffer.beg, "url_encoded_fmt_stream_map", 26) == 0) {
		ug_decode_uri (uyt->query.value, uyt->query.value_len, uyt->query.value);
		uget_youtube_parse_map (uyt, um, uyt->query.value);
	}
	else if (uyt->query.field_len == 5 && strncmp (uyt->buffer.beg, "title", 5) == 0) {
		um->title = ug_strndup (uyt->query.value, uyt->query.value_len);
		ug_decode_uri (um->title, uyt->query.value_len, um->title);
	}
}

static int    grab_items_youtube (UgetMedia* um, CURL* curl)
{
	char*  str;
	char*  video_id_str;
	int    video_id_len;
	CURLcode    code;
	UgBuffer*   buffer;
 	UgUriQuery  query;

	um->data = uget_youtube_new ();
	ug_uri_init (&um->uuri, um->url);
	if (um->uuri.query == -1)
		return 0;

	// "watch?v=xxxx"
	video_id_str = NULL;
	str = um->url + um->uuri.query;
	while (ug_uri_query_part (&query, str)) {
		if (strncmp ("v", str, query.field_len) == 0 && query.value) {
			video_id_str = query.value;
			video_id_len = query.value_len;
			break;
		}
		str = query.field_next;
	}
	if (video_id_str == NULL)
		return 0;

	str = ug_strdup_printf ("%.*sget_video_info?video_id=%.*s",
	                        um->uuri.file, um->url,
	                        video_id_len, video_id_str);

	curl_easy_setopt (curl, CURLOPT_URL, str);
	curl_easy_setopt (curl, CURLOPT_NOSIGNAL, 1L);
	curl_easy_setopt (curl, CURLOPT_WRITEFUNCTION, curl_output_youtube);
	curl_easy_setopt (curl, CURLOPT_WRITEDATA, um);
	curl_easy_setopt (curl, CURLOPT_SSL_VERIFYHOST, 0L);
	curl_easy_setopt (curl, CURLOPT_SSL_VERIFYPEER, 0L);
	code = curl_easy_perform (curl);
	ug_free (str);

	switch (code) {
	case CURLE_OK:
		buffer = &((UgetYouTube*)um->data)->buffer;
		if (buffer->beg != buffer->cur) {
			ug_buffer_write_char (buffer, 0);
			uget_youtube_parse_query (um->data, um);
		}
		break;

	case CURLE_OUT_OF_MEMORY:
		um->event = uget_event_new_error (
				UGET_EVENT_ERROR_OUT_OF_RESOURCE, NULL);
		break;

	default:
		um->event = uget_event_new_error (
				UGET_EVENT_ERROR_CUSTOM,
				_("Error occurred during getting video info."));
		break;
	}

	uget_youtube_free (um->data);
	return um->size;
}

static size_t curl_output_youtube (char* beg, size_t size,
                                   size_t nmemb, void* data)
{
	UgetMedia*  um = data;
	UgBuffer*   buffer = &((UgetYouTube*)um->data)->buffer;
	char*  end;
	char*  cur;

	size *= nmemb;
	end = beg + size;

	for (cur = beg;  cur < end;  cur++) {
		if (cur[0] == '&') {
			ug_buffer_write_data (buffer, beg, cur - beg);
			ug_buffer_write_char (buffer, 0);
			uget_youtube_parse_query (um->data, um);
			// next field
			buffer->cur = buffer->beg;
			beg = cur + 1;  // + '&'
			continue;
		}
	}
	if (cur == end)
		ug_buffer_write_data (buffer, beg, cur - beg);

	return size;
}

