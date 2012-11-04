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

#include <stdlib.h>
#include <UgDefine.h>
#include <UgThread.h>

#if defined _WIN32 || defined _WIN64
#include <windows.h>
#include <process.h>    // _beginthreadex

int  ug_thread_create (UgThread* thread, UgThreadFunc func, void* data)
{
	*thread = _beginthreadex (NULL, 0,    // stack size
			(void*) func, data,
			0, NULL);
	if (*thread)
		return UG_THREAD_OK;
	return 1;
}

int   ug_thread_join (UgThread* thread)
{
	DWORD result;

	result = WaitForSingleObject ((HANDLE) *thread, INFINITE);
	if (result == WAIT_FAILED || result == WAIT_TIMEOUT)
		return 1;
	return UG_THREAD_OK;
}

void  ug_thread_unjoin (UgThread* thread)
{
	// free resource after thread exit
	CloseHandle ((HANDLE) *thread);
}

UgThread  ug_thread_self (void)
{
	return GetCurrentThreadId ();
}

void  ug_mutex_init (UgMutex* mutex)
{
	*mutex = ug_malloc (sizeof (CRITICAL_SECTION));
	InitializeCriticalSection (*mutex);
}

void  ug_mutex_clear (UgMutex* mutex)
{
	DeleteCriticalSection (*mutex);
	ug_free (*mutex);
}

void  ug_mutex_lock (UgMutex* mutex)
{
	EnterCriticalSection (*mutex);
}

void  ug_mutex_unlock (UgMutex* mutex)
{
	LeaveCriticalSection (*mutex);
}

#endif // _WIN32 || _WIN64

