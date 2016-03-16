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

#ifndef UGET_CURL_H
#define UGET_CURL_H

#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif

#ifdef HAVE_CONFIG
#include <config.h>
#endif

#include <UgDefine.h>
#include <UgThread.h>
#include <UgUri.h>
#include <UgetData.h>
#include <UgetEvent.h>
#include <curl/curl.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct UgetCurl     UgetCurl;

typedef int (*UgetCurlFunc) (UgetCurl* ugcurl, void* data);

// UgetCurlState flow:
// UGET_CURL_READY
//         |
//         `-> UGET_CURL_RUN -+-> UGET_CURL_OK    ---> UGET_CURL_RESPLIT
//                            |
//                            +-> UGET_CURL_ERROR
//                            |
//                            +-> UGET_CURL_RETRY
//                            |
//                            +-> UGET_CURL_ABORT
//                            |
//                            `-> UGET_CURL_NOT_RESUMABLE

enum UgetCurlState
{
	UGET_CURL_RESPLIT,          // used by curl plug-in
	UGET_CURL_READY,
	UGET_CURL_RUN,
	UGET_CURL_OK,
	UGET_CURL_ERROR,
	UGET_CURL_RETRY,
	UGET_CURL_ABORT,
	UGET_CURL_NOT_RESUMABLE,    // redownload + retry
};

// ----------------------------------------------------------------------------
// UgetCurl: used by UgetPluginCurl

struct UgetCurl
{
	UG_LINK_MEMBERS (UgetCurl, UgetCurl, self);
//	UgetCurl*    self;
//	UgetCurl*    next;
//	UgetCurl*    prev;

	UgThread     thread;
	CURL*        curl;
	int64_t      beg;
	int64_t      end;
	int64_t      pos;  // current position

	UgetCommon*  common;
	UgetHttp*    http;
	UgetFtp*     ftp;
	UgetEvent*   event;

//	struct curl_slist*  ftp_command;
	struct {
		UgUri    part;
		void*    link;
	} uri;

	// size[0]  = downloaded size
	// size[1]  = uploaded size
	// speed[0] = downloading speed
	// speed[1] = uploading speed
	// limit[0] = download speed limit
	// limit[1] = upload speed limit
	int64_t      size[2];
	int64_t      speed[2];
	int64_t      limit[2];

	// file stream
	struct {
		FILE*    output;
		FILE*    post;
	} file;

	// if user specify prepare.func,
	// UgetCurl will call prepare.func to open file in write function.
	struct {
		UgetCurlFunc  func;
		void*         data;
	} prepare;

	// HTTP header response data
	// set header_store = TRUE to enable this.
	struct {
		char*   uri;
		char*   filename;
	} header;

	long        response;    // from HTTP or FTP
	int         event_code;  // for CURLE_WRITE_ERROR (UGET_CURL_ERROR)
	uint8_t     state;       // UgetCurlState
	uint8_t     progress_count;
	uint8_t     scheme_type:4;
	uint8_t     restart:1;

	// flags
	uint8_t     limit_changed:1; // speed limit changed
	uint8_t     header_store:1;  // save uri and filename from header.
	uint8_t     resumable:1;     // get resumable in header callback
	uint8_t     stopped:1;  // running control & status
	uint8_t     tested:1;   // URI tested
	uint8_t     test_ok:1;  // URI test ok
	uint8_t     split:1;    // split previous
	uint8_t     html:1;     // "Content-Type: text/html"

	char        error_string[CURL_ERROR_SIZE];
};

UgetCurl*  uget_curl_new (void);
void       uget_curl_free (UgetCurl* ugcurl);

void  uget_curl_run (UgetCurl* ugcurl, int joinable);

int   uget_curl_open_file (UgetCurl* ugcurl, const char* filename);
void  uget_curl_close_file (UgetCurl* ugcurl);
void  uget_curl_set_url (UgetCurl* ugcurl, const char* uri);
void  uget_curl_set_speed (UgetCurl* ugcurl, int64_t dlspeed, int64_t ulspeed);

void  uget_curl_set_common (UgetCurl* ugcurl, UgetCommon* common);
void  uget_curl_set_proxy (UgetCurl* ugcurl, UgetProxy* proxy);
int   uget_curl_set_http (UgetCurl* ugcurl, UgetHttp* http);
void  uget_curl_set_ftp (UgetCurl* ugcurl, UgetFtp* ftp);

void  uget_curl_decide_scheme (UgetCurl* ugcurl, const char* uri);
void  uget_curl_decide_login (UgetCurl* ugcurl);

#define uget_curl_join_thread(ugcurl)   ug_thread_join (&(ugcurl)->thread)

void  ug_curl_set_proxy (CURL* curl, UgetProxy* proxy);

#ifdef __cplusplus
}
#endif

#endif  // End of UGET_CURL_H
