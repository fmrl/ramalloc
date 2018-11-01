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

#ifndef RAMSLOT_H_IS_INCLUDED
#define RAMSLOT_H_IS_INCLUDED

#include <ramalloc/vec.h>
#include <ramalloc/fail.h>
#include <ramalloc/want.h>
#include <ramalloc/stdint.h>

#if RAM_WANT_COMPACT
   typedef int16_t ramslot_index_t;
#  define ramslot_sztoidx ram_cast_sztoi16
#  define RAMSLOT_MAXCAPACITY ((size_t)INT16_MAX)
   typedef uint16_t ramslot_size_t;
#else
   typedef int32_t ramslot_index_t;
#  define ramslot_sztoidx ram_cast_sztoi32
#  define RAMSLOT_MAXCAPACITY ((size_t)INT32_MAX)
   typedef uint32_t ramslot_size_t;
#endif

typedef struct ramslot_node ramslot_node_t;
typedef struct ramslot_pool ramslot_pool_t;

typedef ram_reply_t (*ramslot_mknode_t)(ramslot_node_t **node_arg, void **slots_arg, 
   ramslot_pool_t *pool_arg);
typedef ram_reply_t (*ramslot_rmnode_t)(ramslot_node_t *ptr_arg);
typedef ram_reply_t (*ramslot_initslot_t)(void *slot_arg, ramslot_node_t *node_arg);

struct ramslot_node
{
   ramvec_node_t ramslotn_vnode;
   char *ramslotn_slots;
   ramslot_size_t ramslotn_count;
   ramslot_index_t ramslotn_freestk;
};

struct ramslot_pool
{
   ramslot_mknode_t ramslotp_mknode;
   ramslot_rmnode_t ramslotp_rmnode;
   ramslot_initslot_t ramslotp_initslot;
   ramvec_pool_t ramslotp_vpool;
   size_t ramslotp_granularity;
};

ram_reply_t ramslot_mkpool(ramslot_pool_t *pool_arg, size_t granularity_arg, 
   size_t nodesz_arg, ramslot_mknode_t mknode_arg, ramslot_rmnode_t rnnode_arg, 
   ramslot_initslot_t initslot_arg);
ram_reply_t ramslot_acquire(void **newptr_arg, ramslot_pool_t *pool_arg);
ram_reply_t ramslot_release(void *ptr_arg, ramslot_node_t *node_arg);
ram_reply_t ramslot_chkpool(const ramslot_pool_t *pool_arg);
ram_reply_t ramslot_getgranularity(size_t *granularity_arg, const ramslot_pool_t *slotpool_arg);

#endif /* RAMSLOT_H_IS_INCLUDED */
