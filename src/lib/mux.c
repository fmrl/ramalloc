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

#include <ramalloc/mux.h>
#include "ramalloc/cast.h"
#include <memory.h>

static ramsig_signature_t rammux_thesignature = { RAMSIG_MKUINT32('M', 'U', 'X', 'P') };

static ram_reply_t rammux_mkpool2(rammux_pool_t *mpool_arg, rampg_appetite_t appetite_arg);
static ram_reply_t rammux_getalgnpool(ramalgn_pool_t **apool_arg, size_t size_arg, rammux_pool_t *mpool_arg);

ram_reply_t rammux_mkpool(rammux_pool_t *mpool_arg, rampg_appetite_t appetite_arg)
{
   ram_reply_t e = RAM_REPLY_INSANE;

   RAM_FAIL_NOTNULL(mpool_arg);

   e = rammux_mkpool2(mpool_arg, appetite_arg);
   if (RAM_REPLY_OK == e)
      return e;
   else
   {
      memset(mpool_arg, 0, sizeof(*mpool_arg));
      return e;
   }
}

ram_reply_t rammux_mkpool2(rammux_pool_t *mpool_arg, rampg_appetite_t appetite_arg)
{
   assert(mpool_arg != NULL);

   memset(mpool_arg, 0, sizeof(*mpool_arg));
   /* it doesn't seem like increments smaller than the address word make sense, given the
    * amount of waste that would result. smaller sizes smaller than this should be pooled 
    * with an array and indices instead. */
   mpool_arg->rammuxp_step = sizeof(void *);
   mpool_arg->rammuxp_appetite = appetite_arg;
   /* the tag i put into all of my aligned pools will be a signature and my address. this
    * is intended to be enough information to make a safe cast. */
   mpool_arg->rammuxp_tag.ramalgnt_values[0] = rammux_thesignature.ramsigs_n;
   mpool_arg->rammuxp_tag.ramalgnt_values[1] = (uintptr_t)mpool_arg;

   return RAM_REPLY_OK;
}

ram_reply_t rammux_acquire(void **newptr_arg, rammux_pool_t *mpool_arg, size_t size_arg)
{
   ramalgn_pool_t *apool = NULL;
   ram_reply_t e = RAM_REPLY_INSANE;

   RAM_FAIL_NOTNULL(newptr_arg);
   *newptr_arg = NULL;
   RAM_FAIL_NOTNULL(mpool_arg);
   RAM_FAIL_NOTZERO(size_arg);

   e = rammux_getalgnpool(&apool, size_arg, mpool_arg);
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

   RAM_FAIL_TRAP(ramalgn_acquire(newptr_arg, apool));

   return RAM_REPLY_OK;
}

ram_reply_t rammux_getalgnpool(ramalgn_pool_t **apool_arg, size_t size_arg, rammux_pool_t *mpool_arg)
{
   size_t idx = 0;

   RAM_FAIL_NOTNULL(apool_arg);
   *apool_arg = NULL;
   RAM_FAIL_NOTZERO(size_arg);
   RAM_FAIL_NOTNULL(mpool_arg);

   idx = (size_arg + mpool_arg->rammuxp_step - 1) / mpool_arg->rammuxp_step - 1;
   /* if i can't accomidate the size of the pool, i need to inform the caller. */
   if (idx >= RAMMUX_MAXPOOLCOUNT)
      return RAM_REPLY_RANGEFAIL;
   if (!mpool_arg->rammuxp_initflags[idx])
   {
      ram_reply_t e = RAM_REPLY_INSANE;

      e = ramalgn_mkpool(&mpool_arg->rammuxp_apools[idx], mpool_arg->rammuxp_appetite, mpool_arg->rammuxp_step * (idx + 1), &mpool_arg->rammuxp_tag);
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

      assert(mpool_arg->rammuxp_apools[idx].ramalgnp_slotpool.ramslotp_granularity >= size_arg);
      assert(mpool_arg->rammuxp_apools[idx].ramalgnp_slotpool.ramslotp_granularity - size_arg < mpool_arg->rammuxp_step);
      mpool_arg->rammuxp_initflags[idx] = 1;
   }

   *apool_arg = &mpool_arg->rammuxp_apools[idx];
   return RAM_REPLY_OK;
}

ram_reply_t rammux_chkpool(const rammux_pool_t *mpool_arg)
{
   size_t i = 0;

   for (i = 0; i < RAMMUX_MAXPOOLCOUNT; ++i)
   {
      if (mpool_arg->rammuxp_initflags[i])
         RAM_FAIL_TRAP(ramalgn_chkpool(&mpool_arg->rammuxp_apools[i]));
   }

   return RAM_REPLY_OK;
}

ram_reply_t rammux_query(rammux_pool_t **mpool_arg, size_t *size_arg, void *ptr_arg)
{
   ramalgn_pool_t *apool = NULL;
   ram_reply_t e = RAM_REPLY_INSANE;
   const ramalgn_tag_t *tag = NULL;
   ramsig_signature_t sig = {0};

   RAM_FAIL_NOTNULL(mpool_arg);
   *mpool_arg = NULL;
   RAM_FAIL_NOTNULL(size_arg);
   *size_arg = 0;
   RAM_FAIL_NOTNULL(ptr_arg);

   e = ramalgn_query(&apool, ptr_arg);
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

   RAM_FAIL_TRAP(ramalgn_gettag(&tag, apool));
   /* i use the signature in the first half of the tag to increase the possibility that
    * an invalid address in 'ptr_arg' won't crash the process when someone attempts to dereference
    * the pointer that i expect to contain the mux pool.
    * note: `size_t` and `uintptr_t` should be identical types. */
   RAM_FAIL_TRAP(ram_cast_sztou32(&sig.ramsigs_n, tag->ramalgnt_values[0]));
   /* i consider a signature mismatch an expected failure here it this function is used
    * to determine whether a pointer belongs to another allocator. */
   if (0 != RAMSIG_CMP(sig, rammux_thesignature))
      return RAM_REPLY_NOTFOUND;

   RAM_FAIL_TRAP(ramalgn_getgranularity(size_arg, apool));
   *mpool_arg = (rammux_pool_t *)tag->ramalgnt_values[1];

   return RAM_REPLY_OK;
}
