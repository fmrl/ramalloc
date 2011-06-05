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

#include <ramalloc/lazy.h>
#include <ramalloc/cast.h>
#include <string.h>

typedef struct ramlazy_chktrashnode
{
   const ramlazy_pool_t *ramlazyctn_lazypool;
} ramlazy_chktrashnode_t;

static ram_reply_t ramlazy_mkpool2(ramlazy_pool_t *lpool_arg, rampg_appetite_t appetite_arg, size_t disposalratio_arg);
static ram_reply_t ramlazy_chktrashnode(void *ptr_arg, void *context_arg);

ram_reply_t ramlazy_mkpool(ramlazy_pool_t *lpool_arg, rampg_appetite_t appetite_arg, size_t disposalratio_arg)
{
   ram_reply_t e = RAM_REPLY_INSANE;

   RAM_FAIL_NOTNULL(lpool_arg);

   e = ramlazy_mkpool2(lpool_arg, appetite_arg, disposalratio_arg);
   if (RAM_REPLY_OK == e)
      return RAM_REPLY_OK;
   else
   {
      memset(lpool_arg, 0, sizeof(*lpool_arg));
      return e;
   }
}

ram_reply_t ramlazy_mkpool2(ramlazy_pool_t *lpool_arg, rampg_appetite_t appetite_arg, size_t disposalratio_arg)
{
   assert(lpool_arg != NULL);
   RAM_FAIL_NOTZERO(disposalratio_arg);

   RAM_FAIL_TRAP(ramtra_mktrash(&lpool_arg->ramlazyp_trash));
   RAM_FAIL_TRAP(rammux_mkpool(&lpool_arg->ramlazyp_muxpool, appetite_arg));
   lpool_arg->ramlazyp_disposalratio = disposalratio_arg;

   return RAM_REPLY_OK;
}

ram_reply_t ramlazy_rmpool(ramlazy_pool_t *lpool_arg)
{
   RAM_FAIL_NOTNULL(lpool_arg);

   RAM_FAIL_TRAP(ramtra_rmtrash(&lpool_arg->ramlazyp_trash));

   return RAM_REPLY_OK;
}

ram_reply_t ramlazy_acquire(void **newptr_arg, ramlazy_pool_t *lpool_arg, size_t size_arg)
{
   size_t unused = 0;
   ram_reply_t e = RAM_REPLY_INSANE;

   RAM_FAIL_NOTNULL(newptr_arg);
   *newptr_arg = NULL;
   RAM_FAIL_NOTNULL(lpool_arg);
   RAM_FAIL_NOTZERO(size_arg);

   /* first, i need to release anything that's sitting around in the trash. */
   RAM_FAIL_TRAP(ramlazy_reclaim(&unused, lpool_arg, lpool_arg->ramlazyp_disposalratio));
   e = rammux_acquire(newptr_arg, &lpool_arg->ramlazyp_muxpool, size_arg);
   switch (e)
   {
   default:
      RAM_FAIL_TRAP(e);
      /* i shouldn't ever get here. */
      return RAM_REPLY_INSANE;
   case RAM_REPLY_RANGEFAIL:
      return e;
   case RAM_REPLY_OK:
      break;
   }

   return RAM_REPLY_OK;
}

ram_reply_t ramlazy_release(void *ptr_arg)
{
   ramlazy_pool_t *lpool = NULL;
   size_t sz = 0;

   RAM_FAIL_NOTNULL(ptr_arg);

   RAM_FAIL_TRAP(ramlazy_query(&lpool, &sz, ptr_arg));
#if RAM_WANT_MARKFREED
   memset(ptr_arg, RAM_WANT_MARKFREED, sz);
#endif
   /* i push the pointer onto the trash stack; it will be freed on it's home
    * thread with less synchronization and contention than i could manage from 
    * here. */
   RAM_FAIL_TRAP(ramtra_push(&lpool->ramlazyp_trash, ptr_arg));

   return RAM_REPLY_OK;
}

ram_reply_t ramlazy_reclaim(size_t *count_arg, ramlazy_pool_t *lpool_arg, size_t goal_arg)
{
   ram_reply_t e = RAM_REPLY_INSANE;
   void *p = NULL;
   size_t i = 0;

   RAM_FAIL_NOTNULL(count_arg);
   *count_arg = 0;
   RAM_FAIL_NOTNULL(lpool_arg);
   RAM_FAIL_NOTZERO(goal_arg);

   while (i < goal_arg && RAM_REPLY_OK == (e = ramtra_pop(&p, &lpool_arg->ramlazyp_trash)))
   {
      RAM_FAIL_TRAP(rammux_release(p));
      ++i;
   }

   /* it's not a problem if we don't succeed in releasing 'count_arg' items. */
   if (RAM_REPLY_NOTFOUND == e || RAM_REPLY_OK == e)
   {
      *count_arg = i;
      return RAM_REPLY_OK;
   }
   else
      return e;
}

ram_reply_t ramlazy_flush(ramlazy_pool_t *lpool_arg)
{
   size_t count = 0, unused = 0;

   RAM_FAIL_NOTNULL(lpool_arg);

   /* the intent is to flush the allocator but in reality, the best i can hope for
    * is to grab an instantaneous count of the number of items in the trash and 
    * use that as a goal for ramlazy_reclaim(). */
   RAM_FAIL_TRAP(ramtra_size(&count, &lpool_arg->ramlazyp_trash));
   if (count)
      RAM_FAIL_TRAP(ramlazy_reclaim(&unused, lpool_arg, count));

   return RAM_REPLY_OK;
}

ram_reply_t ramlazy_chkpool(const ramlazy_pool_t *lpool_arg)
{
   ramlazy_chktrashnode_t ctn = {0};

   RAM_FAIL_NOTNULL(lpool_arg);

   RAM_FAIL_TRAP(rammux_chkpool(&lpool_arg->ramlazyp_muxpool));
   /* now, i check the trash. i'll be able to verify each pointer in the trash is mine. */
   ctn.ramlazyctn_lazypool = lpool_arg;
   /* it should be safe to cast away the const here. 'ramtra_foreach()' doesn't modify anything
    * and neither does 'ramlazy_chktrashnode().' */
   RAM_FAIL_TRAP(ramtra_foreach((ramtra_trash_t *)&lpool_arg->ramlazyp_trash, &ramlazy_chktrashnode, &ctn));

   return RAM_REPLY_OK;
}

ram_reply_t ramlazy_chktrashnode(void *ptr_arg, void *context_arg)
{
   ramlazy_pool_t *lazypool = NULL;
   ramlazy_chktrashnode_t *ctn = NULL;
   size_t unused = 0;

   assert(context_arg != NULL);
   ctn = (ramlazy_chktrashnode_t *)context_arg;
   assert(ctn->ramlazyctn_lazypool != NULL);

   RAM_FAIL_EXPECT(RAM_REPLY_CORRUPT, ptr_arg);
   /* if an address is in the trash, i can still query it, which gives me a good way to
    * check the trash's integrity. */
   RAM_FAIL_TRAP(ramlazy_query(&lazypool, &unused, ptr_arg));
   RAM_FAIL_EXPECT(RAM_REPLY_CORRUPT, ctn->ramlazyctn_lazypool == lazypool);

   return RAM_REPLY_OK;
}

ram_reply_t ramlazy_query(ramlazy_pool_t **lpool_arg, size_t *size_arg, void *ptr_arg)
{
   rammux_pool_t *muxpool = NULL;
   ram_reply_t e = RAM_REPLY_INSANE;

   RAM_FAIL_NOTNULL(lpool_arg);
   *lpool_arg = NULL;
   RAM_FAIL_NOTNULL(size_arg);
   *size_arg = 0;
   RAM_FAIL_NOTNULL(ptr_arg);

   e = rammux_query(&muxpool, size_arg, ptr_arg);
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

   *lpool_arg = RAM_CAST_STRUCTBASE(ramlazy_pool_t, ramlazyp_muxpool,
         muxpool);
   return RAM_REPLY_OK;
}
