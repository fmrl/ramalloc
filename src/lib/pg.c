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

#include <ramalloc/pg.h>
#include <ramalloc/foot.h>
#include <ramalloc/sys.h>
#include <ramalloc/slot.h>
#include <ramalloc/mem.h>
#include <assert.h>
#include <memory.h>

#if RAMOPT_COMPACT
typedef uint8_t rampg_index_t;
#else
typedef size_t rampg_index_t;
#endif
/* RAMPG_MAXCAPACITY is used to specify the bounds of an array, so it must
 * be a reasonable size for this task regardless of what the integer space
 * can support. */
#define RAMPG_MAXCAPACITY UINT8_MAX

struct rampg_snode;

typedef struct rampg_vnode
{
   ramvec_node_t rampgvn_vnode;
   char *rampgvn_pages;
   rampg_index_t rampgvn_freestksz;
   rampg_index_t rampgvn_freestk[RAMPG_MAXCAPACITY];
   int rampgvn_commitflags[RAMPG_MAXCAPACITY];
} rampg_vnode_t;

typedef struct rampg_slot
{
   ramsig_signature_t rampgg_signature;
   struct rampg_snode *rampgg_snode;
   rampg_vnode_t rampgg_vnode;
} rampg_slot_t;

typedef struct rampg_snode
{
   ramslot_node_t rampgsn_slotnode;
   rampg_slot_t rampgsn_slots[];
} rampg_snode_t;

typedef struct rampg_footer
{
   /* the page footer only needs to store a pointer to it's node. */
   rampg_vnode_t *rampgf_vnode;
} rampg_footer_t;

typedef struct rampg_globals
{
   ramfoot_spec_t rampgg_footerspec;
   size_t rampgg_nodecapacity;
   size_t rampgg_granularity;
   size_t rampgg_pagesize;
   int rampgg_initflag;
} rampg_globals_t;

static ram_reply_t rampg_mkpool2(rampg_pool_t *pool_arg, ramopt_appetite_t appetite_arg);
static ram_reply_t rampg_findvnode(rampg_vnode_t **node_arg, char *ptr_arg);
static ram_reply_t rampg_mkvnode(ramvec_node_t **node_arg, ramvec_pool_t *pool_arg);
static ram_reply_t rampg_initvnode(rampg_vnode_t *node_arg);
static ram_reply_t rampg_rmvnode(rampg_vnode_t *node_arg);
static ram_reply_t rampg_getpage(char **page_arg,
      rampg_vnode_t *vpoolnode_arg, rampg_index_t index_arg);
static ram_reply_t rampg_calcindex(rampg_index_t *index_arg,
      const rampg_vnode_t *vpoolnode_arg, const char *page_arg);
static ram_reply_t rampg_chkvnode(const ramvec_node_t *node_arg);
static ram_reply_t rampg_mksnode(ramslot_node_t **node_arg, void **slots_arg, ramslot_pool_t *pool_arg);
static ram_reply_t rampg_rmsnode(ramslot_node_t *node_arg);
static ram_reply_t rampg_initslot(void *slot_arg, ramslot_node_t *node_arg);
#define RAMPG_ISFULL(Node) (0 == (Node)->rampgvn_freestksz)
#define RAMPG_ISEMPTY(Node) (rampg_theglobals.rampgg_nodecapacity == (Node)->rampgvn_freestksz)

static rampg_globals_t rampg_theglobals;

ram_reply_t rampg_initialize()
{
   if (!rampg_theglobals.rampgg_initflag)
   {
      rampg_globals_t stage = {0};
      size_t mmapgran = 0;

      RAM_FAIL_TRAP(rammem_pagesize(&stage.rampgg_pagesize));
      RAM_FAIL_TRAP(rammem_mmapgran(&mmapgran));
      /* i am the page allocator, so i have access to the entire page 
       * (i.e. 'page_size' == 'writable_zone') */
      RAMFOOT_MKSPEC(&stage.rampgg_footerspec, rampg_footer_t, 
            stage.rampgg_pagesize, "PAGE");
      /* the node capacity is the number of pages a node keeps track of. */
      stage.rampgg_nodecapacity = mmapgran / stage.rampgg_pagesize;
      RAM_FAIL_EXPECT(RAM_REPLY_UNSUPPORTED, 
            stage.rampgg_nodecapacity <= RAMPG_MAXCAPACITY);
      stage.rampgg_granularity = stage.rampgg_footerspec.footer_offset;
      stage.rampgg_initflag = 1;

      rampg_theglobals = stage;
   }

   return RAM_REPLY_OK;
}

ram_reply_t rampg_mkpool(rampg_pool_t *pool_arg, ramopt_appetite_t appetite_arg)
{
   ram_reply_t e = RAM_REPLY_INSANE;

   RAM_FAIL_NOTNULL(pool_arg);
   RAM_FAIL_EXPECT(RAM_REPLY_INCONSISTENT, rampg_theglobals.rampgg_initflag);

   e = rampg_mkpool2(pool_arg, appetite_arg);
   /* i ensure that 'pool_arg' is zeroed out if something goes wrong. */
   if (RAM_REPLY_OK != e)
      memset(pool_arg, 0, sizeof(*pool_arg));

   return e;
}

ram_reply_t rampg_mkpool2(rampg_pool_t *pool_arg, ramopt_appetite_t appetite_arg)
{
   size_t snodecapacity = 0;
   size_t mmapgran = 0;

   assert(pool_arg != NULL);
   assert(rampg_theglobals.rampgg_initflag);

   RAM_FAIL_TRAP(ramvec_mkpool(&pool_arg->rampgp_vpool, rampg_theglobals.rampgg_nodecapacity, 
      &rampg_mkvnode));
   pool_arg->rampgp_appetite = appetite_arg;

   /* slot pool initialization: i must determine how many slots i can store
    * with a slot allocator in the amount of space specified by the virtual
    * memory mapping granularity. */
   RAM_FAIL_TRAP(rammem_mmapgran(&mmapgran));
   snodecapacity = (mmapgran - sizeof(rampg_snode_t))
         / sizeof(rampg_slot_t);
   RAM_FAIL_TRAP(ramslot_mkpool(&pool_arg->rampgp_slotpool, sizeof(rampg_slot_t),
      snodecapacity, rampg_mksnode, rampg_rmsnode, rampg_initslot));
   RAM_FAIL_TRAP(ramsig_init(&pool_arg->rampgp_slotsig, "SLOT"));

   return RAM_REPLY_OK;
}

ram_reply_t rampg_acquire(void **ptr_arg, rampg_pool_t *pool_arg)
{
   rampg_index_t idx = 0;
   char *page = NULL;
   rampg_vnode_t *vnode = NULL;
   rampg_footer_t *foot = NULL;
   ramvec_node_t *p = NULL;

   RAM_FAIL_NOTNULL(ptr_arg);
   *ptr_arg = NULL;
   RAM_FAIL_NOTNULL(pool_arg);
   assert(rampg_theglobals.rampgg_initflag);

   /* first, i acquire a memory object from the next available node in the pool. */
   RAM_FAIL_TRAP(ramvec_getnode(&p, &pool_arg->rampgp_vpool));
   RAMMETA_BACKCAST(vnode, rampg_vnode_t, rampgvn_vnode, p);
   /* ramvec_getnode() should never return a full node. */
   assert(!RAMPG_ISFULL(vnode));
   /* ramvec_getnode() should never return someone else's node. */
   assert(&pool_arg->rampgp_vpool == vnode->rampgvn_vnode.ramvecn_vpool);

   idx = vnode->rampgvn_freestk[vnode->rampgvn_freestksz - 1];
   RAM_FAIL_TRAP(rampg_getpage(&page, vnode, idx));
   RAM_FAIL_TRAP(ramsys_commit(page));

   /* i mark the page as committed. */
   vnode->rampgvn_commitflags[idx] = 1;

   /* at this point, if something goes wrong, the node is inconsistent and
    * there's no longer any hope for recovery. */

   /* i need to shorten the free list; it's not necessary to clear out the old value but it
    * is helpful to see in the debugger. */
   vnode->rampgvn_freestk[--vnode->rampgvn_freestksz] = 0xfe;

   /* i need to write a footer to the page to ensure that i can get to the pool
    * given any address of of the page. */
   /* TODO: should the footer spec be moved out of pool and into this? */
   RAM_FAIL_PANIC(ramfoot_mkfooter((void **)&foot, &rampg_theglobals.rampgg_footerspec, page));
   foot->rampgf_vnode = vnode;

   /* i finalize the acquisition by updating the pool state. */
   RAM_FAIL_PANIC(ramvec_acquire(&vnode->rampgvn_vnode, RAMPG_ISFULL(vnode)));

   /* i zero-out the memory, if that behavior is desired. */
#if RAMOPT_ZEROMEM
   memset(page, 0, rampg_theglobals.rampgg_granularity);
#endif

   *ptr_arg = page;
   return RAM_REPLY_OK;
}

ram_reply_t rampg_release(void *ptr_arg)
{
   rampg_vnode_t *vnode = NULL;
   rampg_pool_t *pool = NULL;
   rampg_index_t idx = 0;
   int emptyflag = 0;

   RAM_FAIL_NOTNULL(ptr_arg);
   RAM_FAIL_EXPECT(RAM_REPLY_INCONSISTENT, rampg_theglobals.rampgg_initflag);

   RAM_FAIL_TRAP(rampg_findvnode(&vnode, (char *)ptr_arg));
   RAMMETA_BACKCAST(pool, rampg_pool_t, rampgp_vpool, vnode->rampgvn_vnode.ramvecn_vpool);
   RAM_FAIL_TRAP(rampg_calcindex(&idx, vnode, ptr_arg));
   /* depending upon the release strategy, i either return the page to the system
    * or i wipe it and deny access to it. */
   if (RAMOPT_FRUGAL == pool->rampgp_appetite)
   {
      assert(vnode->rampgvn_commitflags[idx]);
      RAM_FAIL_TRAP(ramsys_decommit(ptr_arg));
      vnode->rampgvn_commitflags[idx] = 0;
   }
   else
   {
      assert(RAMOPT_GREEDY == pool->rampgp_appetite);
      RAM_FAIL_TRAP(ramsys_reset(ptr_arg));

#if RAMOPT_MARKFREED
      /* it's helpful to see signature bytes for destroyed memory when
       * debugging. */
      memset(ptr_arg, RAMOPT_MARKFREED,
            rampg_theglobals.rampgg_granularity);
#endif

   }

   /* at this point, if something goes wrong, the vnode might be inconsistent and
      * there's no longer any hope for recovery. */

   /* i now add the page's index to the free list. */
   assert(vnode->rampgvn_freestksz < RAMPG_MAXCAPACITY);
   vnode->rampgvn_freestk[vnode->rampgvn_freestksz++] = idx;
   /* now, i pass control to ramvec_release() to finalize the pool state. if the vnode is
    * empty, i'll discard the vnode. */
   emptyflag = RAMPG_ISEMPTY(vnode);
   RAM_FAIL_PANIC(ramvec_release(&vnode->rampgvn_vnode, 1 == vnode->rampgvn_freestksz, emptyflag));
   if (emptyflag)
      RAM_FAIL_PANIC(rampg_rmvnode(vnode));

   return RAM_REPLY_OK;
}

ram_reply_t rampg_findvnode(rampg_vnode_t **node_arg, char *ptr_arg)
{
   rampg_footer_t *foot = NULL;

   assert(rampg_theglobals.rampgg_initflag);
   assert(node_arg != NULL);
   assert(ptr_arg != NULL);

   RAM_FAIL_TRAP(ramfoot_getstorage((void **)&foot, &rampg_theglobals.rampgg_footerspec, ptr_arg));

   *node_arg = foot->rampgf_vnode;
   assert(*node_arg != NULL);
   return RAM_REPLY_OK;
}

ram_reply_t rampg_mkvnode(ramvec_node_t **node_arg, ramvec_pool_t *pool_arg)
{
   rampg_slot_t *slot = NULL;
   rampg_pool_t *pool = NULL;
   ram_reply_t e = RAM_REPLY_INSANE;

   assert(rampg_theglobals.rampgg_initflag);
   assert(node_arg != NULL);

   RAMMETA_BACKCAST(pool, rampg_pool_t, rampgp_vpool, pool_arg);
   RAM_FAIL_TRAP(ramslot_acquire((void **)&slot, &pool->rampgp_slotpool));
   e = rampg_initvnode(&slot->rampgg_vnode);
   if (RAM_REPLY_OK == e)
   {
      *node_arg = &slot->rampgg_vnode.rampgvn_vnode;
      return RAM_REPLY_OK;
   }
   else
   {
      RAM_FAIL_PANIC(rampg_rmvnode(&slot->rampgg_vnode));
      return e;
   }
}

ram_reply_t rampg_initvnode(rampg_vnode_t *node_arg)
{
   size_t i = 0;

   assert(rampg_theglobals.rampgg_initflag);
   assert(node_arg != NULL);

   memset(node_arg, 0, sizeof(node_arg));
   /* i reserve the page because there doesn't seem to be a need until memory is actually
    * acquired. this still uses address space but preserves hardware for those pages that 
    * are actually in use. */
   RAM_FAIL_TRAP(ramsys_reserve(&node_arg->rampgvn_pages));
   /* therefore, all commit flags start out reset. */
   memset(node_arg->rampgvn_commitflags, 0, sizeof(node_arg->rampgvn_commitflags));
   /* this isn't absolutely necessary but it's helpful to not see garbage data in the debugger. */
   memset(node_arg->rampgvn_freestk, 0xff, sizeof(node_arg->rampgvn_freestk));

   /* now, i initialize the free list. all memory objects start out free, so i'll fill the
    * entire free list. */
   for (i = 0; i < rampg_theglobals.rampgg_nodecapacity; ++i)
      node_arg->rampgvn_freestk[i] = i;
   node_arg->rampgvn_freestksz = rampg_theglobals.rampgg_nodecapacity;

   return RAM_REPLY_OK;
}

ram_reply_t rampg_rmvnode(rampg_vnode_t *node_arg)
{
   rampg_pool_t *pool = NULL;
   rampg_slot_t *slot = NULL;

   assert(rampg_theglobals.rampgg_initflag);
   assert(node_arg != NULL);

   RAMMETA_BACKCAST(pool, rampg_pool_t, rampgp_vpool, node_arg->rampgvn_vnode.ramvecn_vpool);
   RAMMETA_BACKCAST(slot, rampg_slot_t, rampgg_vnode, node_arg);
   RAM_FAIL_EXPECT(RAM_REPLY_CORRUPT,
      0 == RAMSIG_CMP(slot->rampgg_signature, pool->rampgp_slotsig));
   RAM_FAIL_TRAP(ramsys_release(node_arg->rampgvn_pages));
   RAM_FAIL_TRAP(ramslot_release(slot, &slot->rampgg_snode->rampgsn_slotnode));
   return RAM_REPLY_OK;
}

ram_reply_t rampg_getpage(char **page_arg,
      rampg_vnode_t *vpoolnode_arg, rampg_index_t index_arg)
{
   RAM_FAIL_NOTNULL(page_arg);
   *page_arg = NULL;
   RAM_FAIL_EXPECT(RAM_REPLY_INCONSISTENT,
         rampg_theglobals.rampgg_initflag);
   RAM_FAIL_NOTNULL(vpoolnode_arg);
   RAM_FAIL_EXPECT(RAM_REPLY_RANGEFAIL, index_arg != RAMPG_MAXCAPACITY);

   *page_arg =
         &vpoolnode_arg->rampgvn_pages[index_arg *
                                       rampg_theglobals.rampgg_pagesize];

   return RAM_REPLY_OK;
}

ram_reply_t rampg_calcindex(rampg_index_t *index_arg,
       const rampg_vnode_t *vpoolnode_arg, const char *page_arg)
{
   int ispage = 0;

   RAM_FAIL_NOTNULL(index_arg);
   *index_arg = RAMPG_MAXCAPACITY;
   RAM_FAIL_EXPECT(RAM_REPLY_INCONSISTENT,
         rampg_theglobals.rampgg_initflag);
   RAM_FAIL_NOTNULL(vpoolnode_arg);
   RAM_FAIL_TRAP(rammem_ispage(&ispage, page_arg));
   RAM_FAIL_EXPECT(RAM_REPLY_DISALLOWED, ispage);
   RAM_FAIL_EXPECT(RAM_REPLY_RANGEFAIL, vpoolnode_arg->rampgvn_pages <= page_arg);
   RAM_FAIL_EXPECT(RAM_REPLY_RANGEFAIL, vpoolnode_arg->rampgvn_pages +
         rampg_theglobals.rampgg_pagesize *
         rampg_theglobals.rampgg_nodecapacity > page_arg);

   *index_arg = (page_arg - vpoolnode_arg->rampgvn_pages) /
         rampg_theglobals.rampgg_pagesize;

   return RAM_REPLY_OK;
}

ram_reply_t rampg_chkpool(const rampg_pool_t *pool_arg)
{
   RAM_FAIL_NOTNULL(pool_arg);
   
   assert(rampg_theglobals.rampgg_initflag);
   RAM_FAIL_TRAP(ramslot_chkpool(&pool_arg->rampgp_slotpool));
   RAM_FAIL_TRAP(ramvec_chkpool(&pool_arg->rampgp_vpool, &rampg_chkvnode));

   return RAM_REPLY_OK;
}

ram_reply_t rampg_chkvnode(const ramvec_node_t *node_arg)
{
   const rampg_vnode_t *node = NULL;
   size_t i = 0;
   int ispage = 0;

   assert(node_arg != NULL);

   RAMMETA_BACKCAST(node, const rampg_vnode_t, rampgvn_vnode, node_arg);

   /* the node cannot be empty. */
   RAM_FAIL_EXPECT(RAM_REPLY_CORRUPT, !RAMPG_ISEMPTY(node));
   /* the base address should be on a page boundary. */
   RAM_FAIL_TRAP(rammem_ispage(&ispage, node->rampgvn_pages));
   RAM_FAIL_EXPECT(RAM_REPLY_CORRUPT, ispage);
   /* the free list length should not exceed the node capacity. */
   RAM_FAIL_EXPECT(RAM_REPLY_CORRUPT, node->rampgvn_freestksz <= rampg_theglobals.rampgg_nodecapacity);
   /* the free list should only contain indices on the range of [0, ramvecvp_nodecapacity). */
   for (i = 0; i < node->rampgvn_freestksz; ++i)
      RAM_FAIL_EXPECT(RAM_REPLY_CORRUPT, node->rampgvn_freestk[i] < rampg_theglobals.rampgg_nodecapacity);
   /* the commit flags should only contain values 0 or 1 */
   for (i = 0; i < rampg_theglobals.rampgg_nodecapacity; ++i)
   {
      RAM_FAIL_EXPECT(RAM_REPLY_CORRUPT, 
         0 == node->rampgvn_commitflags[i] || 1 == node->rampgvn_commitflags[i]);
   }

   return RAM_REPLY_OK;
}

ram_reply_t rampg_mksnode(ramslot_node_t **node_arg, void **slots_arg, ramslot_pool_t *pool_arg)
{
   rampg_snode_t *snode = NULL;

   RAM_FAIL_NOTNULL(node_arg);
   *node_arg = NULL;
   RAM_FAIL_NOTNULL(slots_arg);
   *slots_arg = NULL;
   RAM_FAIL_NOTNULL(pool_arg);
   assert(rampg_theglobals.rampgg_initflag);

   RAM_FAIL_TRAP(ramsys_bulkalloc((char **)&snode));
   *slots_arg = snode->rampgsn_slots;
   *node_arg = &snode->rampgsn_slotnode;
   return RAM_REPLY_OK;
}

ram_reply_t rampg_rmsnode(ramslot_node_t *node_arg)
{
   rampg_snode_t *snode = NULL;

   RAM_FAIL_NOTNULL(node_arg);
   assert(rampg_theglobals.rampgg_initflag);

   RAMMETA_BACKCAST(snode, rampg_snode_t, rampgsn_slotnode, node_arg);
   RAM_FAIL_TRAP(ramsys_release((char *)snode));

   return RAM_REPLY_OK;
}

ram_reply_t rampg_initslot(void *slot_arg, ramslot_node_t *node_arg)
{
   rampg_snode_t *snode = NULL;
   rampg_slot_t *slot = NULL;
   rampg_pool_t *pool = NULL;
   ramslot_pool_t *slotpool = NULL;

   RAM_FAIL_NOTNULL(slot_arg);
   RAM_FAIL_NOTNULL(node_arg);
   assert(rampg_theglobals.rampgg_initflag);

   RAMMETA_BACKCAST(snode, rampg_snode_t, rampgsn_slotnode, node_arg);
   RAMMETA_BACKCAST(slotpool, ramslot_pool_t, ramslotp_vpool, snode->rampgsn_slotnode.ramslotn_vnode.ramvecn_vpool);
   RAMMETA_BACKCAST(pool, rampg_pool_t, rampgp_slotpool, slotpool);
   slot = (rampg_slot_t *)slot_arg;
   slot->rampgg_signature = pool->rampgp_slotsig;
   slot->rampgg_snode = snode;

   return RAM_REPLY_OK;
}

ram_reply_t rampg_getgranularity(size_t *granularity_arg)
{
   RAM_FAIL_NOTNULL(granularity_arg);
   *granularity_arg = 0;
   RAM_FAIL_EXPECT(RAM_REPLY_INCONSISTENT, rampg_theglobals.rampgg_initflag);

   *granularity_arg = rampg_theglobals.rampgg_granularity;

   return RAM_REPLY_OK;
}
