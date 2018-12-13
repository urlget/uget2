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

#ifndef UGET_EVENT_H
#define UGET_EVENT_H

#include <time.h>
#include <stdint.h>
#include <UgList.h>
#include <UgData.h>
#include <UgEntry.h>
#include <UgetNode.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct  UgetEvent       UgetEvent;

typedef enum {
	UGET_EVENT_EMPTY = 0,

	// message log
	UGET_EVENT_WARNING,
	UGET_EVENT_NORMAL,     // e.g. connecting to host
	UGET_EVENT_ERROR,

	// plug-in notifition
	UGET_EVENT_STOP,
	UGET_EVENT_START,
	UGET_EVENT_COMPLETED,  // Download completed
	UGET_EVENT_UPLOADING,  // Uploading
	UGET_EVENT_STOP_UPLOADING,

	// events for uget_task_dispatch()
	UGET_EVENT_NAME,       // UgetNode's name changed
} UgetEventType;

typedef enum
{
	UGET_EVENT_NORMAL_CUSTOM = 0,  // must be 0

	UGET_EVENT_NORMAL_CONNECT,
	UGET_EVENT_NORMAL_TRANSMIT,
	UGET_EVENT_NORMAL_RETRY,
	UGET_EVENT_NORMAL_COMPLETE,    // download completed
	UGET_EVENT_NORMAL_FINISH,      // completed, it will not be used in future.
	// resumable
	UGET_EVENT_NORMAL_RESUMABLE,
	UGET_EVENT_NORMAL_NOT_RESUMABLE,
} UgetEventNormal;

typedef enum {
	UGET_EVENT_WARNING_CUSTOM  = 0,  // must be 0

	UGET_EVENT_WARNING_FILE_RENAME_FAILED,
} UgetEventWarning;

typedef enum {
	UGET_EVENT_ERROR_CUSTOM  = 0,    // must be 0

	UGET_EVENT_ERROR_CONNECT_FAILED,
	UGET_EVENT_ERROR_FOLDER_CREATE_FAILED,
	UGET_EVENT_ERROR_FILE_CREATE_FAILED,
	UGET_EVENT_ERROR_FILE_OPEN_FAILED,
	UGET_EVENT_ERROR_THREAD_CREATE_FAILED,
	UGET_EVENT_ERROR_INCORRECT_SOURCE,
	UGET_EVENT_ERROR_OUT_OF_RESOURCE,    // disk full or out of memory
	UGET_EVENT_ERROR_NO_OUTPUT_FILE,
	UGET_EVENT_ERROR_NO_OUTPUT_SETTING,
	UGET_EVENT_ERROR_TOO_MANY_RETRIES,
	UGET_EVENT_ERROR_UNSUPPORTED_SCHEME,
	UGET_EVENT_ERROR_UNSUPPORTED_PROTOCOL = UGET_EVENT_ERROR_UNSUPPORTED_SCHEME,
	UGET_EVENT_ERROR_UNSUPPORTED_FILE,
	UGET_EVENT_ERROR_POST_FILE_NOT_FOUND,
	UGET_EVENT_ERROR_COOKIE_FILE_NOT_FOUND,

	// plug-in error code
//	UGET_EVENT_ERROR_PLUGIN_INITIALIZE_FAILED = 10000,
} UgetEventError;

extern const UgEntry  UgetEventEntry[];

// ----------------------------------------------------------------------------
// UgetEvent: It can store in UgetLog.

struct UgetEvent
{
	UG_LINK_MEMBERS (UgetEvent, UgetEvent, self);
/*	// ------ UgLink members ------
	UgetEvent*      self;
	UgetEvent*      next;
	UgetEvent*      prev;
 */

	int     type;   // UgetEventType
	time_t  time;   // date & time (seconds)
	char*   string; // User readable string or name parameter for UGET_EVENT_RENAME.

	// extra data
	union {
		void*      data;
		int        code;     // UGET_EVENT_ERROR, UGET_EVENT_WARNING, UGET_EVENT_NORMAL
	} value;
//	} value[3];
};

UgetEvent* uget_event_new (UgetEventType type, ...);
void       uget_event_free (UgetEvent* event);

#define    uget_event_new_error(code, string)  uget_event_new (UGET_EVENT_ERROR, code, string)
#define    uget_event_new_normal(code, string)  uget_event_new (UGET_EVENT_NORMAL, code, string)
#define    uget_event_new_warning(code, string)  uget_event_new (UGET_EVENT_WARNING, code, string)
/*
UgetEvent* uget_event_new_error (int code, const char* string);
UgetEvent* uget_event_new_normal (int code, const char* string);
UgetEvent* uget_event_new_warning (int code, const char* string);
 */

#ifdef __cplusplus
}
#endif

// ----------------------------------------------------------------------------
// C++11 standard-layout

#ifdef __cplusplus

namespace Uget
{
// This one is for directly use only. You can NOT derived it.
struct Event : UgetEvent {};
};  // namespace Uget

#endif  // __cplusplus


#endif  // End of UGET_EVENT_H

