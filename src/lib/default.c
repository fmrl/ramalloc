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

#include <ramalloc/default.h>
#include <ramalloc/para.h>

rampara_pool_t ramdefault_thepool;

ramfail_status_t ramdefault_initialize()
{
   RAMFAIL_RETURN(rampara_mkpool(&ramdefault_thepool, RAMOPT_DEFAULTAPPETITE, RAMOPT_DEFAULTRECLAIMGOAL));

   return RAMFAIL_OK;
}

ramfail_status_t ramdefault_acquire(void **newptr_arg, size_t size_arg)
{
   ramfail_status_t e = RAMFAIL_INSANE;

   e = rampara_acquire(newptr_arg, &ramdefault_thepool, size_arg);
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

ramfail_status_t ramdefault_discard(void *ptr_arg)
{
   RAMFAIL_RETURN(rampara_release(ptr_arg));

   return RAMFAIL_OK;
}

ramfail_status_t ramdefault_reclaim(size_t *count_arg, size_t goal_arg)
{
   RAMFAIL_RETURN(rampara_reclaim(count_arg, &ramdefault_thepool, goal_arg));

   return RAMFAIL_OK;
}

ramfail_status_t ramdefault_flush()
{
   RAMFAIL_RETURN(rampara_flush(&ramdefault_thepool));

   return RAMFAIL_OK;
}

ramfail_status_t ramdefault_query(size_t *size_arg, void *ptr_arg)
{
   rampara_pool_t *parapool = NULL;
   size_t sz = 0;
   ramfail_status_t e = RAMFAIL_INSANE;

   RAMFAIL_DISALLOWZ(size_arg);
   *size_arg = 0;
   RAMFAIL_DISALLOWZ(ptr_arg);

   e = rampara_query(&parapool, &sz, ptr_arg);
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

   if (parapool == &ramdefault_thepool)
   {
      *size_arg = sz;
      return RAMFAIL_OK;
   }
   else
      return RAMFAIL_NOTFOUND;
}

ramfail_status_t ramdefault_check()
{
   RAMFAIL_RETURN(rampara_chkpool(&ramdefault_thepool));

   return RAMFAIL_OK;
}
