/*
 *
 *   Copyright (C) 2012-2018 by C.H. Huang
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


#ifndef UGET_TASK_H
#define UGET_TASK_H

#include <stdint.h>
#include <UgRegistry.h>
#include <UgSLink.h>
#include <UgetData.h>
#include <UgetNode.h>
#include <UgetPlugin.h>

#define UGET_TASK_N_WATCH    4

#ifdef __cplusplus
extern "C" {
#endif

typedef struct  UgetTask        UgetTask;

typedef int (*UgetWatchFunc) (void* instance, UgetEvent* msg,
                              UgetNode* node, void* data);

// ----------------------------------------------------------------------------
// UgetTask : match UgetNode and UgetPlugin

void  uget_task_init(UgetTask* task);
void  uget_task_final(UgetTask* task);

int   uget_task_add(UgetTask* task, UgetNode* node, const UgetPluginInfo* pinfo);
int   uget_task_remove(UgetTask* task, UgetNode* node);
void  uget_task_remove_all(UgetTask* task);
void  uget_task_dispatch(UgetTask* task);
void  uget_task_add_watch(UgetTask* task, UgetWatchFunc func, void* data);

void  uget_task_set_speed(UgetTask* task, int dl_speed, int ul_speed);
void  uget_task_adjust_speed(UgetTask* task);

#ifdef __cplusplus
}
#endif

struct UgetTask
{
	UG_SLINKS_MEMBERS;
/*	// ------ UgSLinks members ------
	UgSLink*  at;
	int       length;
	int       allocated;
	int       element_size;
	int       n_links;
	UgSLink*  used;
	UgSLink*  freed;
 */

	struct {
		UgetWatchFunc   func;
		void*           data;
	} watch[UGET_TASK_N_WATCH];

	struct {
		int  upload;
		int  download;
	} speed, limit;

#ifdef __cplusplus
	// C++11 standard-layout
	inline void init(void)
		{ uget_task_init((UgetTask*) this); }
	inline void final(void)
		{ uget_task_final((UgetTask*) this); }

	inline int  add(UgetNode* node, UgetPluginInfo* info)
		{ return uget_task_add((UgetTask*) this, node, info); }
	inline void remove(UgetNode* node)
		{ uget_task_remove((UgetTask*) this, node); }
	inline void dispatch(void)
		{ uget_task_dispatch((UgetTask*) this); }

#endif  // __cplusplus
};


// ----------------------------------------------------------------------------
// C++11 standard-layout

#ifdef __cplusplus

namespace Uget
{
typedef struct UgetTask    Task;
};  // namespace Uget

#endif  // __cplusplus


#endif  // End of UGET_TASK_H

