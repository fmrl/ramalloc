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
static ramfail_status_t ramvec_initnode(ramvec_node_t *node_arg, ramvec_pool_t *pool_arg);
static ramfail_status_t ramvec_mkpool2(ramvec_pool_t *pool_arg, size_t nodecap_arg,
   ramvec_mknode_t mknode_arg);
static ramfail_status_t ramvec_chkinv(ramlist_list_t *list_arg, void *context_arg);
static ramfail_status_t ramvec_chkavail(ramlist_list_t *list_arg, void *context_arg);

ramfail_status_t ramvec_mkpool2(ramvec_pool_t *pool_arg, size_t nodecap_arg, 
   ramvec_mknode_t mknode_arg)
{
   assert(pool_arg);
   RAMFAIL_DISALLOWZ(nodecap_arg);
   RAMFAIL_DISALLOWZ(mknode_arg);

   RAMFAIL_RETURN(ramlist_mklist(&pool_arg->ramvecvp_inv));
   RAMFAIL_RETURN(ramlist_mklist(&pool_arg->ramvecvp_avail));
   pool_arg->ramvecvp_nodecapacity = nodecap_arg;
   pool_arg->ramvecvp_mknode = mknode_arg;

   return RAMFAIL_OK;
}

ramfail_status_t ramvec_mkpool(ramvec_pool_t *pool_arg, size_t nodecap_arg,
   ramvec_mknode_t mknode_arg)
{
   ramfail_status_t e = RAMFAIL_INSANE;

   RAMFAIL_DISALLOWZ(pool_arg);

   e = ramvec_mkpool2(pool_arg, nodecap_arg, mknode_arg);
   /* i ensure that 'pool_arg' is zeroed out if something goes wrong. */
   if (RAMFAIL_OK != e)
      memset(pool_arg, 0, sizeof(*pool_arg));

   return e;
}

ramfail_status_t ramvec_acquire(ramvec_node_t *node_arg, int isfull_arg)
{
   ramvec_pool_t *pool = NULL;

   RAMFAIL_DISALLOWZ(node_arg);

   pool = node_arg->ramvecn_vpool;
   assert(pool);

   /* now, if the node is full, it becomes unavailable. i remove it from the
    * availability stack. i can ignore the return value of 'ramlist_pop()' because
    * access to it is already preserved through 'pool->ramvecvp_avail'.*/
   if (isfull_arg)
   {
      ramlist_list_t *unused = NULL;

      RAMFAIL_RETURN(ramlist_pop(&unused, &node_arg->ramvecn_avail));
      RAMFAIL_RETURN(ramlist_mknil(&node_arg->ramvecn_avail));
   }

   return RAMFAIL_OK;
}

ramfail_status_t ramvec_getnode(ramvec_node_t **node_arg, ramvec_pool_t *pool_arg)
{
   int hastail = 0;

   RAMFAIL_DISALLOWZ(node_arg);
   *node_arg = NULL;
   RAMFAIL_DISALLOWZ(pool_arg);

   RAMFAIL_RETURN(ramlist_hastail(&hastail, &pool_arg->ramvecvp_avail));
   if (hastail)
   {
      ramlist_list_t *l = NULL;

      /* there's something on the availability stack; i need to retrieve it. */
      RAMFAIL_RETURN(ramlist_next(&l, &pool_arg->ramvecvp_avail));
      RAMMETA_BACKCAST(*node_arg, ramvec_node_t, ramvecn_avail, l);

      return RAMFAIL_OK;
   }
   else
   {
      ramvec_node_t *node = NULL;

      /* there's nothing on the availability stack; i need to make a new node. */
      RAMFAIL_RETURN(pool_arg->ramvecvp_mknode(&node, pool_arg));
      RAMFAIL_RETURN(ramvec_initnode(node, pool_arg));

      RAMFAIL_RETURN(ramlist_splice(&pool_arg->ramvecvp_avail, &node->ramvecn_avail));
      RAMFAIL_RETURN(ramlist_splice(&pool_arg->ramvecvp_inv, &node->ramvecn_inv));

      *node_arg = node;
      return RAMFAIL_OK;
   }
}

ramfail_status_t ramvec_release(ramvec_node_t *node_arg, int wasfull_arg, int isempty_arg)
{
   ramvec_pool_t *pool = NULL;

   RAMFAIL_DISALLOWZ(node_arg);

   pool = node_arg->ramvecn_vpool;
   assert(pool);

   /* if the node is now empty, i can discard it by removing it from both
    * the inventory and availability lists. */
   if (isempty_arg)
   {
      ramlist_list_t *unused = NULL;
      RAMFAIL_RETURN(ramlist_pop(&unused, &node_arg->ramvecn_inv));
      RAMFAIL_RETURN(ramlist_mknil(&node_arg->ramvecn_inv));
      if (!RAMLIST_ISNIL(&node_arg->ramvecn_avail))
      {
         RAMFAIL_RETURN(ramlist_pop(&unused, &node_arg->ramvecn_avail));
         RAMFAIL_RETURN(ramlist_mknil(&node_arg->ramvecn_avail));
      }
   }
   /* otherwise, if the node was full before releasing the memory object,
    * then i need push it onto the availability stack. */
   else if (wasfull_arg)
   {
      RAMFAIL_RETURN(ramlist_mklist(&node_arg->ramvecn_avail));
      RAMFAIL_RETURN(ramlist_splice(&pool->ramvecvp_avail, &node_arg->ramvecn_avail));
   }

   return RAMFAIL_OK;
}

ramfail_status_t ramvec_initnode(ramvec_node_t *node_arg, ramvec_pool_t *pool_arg)
{
   int hastail = 0;

   assert(node_arg);
   assert(pool_arg);
   /* the pool must be empty if a new node is to be be initialized. */
   assert(RAMFAIL_OK == ramlist_hastail(&hastail, &pool_arg->ramvecvp_avail) && !hastail);

   node_arg->ramvecn_vpool = pool_arg;
   RAMFAIL_RETURN(ramlist_mklist(&node_arg->ramvecn_inv));
   RAMFAIL_RETURN(ramlist_mklist(&node_arg->ramvecn_avail));

   return RAMFAIL_OK;
}

ramfail_status_t ramvec_chkpool(const ramvec_pool_t *pool_arg, ramvec_chknode_t chknode_arg)
{
   ramlist_list_t *first = NULL;
   ramvec_chkcontext_t c = {0};

   RAMFAIL_DISALLOWZ(pool_arg);

   c.ramveccc_pool = pool_arg;
   c.ramveccc_chknode = chknode_arg;

   /* the sentinel needs to be checked but cannot be included in the foreach loop
    * because it references no data. it's safe to drop const qualifiers because
    * i know that ramlist_foreach() does not modify values passed into it. */
   RAMFAIL_RETURN(ramlist_chklist(&pool_arg->ramvecvp_inv));
   RAMFAIL_RETURN(ramlist_next(&first, (ramlist_list_t *)&pool_arg->ramvecvp_inv));
   RAMFAIL_RETURN(ramlist_foreach(first, (ramlist_list_t *)&pool_arg->ramvecvp_inv, 
      &ramvec_chkinv, &c));

   RAMFAIL_RETURN(ramlist_chklist(&pool_arg->ramvecvp_avail));
   RAMFAIL_RETURN(ramlist_next(&first, (ramlist_list_t *)&pool_arg->ramvecvp_avail));
   RAMFAIL_RETURN(ramlist_foreach(first, (ramlist_list_t *)&pool_arg->ramvecvp_avail, 
      &ramvec_chkavail, &c));

   return RAMFAIL_OK;
}

ramfail_status_t ramvec_chkinv(ramlist_list_t *list_arg, void *context_arg)
{
   const ramvec_node_t *node = NULL;
   const ramvec_chkcontext_t *c = (ramvec_chkcontext_t *)context_arg;

   RAMFAIL_DISALLOWZ(list_arg);
   assert(context_arg);

   RAMFAIL_RETURN(ramlist_chklist(list_arg));
   RAMMETA_BACKCAST(node, ramvec_node_t, ramvecn_inv, list_arg);
   RAMFAIL_CONFIRM(RAMFAIL_CORRUPT, c->ramveccc_pool == node->ramvecn_vpool);

   /* if additional checking was specified, pass control to that function with
    * its associated context. */
   if (c->ramveccc_chknode)
      c->ramveccc_chknode(node);

   return RAMFAIL_TRYAGAIN;
}

ramfail_status_t ramvec_chkavail(ramlist_list_t *list_arg, void *context_arg)
{
   const ramvec_node_t *node = NULL;
   const ramvec_chkcontext_t *c = (ramvec_chkcontext_t *)context_arg;

   RAMFAIL_DISALLOWZ(list_arg);
   assert(context_arg);

   RAMFAIL_RETURN(ramlist_chklist(list_arg));
   RAMMETA_BACKCAST(node, ramvec_node_t, ramvecn_avail, list_arg);
   RAMFAIL_CONFIRM(RAMFAIL_CORRUPT, c->ramveccc_pool == node->ramvecn_vpool);

   return RAMFAIL_TRYAGAIN;
}
