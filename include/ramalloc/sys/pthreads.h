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

#ifndef RAMALLOC_PTHREADS_H_IS_INCLUDED
#define RAMALLOC_PTHREADS_H_IS_INCLUDED

/* this header isn't intended to be included directly. please include
 * <ramalloc/sys.h> instead. */
#ifdef RAMSYS_PTHREADS

#include <ramalloc/sys/types.h>
#include <ramalloc/fail.h>
#include <pthread.h>

typedef pthread_key_t ramuix_tlskey_t;
typedef pthread_mutex_t ramuix_mutex_t;
typedef pthread_t ramuix_thread_t;
typedef pthread_barrier_t ramuix_barrier_t;

ramfail_status_t ramuix_mktlskey(ramuix_tlskey_t *key_arg);
ramfail_status_t ramuix_rmtlskey(ramuix_tlskey_t key_arg);
ramfail_status_t ramuix_rcltls(void **tls_arg, ramuix_tlskey_t key_arg);
ramfail_status_t ramuix_stotls(ramuix_tlskey_t key_arg, void *value_arg);

ramfail_status_t ramuix_mkmutex(ramuix_mutex_t *mutex_arg);
ramfail_status_t ramuix_rmmutex(ramuix_mutex_t *mutex_arg);
ramfail_status_t ramuix_waitformutex(ramuix_mutex_t *mutex_arg);
ramfail_status_t ramuix_quitmutex(ramuix_mutex_t *mutex_arg);

ramfail_status_t ramuix_mkthread(ramuix_thread_t *thread_arg,
      ramsys_threadmain_t main_arg, void *arg_arg);
ramfail_status_t ramuix_jointhread(ramfail_status_t *reply_arg,
      ramuix_thread_t thread_arg);

ramfail_status_t ramuix_mkbarrier(ramuix_barrier_t *barrier_arg,
      int capacity_arg);
ramfail_status_t ramuix_rmbarrier(ramuix_barrier_t *barrier_arg);
ramfail_status_t ramuix_waitonbarrier(ramuix_barrier_t *barrier_arg);

#endif /* RAMSYS_PTHREADS */

#endif /* RAMALLOC_PTHREADS_H_IS_INCLUDED */
