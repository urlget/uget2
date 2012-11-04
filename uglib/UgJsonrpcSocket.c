/*
 *
 *   Copyright (C) 2012-2014 by C.H. Huang
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
#include <UgDefine.h>
#include <UgThread.h>
#include <UgStdio.h>
#include <UgSocket.h>
#include <UgJsonrpcSocket.h>

#if defined _WIN32 || defined _WIN64
static int  global_ref_count = 0;
#endif

void  ug_jsonrpc_socket_init (UgJsonrpcSocket* jrsock)
{
#if defined _WIN32 || defined _WIN64
	WSADATA  WSAData;

	if (global_ref_count == 0)
		WSAStartup (MAKEWORD (2, 2), &WSAData);
	global_ref_count++;
#endif // _WIN32 || _WIN64

	ug_buffer_init (&jrsock->buffer, 4096);
	ug_json_init (&jrsock->json);
	ug_jsonrpc_init (&jrsock->rpc, &jrsock->json, &jrsock->buffer);
	jrsock->socket = INVALID_SOCKET;

	jrsock->rpc.send.func = (UgJsonrpcFunc) ug_jsonrpc_socket_send;
	jrsock->rpc.send.data = jrsock;
	jrsock->rpc.receive.func = (UgJsonrpcFunc) ug_jsonrpc_socket_receive;
	jrsock->rpc.receive.data = jrsock;
}

void  ug_jsonrpc_socket_final (UgJsonrpcSocket* jrsock)
{
	closesocket (jrsock->socket);
	jrsock->socket = INVALID_SOCKET;

	ug_json_final (&jrsock->json);
	ug_jsonrpc_clear (&jrsock->rpc);
	ug_buffer_clear (&jrsock->buffer, TRUE);

#if defined _WIN32 || defined _WIN64
	global_ref_count--;
	if (global_ref_count == 0)
		WSACleanup ();
#endif
}

void  ug_jsonrpc_socket_close (UgJsonrpcSocket* jrsock)
{
	closesocket (jrsock->socket);
	jrsock->socket = INVALID_SOCKET;
}

int   ug_jsonrpc_socket_connect (UgJsonrpcSocket* jrsock,
                                 const char* addr,
                                 const char* port_or_serv)
{
	SOCKET  fd;

	if (jrsock->socket != INVALID_SOCKET)
		return FALSE;

	fd = socket (AF_INET, SOCK_STREAM, 0);
	if (fd == INVALID_SOCKET)
		return FALSE;

	if (ug_socket_connect (fd, addr, port_or_serv) == SOCKET_ERROR) {
		closesocket (fd);
		return FALSE;
	}

	jrsock->socket = fd;
	return TRUE;
}

#if !(defined _WIN32 || defined _WIN64)
int   ug_jsonrpc_socket_connect_unix (UgJsonrpcSocket* jrsock, const char* path, int path_len)
{
	int    fd;

	if (jrsock->socket != INVALID_SOCKET)
		return FALSE;

	fd = socket (AF_UNIX, SOCK_STREAM, 0);
	if (fd == -1)
		return FALSE;

	if (ug_socket_connect_unix (fd, path, path_len) < 0) {
		close (fd);
		return FALSE;
	}

	jrsock->socket = fd;
	return TRUE;
}
#endif // ! (_WIN32 || _WIN64)

int   ug_jsonrpc_socket_send (UgJsonrpcSocket* jrsock)
{
	int  n;

	n = send (jrsock->socket, jrsock->buffer.beg,
	          jrsock->buffer.cur - jrsock->buffer.beg, 0);
	jrsock->buffer.cur = jrsock->buffer.beg;
	if (n == -1)
		return -1;
	return n;
}

int   ug_jsonrpc_socket_receive (UgJsonrpcSocket* jrsock)
{
	int        length;
	int        buffer_size;
	int        error;
	int        receive_size = 0;

	buffer_size = jrsock->buffer.end - jrsock->buffer.beg;
	do {
		// if connection was closed, read()/recv()/recvXXX() will return zero.
		length = recv (jrsock->socket, jrsock->buffer.beg, buffer_size, 0);
		if (length == -1)
			return -1;
		error = ug_json_parse (&jrsock->json, jrsock->buffer.beg, length);
		if (error < 0 || jrsock->rpc.error == 0)
			jrsock->rpc.error = error;
		receive_size += length;
	} while (length == buffer_size);

	return receive_size;
}

// ----------------------------------------------------------------------------
// Server API

static UG_THREAD_RETURN_TYPE rpc_thread (UgJsonrpcSocket* jr_socket)
{
	UgSocketServer*  server;
	UgJsonrpcServerFunc  callback;

	server = jr_socket->server;
	callback = server->user.data3;
	callback (&jr_socket->rpc, server->user.data, server->user.data2);
	ug_jsonrpc_socket_final (jr_socket);
	ug_free (jr_socket);
	ug_socket_server_unref (server);  // ref() on on_accepted()

	return UG_THREAD_RETURN_VALUE;
}

static void on_receiving (UgSocketServer* server, SOCKET client_fd, void* data)
{
	UgJsonrpcSocket*  jr_socket;
	UgThread  thread;

	ug_socket_server_ref (server);  // unref() on service_thread()
	// create UgJsonrpcSocket to accept JSON-RPC request
	jr_socket = ug_malloc0 (sizeof (UgJsonrpcSocket));
	ug_jsonrpc_socket_init (jr_socket);
	jr_socket->socket = client_fd;
	jr_socket->server = server;
	ug_thread_create (&thread, (UgThreadFunc) rpc_thread, jr_socket);
	ug_thread_unjoin (&thread);
}

UgSocketServer* ug_jsonrpc_socket_server_new (const char* addr, const char* port_or_serv)
{
	SOCKET    server_fd;

	server_fd = socket (AF_INET, SOCK_STREAM, 0);
	if (server_fd == INVALID_SOCKET)
		return NULL;

	if (ug_socket_listen (server_fd, addr, port_or_serv, 5) == SOCKET_ERROR) {
		closesocket (server_fd);
		return NULL;
	}

	return ug_socket_server_new (server_fd);
}

#if !(defined _WIN32 || defined _WIN64)
UgSocketServer* ug_jsonrpc_socket_server_new_unix (const char* path, int path_len)
{
	SOCKET    server_fd;

	server_fd = socket (AF_UNIX, SOCK_STREAM, 0);
	if (server_fd == -1)
		return NULL;

	if (ug_socket_listen_unix (server_fd, path, path_len, 5) == -1) {
		close (server_fd);
		return NULL;
	}

	return ug_socket_server_new (server_fd);
}

#endif // ! (_WIN32 || _WIN64)

void  ug_jsonrpc_socket_server_run (UgSocketServer* server,
                                    UgJsonrpcServerFunc accepter,
                                    void* data, void* data2)
{
	server->user.data  = data;
	server->user.data2 = data2;
	server->user.data3 = accepter;
	ug_socket_server_set_receiver (server, on_receiving, NULL);
	ug_socket_server_start (server);
}

