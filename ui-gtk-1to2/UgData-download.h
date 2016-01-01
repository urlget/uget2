/*
 *
 *   Copyright (C) 2005-2016 by C.H. Huang
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

// define UgDatalist-based structure for downloading.

// UgData
// |
// `- UgDatalist
//    |
//    +- UgCommon
//    |
//    +- UgProxy
//    |
//    +- UgProgress
//    |
//    +- UgHttp
//    |
//    +- UgFtp
//    |
//    `- UgLog

#ifndef UG_DATA_DOWNLOAD_H
#define UG_DATA_DOWNLOAD_H

#include <glib.h>
#include "UgData1.h"

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

// interface address for UgDataset
#define	UgCommonInfo      &ug_common_iface
#define	UgProxyInfo       &ug_proxy_iface
#define	UgProgressInfo    &ug_progress_iface
#define	UgHttpInfo        &ug_http_iface
#define	UgFtpInfo         &ug_ftp_iface
#define	UgLogInfo         &ug_log_iface

typedef struct	UgCommon      UgCommon;
typedef struct	UgProxy       UgProxy;
typedef struct	UgProgress    UgProgress;
typedef struct	UgHttp        UgHttp;
typedef struct	UgFtp         UgFtp;
typedef struct	UgLog         UgLog;

typedef enum	UgProxyType   UgProxyType;

extern	const	UgData1Interface		ug_common_iface;
extern	const	UgData1Interface		ug_proxy_iface;
extern	const	UgData1Interface		ug_progress_iface;
extern	const	UgData1Interface		ug_http_iface;
extern	const	UgData1Interface		ug_ftp_iface;
extern	const	UgData1Interface		ug_log_iface;

// ----------------------------------------------------------------------------
// UgCommon

//  UgData
//  |
//  `- UgDatalist
//     |
//     `- UgCommon

struct UgCommon
{
	UG_DATALIST_MEMBERS (UgCommon);
//	const UgData1Interface*	iface;
//	UgCommon*			next;
//	UgCommon*			prev;

	// common data
	gchar*		name;
	gchar*		url;
	gchar*		mirrors;
	gchar*		file;
	gchar*		folder;
	gchar*		user;
	gchar*		password;
	// timeout
	guint		connect_timeout;	// second
	guint		transmit_timeout;	// second
	// retry
	guint		retry_delay;		// second
	guint		retry_limit;		// limit of retry_count
	guint		retry_count;		// count of UG_MESSAGE_INFO_RETRY

	guint		max_connections;	// max connections per server
	gint64		max_upload_speed;	// bytes per seconds
	gint64		max_download_speed;	// bytes per seconds
	// Retrieve timestamp of the remote file if it is available.
	gboolean	retrieve_timestamp;

	guint		debug_level;

//	gint64		resume_offset;

	struct UgCommonKeeping
	{
		gboolean	name:1;
		gboolean	url:1;
		gboolean	mirrors:1;
		gboolean	file:1;
		gboolean	folder:1;
		gboolean	user:1;
		gboolean	password:1;
		gboolean	timestamp:1;
		gboolean	connect_timeout:1;
		gboolean	transmit_timeout:1;
		gboolean	retry_delay:1;
		gboolean	retry_limit:1;

		gboolean	max_connections:1;
		gboolean	max_upload_speed:1;
		gboolean	max_download_speed:1;
		gboolean    retrieve_timestamp:1;

		gboolean	debug_level:1;
	} keeping;
};


// ---------------------------------------------------------------------------
// UgProxy

//  UgData
//  |
//  `- UgDatalist
//     |
//     `- UgProxy

enum UgProxyType
{
	UG_PROXY_NONE,
	UG_PROXY_DEFAULT,			// Decided by plug-ins
	UG_PROXY_HTTP,
	UG_PROXY_SOCKS4,
	UG_PROXY_SOCKS5,
#ifdef HAVE_LIBPWMD
	UG_PROXY_PWMD,
#endif

	UG_PROXY_N_TYPE,
};

struct UgProxy
{
	UG_DATALIST_MEMBERS (UgProxy);
//	const UgData1Interface*	iface;
//	UgProxy*			next;
//	UgProxy*			prev;

	gchar*				host;
	guint				port;
	UgProxyType			type;

	gchar*				user;
	gchar*				password;

	struct UgProxyKeeping
	{
		gboolean	host:1;
		gboolean	port:1;
		gboolean	type:1;

		gboolean	user:1;
		gboolean	password:1;
	} keeping;

#ifdef HAVE_LIBPWMD
	struct UgProxyPwmd
	{
		gchar*		socket;
		gchar*		socket_args;
		gchar*		file;
		gchar*		element;

		struct UgProxyPwmdKeeping
		{
			gboolean	socket:1;
			gboolean	socket_args:1;
			gboolean	file:1;
			gboolean	element:1;
		} keeping;
	} pwmd;
#endif	// HAVE_LIBPWMD
};


// ---------------------------------------------------------------------------
// UgProgress

//  UgData
//  |
//  `- UgDatalist
//     |
//     `- UgProgress

struct UgProgress
{
	UG_DATALIST_MEMBERS (UgProgress);
//	const UgData1Interface*	iface;
//	UgProgress*				next;
//	UgProgress*				prev;

	gint64		complete;
	gint64		total;
	gdouble		percent;
	gdouble		consume_time;		// Elapsed	(seconds)
	gdouble		remain_time;		// Left		(seconds)

	gint64		download_speed;
	gint64		upload_speed;		// torrent
	gint64		uploaded;			// torrent
	gdouble		ratio;				// torrent

//	guint		n_segments;			// 1 segment = 1 offset + 1 length
//	gint64*		segments;			// offset1, length1, offset2, length2
};


// ---------------------------------------------------------------------------
// UgHttp

//  UgData
//  |
//  `- UgDatalist
//     |
//     `- UgHttp

struct UgHttp
{
	UG_DATALIST_MEMBERS (UgHttp);
//	const UgData1Interface*	iface;
//	UgHttp*					next;
//	UgHttp*					prev;

	gchar*		user;
	gchar*		password;
	gchar*		referrer;
	gchar*		user_agent;

	gchar*		post_data;
	gchar*		post_file;
	gchar*		cookie_data;
	gchar*		cookie_file;

	guint		redirection_limit;	// limit of redirection_count
	guint		redirection_count;	// count of UG_MESSAGE_DATA_HTTP_LOCATION

	struct UgHttpKeeping
	{
		gboolean	user:1;
		gboolean	password:1;
		gboolean	referrer:1;
		gboolean	user_agent:1;

		gboolean	post_data:1;
		gboolean	post_file:1;
		gboolean	cookie_data:1;
		gboolean	cookie_file:1;
		gboolean	redirection_limit:1;
	} keeping;
};


// ---------------------------------------------------------------------------
// UgFtp

//  UgData
//  |
//  `- UgDatalist
//     |
//     `- UgFtp

struct UgFtp
{
	UG_DATALIST_MEMBERS (UgFtp);
//	const UgData1Interface*	iface;
//	UgFtp*				next;
//	UgFtp*				prev;

	gchar*		user;
	gchar*		password;

	gboolean	active_mode;

	struct UgFtpKeeping
	{
		gboolean	user:1;
		gboolean	password:1;
		gboolean	active_mode:1;
	} keeping;
};


// ---------------------------------------------------------------------------
// UgLog

//  UgData
//  |
//  `- UgDatalist
//     |
//     `- UgLog

struct UgLog
{
	UG_DATALIST_MEMBERS (UgLog);
//	const UgData1Interface*	iface;
//	UgLog*				next;
//	UgLog*				prev;

	time_t		added_on;
	time_t		completed_on;
};


#ifdef __cplusplus
}
#endif

#endif  // UG_DATA_DOWNLOAD_H

