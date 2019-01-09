/*
 *
 *   Copyright (C) 2005-2019 by C.H. Huang
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

//#include <UgStdio.h>
#include <UgetOption.h>
#include <UgetData.h>


// ----------------------------------------------------------------------------

void  uget_option_value_init (UgetOptionValue* value)
{
	memset (value, 0, sizeof (UgetOptionValue));
	value->category_index = -1;
	value->ctrl.offline = -1;
}

void  uget_option_value_clear (UgetOptionValue* value)
{
	ug_free (value->input_file);

	ug_free (value->common.folder);
	ug_free (value->common.file);
	ug_free (value->common.user);
	ug_free (value->common.password);

	ug_free (value->proxy.host);
	ug_free (value->proxy.user);
	ug_free (value->proxy.password);

	ug_free (value->http.user);
	ug_free (value->http.password);
	ug_free (value->http.referrer);
	ug_free (value->http.user_agent);
	ug_free (value->http.cookie_data);
	ug_free (value->http.cookie_file);
	ug_free (value->http.post_data);
	ug_free (value->http.post_file);

	ug_free (value->ftp.user);
	ug_free (value->ftp.password);

	uget_option_value_init (value);
}

static int  mem_is_zero (char* beg, int len)
{
	char* end;

	for (end = beg + len;  beg < end;  beg++) {
		if (beg[0])
			return FALSE;
	}
	return TRUE;
}

int   uget_option_value_has_ctrl (UgetOptionValue* value)
{
	if (mem_is_zero ((char*) &value->ctrl, sizeof (value->ctrl)) == FALSE)
		return TRUE;
	else
		return FALSE;
}

int  uget_option_value_to_info (UgetOptionValue* ivalue, UgInfo* info)
{
	union {
		UgetCommon*  common;
		UgetProxy*   proxy;
		UgetHttp*    http;
		UgetFtp*     ftp;
	} temp;

	if (mem_is_zero((char*) &ivalue->common, sizeof(ivalue->common)) == FALSE) {
		temp.common = ug_info_realloc(info, UgetCommonInfo);
		temp.common->keeping.enable = TRUE;
		if (ivalue->common.folder) {
			ug_free(temp.common->folder);
			temp.common->folder = ivalue->common.folder;
			temp.common->keeping.folder = TRUE;
			ivalue->common.folder = NULL;
		}
		if (ivalue->common.file) {
			ug_free(temp.common->file);
			temp.common->file = ivalue->common.file;
			temp.common->keeping.file = TRUE;
			ivalue->common.file = NULL;
		}
		if (ivalue->common.user) {
			ug_free(temp.common->user);
			temp.common->user = ivalue->common.user;
			temp.common->keeping.user = TRUE;
			ivalue->common.user = NULL;
		}
		if (ivalue->common.password) {
			ug_free(temp.common->password);
			temp.common->password = ivalue->common.password;
			temp.common->keeping.password = TRUE;
			ivalue->common.password = NULL;
		}
	}

	if (mem_is_zero((char*) &ivalue->proxy, sizeof(ivalue->proxy)) == FALSE) {
		temp.proxy = ug_info_realloc(info, UgetProxyInfo);
		temp.proxy->keeping.enable = TRUE;
		if (ivalue->proxy.type) {
			temp.proxy->type = ivalue->proxy.type;
			temp.proxy->keeping.type = TRUE;
			ivalue->proxy.type = 0;
		}
		if (ivalue->proxy.host) {
			ug_free(temp.proxy->host);
			temp.proxy->host = ivalue->proxy.host;
			temp.proxy->keeping.host = TRUE;
			ivalue->proxy.host = NULL;
		}
		if (ivalue->proxy.port) {
			temp.proxy->port = ivalue->proxy.port;
			temp.proxy->keeping.port = TRUE;
			ivalue->proxy.port = 0;
		}
		if (ivalue->proxy.user) {
			ug_free(temp.proxy->user);
			temp.proxy->user = ivalue->proxy.user;
			temp.proxy->keeping.user = TRUE;
			ivalue->proxy.user = NULL;
		}
		if (ivalue->proxy.password) {
			ug_free(temp.proxy->password);
			temp.proxy->password = ivalue->proxy.password;
			temp.proxy->keeping.password = TRUE;
			ivalue->proxy.password = NULL;
		}
	}

	if (mem_is_zero((char*) &ivalue->http, sizeof(ivalue->http)) == FALSE) {
		temp.http = ug_info_realloc(info, UgetHttpInfo);
		temp.http->keeping.enable = TRUE;
		if (ivalue->http.user) {
			ug_free(temp.http->user);
			temp.http->user = ivalue->http.user;
			temp.http->keeping.user = TRUE;
			ivalue->http.user = NULL;
		}
		if (ivalue->http.password) {
			ug_free(temp.http->password);
			temp.http->password = ivalue->http.password;
			temp.http->keeping.password = TRUE;
			ivalue->http.password = NULL;
		}
		if (ivalue->http.referrer) {
			ug_free(temp.http->referrer);
			temp.http->referrer = ivalue->http.referrer;
			temp.http->keeping.referrer = TRUE;
			ivalue->http.referrer = NULL;
		}
		if (ivalue->http.user_agent) {
			ug_free(temp.http->user_agent);
			temp.http->user_agent = ivalue->http.user_agent;
			temp.http->keeping.user_agent = TRUE;
			ivalue->http.user_agent = NULL;
		}
		if (ivalue->http.cookie_data) {
			ug_free(temp.http->cookie_data);
			temp.http->cookie_data = ivalue->http.cookie_data;
			temp.http->keeping.cookie_data = TRUE;
			ivalue->http.cookie_data = NULL;
		}
		if (ivalue->http.cookie_file) {
			ug_free(temp.http->cookie_file);
			temp.http->cookie_file = ivalue->http.cookie_file;
			temp.http->keeping.cookie_file = TRUE;
			ivalue->http.cookie_file = NULL;
		}
		if (ivalue->http.post_data) {
			ug_free(temp.http->post_data);
			temp.http->post_data = ivalue->http.post_data;
			temp.http->keeping.post_data = TRUE;
			ivalue->http.post_data = NULL;
		}
		if (ivalue->http.post_file) {
			ug_free(temp.http->post_file);
			temp.http->post_file = ivalue->http.post_file;
			temp.http->keeping.post_file = TRUE;
			ivalue->http.post_file = NULL;
		}
	}

	if (mem_is_zero((char*) &ivalue->ftp, sizeof(ivalue->ftp)) == FALSE) {
		temp.ftp = ug_info_realloc(info, UgetFtpInfo);
		temp.ftp->keeping.enable = TRUE;
		if (ivalue->ftp.user) {
			ug_free(temp.ftp->user);
			temp.ftp->user = ivalue->ftp.user;
			temp.ftp->keeping.user = TRUE;
			ivalue->ftp.user = NULL;
		}
		if (ivalue->ftp.password) {
			ug_free(temp.ftp->password);
			temp.ftp->password = ivalue->ftp.password;
			temp.ftp->keeping.password = TRUE;
			ivalue->ftp.password = NULL;
		}
	}

	return TRUE;
}

UgOptionEntry  uget_option_entry[] =
{
	{"help",            "?", offsetof (UgetOptionValue, version), UG_ENTRY_BOOL,
		"Show help options", NULL, NULL},
	{"version",         "V", offsetof (UgetOptionValue, version), UG_ENTRY_BOOL,
		"display the version of uGet and exit.", NULL, NULL},

	{"quiet",          NULL, offsetof (UgetOptionValue, quiet),   UG_ENTRY_BOOL,
		"add download directly. Don't show dialog.", NULL, NULL},
	{"category-index", NULL, offsetof (UgetOptionValue, category_index), UG_ENTRY_INT,
		"add download to Nth category.", "N", NULL},
	{"input-file",      "i", offsetof (UgetOptionValue, input_file), UG_ENTRY_STRING,
		"add URLs found in FILE.", "FILE", NULL},

	{"set-offline",    NULL, offsetof (UgetOptionValue, ctrl.offline), UG_ENTRY_INT,
		"set offline mode to N. (0=Disable)", "N", NULL},

	{"folder",         NULL, offsetof (UgetOptionValue, common.folder), UG_ENTRY_STRING,
		"placed download file in FOLDER.", "FOLDER", NULL},
	{"filename",       NULL, offsetof (UgetOptionValue, common.file), UG_ENTRY_STRING,
		"set download filename to FILE.",  "FILE", NULL},

	{"user",           NULL, offsetof (UgetOptionValue, common.user), UG_ENTRY_STRING,
		"set both ftp and http user to USER.", "USER", NULL},
	{"password",       NULL, offsetof (UgetOptionValue, common.password), UG_ENTRY_STRING,
		"set both ftp and http password to PASS.", "PASS", NULL},

	{"proxy-type",     NULL, offsetof (UgetOptionValue, proxy.type), UG_ENTRY_INT,
		"set proxy type to N. (0=Don't use)", "N", NULL},
	{"proxy-host",     NULL, offsetof (UgetOptionValue, proxy.host), UG_ENTRY_STRING,
		"set proxy host to HOST.", "HOST", NULL},
	{"proxy-port",     NULL, offsetof (UgetOptionValue, proxy.port), UG_ENTRY_INT,
		"set proxy port to PORT.", "PORT", NULL},
	{"proxy-user",     NULL, offsetof (UgetOptionValue, proxy.user), UG_ENTRY_STRING,
		"set USER as proxy username.", "USER", NULL},
	{"proxy-password", NULL, offsetof (UgetOptionValue, proxy.password), UG_ENTRY_STRING,
		"set PASS as proxy password.", "PASS", NULL},

	{"http-user",      NULL, offsetof (UgetOptionValue, http.user), UG_ENTRY_STRING,
		"set http user to USER.", "USER", NULL},
	{"http-password",  NULL, offsetof (UgetOptionValue, http.password),	UG_ENTRY_STRING,
		"set http password to PASS.", "PASS", NULL},
	{"http-referer",   NULL, offsetof (UgetOptionValue, http.referrer), UG_ENTRY_STRING,
		"include `Referer: URL' header in HTTP request.", "URL", NULL},
	{"http-user-agent", NULL,offsetof (UgetOptionValue, http.user_agent), UG_ENTRY_STRING,
		"identify as AGENT instead of default.", "AGENT", NULL},
	{"http-cookie-data",NULL,offsetof (UgetOptionValue, http.cookie_data), UG_ENTRY_STRING,
		"load cookies from STRING.", "STRING", NULL},
	{"http-cookie-file",NULL, offsetof (UgetOptionValue, http.cookie_file),	UG_ENTRY_STRING,
		"load cookies from FILE.", "FILE", NULL},
	{"http-post-data",  NULL, offsetof (UgetOptionValue, http.post_data), UG_ENTRY_STRING,
		"use the POST method; send STRING as the data.", "STRING", NULL},
	{"http-post-file",  NULL, offsetof (UgetOptionValue, http.post_file), UG_ENTRY_STRING,
		"use the POST method; send contents of FILE", "FILE", NULL},

	{"ftp-user",        NULL, offsetof (UgetOptionValue, ftp.user),	UG_ENTRY_STRING,
		"set ftp user to USER.", "USER", NULL},
	{"ftp-password",    NULL, offsetof (UgetOptionValue, ftp.password),	UG_ENTRY_STRING,
		"set ftp password to PASS.", "PASS", NULL},
	{NULL}
};
