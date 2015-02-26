/*
 *
 *   Copyright (C) 2005-2015 by C.H. Huang
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

#include <stdlib.h>
#include <UgUtil.h>
#include <UgStdio.h>
#include <UgString.h>
#include <UgSocket.h>
#include <UgetIpc.h>
#include <UgetData.h>

#define UGET_IPC_PORT      "14777"
#define UGET_IPC_ADDR      "127.0.0.1"
#define UGET_IPC_NAME      "uGetIPC-"
#define UGET_IPC_NAME_ABS  "com.ugetdm.uget"
#define UGET_IPC_LIMIT     50

#define UGET_IPC_ERROR     'E'
#define UGET_IPC_OK        'O'
#define UGET_IPC_BUSY      'B'
#define UGET_IPC_PING      'P'
#define UGET_IPC_SEND      'S'

// ----------------------------------------------------------------------------
#if !(defined _WIN32 || defined _WIN64)

#define USE_ABSTRACT_SOCKET

#ifndef __ANDROID__
#include <unistd.h>    // uid_t and others
#include <pwd.h>

const char* get_user_name (void)
{
	uid_t  uid = geteuid ();
	struct passwd *pw = getpwuid (uid);

	if (pw)
		return pw->pw_name;
	return "";
}

#if !(defined __linux__ && defined USE_ABSTRACT_SOCKET)
const char* get_tmp_dir (void)
{
	static char*  tmp_dir = NULL;

	if (tmp_dir)
		return tmp_dir;

	tmp_dir = ug_strdup (getenv ("TMPDIR"));
	if (tmp_dir == NULL || *tmp_dir == '\0') {
		ug_free (tmp_dir);
		tmp_dir = ug_strdup (getenv ("TMP"));
	}
	if (tmp_dir == NULL || *tmp_dir == '\0') {
		ug_free (tmp_dir);
		tmp_dir = ug_strdup (getenv ("TEMP"));
	}
	if (tmp_dir == NULL || *tmp_dir == '\0')
		tmp_dir = "/tmp";

	return tmp_dir;
}
#endif // ! (__linux__)
#endif // ! (__ANDROID__)

static const char* get_unix_socket_path (int* result)
{
	static char* path = NULL;
	static int   path_len = -1;

	if (path == NULL) {
#if defined __ANDROID__
		path = "\0" UGET_IPC_NAME_ABS;
		path_len = sizeof (UGET_IPC_NAME_ABS);
#elif defined __linux__ && defined USE_ABSTRACT_SOCKET
		const char* user_name = get_user_name ();

		// abstract socket names (begin with 0) are not null terminated
		path = ug_malloc (3 + sizeof (UGET_IPC_NAME_ABS) + strlen (user_name));
		path[0] = 0;
		strcat (path, "@");  // this will be replaced by 0
		strcat (path, UGET_IPC_NAME_ABS);
		strcat (path, "-");
		strcat (path, user_name);
		path_len = strlen (path);
		path[0] = 0;         // begin with 0
#else
		const char* user_name = get_user_name ();
		const char* tmp_dir   = get_tmp_dir ();

		path = ug_malloc (strlen (tmp_dir) + sizeof (UGET_IPC_NAME) + strlen (user_name) + 2);
		path[0] = 0;
		strcat (path, tmp_dir);
		strcat (path, "/");
		strcat (path, UGET_IPC_NAME);
		strcat (path, user_name);
#endif
	}

	if (result)
		result[0] = path_len;
	return path;
}

#endif // !(_WIN32 || _WIN64)

// ----------------------------------------------------------------------------
// UgLinkStr

typedef struct UgLinkStr    UgLinkStr;

struct UgLinkStr
{
	UG_LINK_MEMBERS (UgLinkStr, UgLinkStr, self);
//	UgLinkStr*  self;
//	UgLinkStr*  next;
//	UgLinkStr*  prev;

	char  string[1];
};

UgLinkStr* ug_link_str_new (const char* string, int length)
{
	UgLinkStr*  link;

	if (length == -1)
		length = strlen (string);
	link = (UgLinkStr*) ug_malloc (sizeof (UgLinkStr) + length);
	link->self = link;
	link->next = NULL;
	link->prev = NULL;
	link->string[length] = 0;
	strncpy (link->string, string, length);
	return link;
}

#define ug_link_str_free    ug_free

// ----------------------------------------------------------------------------

static int  buffer_more (UgBuffer* buffer)
{
	int  fd = (intptr_t) buffer->data;
	int  result;

	if (buffer->end - buffer->beg > 65535)
		return -1;
	if (buffer->data == (void*) -1)
		return 0;

	if (buffer->cur == buffer->end)
		ug_buffer_expand (buffer);
	result = recv (fd, buffer->cur, buffer->end - buffer->cur, 0);
	if (result > 0) {
		buffer->cur += result;
		if (buffer->cur != buffer->end)
			buffer->data = (void*) -1;
	}
	else
		buffer->data = (void*) -1;
	return result;
}

static int  buffer_get_line (UgBuffer* buffer, char** beg, char** end)
{
	char* cur;
	int   length;

	for (cur = beg[0];  ;  ) {
		for (length = 0;  cur < buffer->cur;  cur++, length++) {
			if (cur[0] == '\x00')
				goto exit;
		}
		if (cur == buffer->cur) {
			if (beg[0] != buffer->beg && beg[0] != cur) {
				memmove (buffer->beg, beg[0], cur - beg[0]);
				cur = buffer->beg + (cur - beg[0]);
				beg[0] = buffer->beg;
				buffer->cur = cur;
			}
			if (buffer_more (buffer) > 0)
				continue;
		}
		break;
	}

exit:
	if (**beg == '\r' || **beg == '\n')
		*beg += 1;
	if (end)
		end[0] = cur;
	return length;
}

static void on_receiving (UgSocketServer* server, SOCKET client_fd, void* data)
{
	UgetIpc*  ipc = (UgetIpc*) data;
	UgetArgs* args;
	char*     beg;
	char*     end;
	int       length;

	ug_socket_server_ref (server);

	ipc->buffer.cur  = ipc->buffer.beg;
	ipc->buffer.more = buffer_more;
	ipc->buffer.data = (void*)(intptr_t) client_fd;

	beg = ipc->buffer.beg;
	length = buffer_get_line (&ipc->buffer, &beg, &end);
	if (length !=3 || beg[0] != 'U' || beg[1] != 'G')
		goto exit;

	switch (beg[2]) {
	case UGET_IPC_SEND:
		beg = end + 1;
		length = buffer_get_line (&ipc->buffer, &beg, &end);
		if (length == 0)
			goto exit;
		// get number of arguments and alloc memory for it
		length = strtol (beg, NULL, 10);
		args = uget_args_new (length, NULL);
		// to next line
		beg = end + 1;
		while ((length = buffer_get_line (&ipc->buffer, &beg, &end)) > 0) {
			uget_args_add (args, beg, length);
			beg = end + 1;
		}
		ug_mutex_lock (&ipc->queue_mutex);
		ug_list_append (&ipc->queue, (UgLink*) args);
		ug_mutex_unlock (&ipc->queue_mutex);
//		break;

	case UGET_IPC_PING:
		ipc->buffer.beg[0] = 'U';
		ipc->buffer.beg[1] = 'G';
		ipc->buffer.beg[2] = UGET_IPC_OK;
		ipc->buffer.beg[3] = '\x00';
		send (client_fd, ipc->buffer.beg, 4, 0);
		break;
	}

exit:
	shutdown (client_fd, 0);
	closesocket (client_fd);
	ug_socket_server_unref (server);
}

UgetIpc*  uget_ipc_new (void)
{
	UgetIpc* ipc;

#if defined _WIN32 || defined _WIN64
	WSADATA  WSAData;
	WSAStartup (MAKEWORD (2, 2), &WSAData);
#endif

	ipc = (UgetIpc*) ug_malloc (sizeof (UgetIpc));
	ipc->server = NULL;
	ug_list_init (&ipc->queue);
	ug_mutex_init (&ipc->queue_mutex);
	ug_buffer_init (&ipc->buffer, 4096);
	ug_option_init (&ipc->option);
	ug_option_set_parser (&ipc->option, ug_option_parse_entry,
	                      &ipc->value, uget_option_entry);
	memset (&ipc->value, 0, sizeof (ipc->value));
	ipc->option.discard_unaccepted = TRUE;
	return ipc;
}

static void uget_ipc_on_destroy (void* data)
{
	UgetIpc* ipc = (UgetIpc*) data;

#if defined _WIN32 || defined _WIN64
	WSACleanup ();
#endif

	ug_list_foreach (&ipc->queue, (UgForeachFunc) uget_args_free, NULL);
	ug_list_clear (&ipc->queue, FALSE);
	ug_mutex_clear (&ipc->queue_mutex);
	ug_option_final (&ipc->option);
#if !(defined _WIN32 || defined _WIN64 || defined __linux__ || defined USE_ABSTRACT_SOCKET)
	ug_unlink (get_unix_socket_path (NULL));
#endif
	ipc->server = NULL;
	ug_free (ipc);
}

void  uget_ipc_free (UgetIpc* ipc)
{
	if (ipc->server == NULL)
		uget_ipc_on_destroy (ipc);
	else {
		if (ipc->server->stopped)
			uget_ipc_on_destroy (ipc);
		else {
			ipc->server->destroy.func = uget_ipc_on_destroy;
			ipc->server->destroy.data = ipc;
		}
		ug_socket_server_stop (ipc->server);
		ug_socket_server_unref (ipc->server);
	}
}

int   uget_ipc_server_has_data (UgetIpc* ipc)
{
	if (ipc->queue.head)
		return TRUE;
	else
		return FALSE;
}

int   uget_ipc_server_get (UgetIpc* ipc, UgList* uris, UgInfo* info)
{
	UgetArgs* args;
	UgLink*   link;
	int       index;

	ug_mutex_lock (&ipc->queue_mutex);
	args = (UgetArgs*) ipc->queue.head;
	if (args)
		ug_list_remove (&ipc->queue, (UgLink*) args);
	ug_mutex_unlock (&ipc->queue_mutex);

	if (args == NULL)
		return FALSE;
	if (args->strings.size == 0) {
		uget_args_free (args);
		return FALSE;
	}
	// clear all value
	memset (&ipc->value, 0, sizeof (ipc->value));
	ug_option_clear (&ipc->option);

	// handle arguments
#ifndef NDEBUG
	puts ("*** arguments --- start ---");
#endif
	for (link = args->strings.head;  link;  link = link->next) {
		ug_option_parse (&ipc->option, ((UgLinkStr*)link)->string, -1);
#ifndef NDEBUG
		puts (((UgLinkStr*)link)->string);
#endif
	}
#ifndef NDEBUG
	puts ("*** arguments --- end ---");
#endif
	uget_args_free (args);

	if (ipc->option.others.length > 0 || ipc->value.input_file) {
		uget_option_value_to_info (&ipc->value, info);
		for (index = 0;  index < ipc->option.others.length;  index++) {
			link = ug_link_new ();
			link->data = ipc->option.others.at[index];
			ug_list_append (uris, link);
		}
		ipc->option.others.length = 0;
		return TRUE;
	}
	return TRUE;
}

void  uget_ipc_server_add (UgetIpc* ipc, int argc, char** argv)
{
	UgetArgs* args;

	args = uget_args_new (argc, argv);
	ug_mutex_lock (&ipc->queue_mutex);
	ug_list_append (&ipc->queue, (UgLink*) args);
	ug_mutex_unlock (&ipc->queue_mutex);
}


#if defined _WIN32 || defined _WIN64
int   uget_ipc_server_start (UgetIpc* ipc)
{
	SOCKET  fd;
	int     result;

	// detect server
	fd = socket (AF_INET, SOCK_STREAM, 0);
	result = ug_socket_connect (fd, UGET_IPC_ADDR, UGET_IPC_PORT);
	closesocket (fd);
	if (result != -1) {
		return FALSE;
	}
	// create server socket
	fd = socket (AF_INET, SOCK_STREAM, 0);
	if (ug_socket_listen (fd, UGET_IPC_ADDR, UGET_IPC_PORT, 5) == -1) {
		closesocket (fd);
		return FALSE;
	}
	// start server thread
	ipc->server = ug_socket_server_new (fd);
	ug_socket_server_set_receiver (ipc->server, on_receiving, ipc);
	ug_socket_server_start (ipc->server);
	return TRUE;
}
#else
int   uget_ipc_server_start (UgetIpc* ipc)
{
	SOCKET  fd;
	int     result;
	int     path_len;
	const char* path;

	// detect server
	fd = socket (AF_UNIX, SOCK_STREAM, 0);
	path = get_unix_socket_path (&path_len);
	result = ug_socket_connect_unix (fd, path, path_len);
	closesocket (fd);
	if (result != -1) {
		return FALSE;
	}
	// delete UNIX socket file if path isn't abstract socket names
	if (path[0])
		ug_unlink (path);
	// create server socket
	fd = socket (AF_UNIX, SOCK_STREAM, 0);
	if (ug_socket_listen_unix (fd, path, path_len, 5) == -1) {
		closesocket (fd);
		return FALSE;
	}
	// start server thread
	ipc->server = ug_socket_server_new (fd);
	ug_socket_server_set_receiver (ipc->server, on_receiving, ipc);
	ug_socket_server_start (ipc->server);
	return TRUE;
}
#endif // _WIN32 || _WIN64

int   uget_ipc_client_send (UgetIpc* ipc, int argc, char** argv)
{
	SOCKET  fd;
	int     n;
#if !(defined _WIN32 || defined _WIN64)
	int     path_len;
	const char* path;

	fd = socket (AF_UNIX, SOCK_STREAM, 0);
	path = get_unix_socket_path (&path_len);
	if (ug_socket_connect_unix (fd, path, path_len) == -1) {
		closesocket (fd);
		return FALSE;
	}
#else
	fd = socket (AF_INET, SOCK_STREAM, 0);
	if (ug_socket_connect (fd, UGET_IPC_ADDR, UGET_IPC_PORT) == -1) {
		closesocket (fd);
		return FALSE;
	}
#endif

	// UGET_IPC_SEND
	ug_buffer_write (&ipc->buffer, "UGS", -1);
	ug_buffer_write_char (&ipc->buffer, '\x00');
	n = snprintf (ipc->buffer.cur, ug_buffer_remain (&ipc->buffer),
	              "%d%c", argc, '\x00');
	ipc->buffer.cur += n;
	for (n = 0;  n < argc;  n++) {
		ug_buffer_write (&ipc->buffer, argv[n], -1);
		ug_buffer_write_char (&ipc->buffer, '\x00');
	}
	if (send (fd, ipc->buffer.beg, ug_buffer_length (&ipc->buffer), 0) > 0)
		recv (fd, ipc->buffer.beg, 4, 0);
	closesocket (fd);

	if (ipc->buffer.beg[2] != UGET_IPC_OK)
		return FALSE;
	return TRUE;
}

// ----------------------------------------------------------------------------

UgetArgs*  uget_args_new (int argc, char** argv)
{
	UgetArgs* args;
	int       index;

	args = ug_malloc (sizeof (UgetArgs));
	args->self = args;
	args->next = NULL;
	args->prev = NULL;
	ug_list_init (&args->strings);
	if (argv == NULL)
		return args;

	for (index = 0;  index < argc;  index++)
		uget_args_add (args, argv[index], -1);
	return args;
}

void  uget_args_free (UgetArgs* args)
{
	ug_list_foreach (&args->strings, (UgForeachFunc) ug_link_str_free, NULL);
	ug_list_clear (&args->strings, FALSE);
	ug_free (args);
}

void uget_args_add (UgetArgs* args, const char* string, int length)
{
	UgLinkStr*  link;

	link = ug_link_str_new (string, length);
	ug_list_append (&args->strings, (UgLink*) link);
}


// ----------------------------------------------------------------------------

// find -?, -h, --help, or --help- in command-line options and return it.
char*	uget_args_find_help (int argc, char** argv)
{
	char*	arg;
	int     arg_len;

	for (argc -= 1;  argc >= 0;  argc--) {
		arg = argv[argc];
		arg_len = strlen (arg);
#ifdef _WIN32
		// Check and remove some character (space,0x20) in tail of argument from command line.
		// This problem only happen in Windows platform.
//		ug_str_clear_tail_charset (arg, arg_len, " \n");
#endif
		// check short_name: -h or -?
		if (arg_len < 2 || arg[0] != '-')
			continue;
		if (arg_len == 2 && (arg[1] == 'h' || arg[1] == '?'))
			return arg;
		// check long name: --help
		if (arg_len < 6 || arg[1] != '-')
			continue;
		if (strncmp (arg+2, "help", 4) == 0) {
			if (arg_len == 6 || arg[6] == '-')
				return arg;
		}
	}
	return NULL;
}

// find -V, --version
char*	uget_args_find_version (int argc, char** argv)
{
	char*	arg;
	int     arg_len;

	for (argc -= 1;  argc >= 0;  argc--) {
		arg = argv[argc];
		arg_len = strlen (arg);
#ifdef _WIN32
		// Check and remove some character (space,0x20) in tail of argument from command line.
		// This problem only happen in Windows platform.
//		ug_str_clear_tail_charset (arg, arg_len, " \n");
#endif
		// check short_name: -V
		if (arg_len < 2 || arg[0] != '-')
			continue;
		if (arg_len == 2 && arg[1] == 'V')
			return arg;
		// check long name: --version
		if (arg_len != 9 || arg[1] != '-')
			continue;
		if (strncmp (arg+2, "version", 7) == 0)
			return arg;
	}
	return NULL;
}

