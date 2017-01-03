/*
 *
 *   Copyright (C) 2015-2017 by C.H. Huang
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

#ifndef UGET_RPC_H
#define UGET_RPC_H

#include <UgList.h>
#include <UgThread.h>
#include <UgJsonrpcSocket.h>
#include <UgetOption.h>

#ifdef __cplusplus
extern "C" {
#endif

enum {
	UGET_RPC_DO_NOTHING,
	UGET_RPC_SEND_COMMAND,
	UGET_RPC_PRESENT,
};

typedef struct UgetRpc           UgetRpc;
typedef struct UgetRpcReq        UgetRpcReq;
typedef struct UgetRpcCmd        UgetRpcCmd;

struct UgetRpc {
	UG_JSONRPC_SOCKET_MEMBERS;
//	UgJson           json;
//	UgJsonrpc        rpc;
//	UgBuffer         buffer;
//	int              socket;

	UgSocketServer*  server;
	UgJsonrpcObject  jobject;
	UgJsonrpcArray   jarray;

	UgOption         option;
	UgList           queue;
	UgMutex          queue_lock;
	char*            backup_dir;

#ifdef USE_UNIX_DOMAIN_SOCKET
	char*            socket_path;
	int              socket_path_len;
#endif
};

UgetRpc*  uget_rpc_new (const char* backup_dir);
void      uget_rpc_free (UgetRpc* urpc);

#ifdef USE_UNIX_DOMAIN_SOCKET
void  uget_rpc_use_unix_socket (UgetRpc* urpc, const char* path, int path_len);
#endif

int   uget_rpc_do_request (UgetRpc* urpc, UgJsonrpcObject* jobj);
void  uget_rpc_send_command (UgetRpc* urpc, int argc, char** argv);
void  uget_rpc_present (UgetRpc* urpc);

// return TRUE if server start
int   uget_rpc_start_server (UgetRpc* urpc);
void  uget_rpc_stop_server  (UgetRpc* urpc);

int          uget_rpc_has_request (UgetRpc* urpc);
UgetRpcReq*  uget_rpc_get_request (UgetRpc* urpc);

// ----------------------------------------------------------------------------
// UgetRpcReq - Request

// UgLink
#define UGET_RPC_REQ_MEMBERS  \
	UG_LINK_INT_MEMBERS (UgetRpcReq, method_id);  \
	UgDeleteFunc free

struct UgetRpcReq
{
	UGET_RPC_REQ_MEMBERS;    // UgLink
//	intptr_t     method_id;
//	UgetRpcReq*  next;
//	UgetRpcReq*  prev;
//	UgDeleteFunc free;
};

UgetRpcReq*  uget_rpc_req_new (void);
#define      uget_rpc_req_free   ug_free

// ----------------------------------------------------------------------------
// UgetRpcCmd

struct UgetRpcCmd
{
	UGET_RPC_REQ_MEMBERS;    // UgLink
//	intptr_t     method_id;
//	UgetRpcReq*  next;
//	UgetRpcReq*  prev;
//	UgDeleteFunc free;

	UgetOptionValue  value;
	UgList           uris;
};

UgetRpcCmd*  uget_rpc_cmd_new (void);
void         uget_rpc_cmd_free (UgetRpcCmd* urcmd);


#ifdef __cplusplus
}
#endif

#endif  // End of UGET_RPC_H

