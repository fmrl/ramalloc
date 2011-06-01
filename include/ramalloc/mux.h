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

#ifndef RAMMUX_H_IS_INCLUDED
#define RAMMUX_H_IS_INCLUDED

#include <ramalloc/algn.h>
#include <ramalloc/opt.h>

#define RAMMUX_MAXPOOLCOUNT 128

typedef struct rammux_pool
{
   ramalgn_tag_t rammuxp_tag;
   size_t rammuxp_step;
   ramalgn_pool_t rammuxp_apools[RAMMUX_MAXPOOLCOUNT];
   ramopt_appetite_t rammuxp_appetite;
   int8_t rammuxp_initflags[RAMMUX_MAXPOOLCOUNT];
} rammux_pool_t;

ram_reply_t rammux_mkpool(rammux_pool_t *mpool_arg, ramopt_appetite_t appetite_arg);
ram_reply_t rammux_acquire(void **newptr_arg, rammux_pool_t *mpool_arg, size_t size_arg);
#define rammux_release ramalgn_release
ram_reply_t rammux_query(rammux_pool_t **mpool_arg, size_t *size_arg, void *ptr_arg);
ram_reply_t rammux_chkpool(const rammux_pool_t *mpool_arg);

#endif /* RAMMUX_H_IS_INCLUDED */