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

#ifndef RAMPARA_H_IS_INCLUDED
#define RAMPARA_H_IS_INCLUDED

#include <ramalloc/fail.h>
#include <ramalloc/tls.h>
#include <ramalloc/lazy.h>

typedef struct rampara_pool
{
   ramtls_key_t ramparap_tlskey;
   ramopt_appetite_t ramparap_appetite;
   size_t ramparap_reclaimratio;
} rampara_pool_t;

ramfail_status_t rampara_mkpool(rampara_pool_t *parapool_arg, ramopt_appetite_t appetite_arg, size_t reclaimratio_arg);
ramfail_status_t rampara_rmpool(rampara_pool_t *parapool_arg);
ramfail_status_t rampara_acquire(void **newptr_arg, rampara_pool_t *parapool_arg, size_t size_arg);
#define rampara_release ramlazy_release
ramfail_status_t rampara_reclaim(size_t *count_arg, rampara_pool_t *parapool_arg, size_t goal_arg);
ramfail_status_t rampara_flush(rampara_pool_t *parapool_arg);
ramfail_status_t rampara_query(rampara_pool_t **parapool_arg, size_t *size_arg, void *ptr_arg);
ramfail_status_t rampara_chkpool(const rampara_pool_t *parapool_arg);

#endif /* RAMPARA_H_IS_INCLUDED */