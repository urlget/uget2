/*
 *
 *   Copyright (C) 2005-2014 by C.H. Huang
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

#ifndef UGET_IPC_H
#define UGET_IPC_H

#include <stdint.h>
#include <UgInfo.h>
#include <UgList.h>
#include <UgOption.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct UgetIpc           UgetIpc;
typedef struct UgetArgs          UgetArgs;
typedef struct UgetOptionValue   UgetOptionValue;
typedef struct UgSocketServer    UgSocketServer;

extern UgOptionEntry  uget_option_entry[];

struct UgetOptionValue
{
	int   version;
	int   offline;

	int   quiet;
	int   category_index;
	char* input_file;

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

int  uget_option_value_to_info (UgetOptionValue* ivalue, UgInfo* info);

// ----------------------------------------------------------------------------

struct UgetIpc
{
	UgSocketServer*  server;

	UgList   queue;
	UgMutex  queue_mutex;
	UgBuffer buffer;

	UgOption         option;
	UgetOptionValue  value;
};

UgetIpc*   uget_ipc_new (void);
void       uget_ipc_free (UgetIpc* ipc);

// server functions
// return TRUE or FALSE
int   uget_ipc_server_start (UgetIpc* ipc);
int   uget_ipc_server_has_data (UgetIpc* ipc);
int   uget_ipc_server_get (UgetIpc* ipc, UgList* uris, UgInfo* info);
void  uget_ipc_server_add (UgetIpc* ipc, int argc, char** argv);

// client functions, ping server or send arguments to server.
// return TRUE or FALSE
int   uget_ipc_client_send (UgetIpc* ipc, int argc, char** argv);

// ----------------------------------------------------------------------------

struct UgetArgs
{
	UG_LINK_MEMBERS (UgetArgs, UgetArgs, self);

	UgList  strings;
};

UgetArgs* uget_args_new (int argc, char** argv);
void      uget_args_free (UgetArgs* args);

void  uget_args_add (UgetArgs* args, const char* string, int length);

char* uget_args_find_version (int argc, char** argv);
char* uget_args_find_help (int argc, char** argv);

#ifdef __cplusplus
}
#endif

#endif	// UGET_IPC_H

