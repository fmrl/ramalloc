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

#include <ramalloc/list.h>
#include <ramalloc/misc.h>
#include <stdlib.h>

ramfail_status_t ramlist_chklist(const ramlist_list_t *list_arg)
{
   /* NULL represents a nil list but i'm going to expect that to be checked
    * ahead of time because once a non-nil list, NULL pointers are not
    * allowed. */
   RAMFAIL_DISALLOWZ(list_arg);

   /* neither the predecessor nor the sucessor can be NULL. */
   RAMFAIL_CONFIRM(RAMFAIL_CORRUPT, list_arg->ramlistl_next);
   RAMFAIL_CONFIRM(RAMFAIL_CORRUPT, list_arg->ramlistl_prev);

   /* either both of the fields can point to 'list_arg' or none of them can. */
   RAMFAIL_CONFIRM(RAMFAIL_CORRUPT,
      (list_arg != list_arg->ramlistl_prev && list_arg != list_arg->ramlistl_next)
      || (list_arg->ramlistl_prev == list_arg->ramlistl_next)
      );

   /* everything is okay, as far as i can tell. */
   return RAMFAIL_OK;
}

ramfail_status_t ramlist_mklist(ramlist_list_t *list_arg)
{
   RAMFAIL_DISALLOWZ(list_arg);

   list_arg->ramlistl_next = list_arg->ramlistl_prev = list_arg;

   return RAMFAIL_OK;
}

ramfail_status_t ramlist_splice(ramlist_list_t *lhs_arg, ramlist_list_t *rhs_arg)
{
   ramlist_list_t *rp = NULL;
   ramlist_list_t *ls = NULL;

   RAMFAIL_DISALLOWZ(lhs_arg);
   RAMFAIL_DISALLOWZ(rhs_arg);

   rp = rhs_arg->ramlistl_prev;
   ls = lhs_arg->ramlistl_next;
   RAMFAIL_RETURN(rammisc_swap(&rp->ramlistl_next, &lhs_arg->ramlistl_next, sizeof(ramlist_list_t *)));
   RAMFAIL_RETURN(rammisc_swap(&ls->ramlistl_prev, &rhs_arg->ramlistl_prev, sizeof(ramlist_list_t *)));

   return RAMFAIL_OK;
}

ramfail_status_t ramlist_pop(ramlist_list_t **tail_arg, ramlist_list_t *head_arg)
{
   RAMFAIL_DISALLOWZ(tail_arg);
   *tail_arg = NULL;
   RAMFAIL_DISALLOWZ(head_arg);

   if (head_arg->ramlistl_next == head_arg)
      return RAMFAIL_OK;
   else
   {
      ramlist_list_t *tail = head_arg->ramlistl_next;

      tail->ramlistl_prev = head_arg->ramlistl_prev;
      head_arg->ramlistl_prev->ramlistl_next = tail;
      head_arg->ramlistl_next = head_arg->ramlistl_prev = head_arg;

      *tail_arg = tail;
      return RAMFAIL_OK;
   }
}

ramfail_status_t ramlist_hastail(int *result_arg, const ramlist_list_t *list_arg)
{
   RAMFAIL_DISALLOWZ(result_arg);
   *result_arg = 0;
   RAMFAIL_DISALLOWZ(list_arg);

   *result_arg = (list_arg != list_arg->ramlistl_next);
   return RAMFAIL_OK;
}

ramfail_status_t ramlist_next(ramlist_list_t **succ_arg, ramlist_list_t *of_arg)
{
   RAMFAIL_DISALLOWZ(succ_arg);
   *succ_arg = 0;
   RAMFAIL_DISALLOWZ(of_arg);

   *succ_arg = of_arg->ramlistl_next;
   return RAMFAIL_OK;
}

ramfail_status_t ramlist_foreach(ramlist_list_t *begin_arg, ramlist_list_t *end_arg,
   ramlist_foreach_t func_arg, void *context_arg)
{
   ramlist_list_t *i = NULL;
   ramfail_status_t e = RAMFAIL_TRYAGAIN;

   RAMFAIL_DISALLOWZ(begin_arg);
   RAMFAIL_DISALLOWZ(end_arg);
   RAMFAIL_DISALLOWZ(func_arg);

   for (i = begin_arg; RAMFAIL_TRYAGAIN == e && i != end_arg; i = i->ramlistl_next)
      e = func_arg(i, context_arg);

   if (RAMFAIL_TRYAGAIN == e)
      return RAMFAIL_OK;
   else
      return e;
}
