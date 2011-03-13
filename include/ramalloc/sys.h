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

#ifndef RAMSYS_H_IS_INCLUDED
#define RAMSYS_H_IS_INCLUDED

#include <ramalloc/fail.h>
#include <ramalloc/sys/sysdef.h>
#include <ramalloc/sys/detect.h>

#ifdef RAMSYS_WINDOWS
#  include <ramalloc/sys/win.h>
#  define ramsys_reserve ramwin_reserve
#  define ramsys_commit ramwin_commit
#  define ramsys_decommit ramwin_decommit
#  define ramsys_reset ramwin_reset
#  define ramsys_bulkalloc ramwin_bulkalloc
#  define ramsys_release ramwin_release
/* thread local storage */
typedef ramwin_tlskey_t ramsys_tlskey_t;
#  define ramsys_mktlskey ramwin_mktlskey
#  define ramsys_rmtlskey ramwin_rmtlskey
#  define ramsys_rcltls ramwin_rcltls
#  define ramsys_stotls ramwin_stotls
/* mutexes */
typedef ramwin_mutex_t ramsys_mutex_t;
#  define ramsys_mkmutex ramwin_mkmutex
#  define ramsys_rmmutex ramwin_rmmutex
#  define ramsys_waitformutex ramwin_waitformutex
#  define ramsys_quitmutex ramwin_quitmutex
/* threads */
typedef ramsys_thread_t ramwin_thread_t;
#define ramsys_mkthread ramwin_mkthread
#define ramwin_jointhread ramwin_jointhread
/* barriers */
typedef ramwin_barrier_t ramsys_barrier_t;
#define ramsys_mkbarrier ramwin_mkbarrier
#define ramsys_waitonbarrier ramwin_waitonbarrier
#elif defined(RAMSYS_UNIX)
#  include <ramalloc/sys/uix.h>
#  define ramsys_reserve ramuix_reserve
#  define ramsys_commit ramuix_commit
#  define ramsys_decommit ramuix_decommit
#  define ramsys_reset ramuix_reset
#  define ramsys_bulkalloc ramuix_bulkalloc
#  define ramsys_release ramuix_release
/* thread local storage */
typedef ramuix_tlskey_t ramsys_tlskey_t;
#  define ramsys_mktlskey ramuix_mktlskey
#  define ramsys_rmtlskey ramuix_rmtlskey
#  define ramsys_rcltls ramuix_rcltls
#  define ramsys_stotls ramuix_stotls
/* mutexes */
typedef ramuix_mutex_t ramsys_mutex_t;
#  define ramsys_mkmutex ramuix_mkmutex
#  define ramsys_rmmutex ramuix_rmmutex
#  define ramsys_waitformutex ramuix_waitformutex
#  define ramsys_quitmutex ramuix_quitmutex
/* threads */
typedef ramuix_thread_t ramsys_thread_t;
#define ramsys_mkthread ramuix_mkthread
#define ramsys_jointhread ramuix_jointhread
/* barriers */
typedef ramuix_barrier_t ramsys_barrier_t;
#define ramsys_mkbarrier ramuix_mkbarrier
#define ramsys_waitonbarrier ramuix_waitonbarrier
#else
#  error <ramalloc/sys/detect.h> has not detected a platform i recognize.
#endif /* platform check */

ramfail_status_t ramsys_initialize();
ramfail_status_t ramsys_getglobals(const ramsys_globals_t **globals_arg);
#define RAMSYS_ISPAGE(Ptr, Globals) (0 == ((uintptr_t)(Ptr) & ~(Globals)->ramsysg_pagemask))

#endif /* RAMSYS_H_IS_INCLUDED */

