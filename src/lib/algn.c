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
#include <ramalloc/opt.h>
#include <ramalloc/sys.h>
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

static ramfail_status_t ramalgn_mkpool2(ramalgn_pool_t *pool_arg, ramopt_appetite_t appetite_arg, 
   size_t granularity_arg, const ramalgn_tag_t *tag_arg);
static ramfail_status_t ramalgn_findnode(ramalgn_node_t **node_arg, char *ptr_arg);
static ramfail_status_t ramalgn_mknode(ramslot_node_t **node_arg, void **slots_arg, ramslot_pool_t *pool_arg);
static ramfail_status_t ramalgn_mknode2(ramalgn_node_t **node_arg, ramalgn_pool_t *pool_arg, char *page_arg);
static ramfail_status_t ramalgn_rmnode(ramslot_node_t *node_arg, ramslot_pool_t *pool_arg);

static ramalgn_globals_t ramalgn_theglobals;

ramfail_status_t ramalgn_initialize()
{
   if (!ramalgn_theglobals.ramalgng_initflag)
   {
      ramalgn_globals_t stage = {0};
      size_t writezone = 0;

      /* the page pool's granularity is my writable zone. */
      RAMFAIL_RETURN(rampg_getgranularity(&writezone));
      RAMFOOT_MKSPEC(&stage.ramalgng_footerspec, ramalgn_footer_t, writezone, "ALIG");
      stage.ramalgng_initflag = 1;

      ramalgn_theglobals = stage;
   }

   return RAMFAIL_OK;
}

ramfail_status_t ramalgn_mkpool(ramalgn_pool_t *pool_arg, ramopt_appetite_t appetite_arg, size_t granularity_arg, const ramalgn_tag_t *tag_arg)
{
   ramfail_status_t e = RAMFAIL_INSANE;

   RAMFAIL_DISALLOWZ(pool_arg);
   RAMFAIL_CONFIRM(RAMFAIL_UNINITIALIZED, ramalgn_theglobals.ramalgng_initflag);

   e = ramalgn_mkpool2(pool_arg, appetite_arg, granularity_arg, tag_arg);
   /* i ensure that 'pool_arg' is zeroed out if something goes wrong. */
   if (RAMFAIL_OK != e)
      memset(pool_arg, 0, sizeof(*pool_arg));

   return e;
}

ramfail_status_t ramalgn_mkpool2(ramalgn_pool_t *pool_arg, ramopt_appetite_t appetite_arg, 
   size_t granularity_arg, const ramalgn_tag_t *tag_arg)
{
   size_t capacity = 0;

   assert(pool_arg);
   RAMFAIL_DISALLOWZ(granularity_arg);
   assert(ramalgn_theglobals.ramalgng_initflag);

   RAMFAIL_RETURN(rampg_mkpool(&pool_arg->ramalgnp_pgpool, appetite_arg));
   capacity = ramalgn_theglobals.ramalgng_footerspec.footer_offset / granularity_arg;
   /* if the capacity doesn't meet certain requirements, then i must inform the caller. */
   /* TODO: why is the slot capacity limit tested here and not in ramslot_mkpool()? */
   if (RAMOPT_MINDENSITY > capacity || RAMSLOT_MAXCAPACITY < capacity)
      return RAMFAIL_RANGE;
   RAMFAIL_RETURN(ramslot_mkpool(&pool_arg->ramalgnp_slotpool, granularity_arg, 
      capacity, &ramalgn_mknode, &ramalgn_rmnode, NULL));
   if (tag_arg)
      pool_arg->ramalgnp_tag = *tag_arg;
   else
      memset(&pool_arg->ramalgnp_tag, 0, sizeof(pool_arg->ramalgnp_tag));

   return RAMFAIL_OK;
}

ramfail_status_t ramalgn_acquire(void **ptr_arg, ramalgn_pool_t *pool_arg)
{
   RAMFAIL_DISALLOWZ(ptr_arg);
   *ptr_arg = NULL;
   RAMFAIL_DISALLOWZ(pool_arg);
   assert(ramalgn_theglobals.ramalgng_initflag);

   RAMFAIL_RETURN(ramslot_acquire(ptr_arg, &pool_arg->ramalgnp_slotpool));

   return RAMFAIL_OK;
}

ramfail_status_t ramalgn_release(void *ptr_arg)
{
   ramalgn_node_t *node = NULL;

   RAMFAIL_CONFIRM(RAMFAIL_UNINITIALIZED, ramalgn_theglobals.ramalgng_initflag);

   RAMFAIL_RETURN(ramalgn_findnode(&node, (char *)ptr_arg));
   RAMFAIL_RETURN(ramslot_release(ptr_arg, &node->ramalgnn_slotnode));

   return RAMFAIL_OK;
}

ramfail_status_t ramalgn_findnode(ramalgn_node_t **node_arg, char *ptr_arg)
{
   ramalgn_footer_t *foot = NULL;
   ramfail_status_t e = RAMFAIL_INSANE;

   assert(node_arg);
   assert(ptr_arg);
   assert(ramalgn_theglobals.ramalgng_initflag);

   e = ramfoot_getstorage((void **)&foot, &ramalgn_theglobals.ramalgng_footerspec, ptr_arg);
   switch (e)
   {
   default:
      RAMFAIL_RETURN(e);
      /* i shouldn't ever get here. */
      return RAMFAIL_INSANE;
   case RAMFAIL_NOTFOUND:
      return e;
   case RAMFAIL_OK:
      break;
   }

   *node_arg = &foot->ramalgnf_node;
   return RAMFAIL_OK;
}

ramfail_status_t ramalgn_chkpool(const ramalgn_pool_t *pool_arg)
{
   RAMFAIL_DISALLOWZ(pool_arg);
   assert(ramalgn_theglobals.ramalgng_initflag);

   RAMFAIL_RETURN(rampg_chkpool(&pool_arg->ramalgnp_pgpool));
   RAMFAIL_RETURN(ramslot_chkpool(&pool_arg->ramalgnp_slotpool));

   return RAMFAIL_OK;
}

ramfail_status_t ramalgn_mknode(ramslot_node_t **node_arg, void **slots_arg, ramslot_pool_t *pool_arg)
{
   ramalgn_pool_t *pool = NULL;
   void *page = NULL;
   ramfail_status_t e = RAMFAIL_INSANE;
   ramalgn_node_t *node = NULL;

   RAMFAIL_DISALLOWZ(node_arg);
   *node_arg = NULL;
   RAMFAIL_DISALLOWZ(slots_arg);
   *slots_arg = NULL;
   RAMFAIL_DISALLOWZ(pool_arg);
   assert(ramalgn_theglobals.ramalgng_initflag);

   RAMMETA_BACKCAST(pool, ramalgn_pool_t, ramalgnp_slotpool, pool_arg);
   RAMFAIL_RETURN(rampg_acquire(&page, &pool->ramalgnp_pgpool));
   e = ramalgn_mknode2(&node, pool, (char *)page);
   if (RAMFAIL_OK == e)
   {
      *node_arg = &node->ramalgnn_slotnode;
      *slots_arg = page;
      return RAMFAIL_OK;
   }
   else
   {
      RAMFAIL_EPICFAIL(rampg_release(page));
      return e;
   }
}

ramfail_status_t ramalgn_mknode2(ramalgn_node_t **node_arg, ramalgn_pool_t *pool_arg, char *page_arg)
{
   ramalgn_footer_t *foot = NULL;

   RAMFAIL_DISALLOWZ(node_arg);
   *node_arg = NULL;
   RAMFAIL_DISALLOWZ(pool_arg);
   RAMFAIL_DISALLOWZ(page_arg);
   assert(ramalgn_theglobals.ramalgng_initflag);

   /* i need to write a footer to the page to ensure that i can get to the pool
    * given any address of of the page. */
   RAMFAIL_RETURN(ramfoot_mkfooter((void **)&foot,
      &ramalgn_theglobals.ramalgng_footerspec, page_arg));
   *node_arg = &foot->ramalgnf_node;

   return RAMFAIL_OK;
}

ramfail_status_t ramalgn_rmnode(ramslot_node_t *node_arg, ramslot_pool_t *pool_arg)
{
   RAMFAIL_DISALLOWZ(node_arg);
   assert(ramalgn_theglobals.ramalgng_initflag);

   RAMFAIL_RETURN(rampg_release(node_arg->ramslotn_slots));
   return RAMFAIL_OK;
}

ramfail_status_t ramalgn_query(ramalgn_pool_t **apool_arg, void *ptr_arg)
{
   ramfail_status_t e = RAMFAIL_INSANE;
   ramalgn_node_t *anode = NULL;
   ramslot_pool_t *spool = NULL;
   ramalgn_pool_t *apool = NULL;

   RAMFAIL_DISALLOWZ(apool_arg);
   *apool_arg = NULL;
   RAMFAIL_DISALLOWZ(ptr_arg);
   RAMFAIL_CONFIRM(RAMFAIL_UNINITIALIZED, ramalgn_theglobals.ramalgng_initflag);

   e = ramalgn_findnode(&anode, (char *)ptr_arg);
   switch (e)
   {
   default:
      RAMFAIL_RETURN(e);
      /* i shouldn't ever get here. */
      return RAMFAIL_INSANE;
   case RAMFAIL_NOTFOUND:
      return e;
   case RAMFAIL_OK:
      break;
   }

   /* i need to find the slot pool before i can back-cast to get the aligned pool. */
   RAMMETA_BACKCAST(spool, ramslot_pool_t, ramslotp_vpool, 
      anode->ramalgnn_slotnode.ramslotn_vnode.ramvecn_vpool);
   RAMMETA_BACKCAST(apool, ramalgn_pool_t, ramalgnp_slotpool, spool);

   *apool_arg = apool;
   return RAMFAIL_OK;
}

ramfail_status_t ramalgn_gettag(const ramalgn_tag_t **tag_arg, const ramalgn_pool_t *apool_arg)
{
   RAMFAIL_DISALLOWZ(tag_arg);
   *tag_arg = NULL;
   RAMFAIL_DISALLOWZ(apool_arg);
   assert(ramalgn_theglobals.ramalgng_initflag);

   *tag_arg = &apool_arg->ramalgnp_tag;
   return RAMFAIL_OK;
}

ramfail_status_t ramalgn_getgranularity(size_t *granularity_arg, const ramalgn_pool_t *apool_arg)
{
   RAMFAIL_DISALLOWZ(granularity_arg);
   *granularity_arg = 0;
   RAMFAIL_DISALLOWZ(apool_arg);
   assert(ramalgn_theglobals.ramalgng_initflag);

   RAMFAIL_RETURN(ramslot_getgranularity(granularity_arg, &apool_arg->ramalgnp_slotpool));

   return RAMFAIL_OK;
}
