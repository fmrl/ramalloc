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

typedef struct ramlazy_chktrashnode
{
   const ramlazy_pool_t *ramlazyctn_lazypool;
} ramlazy_chktrashnode_t;

static ramfail_status_t ramlazy_mkpool2(ramlazy_pool_t *lpool_arg, ramopt_appetite_t appetite_arg, size_t disposalratio_arg);
static ramfail_status_t ramlazy_chktrashnode(void *ptr_arg, void *context_arg);

ramfail_status_t ramlazy_mkpool(ramlazy_pool_t *lpool_arg, ramopt_appetite_t appetite_arg, size_t disposalratio_arg)
{
   ramfail_status_t e = RAMFAIL_INSANE;

   RAMFAIL_DISALLOWZ(lpool_arg);

   e = ramlazy_mkpool2(lpool_arg, appetite_arg, disposalratio_arg);
   if (RAMFAIL_OK == e)
      return RAMFAIL_OK;
   else
   {
      memset(lpool_arg, 0, sizeof(*lpool_arg));
      return e;
   }
}

ramfail_status_t ramlazy_mkpool2(ramlazy_pool_t *lpool_arg, ramopt_appetite_t appetite_arg, size_t disposalratio_arg)
{
   assert(lpool_arg);
   RAMFAIL_DISALLOWZ(disposalratio_arg);

   RAMFAIL_RETURN(ramtra_mktrash(&lpool_arg->ramlazyp_trash));
   RAMFAIL_RETURN(rammux_mkpool(&lpool_arg->ramlazyp_muxpool, appetite_arg));
   lpool_arg->ramlazyp_disposalratio = disposalratio_arg;

   return RAMFAIL_OK;
}

ramfail_status_t ramlazy_rmpool(ramlazy_pool_t *lpool_arg)
{
   RAMFAIL_DISALLOWZ(lpool_arg);

   RAMFAIL_RETURN(ramtra_rmtrash(&lpool_arg->ramlazyp_trash));

   return RAMFAIL_OK;
}

ramfail_status_t ramlazy_acquire(void **newptr_arg, ramlazy_pool_t *lpool_arg, size_t size_arg)
{
   size_t unused = 0;
   ramfail_status_t e = RAMFAIL_INSANE;

   RAMFAIL_DISALLOWZ(newptr_arg);
   *newptr_arg = NULL;
   RAMFAIL_DISALLOWZ(lpool_arg);
   RAMFAIL_DISALLOWZ(size_arg);

   /* first, i need to release anything that's sitting around in the trash. */
   RAMFAIL_RETURN(ramlazy_reclaim(&unused, lpool_arg, lpool_arg->ramlazyp_disposalratio));
   e = rammux_acquire(newptr_arg, &lpool_arg->ramlazyp_muxpool, size_arg);
   switch (e)
   {
   default:
      RAMFAIL_RETURN(e);
      /* i shouldn't ever get here. */
      return RAMFAIL_INSANE;
   case RAMFAIL_RANGE:
      return e;
   case RAMFAIL_OK:
      break;
   }

   return RAMFAIL_OK;
}

ramfail_status_t ramlazy_release(void *ptr_arg)
{
   ramlazy_pool_t *lpool = NULL;
   size_t sz = 0;

   RAMFAIL_DISALLOWZ(ptr_arg);

   RAMFAIL_RETURN(ramlazy_query(&lpool, &sz, ptr_arg));
   /* i zero out the memory so that any dangling pointers don't get access to stale
    * data. */
   memset(ptr_arg, 0, sz);
   /* i push the pointer onto the trash stack; it will be freed on it's home
    * thread with less synchronization and contention than i could manage from 
    * here. */
   RAMFAIL_RETURN(ramtra_push(&lpool->ramlazyp_trash, ptr_arg));

   return RAMFAIL_OK;
}

ramfail_status_t ramlazy_reclaim(size_t *count_arg, ramlazy_pool_t *lpool_arg, size_t goal_arg)
{
   ramfail_status_t e = RAMFAIL_INSANE;
   void *p = NULL;
   size_t i = 0;

   RAMFAIL_DISALLOWZ(count_arg);
   *count_arg = 0;
   RAMFAIL_DISALLOWZ(lpool_arg);
   RAMFAIL_DISALLOWZ(goal_arg);

   while (i < goal_arg && RAMFAIL_OK == (e = ramtra_pop(&p, &lpool_arg->ramlazyp_trash)))
   {
      RAMFAIL_RETURN(rammux_release(p));
      ++i;
   }

   /* it's not a problem if we don't succeed in releasing 'count_arg' items. */
   if (RAMFAIL_NOTFOUND == e || RAMFAIL_OK == e)
   {
      *count_arg = i;
      return RAMFAIL_OK;
   }
   else
      return e;
}

ramfail_status_t ramlazy_flush(ramlazy_pool_t *lpool_arg)
{
   size_t count = 0, unused = 0;

   RAMFAIL_DISALLOWZ(lpool_arg);

   /* the intent is to flush the allocator but in reality, the best i can hope for
    * is to grab an instantaneous count of the number of items in the trash and 
    * use that as a goal for ramlazy_reclaim(). */
   RAMFAIL_RETURN(ramtra_size(&count, &lpool_arg->ramlazyp_trash));
   if (count)
      RAMFAIL_RETURN(ramlazy_reclaim(&unused, lpool_arg, count));

   return RAMFAIL_OK;
}

ramfail_status_t ramlazy_chkpool(const ramlazy_pool_t *lpool_arg)
{
   ramlazy_chktrashnode_t ctn = {0};

   RAMFAIL_DISALLOWZ(lpool_arg);

   RAMFAIL_RETURN(rammux_chkpool(&lpool_arg->ramlazyp_muxpool));
   /* now, i check the trash. i'll be able to verify each pointer in the trash is mine. */
   ctn.ramlazyctn_lazypool = lpool_arg;
   /* it should be safe to cast away the const here. 'ramtra_foreach()' doesn't modify anything
    * and neither does 'ramlazy_chktrashnode().' */
   RAMFAIL_RETURN(ramtra_foreach((ramtra_trash_t *)&lpool_arg->ramlazyp_trash, &ramlazy_chktrashnode, &ctn));

   return RAMFAIL_OK;
}

ramfail_status_t ramlazy_chktrashnode(void *ptr_arg, void *context_arg)
{
   ramlazy_pool_t *lazypool = NULL;
   ramlazy_chktrashnode_t *ctn = NULL;
   size_t unused = 0;

   assert(context_arg);
   ctn = (ramlazy_chktrashnode_t *)context_arg;
   assert(ctn->ramlazyctn_lazypool);

   RAMFAIL_CONFIRM(RAMFAIL_CORRUPT, ptr_arg);
   /* if an address is in the trash, i can still query it, which gives me a good way to
    * check the trash's integrity. */
   RAMFAIL_RETURN(ramlazy_query(&lazypool, &unused, ptr_arg));
   RAMFAIL_CONFIRM(RAMFAIL_CORRUPT, ctn->ramlazyctn_lazypool == lazypool);

   return RAMFAIL_OK;
}

ramfail_status_t ramlazy_query(ramlazy_pool_t **lpool_arg, size_t *size_arg, void *ptr_arg)
{
   rammux_pool_t *muxpool = NULL;
   ramfail_status_t e = RAMFAIL_INSANE;

   RAMFAIL_DISALLOWZ(lpool_arg);
   *lpool_arg = NULL;
   RAMFAIL_DISALLOWZ(size_arg);
   *size_arg = 0;
   RAMFAIL_DISALLOWZ(ptr_arg);

   e = rammux_query(&muxpool, size_arg, ptr_arg);
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

   RAMMETA_BACKCAST(*lpool_arg, ramlazy_pool_t, ramlazyp_muxpool, muxpool);
   return RAMFAIL_OK;
}
