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

#include <string.h>
#include <UgDefine.h>
#include <UgetNode.h>
#include <UgetData.h>

// ----------------------------------------------------------------------------
// compare functions for sorting

int   uget_node_compare_name (UgetNode* node1, UgetNode* node2)
{
	node1 = node1->base;
	node2 = node2->base;

	if (node1->name) {
		if (node2->name == NULL)
			return 1;
	}
	else {
		if (node2->name == NULL)
			return 0;
		else
			return -1;
	}
	return strcmp (node1->name, node2->name);
}

int   uget_node_compare_complete (UgetNode* node1, UgetNode* node2)
{
	UgetProgress*  progress1;
	UgetProgress*  progress2;

	node1 = node1->base;
	node2 = node2->base;
	progress1 = ug_data_get (node1->data, UgetProgressInfo);
	progress2 = ug_data_get (node2->data, UgetProgressInfo);
	if (progress1) {
		if (progress2 == NULL)
			return 1;
	}
	else {
		if (progress2 == NULL)
			return uget_node_compare_name (node1, node2);  // 0
		else
			return -1;
	}

	// if these are the same, compare name
	if (progress1->complete == progress2->complete)
		return uget_node_compare_name (node1, node2);
	// return diff of complete
	else
		return (int)(progress1->complete - progress2->complete);
}

int   uget_node_compare_size (UgetNode* node1, UgetNode* node2)
{
	UgetProgress*  progress1;
	UgetProgress*  progress2;

	node1 = node1->base;
	node2 = node2->base;
	progress1 = ug_data_get (node1->data, UgetProgressInfo);
	progress2 = ug_data_get (node2->data, UgetProgressInfo);
	if (progress1) {
		if (progress2 == NULL)
			return 1;
	}
	else {
		if (progress2 == NULL)
			return uget_node_compare_name (node1, node2);  // 0
		else
			return -1;
	}

	// if these are the same, compare name
	if (progress1->total == progress2->total)
		return uget_node_compare_name (node1, node2);
	// return diff of total
	else
		return (int)(progress1->total - progress2->total);
}

int   uget_node_compare_percent (UgetNode* node1, UgetNode* node2)
{
	UgetProgress*  progress1;
	UgetProgress*  progress2;

	node1 = node1->base;
	node2 = node2->base;
	progress1 = ug_data_get (node1->data, UgetProgressInfo);
	progress2 = ug_data_get (node2->data, UgetProgressInfo);
	if (progress1) {
		if (progress2 == NULL)
			return 1;
	}
	else {
		if (progress2 == NULL)
			return uget_node_compare_name (node1, node2);  // 0
		else
			return -1;
	}

	// if these are the same, compare name
	if (progress1->percent == progress2->percent)
		return uget_node_compare_name (node1, node2);
	// return diff of percent
	else
		return progress1->percent - progress2->percent;
}

int   uget_node_compare_elapsed (UgetNode* node1, UgetNode* node2)
{
	UgetProgress*  progress1;
	UgetProgress*  progress2;

	node1 = node1->base;
	node2 = node2->base;
	progress1 = ug_data_get (node1->data, UgetProgressInfo);
	progress2 = ug_data_get (node2->data, UgetProgressInfo);
	if (progress1) {
		if (progress2 == NULL)
			return 1;
	}
	else {
		if (progress2 == NULL)
			return uget_node_compare_name (node1, node2);  // 0
		else
			return -1;
	}

	// if these are the same, compare name
	if (progress1->elapsed == progress2->elapsed)
		return uget_node_compare_name (node1, node2);
	// return diff of elapsed (consume time)
	else
		return (int)(progress1->elapsed - progress2->elapsed);
}

int   uget_node_compare_left (UgetNode* node1, UgetNode* node2)
{
	UgetProgress*  progress1;
	UgetProgress*  progress2;

	node1 = node1->base;
	node2 = node2->base;
	progress1 = ug_data_get (node1->data, UgetProgressInfo);
	progress2 = ug_data_get (node2->data, UgetProgressInfo);
	if (progress1) {
		if (progress2 == NULL)
			return 1;
	}
	else {
		if (progress2 == NULL)
			return uget_node_compare_name (node1, node2);  // 0
		else
			return -1;
	}

	// if these are the same, compare name
	if (progress1->left == progress2->left)
		return uget_node_compare_name (node1, node2);
	// return diff of left (remain time)
	else
		return (int)(progress1->left - progress2->left);
}

int   uget_node_compare_speed (UgetNode* node1, UgetNode* node2)
{
	UgetProgress*  progress1;
	UgetProgress*  progress2;

	node1 = node1->base;
	node2 = node2->base;
	progress1 = ug_data_get (node1->data, UgetProgressInfo);
	progress2 = ug_data_get (node2->data, UgetProgressInfo);
	if (progress1) {
		if (progress2 == NULL)
			return 1;
	}
	else {
		if (progress2 == NULL)
			return uget_node_compare_name (node1, node2);  // 0
		else
			return -1;
	}

	// if these are the same, compare name
	if (progress1->download_speed == progress2->download_speed)
		return uget_node_compare_name (node1, node2);
	// return diff of download_speed
	else
		return (int)(progress1->download_speed - progress2->download_speed);
}

int   uget_node_compare_upload_speed (UgetNode* node1, UgetNode* node2)
{
	UgetProgress*  progress1;
	UgetProgress*  progress2;

	node1 = node1->base;
	node2 = node2->base;
	progress1 = ug_data_get (node1->data, UgetProgressInfo);
	progress2 = ug_data_get (node2->data, UgetProgressInfo);
	if (progress1) {
		if (progress2 == NULL)
			return 1;
	}
	else {
		if (progress2 == NULL)
			return uget_node_compare_name (node1, node2);  // 0
		else
			return -1;
	}

	// if these are the same, compare name
	if (progress1->upload_speed == progress2->upload_speed)
		return uget_node_compare_name (node1, node2);
	// return diff of upload_speed
	else
		return (int)(progress1->upload_speed - progress2->upload_speed);
}

int   uget_node_compare_uploaded (UgetNode* node1, UgetNode* node2)
{
	UgetProgress*  progress1;
	UgetProgress*  progress2;

	node1 = node1->base;
	node2 = node2->base;
	progress1 = ug_data_get (node1->data, UgetProgressInfo);
	progress2 = ug_data_get (node2->data, UgetProgressInfo);
	if (progress1) {
		if (progress2 == NULL)
			return 1;
	}
	else {
		if (progress2 == NULL)
			return uget_node_compare_name (node1, node2);  // 0
		else
			return -1;
	}

	// if these are the same, compare name
	if (progress1->uploaded == progress2->uploaded)
		return uget_node_compare_name (node1, node2);
	// return diff of uploaded
	else
		return (int)(progress1->uploaded - progress2->uploaded);
}

int   uget_node_compare_ratio (UgetNode* node1, UgetNode* node2)
{
	UgetProgress*  progress1;
	UgetProgress*  progress2;

	node1 = node1->base;
	node2 = node2->base;
	progress1 = ug_data_get (node1->data, UgetProgressInfo);
	progress2 = ug_data_get (node2->data, UgetProgressInfo);
	if (progress1) {
		if (progress2 == NULL)
			return 1;
	}
	else {
		if (progress2 == NULL)
			return uget_node_compare_name (node1, node2);  // 0
		else
			return -1;
	}

	// if these are the same, compare name
	if (progress1->ratio == progress2->ratio)
		return uget_node_compare_name (node1, node2);
	// return diff of ratio
	else
		return (int)(progress1->ratio - progress2->ratio);
}

int   uget_node_compare_retry (UgetNode* node1, UgetNode* node2)
{
	UgetCommon*  common1;
	UgetCommon*  common2;

	node1 = node1->base;
	node2 = node2->base;
	common1 = ug_data_get (node1->data, UgetCommonInfo);
	common2 = ug_data_get (node2->data, UgetCommonInfo);
	if (common1) {
		if (common2 == NULL)
			return 1;
	}
	else {
		if (common2 == NULL)
			return uget_node_compare_name (node1, node2);  // 0
		else
			return -1;
	}

	// if these are the same, compare name
	if (common1->retry_count == common2->retry_count)
		return uget_node_compare_name (node1, node2);
	// return diff of retry_count
	else
		return common1->retry_count - common2->retry_count;
}

int   uget_node_compare_parent_name (UgetNode* node1, UgetNode* node2)
{
	node1 = node1->base->parent;
	node2 = node2->base->parent;

	if (node1->name) {
		if (node2->name == NULL)
			return 1;
	}
	else {
		if (node2->name == NULL)
			return 0;
		else
			return -1;
	}
	return strcmp (node1->name, node2->name);
}

int   uget_node_compare_uri (UgetNode* node1, UgetNode* node2)
{
	UgetCommon*  common1;
	UgetCommon*  common2;

	node1 = node1->base;
	node2 = node2->base;
	common1 = ug_data_get (node1->data, UgetCommonInfo);
	common2 = ug_data_get (node2->data, UgetCommonInfo);
	if (common1) {
		if (common2 == NULL)
			return 1;
	}
	else {
		if (common2 == NULL)
			return uget_node_compare_name (node1, node2);  // 0
		else
			return -1;
	}
	return strcmp (common1->uri, common2->uri);
}

int   uget_node_compare_added_time (UgetNode* node1, UgetNode* node2)
{
	UgetLog*  log1;
	UgetLog*  log2;

	node1 = node1->base;
	node2 = node2->base;
	log1 = ug_data_get (node1->data, UgetLogInfo);
	log2 = ug_data_get (node2->data, UgetLogInfo);
	if (log1) {
		if (log2 == NULL)
			return 1;
	}
	else {
		if (log2 == NULL)
			return uget_node_compare_name (node1, node2);  // 0
		else
			return -1;
	}

	// if these added_time are the same, compare name
	if (log1->added_time == log2->added_time)
		return uget_node_compare_name (node1, node2);
	// return diff of added_time
	else
		return (int)(log1->added_time - log2->added_time);
}

int   uget_node_compare_completed_time (UgetNode* node1, UgetNode* node2)
{
	UgetLog*  log1;
	UgetLog*  log2;

	node1 = node1->base;
	node2 = node2->base;
	log1 = ug_data_get (node1->data, UgetLogInfo);
	log2 = ug_data_get (node2->data, UgetLogInfo);
	if (log1) {
		if (log2 == NULL)
			return 1;
	}
	else {
		if (log2 == NULL)
			return uget_node_compare_name (node1, node2);  // 0
		else
			return -1;
	}

	// if these completed_time are the same, compare name
	if (log1->completed_time == log2->completed_time)
		return uget_node_compare_name (node1, node2);
	// return diff of completed_time
	else
		return (int)(log1->completed_time - log2->completed_time);
}

