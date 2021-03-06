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

rampara_pool_t ram_default_thepool;

ram_reply_t ram_default_initialize()
{
   RAM_FAIL_TRAP(rampara_mkpool(&ram_default_thepool,
         RAM_WANT_DEFAULTAPPETITE, RAM_WANT_DEFAULTRECLAIMGOAL));

   return RAM_REPLY_OK;
}

ram_reply_t ram_default_acquire(void **newptr_arg, size_t size_arg)
{
   ram_reply_t reply = RAM_REPLY_INSANE;

   reply = rampara_acquire(newptr_arg, &ram_default_thepool, size_arg);
   switch (reply)
   {
   default:
      RAM_FAIL_TRAP(reply);
      RAM_FAIL_UNREACHABLE();
   case RAM_REPLY_RANGEFAIL:
      return reply;
   case RAM_REPLY_OK:
      break;
   }

   return RAM_REPLY_OK;
}

ram_reply_t ram_default_discard(void *ptr_arg)
{
   RAM_FAIL_TRAP(rampara_release(ptr_arg));

   return RAM_REPLY_OK;
}

ram_reply_t ram_default_reclaim(size_t *count_arg, size_t goal_arg)
{
   RAM_FAIL_TRAP(rampara_reclaim(count_arg, &ram_default_thepool,
         goal_arg));

   return RAM_REPLY_OK;
}

ram_reply_t ram_default_flush()
{
   RAM_FAIL_TRAP(rampara_flush(&ram_default_thepool));

   return RAM_REPLY_OK;
}

ram_reply_t ram_default_query(size_t *size_arg, void *ptr_arg)
{
   rampara_pool_t *parapool = NULL;
   size_t sz = 0;
   ram_reply_t reply = RAM_REPLY_INSANE;

   RAM_FAIL_NOTNULL(size_arg);
   *size_arg = 0;
   RAM_FAIL_NOTNULL(ptr_arg);

   reply = rampara_query(&parapool, &sz, ptr_arg);
   switch (reply)
   {
   default:
      RAM_FAIL_TRAP(reply);
      RAM_FAIL_UNREACHABLE();
   case RAM_REPLY_NOTFOUND:
      return reply;
   case RAM_REPLY_OK:
      break;
   }

   if (&ram_default_thepool == parapool)
   {
      *size_arg = sz;
      return RAM_REPLY_OK;
   }
   else
      return RAM_REPLY_NOTFOUND;
}

ram_reply_t ram_default_check()
{
   RAM_FAIL_TRAP(rampara_chkpool(&ram_default_thepool));

   return RAM_REPLY_OK;
}
