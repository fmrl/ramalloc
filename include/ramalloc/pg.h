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

#ifndef RAMPG_H_IS_INCLUDED
#define RAMPG_H_IS_INCLUDED

#include <ramalloc/vec.h>
#include <ramalloc/fail.h>
#include <ramalloc/opt.h>
#include <ramalloc/sig.h>
#include <ramalloc/slot.h>
#include <ramalloc/ramalloc.h>

typedef struct rampg_pool
{
   ramvec_pool_t rampgp_vpool;
   ramslot_pool_t rampgp_slotpool;
   ramopt_appetite_t rampgp_appetite;
   ramsig_signature_t rampgp_slotsig;
} rampg_pool_t;

ramfail_status_t rampg_initialize();
ramfail_status_t rampg_mkpool(rampg_pool_t *pool_arg, ramopt_appetite_t appetite_arg);
ramfail_status_t rampg_acquire(void **newptr_arg, rampg_pool_t *pool_arg);
ramfail_status_t rampg_release(void *ptr_arg);
ramfail_status_t rampg_chkpool(const rampg_pool_t *pool_arg);
ramfail_status_t rampg_getgranularity(size_t *granularity_arg);

#endif /* RAMPG_H_IS_INCLUDED */