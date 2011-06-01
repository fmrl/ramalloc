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

#ifndef RAMALGN_H_IS_INCLUDED
#define RAMALGN_H_IS_INCLUDED

#include <ramalloc/slot.h>
#include <ramalloc/pg.h>

typedef struct ramalgn_tag
{
   uintptr_t ramalgnt_values[2];
} ramalgn_tag_t;

typedef struct ramalgn_pool
{
   rampg_pool_t ramalgnp_pgpool;
   ramslot_pool_t ramalgnp_slotpool;
   ramalgn_tag_t ramalgnp_tag;
} ramalgn_pool_t;

ram_reply_t ramalgn_initialize();
ram_reply_t ramalgn_mkpool(ramalgn_pool_t *pool_arg, ramopt_appetite_t appetite_arg, 
   size_t granularity_arg, const ramalgn_tag_t *tag_arg);
ram_reply_t ramalgn_acquire(void **newptr_arg, ramalgn_pool_t *pool_arg);
ram_reply_t ramalgn_release(void *ptr_arg);
ram_reply_t ramalgn_chkpool(const ramalgn_pool_t *pool_arg);
ram_reply_t ramalgn_query(ramalgn_pool_t **apool_arg, void *ptr_arg);
ram_reply_t ramalgn_gettag(const ramalgn_tag_t **tag_arg, const ramalgn_pool_t *apool_arg);
ram_reply_t ramalgn_getgranularity(size_t *granularity_arg, const ramalgn_pool_t *apool_arg);

#endif /* RAMALGN_H_IS_INCLUDED */
