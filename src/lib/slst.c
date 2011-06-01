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

#include <ramalloc/slst.h>

ram_reply_t ramslst_mklist(ramslst_slist_t *slist_arg)
{
   RAM_FAIL_NOTNULL(slist_arg);

   slist_arg->ramslstsl_next = NULL;
   return RAM_REPLY_OK;
}

ram_reply_t ramslst_insert(ramslst_slist_t *what_arg, ramslst_slist_t *after_arg)
{
   RAM_FAIL_NOTNULL(what_arg);
   RAM_FAIL_NOTNULL(after_arg);

   what_arg->ramslstsl_next = after_arg->ramslstsl_next;
   after_arg->ramslstsl_next = what_arg;
   return RAM_REPLY_OK;
}

ram_reply_t ramslst_remove(ramslst_slist_t *pred_arg)
{
   RAM_FAIL_NOTNULL(pred_arg);

   if (RAMSLST_ISTAIL(pred_arg))
      return RAM_REPLY_NOTFOUND;
   else
   {
      pred_arg->ramslstsl_next = pred_arg->ramslstsl_next->ramslstsl_next;
      return RAM_REPLY_OK;      
   }
}

ram_reply_t ramslst_foreach(ramslst_slist_t *list_arg, ramslst_foreach_t func_arg, void *context_arg)
{
   ramslst_slist_t *node = list_arg;
   ram_reply_t e = RAM_REPLY_AGAIN;

   RAM_FAIL_NOTNULL(func_arg);

   while (RAM_REPLY_AGAIN == e && node)
   {
      e = func_arg(node, context_arg);
      node = node->ramslstsl_next;
   }

   if (RAM_REPLY_AGAIN == e)
      return RAM_REPLY_OK;
   else
      return e;
}