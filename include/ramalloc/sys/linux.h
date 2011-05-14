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

#ifndef RAMALLOC_LINUX_H_IS_INCLUDED
#define RAMALLOC_LINUX_H_IS_INCLUDED

/* this header isn't intended to be included directly. please include
 * <ramalloc/sys.h> instead. */
#ifdef RAMSYS_LINUX

#include <ramalloc/sys/posix.h>
#include <ramalloc/sys/pthreads.h>
#include <ramalloc/fail.h>
#include <ramalloc/stdint.h>
#include <ramalloc/opt.h>

typedef struct ramlin_barrier
{
   ramuix_mutex_t ramlinb_mutex;
   pthread_cond_t ramlinb_cond;
   volatile size_t ramlinb_vacancy;
   size_t ramlinb_capacity;
   uintptr_t ramlinb_cycle;
} ramlin_barrier_t;

ramfail_status_t ramlin_mkbarrier(ramlin_barrier_t *barrier_arg,
      size_t capacity_arg);
ramfail_status_t ramlin_rmbarrier(ramlin_barrier_t *barrier_arg);
ramfail_status_t ramlin_waitonbarrier(ramlin_barrier_t *barrier_arg);

#define ramsys_initialize ramuix_initialize
/* virtual memory mapping */
#define ramsys_pagesize ramuix_pagesize
#define ramsys_mmapgran ramuix_mmapgran
#define ramsys_cpucount ramuix_cpucount
#define ramsys_reserve ramuix_reserve
#define ramsys_commit ramuix_commit
#define ramsys_decommit ramuix_decommit
#define ramsys_reset ramuix_reset
#define ramsys_bulkalloc ramuix_bulkalloc
#define ramsys_release ramuix_release
/* thread local storage */
typedef ramuix_tlskey_t ramsys_tlskey_t;
#define ramsys_mktlskey ramuix_mktlskey
#define ramsys_rmtlskey ramuix_rmtlskey
#define ramsys_rcltls ramuix_rcltls
#define ramsys_stotls ramuix_stotls
/* mutexes */
typedef ramuix_mutex_t ramsys_mutex_t;
#define ramsys_mkmutex ramuix_mkmutex
#define ramsys_rmmutex ramuix_rmmutex
#define ramsys_waitformutex ramuix_waitformutex
#define ramsys_quitmutex ramuix_quitmutex
/* threads */
typedef ramuix_thread_t ramsys_thread_t;
#define ramsys_mkthread ramuix_mkthread
#define ramsys_jointhread ramuix_jointhread
/* barriers */
#if RAMOPT_BARRIERDEADLOCK
typedef ramuix_barrier_t ramsys_barrier_t;
#  define ramsys_mkbarrier ramuix_mkbarrier
#  define ramsys_waitonbarrier ramuix_waitonbarrier
#else
typedef ramlin_barrier_t ramsys_barrier_t;
#  define ramsys_mkbarrier ramlin_mkbarrier
#  define ramsys_rmbarrier ramlin_rmbarrier
#  define ramsys_waitonbarrier ramlin_waitonbarrier
#endif
/* file manipulation */
#define RAMSYS_PATH_MAX RAMUIX_PATH_MAX
#define ramsys_basename ramuix_basename

#endif /* RAMSYS_LINUX */

#endif /* RAMALLOC_LINUX_H_IS_INCLUDED */
