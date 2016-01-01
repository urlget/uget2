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

#include <stdlib.h>
#include <string.h>
#include <memory.h>
// uglib
#include <UgString.h>
#include "UgRegistry1.h"
#include "UgData-download.h"


// ----------------------------------------------------------------------------
// UgCommon

static void ug_common_init   (UgCommon* common);
static void ug_common_final  (UgCommon* common);

static const UgDataEntry	ug_common_entry[] =
{
	{"name",				G_STRUCT_OFFSET (UgCommon, name),		UG_TYPE_STRING,	NULL,	NULL},
	{"url",					G_STRUCT_OFFSET (UgCommon, url),		UG_TYPE_STRING,	NULL,	NULL},
	{"mirrors",				G_STRUCT_OFFSET (UgCommon, mirrors),	UG_TYPE_STRING,	NULL,	NULL},
	{"file",				G_STRUCT_OFFSET (UgCommon, file),		UG_TYPE_STRING,	NULL,	NULL},
	{"folder",				G_STRUCT_OFFSET (UgCommon, folder),		UG_TYPE_STRING,	NULL,	NULL},
	{"user",				G_STRUCT_OFFSET (UgCommon, user),		UG_TYPE_STRING,	NULL,	NULL},
	{"password",			G_STRUCT_OFFSET (UgCommon, password),	UG_TYPE_STRING,	NULL,	NULL},
	{"ConnectTimeout",		G_STRUCT_OFFSET (UgCommon, connect_timeout),	UG_TYPE_UINT,	NULL,	NULL},
	{"TransmitTimeout",		G_STRUCT_OFFSET (UgCommon, transmit_timeout),	UG_TYPE_UINT,	NULL,	NULL},
	{"RetryDelay",			G_STRUCT_OFFSET (UgCommon, retry_delay),		UG_TYPE_UINT,	NULL,	NULL},
	{"RetryLimit",			G_STRUCT_OFFSET (UgCommon, retry_limit),		UG_TYPE_UINT,	NULL,	NULL},
	{"MaxConnections",		G_STRUCT_OFFSET (UgCommon, max_connections),	UG_TYPE_UINT,	NULL,	NULL},
	{"MaxUploadSpeed",		G_STRUCT_OFFSET (UgCommon, max_upload_speed),	UG_TYPE_INT64,	NULL,	NULL},
	{"MaxDownloadSpeed",	G_STRUCT_OFFSET (UgCommon, max_download_speed),	UG_TYPE_INT64,	NULL,	NULL},
	{"RetrieveTimestamp",	G_STRUCT_OFFSET (UgCommon, retrieve_timestamp),	UG_TYPE_INT,	NULL,	NULL},
	{NULL}		// null-terminated
};
// extern
const UgData1Interface	ug_common_iface =
{
	sizeof (UgCommon),	// instance_size
	"common",			// name
	ug_common_entry,	// entry

	(UgInitFunc)     ug_common_init,
	(UgFinalizeFunc) ug_common_final,
	(UgAssignFunc)   NULL,
};


static void ug_common_init (UgCommon* common)
{
	common->retrieve_timestamp = TRUE;
	common->connect_timeout  = 30;
	common->transmit_timeout = 30;
	common->retry_delay = 6;
	common->retry_limit = 99;
	common->max_connections = 1;
}

static void ug_common_final (UgCommon* common)
{
	g_free (common->name);
	g_free (common->url);
	g_free (common->mirrors);
	g_free (common->file);
	g_free (common->folder);
	g_free (common->user);
	g_free (common->password);
}


// ----------------------------------------------------------------------------
// UgProxy

static void	ug_proxy_final  (UgProxy* proxy);

static const UgDataEntry	ug_proxy_entry[] =
{
	{"host",		G_STRUCT_OFFSET (UgProxy, host),		UG_TYPE_STRING,	NULL,	NULL},
	{"port",		G_STRUCT_OFFSET (UgProxy, port),		UG_TYPE_UINT,	NULL,	NULL},
	{"type",		G_STRUCT_OFFSET (UgProxy, type),		UG_TYPE_UINT,	NULL,	NULL},
	{"user",		G_STRUCT_OFFSET (UgProxy, user),		UG_TYPE_STRING,	NULL,	NULL},
	{"password",	G_STRUCT_OFFSET (UgProxy, password),	UG_TYPE_STRING,	NULL,	NULL},
#ifdef HAVE_LIBPWMD
	{"pwmd-socket",	G_STRUCT_OFFSET (UgProxy, pwmd.socket),	UG_TYPE_STRING,	NULL,	NULL},
	{"pwmd-socket-args",	G_STRUCT_OFFSET (UgProxy, pwmd.socket_args),	UG_TYPE_STRING,	NULL,	NULL},
	{"pwmd-file",	G_STRUCT_OFFSET (UgProxy, pwmd.file),	UG_TYPE_STRING,	NULL,	NULL},
	{"pwmd-element",G_STRUCT_OFFSET (UgProxy, pwmd.element),UG_TYPE_STRING,	NULL,	NULL},
#endif
	{NULL},		// null-terminated
};
// extern
const UgData1Interface	ug_proxy_iface =
{
	sizeof (UgProxy),	// instance_size
	"proxy",			// name
	ug_proxy_entry,		// entry

	(UgInitFunc)		NULL,
	(UgFinalizeFunc)	ug_proxy_final,
	(UgAssignFunc)		NULL,
};


static void	ug_proxy_final	(UgProxy* proxy)
{
	g_free (proxy->host);
	g_free (proxy->user);
	g_free (proxy->password);

#ifdef HAVE_LIBPWMD
	g_free(proxy->pwmd.socket);
	g_free(proxy->pwmd.socket_args);
	g_free(proxy->pwmd.file);
	g_free(proxy->pwmd.element);
#endif	// HAVE_LIBPWMD
}


// ----------------------------------------------------------------------------
// UgProgress

static const UgDataEntry	ug_progress_entry[] =
{
	{"complete",	G_STRUCT_OFFSET (UgProgress, complete),		UG_TYPE_INT64,	NULL,	NULL},
	{"total",		G_STRUCT_OFFSET (UgProgress, total),		UG_TYPE_INT64,	NULL,	NULL},
	{"percent",		G_STRUCT_OFFSET (UgProgress, percent),		UG_TYPE_DOUBLE,	NULL,	NULL},
	{"elapsed",		G_STRUCT_OFFSET (UgProgress, consume_time),	UG_TYPE_DOUBLE,	NULL,	NULL},
	{"uploaded",	G_STRUCT_OFFSET (UgProgress, uploaded),		UG_TYPE_INT64,	NULL,	NULL},
	{"ratio",		G_STRUCT_OFFSET (UgProgress, ratio),		UG_TYPE_DOUBLE,	NULL,	NULL},
	{NULL},		// null-terminated
};
// extern
const UgData1Interface	ug_progress_iface =
{
	sizeof (UgProgress),	// instance_size
	"progress",				// name
	ug_progress_entry,		// entry

	(UgInitFunc)		NULL,
	(UgFinalizeFunc)	NULL,
	(UgAssignFunc)		NULL,
};


// ---------------------------------------------------------------------------
// UgHttp

static void	ug_http_init   (UgHttp* http);
static void	ug_http_final  (UgHttp* http);

static const UgDataEntry	ug_http_entry[] =
{
	{"user",				G_STRUCT_OFFSET (UgHttp, user),				UG_TYPE_STRING,	NULL,	NULL},
	{"password",			G_STRUCT_OFFSET (UgHttp, password),			UG_TYPE_STRING,	NULL,	NULL},
	{"referrer",			G_STRUCT_OFFSET (UgHttp, referrer),			UG_TYPE_STRING,	NULL,	NULL},
	{"UserAgent",			G_STRUCT_OFFSET (UgHttp, user_agent),			UG_TYPE_STRING,	NULL,	NULL},
	{"PostData",			G_STRUCT_OFFSET (UgHttp, post_data),			UG_TYPE_STRING,	NULL,	NULL},
	{"PostFile",			G_STRUCT_OFFSET (UgHttp, post_file),			UG_TYPE_STRING,	NULL,	NULL},
	{"CookieData",			G_STRUCT_OFFSET (UgHttp, cookie_data),		UG_TYPE_STRING,	NULL,	NULL},
	{"CookieFile",			G_STRUCT_OFFSET (UgHttp, cookie_file),		UG_TYPE_STRING,	NULL,	NULL},
	{"RedirectionLimit",	G_STRUCT_OFFSET (UgHttp, redirection_limit),	UG_TYPE_UINT,	NULL,	NULL},
	{"RedirectionCount",	G_STRUCT_OFFSET (UgHttp, redirection_count),	UG_TYPE_UINT,	NULL,	NULL},
	{NULL},		// null-terminated
};
// extern
const UgData1Interface	ug_http_iface =
{
	sizeof (UgHttp),	// instance_size
	"http",				// name
	ug_http_entry,		// entry

	(UgInitFunc)		ug_http_init,
	(UgFinalizeFunc)	ug_http_final,
	(UgAssignFunc)		NULL,
};


static void	ug_http_init (UgHttp* http)
{
	http->redirection_limit = 30;
}

static void	ug_http_final (UgHttp* http)
{
	g_free (http->user);
	g_free (http->password);
	g_free (http->referrer);
	g_free (http->user_agent);
	g_free (http->post_data);
	g_free (http->post_file);
	g_free (http->cookie_data);
	g_free (http->cookie_file);
}


// ---------------------------------------------------------------------------
// UgFtp
//
static void	ug_ftp_final  (UgFtp* ftp);

static const UgDataEntry  ug_ftp_entry[] =
{
	{"user",		G_STRUCT_OFFSET (UgFtp, user),		UG_TYPE_STRING,	NULL,	NULL},
	{"password",	G_STRUCT_OFFSET (UgFtp, password),	UG_TYPE_STRING,	NULL,	NULL},
	{"ActiveMode",	G_STRUCT_OFFSET (UgFtp, active_mode),	UG_TYPE_INT,NULL,	NULL},
	{NULL},		// null-terminated
};
// extern
const UgData1Interface  ug_ftp_iface =
{
	sizeof (UgFtp),	// instance_size
	"ftp",			// name
	ug_ftp_entry,	// entry

	(UgInitFunc)		NULL,
	(UgFinalizeFunc)	ug_ftp_final,
	(UgAssignFunc)		NULL,
};


static void	ug_ftp_final	(UgFtp* ftp)
{
	g_free (ftp->user);
	g_free (ftp->password);
}


// ---------------------------------------------------------------------------
// UgLog
//
static const UgDataEntry  ug_log_entry[] =
{
	{NULL},		// null-terminated
};
// extern
const UgData1Interface  ug_log_iface =
{
	sizeof (UgLog),	// instance_size
	"log",			// name
	ug_log_entry,	// entry

	(UgInitFunc)		NULL,
	(UgFinalizeFunc)	NULL,
	(UgAssignFunc)		NULL,
};


