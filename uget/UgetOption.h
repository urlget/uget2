/*
 *
 *   Copyright (C) 2005-2017 by C.H. Huang
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

#ifndef UGET_OPTION_H
#define UGET_OPTION_H

#include <UgInfo.h>
#include <UgList.h>
#include <UgOption.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct UgetOptionValue   UgetOptionValue;

extern UgOptionEntry  uget_option_entry[];

struct UgetOptionValue
{
	int   version;
	int   quiet;
	int   category_index;  // default = -1
	char* input_file;

	struct
	{
		int   offline;     // default = -1
	} ctrl;

	struct
	{
		char* folder;
		char* file;
		char* user;
		char* password;
	} common;

	struct
	{
		int   type;
		char* host;
		int   port;
		char* user;
		char* password;
	} proxy;

	struct
	{
		char* user;
		char* password;
		char* referrer;
		char* user_agent;
		char* cookie_data;
		char* cookie_file;
		char* post_data;
		char* post_file;
	} http;

	struct
	{
		char* user;
		char* password;
	} ftp;
};

void  uget_option_value_init (UgetOptionValue* value);
void  uget_option_value_clear (UgetOptionValue* value);

int   uget_option_value_has_ctrl (UgetOptionValue* value);
int   uget_option_value_to_info (UgetOptionValue* ivalue, UgInfo* info);

#ifdef __cplusplus
}
#endif

#endif	// UGET_OPTION_H

