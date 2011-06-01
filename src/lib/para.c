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

#include <ramalloc/para.h>
#include <ramalloc/mem.h>
#include <string.h>

typedef struct rampara_tls
{
   rampara_pool_t *ramparat_backref;
   ramlazy_pool_t ramparat_lazypool;
} rampara_tls_t;

static ram_reply_t rampara_mkpool2(rampara_pool_t *parapool_arg, ramopt_appetite_t appetite_arg, size_t disposalratio_arg);
static ram_reply_t rampara_mktls(rampara_tls_t **newtls_arg, rampara_pool_t *parapool_arg);
static ram_reply_t rampara_rcltls(rampara_tls_t **tls_arg, rampara_pool_t *parapool_arg);
static ram_reply_t rampara_querytls(rampara_tls_t **tls_arg, size_t *size_arg, void *ptr_arg);

ram_reply_t rampara_mkpool(rampara_pool_t *parapool_arg, ramopt_appetite_t appetite_arg, size_t disposalratio_arg)
{
   ram_reply_t e = RAM_REPLY_INSANE;

   RAM_FAIL_NOTNULL(parapool_arg);

   e = rampara_mkpool2(parapool_arg, appetite_arg, disposalratio_arg);
   if (RAM_REPLY_OK == e)
      return RAM_REPLY_OK;
   else
   {
      memset(parapool_arg, 0, sizeof(*parapool_arg));
      return e;
   }
}

ram_reply_t rampara_mkpool2(rampara_pool_t *parapool_arg, ramopt_appetite_t appetite_arg, size_t disposalratio_arg)
{
   assert(parapool_arg != NULL);
   RAM_FAIL_NOTZERO(disposalratio_arg);

   RAM_FAIL_TRAP(ramtls_mkkey(&parapool_arg->ramparap_tlskey));
   parapool_arg->ramparap_appetite = appetite_arg;
   parapool_arg->ramparap_reclaimratio = disposalratio_arg;

   return RAM_REPLY_OK;
}

ram_reply_t rampara_rmpool(rampara_pool_t *parapool_arg)
{
   RAM_FAIL_NOTNULL(parapool_arg);

   RAM_FAIL_TRAP(ramtls_rmkey(parapool_arg->ramparap_tlskey));
   memset(parapool_arg, 0, sizeof(*parapool_arg));

   return RAM_REPLY_OK;
}

ram_reply_t rampara_acquire(void **newptr_arg, rampara_pool_t *parapool_arg, size_t size_arg)
{
   rampara_tls_t *tls = NULL;
   ram_reply_t e = RAM_REPLY_INSANE;

   RAM_FAIL_NOTNULL(newptr_arg);
   *newptr_arg = NULL;
   RAM_FAIL_NOTNULL(parapool_arg);
   RAM_FAIL_NOTZERO(size_arg);

   RAM_FAIL_TRAP(rampara_rcltls(&tls, parapool_arg));
   e = ramlazy_acquire(newptr_arg, &tls->ramparat_lazypool, size_arg);
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

ram_reply_t rampara_reclaim(size_t *count_arg, rampara_pool_t *parapool_arg, size_t goal_arg)
{
   rampara_tls_t *tls = NULL;

   RAM_FAIL_NOTNULL(count_arg);
   *count_arg = 0;
   RAM_FAIL_NOTNULL(parapool_arg);
   RAM_FAIL_NOTZERO(goal_arg);

   RAM_FAIL_TRAP(rampara_rcltls(&tls, parapool_arg));
   RAM_FAIL_TRAP(ramlazy_reclaim(count_arg, &tls->ramparat_lazypool, goal_arg));

   return RAM_REPLY_OK;
}

ram_reply_t rampara_flush(rampara_pool_t *parapool_arg)
{
   rampara_tls_t *tls = NULL;

   RAM_FAIL_NOTNULL(parapool_arg);

   RAM_FAIL_TRAP(rampara_rcltls(&tls, parapool_arg));
   RAM_FAIL_TRAP(ramlazy_flush(&tls->ramparat_lazypool));

   return RAM_REPLY_OK;
}

ram_reply_t rampara_chkpool(const rampara_pool_t *parapool_arg)
{
   rampara_tls_t *tls = NULL;

   RAM_FAIL_NOTNULL(parapool_arg);

   /* TODO: i'm not happy with the following cast; i really need to reevaluate the 
    * usefullness of *const* in standard C. */
   RAM_FAIL_TRAP(rampara_rcltls(&tls, (rampara_pool_t *)parapool_arg));
   RAM_FAIL_TRAP(ramlazy_chkpool(&tls->ramparat_lazypool));

   return RAM_REPLY_OK;
}

ram_reply_t rampara_query(rampara_pool_t **parapool_arg, size_t *size_arg, void *ptr_arg)
{
   rampara_tls_t *tls = NULL;
   ram_reply_t e = RAM_REPLY_INSANE;

   RAM_FAIL_NOTNULL(parapool_arg);
   *parapool_arg = NULL;
   RAM_FAIL_NOTNULL(size_arg);
   *size_arg = 0;
   RAM_FAIL_NOTNULL(ptr_arg);

   e = rampara_querytls(&tls, size_arg, ptr_arg);
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

   *parapool_arg = tls->ramparat_backref;
   return RAM_REPLY_OK;
}

ram_reply_t rampara_mktls(rampara_tls_t **newtls_arg, rampara_pool_t *parapool_arg)
{
   rampara_tls_t *p = NULL;
   ram_reply_t e = RAM_REPLY_INSANE;

   RAM_FAIL_NOTNULL(newtls_arg);
   *newtls_arg = NULL;
   RAM_FAIL_NOTNULL(parapool_arg);

   /* i'm not terribly concerned about the use of a heap allocator here. it's an allocation
    * that will occur once per thread, which isn't going to impact performance. if the caller
    * is creating and destroying threads with frequency, i'm certainly not going to be the 
    * biggest performance headache. */
   p = rammem_supmalloc(sizeof(*p));
   RAM_FAIL_EXPECT(RAM_REPLY_RESOURCEFAIL, p != NULL);
   memset(p, 0, sizeof(*p));
   p->ramparat_backref = parapool_arg;
   e = ramlazy_mkpool(&p->ramparat_lazypool, parapool_arg->ramparap_appetite, parapool_arg->ramparap_reclaimratio);
   if (RAM_REPLY_OK == e)
   {
      *newtls_arg = p;
      return RAM_REPLY_OK;
   }
   else
   {
      rammem_supfree(p);
      return e;
   }
}

ram_reply_t rampara_rcltls(rampara_tls_t **tls_arg, rampara_pool_t *parapool_arg)
{
   rampara_tls_t *tls = NULL;
   void *p = NULL;

   RAM_FAIL_NOTNULL(tls_arg);
   *tls_arg = NULL;
   RAM_FAIL_NOTNULL(parapool_arg);

   RAM_FAIL_TRAP(ramtls_rcl(&p, parapool_arg->ramparap_tlskey));
   tls = (rampara_tls_t *)p;
   if (NULL == tls)
   {
      /* BUG: the pool is leaked here if ramtls_sto() fails-- also when threads are destroyed. */
      RAM_FAIL_TRAP(rampara_mktls(&tls, parapool_arg));
      RAM_FAIL_TRAP(ramtls_sto(parapool_arg->ramparap_tlskey, tls));
   }

   *tls_arg = tls;
   return RAM_REPLY_OK;
}

ram_reply_t rampara_querytls(rampara_tls_t **tls_arg, size_t *size_arg, void *ptr_arg)
{
   ramlazy_pool_t *lazypool = NULL;
   ram_reply_t e = RAM_REPLY_INSANE;

   assert(tls_arg != NULL);
   assert(size_arg != NULL);
   assert(ptr_arg != NULL);

   e = ramlazy_query(&lazypool, size_arg, ptr_arg);
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

   RAMMETA_BACKCAST(*tls_arg, rampara_tls_t, ramparat_lazypool, lazypool);
   return RAM_REPLY_OK;
}
