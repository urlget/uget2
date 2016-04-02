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

#include <UgUri.h>
#include <UgString.h>
#include <UgetData.h>
#include <UgetTask.h>

// static function
static int  uget_task_dispatch1 (UgetTask* task, UgetNode* node, UgetPlugin* plugin);

void  uget_task_init (UgetTask* task)
{
	int  count;

	ug_slinks_init ((UgSLinks*) task, 32);
	for (count = 0;  count < UGET_TASK_N_WATCH;  count++) {
		task->watch[count].func = NULL;
		task->watch[count].data = NULL;
	}
}

void  uget_task_final (UgetTask* task)
{
	uget_task_remove_all (task);
//	ug_slinks_final ((UgSLinks*) task);
	ug_array_clear (task);
}

int   uget_task_add (UgetTask* task, UgetNode* node, const UgetPluginInfo* info)
{
	UgetRelation*  relation;
	int            dl_ul_int_array[2];
	union {
		UgetProgress*  progress;
		UgetCommon*    common;
	} temp;

	// UgetRelation: check exist plug-in
	relation = ug_info_realloc (&node->info, UgetRelationInfo);
	if (relation->task.plugin)
		return FALSE;

	// UgetProgress: clear progress when it completed
	temp.progress = ug_info_get (&node->info, UgetProgressInfo);
	if (temp.progress) {
		// reset progress if it's percent is 100%.
		if (temp.progress->percent == 100) {
			temp.progress->percent = 0;
			temp.progress->uploaded = 0;
			temp.progress->complete = 0;
			temp.progress->total = 0;
		}
		temp.progress->download_speed = 0;
		temp.progress->upload_speed = 0;
	}

	// UgetCommon: clear retry_count
	temp.common = ug_info_get (&node->info, UgetCommonInfo);
	if (temp.common)
		temp.common->retry_count = 0;

	// create plug-in and control it
	relation->task.plugin = uget_plugin_new (info);
	if (task->limit.download || task->limit.upload) {
		dl_ul_int_array[0] = task->limit.download / (task->n_links + 1);
		dl_ul_int_array[1] = task->limit.upload   / (task->n_links + 1);
		uget_plugin_ctrl_speed (relation->task.plugin, dl_ul_int_array);
	}
	if (uget_plugin_start (relation->task.plugin, node) == FALSE) {
		// dispatch error message from plug-in
		uget_task_dispatch1 (task, node, relation->task.plugin);
		// release plug-in
		uget_plugin_unref (relation->task.plugin);
		relation->task.plugin = NULL;
		return FALSE;
	}
	relation->task.plugin_name = ug_strdup (info->name);

	ug_slinks_add ((UgSLinks*) task, node);
	return TRUE;
}

int  uget_task_remove (UgetTask* task, UgetNode* node)
{
	UgSLink*   prev;
	UgetRelation* relation;

	if (ug_slinks_find ((UgSLinks*) task, node, &prev)) {
		ug_slinks_remove ((UgSLinks*) task, node, prev);
		node->state &= ~UGET_STATE_ACTIVE;
		// UgetRelation
		relation = ug_info_get (&node->info, UgetRelationInfo);
		if (relation) {
//			uget_plugin_post (relation->task.plugin,
//					uget_event_new_state (node, UGET_STATE_QUEUING));
			uget_plugin_stop (relation->task.plugin);
			uget_plugin_unref (relation->task.plugin);
			relation->task.plugin = NULL;
		}
		return TRUE;
	}
	return FALSE;
}

void  uget_task_remove_all (UgetTask* task)
{
	while (task->used)
		uget_task_remove (task, task->used->data);
}

static int  uget_task_dispatch1 (UgetTask* task, UgetNode* node, UgetPlugin* plugin)
{
	UgetEvent*  event;
	UgetEvent*  next;
	int         active;
	union {
		int           count;
		UgetLog*      log;
	} temp;

	active = uget_plugin_sync (plugin);
	// plug-in was paused by user (see function uget_app_pause_download)
	if (node->state & UGET_STATE_PAUSED)
		active = FALSE;
	// plug-in has stopped if uget_plugin_sync() return FALSE.
	if (active == FALSE)
		node->state &= ~UGET_STATE_ACTIVE;

	event = uget_plugin_pop (plugin);
	for (;  event;  event = next) {
		for (temp.count = 0;  temp.count < UGET_TASK_N_WATCH;  temp.count++) {
			if (task->watch[temp.count].func) {
				task->watch[temp.count].func (task, event,
						node, task->watch[temp.count].data);
			}
		}

		// unlink current UgetEvent
		next = event->next;
		event->next = NULL;
		event->prev = NULL;

		switch (event->type) {
		case UGET_EVENT_ERROR:
			node->state |= UGET_STATE_ERROR;  // don't break here
		case UGET_EVENT_WARNING:
		case UGET_EVENT_NORMAL:
			temp.log = ug_info_realloc (&node->info, UgetLogInfo);
			ug_list_prepend (&temp.log->messages, (UgLink*) event);
			break;

		case UGET_EVENT_START:
			node->state |= UGET_STATE_ACTIVE;
			uget_event_free (event);
			break;

		case UGET_EVENT_STOP:
			node->state &= ~UGET_STATE_ACTIVE;
			uget_event_free (event);
			active = FALSE;
			break;

#if 0
		case UGET_EVENT_COMPLETED:
			node->state |= UGET_STATE_COMPLETED;
			uget_event_free (event);
			break;
#endif

//		case UGET_EVENT_INFO:
//		case UGET_EVENT_REMOVE:
//		case UGET_EVENT_INSERT:
		default:
			uget_event_free (event);
			break;
		}
	}

	return active;
}

void  uget_task_dispatch (UgetTask* task)
{
	UgSLink*      link;
	UgetNode*     node;
	UgetProgress* progress;
	UgetRelation* relation;

	task->speed.download = 0;
	task->speed.upload = 0;

	for (link = task->used;  link;  link = link->next) {
		node = link->data;
		relation = ug_info_get (&node->info, UgetRelationInfo);
		if (uget_task_dispatch1 (task, node, relation->task.plugin) == FALSE)
			continue;
		// speed
		progress = ug_info_get (&node->info, UgetProgressInfo);
		if (progress) {
			task->speed.download += progress->download_speed;
			task->speed.upload   += progress->upload_speed;
			relation->task.speed[0] = progress->download_speed;
			relation->task.speed[1] = progress->upload_speed;
		}
	}
}

void  uget_task_add_watch (UgetTask* task, UgetWatchFunc func, void* data)
{
	int  count;

	for (count = 0;  count < UGET_TASK_N_WATCH;  count++) {
		if (task->watch[count].func)
			continue;
		task->watch[count].func = func;
		task->watch[count].data = data;
		break;
	}
}

// ----------------------------------------------------------------------------
// speed control

#define SPEED_MIN        512

static void uget_task_disable_limit_index (UgetTask* task, int idx);
static void uget_task_adjust_speed_index (UgetTask* task, int idx, int limit_new);

void  uget_task_set_speed (UgetTask* task, int dl_speed, int ul_speed)
{
	// download
	task->limit.download = dl_speed;
	if (dl_speed == 0)
		uget_task_disable_limit_index (task, 0);
	else if (task->n_links > 0)
		uget_task_adjust_speed_index (task, 0, dl_speed - task->speed.download);

	// upload
	task->limit.upload = ul_speed;
	if (ul_speed == 0)
		uget_task_disable_limit_index (task, 1);
	else if (task->n_links > 0)
		uget_task_adjust_speed_index (task, 1, ul_speed - task->speed.upload);
}

void  uget_task_adjust_speed (UgetTask* task)
{
	if (task->n_links == 0)
		return;

	if (task->limit.download > 0)
		uget_task_adjust_speed_index (task, 0, task->limit.download - task->speed.download);
	if (task->limit.upload > 0)
		uget_task_adjust_speed_index (task, 1, task->limit.upload - task->speed.upload);
}

static void uget_task_adjust_speed_index (UgetTask* task, int idx, int remain)
{
	UgSLink*       link;
	UgetNode*      node;
	UgetRelation*  relation = NULL;
	UgetRelation*  prev = NULL;
	int            n_piece = 0;

	if (remain > 0) {
		// increase speed by priority
		for (link = task->used;  link;  link = link->next) {
			node = (UgetNode*) link->data;
			relation = ug_info_get (&node->info, UgetRelationInfo);
			relation->task.prev = prev;
			prev = relation;
			n_piece += relation->task.priority + 1;
		}

		remain = remain / n_piece;
		for (;  relation;  relation = prev) {
			relation->task.limit[idx] = relation->task.speed[idx] +
			                            remain * (relation->task.priority+1);
			if (relation->task.limit[idx] < SPEED_MIN)
				relation->task.limit[idx] = SPEED_MIN;
			uget_plugin_ctrl_speed (relation->task.plugin,
			                        relation->task.limit);
			prev = relation->task.prev;
			relation->task.prev = NULL;
		}
	}
	else {
		// reduce speed
		remain = remain / task->n_links;
		for (link = task->used;  link;  link = link->next) {
			node = (UgetNode*) link->data;
			relation = ug_info_get (&node->info, UgetRelationInfo);
			relation->task.limit[idx] = relation->task.speed[idx] + remain;
			if (relation->task.limit[idx] < SPEED_MIN)
				relation->task.limit[idx] = SPEED_MIN;
			uget_plugin_ctrl_speed (relation->task.plugin,
			                        relation->task.limit);
		}
	}
}

static void uget_task_disable_limit_index (UgetTask* task, int idx)
{
	UgSLink*       link;
	UgetNode*      node;
	UgetRelation*  relation;

	for (link = task->used;  link;  link = link->next) {
		node = (UgetNode*) link->data;
		relation = ug_info_get (&node->info, UgetRelationInfo);
		relation->task.limit[idx] = 0;
		uget_plugin_ctrl_speed (relation->task.plugin,
		                        relation->task.limit);
	}
}
