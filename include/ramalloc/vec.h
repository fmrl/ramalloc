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

#ifndef RAMVEC_H_IS_INCLUDED
#define RAMVEC_H_IS_INCLUDED

#include <ramalloc/fail.h>
#include <ramalloc/list.h>
#include <stdlib.h>

typedef struct ramvec_pool ramvec_pool_t;
typedef struct ramvec_node ramvec_node_t;

typedef ram_reply_t (*ramvec_mknode_t)(ramvec_node_t **node_arg, ramvec_pool_t *pool_arg);
typedef ram_reply_t (*ramvec_chknode_t)(const ramvec_node_t *node_arg);

struct ramvec_node
{
   ramvec_pool_t *ramvecn_vpool;
   ramlist_list_t ramvecn_inv;
   ramlist_list_t ramvecn_avail;
};

struct ramvec_pool
{
   ramvec_mknode_t ramvecvp_mknode;
   ramlist_list_t ramvecvp_inv;
   ramlist_list_t ramvecvp_avail;
   size_t ramvecvp_nodecapacity;
};

ram_reply_t ramvec_mkpool(ramvec_pool_t *pool_arg, size_t nodecap_arg,
   ramvec_mknode_t mknode_arg);
ram_reply_t ramvec_getnode(ramvec_node_t **node_arg, ramvec_pool_t *pool_arg);
ram_reply_t ramvec_acquire(ramvec_node_t *node_arg, int isfull_arg);
ram_reply_t ramvec_release(ramvec_node_t *node_arg, int wasfull_arg, int isempty_arg);
ram_reply_t ramvec_chkpool(const ramvec_pool_t *pool_arg, ramvec_chknode_t chknode_arg);

#endif /* RAMVEC_H_IS_INCLUDED */
