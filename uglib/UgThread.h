/*
 *
 *   Copyright (C) 2012-2017 by C.H. Huang
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

#ifndef UG_THREAD_H
#define UG_THREAD_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#if defined _WIN32 || defined _WIN64

typedef uintptr_t    UgThread;
typedef void*        UgMutex;

typedef unsigned (*UgThreadFunc)(void*);

#define UG_THREAD_RETURN_TYPE      unsigned
#define UG_THREAD_RETURN_VALUE     0
#define UG_THREAD_OK               0

int   ug_thread_create (UgThread* thread, UgThreadFunc func, void* data);
int   ug_thread_join   (UgThread* thread);
void  ug_thread_unjoin (UgThread* thread);
UgThread  ug_thread_self (void);

// mutex ------
void  ug_mutex_init (UgMutex* mutex);
void  ug_mutex_clear (UgMutex* mutex);
void  ug_mutex_lock (UgMutex* mutex);
void  ug_mutex_unlock (UgMutex* mutex);

//#elif defined(HAVE_PTHREAD)
#else
#include <pthread.h>

typedef pthread_t          UgThread;
typedef pthread_mutex_t    UgMutex;

typedef void* (*UgThreadFunc)(void*);

#define UG_THREAD_RETURN_TYPE      void*
#define UG_THREAD_RETURN_VALUE     NULL
#define UG_THREAD_OK               0

#define ug_thread_create(thread, func, data)    \
		pthread_create(thread, NULL, func, data)

#define ug_thread_join(thread)    \
		pthread_join(*(thread), NULL)

#define ug_thread_unjoin(thread)    \
		pthread_detach(*(thread))

#define ug_thread_self()    pthread_self()

// mutex ------
// void  ug_mutex_init (UgMutex* mutex);
#define  ug_mutex_init(mutex)    pthread_mutex_init (mutex, NULL)

// void  ug_mutex_clear (UgMutex* mutex);
#define  ug_mutex_clear(mutex)   pthread_mutex_destroy (mutex)

// void  ug_mutex_lock (UgMutex* mutex);
#define  ug_mutex_lock(mutex)    pthread_mutex_lock (mutex)

// void  ug_mutex_unlock (UgMutex* mutex);
#define  ug_mutex_unlock(mutex)  pthread_mutex_unlock (mutex)

#endif  // _WIN32 || _WIN64


#ifdef __cplusplus
}
#endif

#endif  // UG_THREAD_H

