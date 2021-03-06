/* ex: set softtabstop=3 shiftwidth=3 expandtab: */

/* This file is part of the *ramalloc* project at <http://fmrl.org>.
 * Copyright (c) 2011, Michael Lowell Roberts.
 * All rights reserved. 
 *
 * Redistribution and use in source and binary forms, with or without 
 * modification, are permitted provided that the following conditions are 
 * met: 
 *
 *  * Redistributions of source code must retain the above copyright 
 *  notice, this list of conditions and the following disclaimer. 
 *
 *  * Redistributions in binary form must reproduce the above copyright 
 *  notice, this list of conditions and the following disclaimer in the 
 *  documentation and/or other materials provided with the distribution.
 * 
 *  * Neither the name of the copyright holder nor the names of 
 *  contributors may be used to endorse or promote products derived 
 *  from this software without specific prior written permission. 
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS 
 * IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED 
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A 
 * PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER 
 * OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED 
 * TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR 
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF 
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING 
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS 
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. */

#ifndef RAMWIN_H_IS_INCLUDED
#define RAMWIN_H_IS_INCLUDED

/* this header isn't intended to be included directly. please include
 * <ramalloc/sys.h> instead. */
#ifdef RAMSYS_WINDOWS

/* splint has some issues with Windows that i cannot seem to address 
 * through a splintrc file. */
#ifdef RAMSYS_SPLINT
#  define __int64 long long
#endif

#include <ramalloc/sys/types.h>
#include <ramalloc/fail.h>
#include <Windows.h>

typedef SSIZE_T ssize_t;

typedef DWORD ramwin_tlskey_t;
#define RAMWIN_NILTLSKEY TLS_OUT_OF_INDEXES
typedef CRITICAL_SECTION ramwin_mutex_t;
typedef HANDLE ramwin_thread_t;
typedef struct ramwin_barrier
{
   HANDLE ramwinb_event;
   volatile LONG ramwinb_vacancy;
   LONG ramwinb_capacity;
} ramwin_barrier_t;

ram_reply_t ramwin_initialize();

ram_reply_t ramwin_pagesize(size_t *pagesz_arg);
ram_reply_t ramwin_mmapgran(size_t *mmapgran_arg);
ram_reply_t ramwin_cpucount(size_t *cpucount_arg);
ram_reply_t ramwin_commit(char *page_arg);
ram_reply_t ramwin_decommit(char *page_arg);
ram_reply_t ramwin_reset(char *page_arg);
ram_reply_t ramwin_reserve(char **pages_arg);
ram_reply_t ramwin_bulkalloc(char **pages_arg);
ram_reply_t ramwin_release(char *pages_arg);

ram_reply_t ramwin_mktlskey(ramwin_tlskey_t *key_arg);
ram_reply_t ramwin_rmtlskey(ramwin_tlskey_t key_arg);
ram_reply_t ramwin_rcltls(void **value_arg, ramwin_tlskey_t key_arg);
ram_reply_t ramwin_stotls(ramwin_tlskey_t key_arg, void *value_arg);

ram_reply_t ramwin_mkmutex(ramwin_mutex_t *mutex_arg);
ram_reply_t ramwin_rmmutex(ramwin_mutex_t *mutex_arg);
ram_reply_t ramwin_waitformutex(ramwin_mutex_t *mutex_arg);
ram_reply_t ramwin_quitmutex(ramwin_mutex_t *mutex_arg);

ram_reply_t ramwin_mkthread(ramwin_thread_t *thread_arg, 
      ramsys_threadmain_t main_arg, void *arg_arg);
ram_reply_t ramwin_jointhread(ram_reply_t *reply_arg,
      ramwin_thread_t thread_arg);

ram_reply_t ramwin_mkbarrier(ramwin_barrier_t *barrier_arg, 
      size_t capacity_arg);
ram_reply_t ramwin_rmbarrier(ramwin_barrier_t *barrier_arg);
ram_reply_t ramwin_waitonbarrier(ramwin_barrier_t *barrier_arg);

ram_reply_t ramwin_basename(char *dest_arg, size_t len_arg, 
   const char *pathn_arg);

#define ramsys_initialize ramwin_initialize
/* virtual memory mapping */
#define ramsys_pagesize ramwin_pagesize
#define ramsys_mmapgran ramwin_mmapgran
#define ramsys_cpucount ramwin_cpucount
#define ramsys_reserve ramwin_reserve
#define ramsys_commit ramwin_commit
#define ramsys_decommit ramwin_decommit
#define ramsys_reset ramwin_reset
#define ramsys_bulkalloc ramwin_bulkalloc
#define ramsys_release ramwin_release
/* thread local storage */
typedef ramwin_tlskey_t ramsys_tlskey_t;
#define ramsys_mktlskey ramwin_mktlskey
#define ramsys_rmtlskey ramwin_rmtlskey
#define ramsys_rcltls ramwin_rcltls
#define ramsys_stotls ramwin_stotls
/* mutexes */
typedef ramwin_mutex_t ramsys_mutex_t;
#define ramsys_mkmutex ramwin_mkmutex
#define ramsys_rmmutex ramwin_rmmutex
#define ramsys_waitformutex ramwin_waitformutex
#define ramsys_quitmutex ramwin_quitmutex
/* threads */
typedef ramwin_thread_t ramsys_thread_t;
#define ramsys_mkthread ramwin_mkthread
#define ramsys_jointhread ramwin_jointhread
/* barriers */
typedef ramwin_barrier_t ramsys_barrier_t;
#define ramsys_mkbarrier ramwin_mkbarrier
#define ramsys_rmbarrier ramwin_rmbarrier
#define ramsys_waitonbarrier ramwin_waitonbarrier
/* filename manipulation */
#define RAMSYS_PATH_MAX MAX_PATH
#define ramsys_basename ramwin_basename

#endif /* RAMSYS_WINDOWS */

#endif /* RAMWIN_H_IS_INCLUDED */

