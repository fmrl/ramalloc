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

#include <ramalloc/opt.h>
#include <ramalloc/vec.h>
#include <assert.h>
#include <memory.h>

typedef struct ramvec_chkcontext
{
   const ramvec_pool_t *ramveccc_pool;
   ramvec_chknode_t ramveccc_chknode;
} ramvec_chkcontext_t;

/* ramvec_rmnode() removes a node from the pool. */
static ram_reply_t ramvec_initnode(ramvec_node_t *node_arg, ramvec_pool_t *pool_arg);
static ram_reply_t ramvec_mkpool2(ramvec_pool_t *pool_arg, size_t nodecap_arg,
   ramvec_mknode_t mknode_arg);
static ram_reply_t ramvec_chkinv(ramlist_list_t *list_arg, void *context_arg);
static ram_reply_t ramvec_chkavail(ramlist_list_t *list_arg, void *context_arg);

ram_reply_t ramvec_mkpool2(ramvec_pool_t *pool_arg, size_t nodecap_arg, 
   ramvec_mknode_t mknode_arg)
{
   assert(pool_arg != NULL);
   RAM_FAIL_NOTZERO(nodecap_arg);
   RAM_FAIL_NOTNULL(mknode_arg);

   RAM_FAIL_TRAP(ramlist_mklist(&pool_arg->ramvecvp_inv));
   RAM_FAIL_TRAP(ramlist_mklist(&pool_arg->ramvecvp_avail));
   pool_arg->ramvecvp_nodecapacity = nodecap_arg;
   pool_arg->ramvecvp_mknode = mknode_arg;

   return RAM_REPLY_OK;
}

ram_reply_t ramvec_mkpool(ramvec_pool_t *pool_arg, size_t nodecap_arg,
   ramvec_mknode_t mknode_arg)
{
   ram_reply_t e = RAM_REPLY_INSANE;

   RAM_FAIL_NOTNULL(pool_arg);

   e = ramvec_mkpool2(pool_arg, nodecap_arg, mknode_arg);
   /* i ensure that 'pool_arg' is zeroed out if something goes wrong. */
   if (RAM_REPLY_OK != e)
      memset(pool_arg, 0, sizeof(*pool_arg));

   return e;
}

ram_reply_t ramvec_acquire(ramvec_node_t *node_arg, int isfull_arg)
{
   ramvec_pool_t *pool = NULL;

   RAM_FAIL_NOTNULL(node_arg);

   pool = node_arg->ramvecn_vpool;
   assert(pool != NULL);

   /* now, if the node is full, it becomes unavailable. i remove it from the
    * availability stack. i can ignore the return value of 'ramlist_pop()' because
    * access to it is already preserved through 'pool->ramvecvp_avail'.*/
   if (isfull_arg)
   {
      ramlist_list_t *unused = NULL;

      RAM_FAIL_TRAP(ramlist_pop(&unused, &node_arg->ramvecn_avail));
      RAM_FAIL_TRAP(ramlist_mknil(&node_arg->ramvecn_avail));
   }

   return RAM_REPLY_OK;
}

ram_reply_t ramvec_getnode(ramvec_node_t **node_arg, ramvec_pool_t *pool_arg)
{
   int hastail = 0;

   RAM_FAIL_NOTNULL(node_arg);
   *node_arg = NULL;
   RAM_FAIL_NOTNULL(pool_arg);

   RAM_FAIL_TRAP(ramlist_hastail(&hastail, &pool_arg->ramvecvp_avail));
   if (hastail)
   {
      ramlist_list_t *l = NULL;

      /* there's something on the availability stack; i need to retrieve it. */
      RAM_FAIL_TRAP(ramlist_next(&l, &pool_arg->ramvecvp_avail));
      RAMMETA_BACKCAST(*node_arg, ramvec_node_t, ramvecn_avail, l);

      return RAM_REPLY_OK;
   }
   else
   {
      ramvec_node_t *node = NULL;

      /* there's nothing on the availability stack; i need to make a new node. */
      RAM_FAIL_TRAP(pool_arg->ramvecvp_mknode(&node, pool_arg));
      RAM_FAIL_TRAP(ramvec_initnode(node, pool_arg));

      RAM_FAIL_TRAP(ramlist_splice(&pool_arg->ramvecvp_avail, &node->ramvecn_avail));
      RAM_FAIL_TRAP(ramlist_splice(&pool_arg->ramvecvp_inv, &node->ramvecn_inv));

      *node_arg = node;
      return RAM_REPLY_OK;
   }
}

ram_reply_t ramvec_release(ramvec_node_t *node_arg, int wasfull_arg, int isempty_arg)
{
   ramvec_pool_t *pool = NULL;

   RAM_FAIL_NOTNULL(node_arg);

   pool = node_arg->ramvecn_vpool;
   assert(pool != NULL);

   /* if the node is now empty, i can discard it by removing it from both
    * the inventory and availability lists. */
   if (isempty_arg)
   {
      ramlist_list_t *unused = NULL;
      RAM_FAIL_TRAP(ramlist_pop(&unused, &node_arg->ramvecn_inv));
      RAM_FAIL_TRAP(ramlist_mknil(&node_arg->ramvecn_inv));
      if (!RAMLIST_ISNIL(&node_arg->ramvecn_avail))
      {
         RAM_FAIL_TRAP(ramlist_pop(&unused, &node_arg->ramvecn_avail));
         RAM_FAIL_TRAP(ramlist_mknil(&node_arg->ramvecn_avail));
      }
   }
   /* otherwise, if the node was full before releasing the memory object,
    * then i need push it onto the availability stack. */
   else if (wasfull_arg)
   {
      RAM_FAIL_TRAP(ramlist_mklist(&node_arg->ramvecn_avail));
      RAM_FAIL_TRAP(ramlist_splice(&pool->ramvecvp_avail, &node_arg->ramvecn_avail));
   }

   return RAM_REPLY_OK;
}

ram_reply_t ramvec_initnode(ramvec_node_t *node_arg, ramvec_pool_t *pool_arg)
{
#ifndef NDEBUG
   int hastail = 0;
#endif

   assert(node_arg != NULL);
   assert(pool_arg != NULL);
   /* the pool must be empty if a new node is to be be initialized. */
   assert(RAM_REPLY_OK == ramlist_hastail(&hastail, &pool_arg->ramvecvp_avail) && !hastail);

   node_arg->ramvecn_vpool = pool_arg;
   RAM_FAIL_TRAP(ramlist_mklist(&node_arg->ramvecn_inv));
   RAM_FAIL_TRAP(ramlist_mklist(&node_arg->ramvecn_avail));

   return RAM_REPLY_OK;
}

ram_reply_t ramvec_chkpool(const ramvec_pool_t *pool_arg, ramvec_chknode_t chknode_arg)
{
   ramlist_list_t *first = NULL;
   ramvec_chkcontext_t c = {0};

   RAM_FAIL_NOTNULL(pool_arg);

   c.ramveccc_pool = pool_arg;
   c.ramveccc_chknode = chknode_arg;

   /* the sentinel needs to be checked but cannot be included in the foreach loop
    * because it references no data. it's safe to drop const qualifiers because
    * i know that ramlist_foreach() does not modify values passed into it. */
   RAM_FAIL_TRAP(ramlist_chklist(&pool_arg->ramvecvp_inv));
   RAM_FAIL_TRAP(ramlist_next(&first, (ramlist_list_t *)&pool_arg->ramvecvp_inv));
   RAM_FAIL_TRAP(ramlist_foreach(first, (ramlist_list_t *)&pool_arg->ramvecvp_inv, 
      &ramvec_chkinv, &c));

   RAM_FAIL_TRAP(ramlist_chklist(&pool_arg->ramvecvp_avail));
   RAM_FAIL_TRAP(ramlist_next(&first, (ramlist_list_t *)&pool_arg->ramvecvp_avail));
   RAM_FAIL_TRAP(ramlist_foreach(first, (ramlist_list_t *)&pool_arg->ramvecvp_avail, 
      &ramvec_chkavail, &c));

   return RAM_REPLY_OK;
}

ram_reply_t ramvec_chkinv(ramlist_list_t *list_arg, void *context_arg)
{
   const ramvec_node_t *node = NULL;
   const ramvec_chkcontext_t *c = (ramvec_chkcontext_t *)context_arg;

   RAM_FAIL_NOTNULL(list_arg);
   assert(context_arg != NULL);

   RAM_FAIL_TRAP(ramlist_chklist(list_arg));
   RAMMETA_BACKCAST(node, ramvec_node_t, ramvecn_inv, list_arg);
   RAM_FAIL_EXPECT(RAM_REPLY_CORRUPT, c->ramveccc_pool == node->ramvecn_vpool);

   /* if additional checking was specified, pass control to that function with
    * its associated context. */
   if (c->ramveccc_chknode)
      RAM_FAIL_TRAP(c->ramveccc_chknode(node));

   return RAM_REPLY_AGAIN;
}

ram_reply_t ramvec_chkavail(ramlist_list_t *list_arg, void *context_arg)
{
   const ramvec_node_t *node = NULL;
   const ramvec_chkcontext_t *c = (ramvec_chkcontext_t *)context_arg;

   RAM_FAIL_NOTNULL(list_arg);
   assert(context_arg != NULL);

   RAM_FAIL_TRAP(ramlist_chklist(list_arg));
   RAMMETA_BACKCAST(node, ramvec_node_t, ramvecn_avail, list_arg);
   RAM_FAIL_EXPECT(RAM_REPLY_CORRUPT, c->ramveccc_pool == node->ramvecn_vpool);

   return RAM_REPLY_AGAIN;
}
