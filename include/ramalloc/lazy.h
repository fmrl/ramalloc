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

#ifndef RAMLAZY_H_IS_INCLUDED
#define RAMLAZY_H_IS_INCLUDED

#include <ramalloc/fail.h>
#include <ramalloc/mux.h>
#include <ramalloc/tra.h>

typedef struct ramlazy_pool
{
   rammux_pool_t ramlazyp_muxpool;
   ramtra_trash_t ramlazyp_trash;
   size_t ramlazyp_disposalratio;
} ramlazy_pool_t;

ram_reply_t ramlazy_mkpool(ramlazy_pool_t *lpool_arg, ramopt_appetite_t appetite_arg, size_t disposalratio_arg);
ram_reply_t ramlazy_rmpool(ramlazy_pool_t *lpool_arg);
ram_reply_t ramlazy_acquire(void **newptr_arg, ramlazy_pool_t *lpool_arg, size_t size_arg);
ram_reply_t ramlazy_release(void *ptr_arg);
ram_reply_t ramlazy_reclaim(size_t *count_arg, ramlazy_pool_t *lpool_arg, size_t goal_arg);
ram_reply_t ramlazy_flush(ramlazy_pool_t *lpool_arg);
ram_reply_t ramlazy_query(ramlazy_pool_t **lpool_arg, size_t *size_arg, void *ptr_arg);
ram_reply_t ramlazy_chkpool(const ramlazy_pool_t *lpool_arg);

#endif /* RAMLAZY_H_IS_INCLUDED */