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

#include <ramalloc/slot.h>
#include <ramalloc/cast.h>
#include <assert.h>
#include <memory.h>

#define RAMSLOT_NIL_INDEX (-1)

typedef struct ramslot_footer
{
   /* in the object pool, the node is stored directly in the footer. */
   ramslot_node_t ramslotf_node;
} ramslot_footer_t;

typedef struct ramslot_freeslot
{
   ramslot_index_t ramslotfs_next;
} ramslot_freeslot_t;

static ram_reply_t ramslot_mkpool2(ramslot_pool_t *pool_arg, size_t granularity_arg, 
   size_t nodecap_arg, ramslot_mknode_t mknode_arg, ramslot_rmnode_t rmnode_arg,
   ramslot_initslot_t initslot_arg);
static ram_reply_t ramslot_mknode(ramvec_node_t **node_arg, ramvec_pool_t *pool_arg);
static ram_reply_t ramslot_initnode(ramslot_node_t *node_arg, ramslot_pool_t *pool_arg, char *slots_arg);
static ram_reply_t ramslot_calcindex(ramslot_index_t *idx_arg, const ramslot_node_t *node_arg, 
   const char *ptr_arg);
static ram_reply_t ramslot_chknode(const ramvec_node_t *node_arg);
static ram_reply_t ramslot_chkfree(const ramslot_node_t *node_arg);
#define RAMSLOT_ISFULL(Node) (RAMSLOT_NIL_INDEX == (Node)->ramslotn_freestk)
#define RAMSLOT_ISEMPTY(Node) (0 == (Node)->ramslotn_count)
#define RAMSLOT_GETSLOT(Node, Index, Granularity) \
   (((Node)->ramslotn_slots) + (Index) * (Granularity))



ram_reply_t ramslot_mkpool(ramslot_pool_t *pool_arg, size_t granularity_arg, 
   size_t nodecap_arg, ramslot_mknode_t mknode_arg, ramslot_rmnode_t rmnode_arg,
   ramslot_initslot_t initslot_arg)
{
   ram_reply_t e = RAM_REPLY_INSANE;

   RAM_FAIL_NOTNULL(pool_arg);

   e = ramslot_mkpool2(pool_arg, granularity_arg, nodecap_arg, mknode_arg, rmnode_arg, initslot_arg);
   /* i ensure that 'pool_arg' is zeroed out if something goes wrong. */
   if (RAM_REPLY_OK != e)
      memset(pool_arg, 0, sizeof(*pool_arg));

   return e;
}

ram_reply_t ramslot_mkpool2(ramslot_pool_t *pool_arg, size_t granularity_arg, 
   size_t nodecap_arg, ramslot_mknode_t mknode_arg, 
   ramslot_rmnode_t rmnode_arg, ramslot_initslot_t initslot_arg)
{

   assert(pool_arg != NULL);
   RAM_FAIL_NOTZERO(granularity_arg);
   RAM_FAIL_NOTZERO(nodecap_arg);
   RAM_FAIL_NOTNULL(mknode_arg);
   RAM_FAIL_NOTNULL(rmnode_arg);
   /* 'initslot_arg' is allowed to be NULL. */

   RAM_FAIL_TRAP(ramvec_mkpool(&pool_arg->ramslotp_vpool, nodecap_arg, &ramslot_mknode));
   pool_arg->ramslotp_granularity = granularity_arg;
   pool_arg->ramslotp_mknode = mknode_arg;
   pool_arg->ramslotp_rmnode = rmnode_arg;
   pool_arg->ramslotp_initslot = initslot_arg;

   return RAM_REPLY_OK;
}

ram_reply_t ramslot_acquire(void **ptr_arg, ramslot_pool_t *pool_arg)
{
   ramslot_index_t idx = 0;
   char *p = NULL;
   ramslot_node_t *node = NULL;
   ramvec_node_t *vnode = NULL;

   RAM_FAIL_NOTNULL(ptr_arg);
   *ptr_arg = NULL;
   RAM_FAIL_NOTNULL(pool_arg);

   /* first, i acquire a memory object from the next available node in the pool. */
   RAM_FAIL_TRAP(ramvec_getnode(&vnode, &pool_arg->ramslotp_vpool));
   node = RAM_CAST_STRUCTBASE(ramslot_node_t, ramslotn_vnode, vnode);
   /* ramvec_getnode() should never return a full node. */
   assert(!RAMSLOT_ISFULL(node));
   /* ramvec_getnode() should never return someone else's node. */
   assert(&pool_arg->ramslotp_vpool == node->ramslotn_vnode.ramvecn_vpool);

   /* i calculate the address of the pointer at the head of the free list.
    * once obtained, i retrieve the index of the next free slot and store
    * it in the node's reference to the head of the free list. */
   idx = node->ramslotn_freestk;
   assert(idx >= 0);
   assert(pool_arg->ramslotp_vpool.ramvecvp_nodecapacity <= RAMSLOT_MAXCAPACITY);
   assert(idx < (ramslot_index_t)pool_arg->ramslotp_vpool.ramvecvp_nodecapacity);
   p = RAMSLOT_GETSLOT(node, idx, pool_arg->ramslotp_granularity);
   node->ramslotn_freestk = ((ramslot_freeslot_t *)p)->ramslotfs_next;
   ++node->ramslotn_count;

   /* i finalize the acquisition by updating the pool state. */
   RAM_FAIL_PANIC(ramvec_acquire(&node->ramslotn_vnode, RAMSLOT_ISFULL(node)));

   /* i zero-out the memory, if that behavior is desired. */
#if RAM_WANT_ZEROMEM
   memset(p, 0, pool_arg->ramslotp_granularity);
#endif

   /* if the caller provided a function to initialize a slot, do so now. */
   if (pool_arg->ramslotp_initslot)
      RAM_FAIL_TRAP(pool_arg->ramslotp_initslot(p, node));

   *ptr_arg = p;
   return RAM_REPLY_OK;
}

ram_reply_t ramslot_release(void *ptr_arg, ramslot_node_t *node_arg)
{
   ramslot_pool_t *pool = NULL;
   ramslot_index_t idx = 0;
   int wasfull = 0;
   int isempty = 0;

   RAM_FAIL_NOTNULL(ptr_arg);

   pool = RAM_CAST_STRUCTBASE(ramslot_pool_t, ramslotp_vpool,
         node_arg->ramslotn_vnode.ramvecn_vpool);
   RAM_FAIL_TRAP(ramslot_calcindex(&idx, node_arg, ptr_arg));
   wasfull = RAMSLOT_ISFULL(node_arg);

   /* at this point, if something goes wrong, the node might be inconsistent and
    * there's no longer any hope for recovery. */
#if RAM_WANT_MARKFREED
   /* it's helpful to see signature bytes for destroyed memory when debugging. */
   memset(ptr_arg, RAM_WANT_MARKFREED, pool->ramslotp_granularity);
#endif

   /* now that i know the index that's associated with 'ptr_arg', i push
    * it onto the free slot stack. */
   ((ramslot_freeslot_t *)(ptr_arg))->ramslotfs_next = node_arg->ramslotn_freestk;
   node_arg->ramslotn_freestk = idx;
   --node_arg->ramslotn_count;
   isempty = RAMSLOT_ISEMPTY(node_arg);

   RAM_FAIL_PANIC(ramvec_release(&node_arg->ramslotn_vnode, wasfull, isempty));
   if (isempty)
   {
      /* if i fail to destroy the node, it doesn't leave anything in an inconsistent state.
       * it just means i'm leaking resources. recovery is technically possible. */
      RAM_FAIL_TRAP(pool->ramslotp_rmnode(node_arg));
   }

   return RAM_REPLY_OK;
}

ram_reply_t ramslot_mknode(ramvec_node_t **node_arg, ramvec_pool_t *pool_arg)
{
   ram_reply_t e = RAM_REPLY_INSANE;
   ramslot_node_t *node = NULL;
   ramslot_pool_t *pool = NULL;
   void *slots = NULL;

   assert(node_arg != NULL);
   assert(pool_arg != NULL);

   pool = RAM_CAST_STRUCTBASE(ramslot_pool_t, ramslotp_vpool, pool_arg);
   RAM_FAIL_TRAP(pool->ramslotp_mknode(&node, &slots, pool));
   e = ramslot_initnode(node, pool, (char *)slots);
   if (RAM_REPLY_OK == e)
   {
      *node_arg = &node->ramslotn_vnode;
      return RAM_REPLY_OK;
   }
   else
   {
      RAM_FAIL_PANIC(pool->ramslotp_rmnode(node));
      return e;
   }
}

ram_reply_t ramslot_initnode(ramslot_node_t *node_arg, ramslot_pool_t *pool_arg, char *slots_arg)
{
   ramslot_index_t i = 0, ii = 0, j = RAMSLOT_NIL_INDEX;

   assert(node_arg != NULL);
   assert(pool_arg != NULL);
   RAM_FAIL_NOTNULL(slots_arg);

   /* i must initialize the pointer to the beginning of slot storage first. */
   node_arg->ramslotn_slots = slots_arg;
   /* the node starts out empty... */
   node_arg->ramslotn_count = 0;
   /* ...meaning the free list starts out full. */
   RAM_FAIL_TRAP(ramslot_sztoidx(&ii, pool_arg->ramslotp_vpool.ramvecvp_nodecapacity));
   for (i = ii - 1; i >= 0; j = (i--))
   {
      ramslot_freeslot_t * const s = 
         (ramslot_freeslot_t *)RAMSLOT_GETSLOT(node_arg, i, pool_arg->ramslotp_granularity);
      
      s->ramslotfs_next = j;
   }
   node_arg->ramslotn_freestk = 0;
   
   return RAM_REPLY_OK;
}

ram_reply_t ramslot_calcindex(ramslot_index_t *idx_arg, const ramslot_node_t *node_arg, 
   const char *ptr_arg)
{
   div_t d = {0};
   ramslot_pool_t *pool = NULL;
   int n = 0;

   RAM_FAIL_NOTNULL(idx_arg);
   *idx_arg = RAMSLOT_NIL_INDEX;
   RAM_FAIL_NOTNULL(node_arg);
   RAM_FAIL_NOTNULL(ptr_arg);

   pool = RAM_CAST_STRUCTBASE(ramslot_pool_t, ramslotp_vpool,
         node_arg->ramslotn_vnode.ramvecn_vpool);

   RAM_FAIL_TRAP(ram_cast_sztoint(&n, pool->ramslotp_granularity));
   d = div(ptr_arg - node_arg->ramslotn_slots, n);
   /* it's safe to cast the node capacity to ramslot_index_t because
    * when the pool was initialized, i ensured that the node capacity
    * could not exceed RAMSLOT_MAXCAPACITY. */
   assert(pool->ramslotp_vpool.ramvecvp_nodecapacity < RAMSLOT_MAXCAPACITY);
   if (0 == d.rem && d.quot >= 0
         && (size_t)d.quot < RAMSLOT_MAXCAPACITY &&
         (ramslot_index_t)d.quot <
            (ramslot_index_t)pool->ramslotp_vpool.ramvecvp_nodecapacity)
   {
      *idx_arg = (ramslot_index_t)d.quot;
      return RAM_REPLY_OK;
   }
   else
      return RAM_REPLY_NOTFOUND;
}

ram_reply_t ramslot_chkpool(const ramslot_pool_t *pool_arg)
{
   RAM_FAIL_NOTNULL(pool_arg);
   
   RAM_FAIL_TRAP(ramvec_chkpool(&pool_arg->ramslotp_vpool, &ramslot_chknode));

   return RAM_REPLY_OK;
}

ram_reply_t ramslot_chknode(const ramvec_node_t *node_arg)
{
   ramslot_node_t *node = NULL;

   assert(node_arg != NULL);

   node = RAM_CAST_STRUCTBASE(ramslot_node_t, ramslotn_vnode, node_arg);

   /* the node cannot be empty. */
   RAM_FAIL_EXPECT(RAM_REPLY_CORRUPT, !RAMSLOT_ISEMPTY(node));
   /* the node count cannot exceed the capacity. */
   RAM_FAIL_EXPECT(RAM_REPLY_CORRUPT, 
         node->ramslotn_count <= node->ramslotn_vnode.ramvecn_vpool->ramvecvp_nodecapacity);
   /* if the node is full, the count should match the capacity. */
   if (RAMSLOT_ISFULL(node))
   {
      RAM_FAIL_EXPECT(RAM_REPLY_CORRUPT, 
         node->ramslotn_count == node->ramslotn_vnode.ramvecn_vpool->ramvecvp_nodecapacity);
   }
   /* i check the free stack. */
   RAM_FAIL_TRAP(ramslot_chkfree(node));

   return RAM_REPLY_OK;
}

ram_reply_t ramslot_chkfree(const ramslot_node_t *node_arg)
{
   ramslot_index_t idx = RAMSLOT_NIL_INDEX;
   size_t i = 0, count = 0;
   ramslot_pool_t *pool = NULL;

   assert(node_arg != NULL);

   pool = RAM_CAST_STRUCTBASE(ramslot_pool_t, ramslotp_vpool,
         node_arg->ramslotn_vnode.ramvecn_vpool);

   /* i traverse the free stack (also a linked list) and count the
    * number of elements */
   count = pool->ramslotp_vpool.ramvecvp_nodecapacity - node_arg->ramslotn_count;
   idx = node_arg->ramslotn_freestk;
   for (i = 0; i < count && idx != RAMSLOT_NIL_INDEX; ++i)
   {
      ramslot_freeslot_t * const s = 
         (ramslot_freeslot_t *)RAMSLOT_GETSLOT(node_arg, idx, pool->ramslotp_granularity);

      idx = s->ramslotfs_next;
   }

   /* if the length of the free list was longer than what was expected,
    * then something is wrong. */
   if (i != count)
      return RAM_REPLY_CORRUPT;

   return RAM_REPLY_OK;
}

ram_reply_t ramslot_getgranularity(size_t *granularity_arg, const ramslot_pool_t *slotpool_arg)
{
   RAM_FAIL_NOTNULL(granularity_arg);
   *granularity_arg = 0;
   RAM_FAIL_NOTNULL(slotpool_arg);

   *granularity_arg = slotpool_arg->ramslotp_granularity;
   return RAM_REPLY_OK;
}
