/* ex: set softtabstop=3 shiftwidth=3 expandtab: */

/* 
 * This file is part of the *ramalloc* project at <http://fmrl.org>.
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
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <ramalloc/algn.h>
#include <ramalloc/foot.h>
#include <ramalloc/want.h>
#include <ramalloc/sys.h>
#include <ramalloc/cast.h>
#include <assert.h>
#include <memory.h>

typedef struct ramalgn_node
{
   ramslot_node_t ramalgnn_slotnode;
} ramalgn_node_t;

typedef struct ramalgn_footer
{
   /* in an aligned pool, the node is stored directly in the footer. */
   ramalgn_node_t ramalgnf_node;
} ramalgn_footer_t;

typedef struct ramalgn_globals
{
   ramfoot_spec_t ramalgng_footerspec;
   int ramalgng_initflag;
} ramalgn_globals_t;

static ram_reply_t ramalgn_mkpool2(ramalgn_pool_t *pool_arg, rampg_appetite_t appetite_arg, 
   size_t granularity_arg, const ramalgn_tag_t *tag_arg);
static ram_reply_t ramalgn_findnode(ramalgn_node_t **node_arg, char *ptr_arg);
static ram_reply_t ramalgn_mknode(ramslot_node_t **node_arg, void **slots_arg, ramslot_pool_t *pool_arg);
static ram_reply_t ramalgn_mknode2(ramalgn_node_t **node_arg, ramalgn_pool_t *pool_arg, char *page_arg);
static ram_reply_t ramalgn_rmnode(ramslot_node_t *node_arg);

static ramalgn_globals_t ramalgn_theglobals;

ram_reply_t ramalgn_initialize()
{
   if (!ramalgn_theglobals.ramalgng_initflag)
   {
      ramalgn_globals_t stage = {0};
      size_t writezone = 0;

      /* the page pool's granularity is my writable zone. */
      RAM_FAIL_TRAP(rampg_getgranularity(&writezone));
      RAMFOOT_MKSPEC(&stage.ramalgng_footerspec, ramalgn_footer_t, writezone, "ALIG");
      stage.ramalgng_initflag = 1;

      ramalgn_theglobals = stage;
   }

   return RAM_REPLY_OK;
}

ram_reply_t ramalgn_mkpool(ramalgn_pool_t *pool_arg, rampg_appetite_t appetite_arg, size_t granularity_arg, const ramalgn_tag_t *tag_arg)
{
   ram_reply_t e = RAM_REPLY_INSANE;

   RAM_FAIL_NOTNULL(pool_arg);
   RAM_FAIL_EXPECT(RAM_REPLY_INCONSISTENT, ramalgn_theglobals.ramalgng_initflag);

   e = ramalgn_mkpool2(pool_arg, appetite_arg, granularity_arg, tag_arg);
   /* i ensure that 'pool_arg' is zeroed out if something goes wrong. */
   if (RAM_REPLY_OK != e)
      memset(pool_arg, 0, sizeof(*pool_arg));

   return e;
}

ram_reply_t ramalgn_mkpool2(ramalgn_pool_t *pool_arg, rampg_appetite_t appetite_arg, 
   size_t granularity_arg, const ramalgn_tag_t *tag_arg)
{
   size_t capacity = 0;

   assert(pool_arg != NULL);
   RAM_FAIL_NOTZERO(granularity_arg);
   assert(ramalgn_theglobals.ramalgng_initflag);

   RAM_FAIL_TRAP(rampg_mkpool(&pool_arg->ramalgnp_pgpool, appetite_arg));
   capacity = ramalgn_theglobals.ramalgng_footerspec.footer_offset / granularity_arg;
   /* if the capacity doesn't meet certain requirements, then i must inform the caller. */
   /* TODO: why is the slot capacity limit tested here and not in ramslot_mkpool()? */
   if (RAM_WANT_MINPAGECAPACITY > capacity || RAMSLOT_MAXCAPACITY < capacity)
      return RAM_REPLY_RANGEFAIL;
   RAM_FAIL_TRAP(ramslot_mkpool(&pool_arg->ramalgnp_slotpool, granularity_arg, 
      capacity, &ramalgn_mknode, &ramalgn_rmnode, NULL));
   if (tag_arg)
      pool_arg->ramalgnp_tag = *tag_arg;
   else
      memset(&pool_arg->ramalgnp_tag, 0, sizeof(pool_arg->ramalgnp_tag));

   return RAM_REPLY_OK;
}

ram_reply_t ramalgn_acquire(void **ptr_arg, ramalgn_pool_t *pool_arg)
{
   RAM_FAIL_NOTNULL(ptr_arg);
   *ptr_arg = NULL;
   RAM_FAIL_NOTNULL(pool_arg);
   assert(ramalgn_theglobals.ramalgng_initflag);

   RAM_FAIL_TRAP(ramslot_acquire(ptr_arg, &pool_arg->ramalgnp_slotpool));

   return RAM_REPLY_OK;
}

ram_reply_t ramalgn_release(void *ptr_arg)
{
   ramalgn_node_t *node = NULL;

   RAM_FAIL_EXPECT(RAM_REPLY_INCONSISTENT, ramalgn_theglobals.ramalgng_initflag);

   RAM_FAIL_TRAP(ramalgn_findnode(&node, (char *)ptr_arg));
   RAM_FAIL_TRAP(ramslot_release(ptr_arg, &node->ramalgnn_slotnode));

   return RAM_REPLY_OK;
}

ram_reply_t ramalgn_findnode(ramalgn_node_t **node_arg, char *ptr_arg)
{
   ramalgn_footer_t *foot = NULL;
   ram_reply_t e = RAM_REPLY_INSANE;

   assert(node_arg != NULL);
   assert(ptr_arg != NULL);
   assert(ramalgn_theglobals.ramalgng_initflag);

   e = ramfoot_getstorage((void **)&foot, &ramalgn_theglobals.ramalgng_footerspec, ptr_arg);
   switch (e)
   {
   default:
      RAM_FAIL_TRAP(e);
      /* i shouldn't ever get here. */
      return RAM_REPLY_INSANE;
   case RAM_REPLY_NOTFOUND:
      return e;
   case RAM_REPLY_OK:
      break;
   }

   *node_arg = &foot->ramalgnf_node;
   return RAM_REPLY_OK;
}

ram_reply_t ramalgn_chkpool(const ramalgn_pool_t *pool_arg)
{
   RAM_FAIL_NOTNULL(pool_arg);
   assert(ramalgn_theglobals.ramalgng_initflag);

   RAM_FAIL_TRAP(rampg_chkpool(&pool_arg->ramalgnp_pgpool));
   RAM_FAIL_TRAP(ramslot_chkpool(&pool_arg->ramalgnp_slotpool));

   return RAM_REPLY_OK;
}

ram_reply_t ramalgn_mknode(ramslot_node_t **node_arg, void **slots_arg, ramslot_pool_t *pool_arg)
{
   ramalgn_pool_t *pool = NULL;
   void *page = NULL;
   ram_reply_t e = RAM_REPLY_INSANE;
   ramalgn_node_t *node = NULL;

   RAM_FAIL_NOTNULL(node_arg);
   *node_arg = NULL;
   RAM_FAIL_NOTNULL(slots_arg);
   *slots_arg = NULL;
   RAM_FAIL_NOTNULL(pool_arg);
   assert(ramalgn_theglobals.ramalgng_initflag);

   pool = RAM_CAST_STRUCTBASE(ramalgn_pool_t, ramalgnp_slotpool, pool_arg);
   RAM_FAIL_TRAP(rampg_acquire(&page, &pool->ramalgnp_pgpool));
   e = ramalgn_mknode2(&node, pool, (char *)page);
   if (RAM_REPLY_OK == e)
   {
      *node_arg = &node->ramalgnn_slotnode;
      *slots_arg = page;
      return RAM_REPLY_OK;
   }
   else
   {
      RAM_FAIL_PANIC(rampg_release(page));
      return e;
   }
}

ram_reply_t ramalgn_mknode2(ramalgn_node_t **node_arg, ramalgn_pool_t *pool_arg, char *page_arg)
{
   ramalgn_footer_t *foot = NULL;

   RAM_FAIL_NOTNULL(node_arg);
   *node_arg = NULL;
   RAM_FAIL_NOTNULL(pool_arg);
   RAM_FAIL_NOTNULL(page_arg);
   assert(ramalgn_theglobals.ramalgng_initflag);

   /* i need to write a footer to the page to ensure that i can get to the pool
    * given any address of of the page. */
   RAM_FAIL_TRAP(ramfoot_mkfooter((void **)&foot,
      &ramalgn_theglobals.ramalgng_footerspec, page_arg));
   *node_arg = &foot->ramalgnf_node;

   return RAM_REPLY_OK;
}

ram_reply_t ramalgn_rmnode(ramslot_node_t *node_arg)
{
   RAM_FAIL_NOTNULL(node_arg);
   assert(ramalgn_theglobals.ramalgng_initflag);

   RAM_FAIL_TRAP(rampg_release(node_arg->ramslotn_slots));
   return RAM_REPLY_OK;
}

ram_reply_t ramalgn_query(ramalgn_pool_t **apool_arg, void *ptr_arg)
{
   ram_reply_t e = RAM_REPLY_INSANE;
   ramalgn_node_t *anode = NULL;
   ramslot_pool_t *spool = NULL;
   ramalgn_pool_t *apool = NULL;

   RAM_FAIL_NOTNULL(apool_arg);
   *apool_arg = NULL;
   RAM_FAIL_NOTNULL(ptr_arg);
   RAM_FAIL_EXPECT(RAM_REPLY_INCONSISTENT, ramalgn_theglobals.ramalgng_initflag);

   e = ramalgn_findnode(&anode, (char *)ptr_arg);
   switch (e)
   {
   default:
      RAM_FAIL_TRAP(e);
      /* i shouldn't ever get here. */
      return RAM_REPLY_INSANE;
   case RAM_REPLY_NOTFOUND:
      return e;
   case RAM_REPLY_OK:
      break;
   }

   /* i need to find the slot pool before i can back-cast to get the aligned pool. */
   spool = RAM_CAST_STRUCTBASE(ramslot_pool_t, ramslotp_vpool,
         anode->ramalgnn_slotnode.ramslotn_vnode.ramvecn_vpool);
   apool = RAM_CAST_STRUCTBASE(ramalgn_pool_t, ramalgnp_slotpool, spool);

   *apool_arg = apool;
   return RAM_REPLY_OK;
}

ram_reply_t ramalgn_gettag(const ramalgn_tag_t **tag_arg, const ramalgn_pool_t *apool_arg)
{
   RAM_FAIL_NOTNULL(tag_arg);
   *tag_arg = NULL;
   RAM_FAIL_NOTNULL(apool_arg);
   assert(ramalgn_theglobals.ramalgng_initflag);

   *tag_arg = &apool_arg->ramalgnp_tag;
   return RAM_REPLY_OK;
}

ram_reply_t ramalgn_getgranularity(size_t *granularity_arg, const ramalgn_pool_t *apool_arg)
{
   RAM_FAIL_NOTNULL(granularity_arg);
   *granularity_arg = 0;
   RAM_FAIL_NOTNULL(apool_arg);
   assert(ramalgn_theglobals.ramalgng_initflag);

   RAM_FAIL_TRAP(ramslot_getgranularity(granularity_arg, &apool_arg->ramalgnp_slotpool));

   return RAM_REPLY_OK;
}
