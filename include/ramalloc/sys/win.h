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

#include <ramalloc/fail.h>
#include <ramalloc/sys/sysdef.h>
#include <Windows.h>

ramfail_status_t ramwin_mkglobals(ramsys_globals_t *globals_arg);
ramfail_status_t ramwin_commit(char *page_arg);
ramfail_status_t ramwin_decommit(char *page_arg);
ramfail_status_t ramwin_reset(char *page_arg);
ramfail_status_t ramwin_reserve(char **pages_arg);
ramfail_status_t ramwin_bulkalloc(char **pages_arg);
ramfail_status_t ramwin_release(char *pages_arg);

typedef DWORD ramwin_tlskey_t;
#define RAMWIN_NILTLSKEY TLS_OUT_OF_INDEXES

ramfail_status_t ramwin_mktlskey(ramwin_tlskey_t *key_arg);
ramfail_status_t ramwin_rmtlskey(ramwin_tlskey_t key_arg);
ramfail_status_t ramwin_rcltls(void **value_arg, ramwin_tlskey_t key_arg);
ramfail_status_t ramwin_stotls(ramwin_tlskey_t key_arg, void *value_arg);

typedef CRITICAL_SECTION ramwin_mutex_t;

ramfail_status_t ramwin_mkmutex(ramwin_mutex_t *mutex_arg);
ramfail_status_t ramwin_rmmutex(ramwin_mutex_t *mutex_arg);
ramfail_status_t ramwin_waitformutex(ramwin_mutex_t *mutex_arg);
ramfail_status_t ramwin_quitmutex(ramwin_mutex_t *mutex_arg);

ramfail_status_t ramwin_mkthread(ramsys_threadmain_t main_arg, void *arg_arg);

typedef struct ramwin_barrier
{
   HANDLE ramwinb_event;
   volatile LONG ramwinb_vacancy;
   LONG ramwinb_capacity;
} ramwin_barrier_t;

ramfail_status_t ramwin_mkbarrier(ramwin_barrier_t *barrier_arg, int capacity_arg);
ramfail_status_t ramwin_waitonbarrier(ramwin_barrier_t *barrier_arg);

#endif /* RAMSYS_WINDOWS */

#endif /* RAMWIN_H_IS_INCLUDED */

