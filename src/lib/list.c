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

ram_reply_t ramlist_chklist(const ramlist_list_t *list_arg)
{
   /* NULL represents a nil list but i'm going to expect that to be checked
    * ahead of time because once a non-nil list, NULL pointers are not
    * allowed. */
   RAM_FAIL_NOTNULL(list_arg);

   /* if a nil list is acceptable in a given situation, filter it before
    * calling this function. */
   RAM_FAIL_EXPECT(RAM_REPLY_DISALLOWED, !RAMLIST_ISNIL(list_arg));

   /* neither the predecessor nor the successor can be NULL. */
   RAM_FAIL_EXPECT(RAM_REPLY_CORRUPT, list_arg->ramlistl_next);
   RAM_FAIL_EXPECT(RAM_REPLY_CORRUPT, list_arg->ramlistl_prev);

   /* either both of the fields can point to 'list_arg' or none of them can. */
   RAM_FAIL_EXPECT(RAM_REPLY_CORRUPT,
      (list_arg != list_arg->ramlistl_prev && list_arg != list_arg->ramlistl_next)
      || (list_arg->ramlistl_prev == list_arg->ramlistl_next)
      );

   /* everything is okay, as far as i can tell. */
   return RAM_REPLY_OK;
}

ram_reply_t ramlist_mklist(ramlist_list_t *list_arg)
{
   RAM_FAIL_NOTNULL(list_arg);

   list_arg->ramlistl_next = list_arg->ramlistl_prev = list_arg;

   return RAM_REPLY_OK;
}

ram_reply_t ramlist_splice(ramlist_list_t *lhs_arg, ramlist_list_t *rhs_arg)
{
   ramlist_list_t *rp = NULL;
   ramlist_list_t *ls = NULL;

   RAM_FAIL_NOTNULL(lhs_arg);
   RAM_FAIL_NOTNULL(rhs_arg);
   RAM_FAIL_EXPECT(RAM_REPLY_DISALLOWED, !RAMLIST_ISNIL(lhs_arg));
   RAM_FAIL_EXPECT(RAM_REPLY_DISALLOWED, !RAMLIST_ISNIL(rhs_arg));

   rp = rhs_arg->ramlistl_prev;
   ls = lhs_arg->ramlistl_next;
   RAM_FAIL_TRAP(rammisc_swap(&rp->ramlistl_next, &lhs_arg->ramlistl_next, sizeof(ramlist_list_t *)));
   RAM_FAIL_TRAP(rammisc_swap(&ls->ramlistl_prev, &rhs_arg->ramlistl_prev, sizeof(ramlist_list_t *)));

   return RAM_REPLY_OK;
}

ram_reply_t ramlist_pop(ramlist_list_t **tail_arg, ramlist_list_t *head_arg)
{
   RAM_FAIL_NOTNULL(tail_arg);
   *tail_arg = NULL;
   RAM_FAIL_NOTNULL(head_arg);
   RAM_FAIL_EXPECT(RAM_REPLY_DISALLOWED, !RAMLIST_ISNIL(head_arg));

   if (head_arg->ramlistl_next == head_arg)
      return RAM_REPLY_OK;
   else
   {
      ramlist_list_t *tail = head_arg->ramlistl_next;

      tail->ramlistl_prev = head_arg->ramlistl_prev;
      head_arg->ramlistl_prev->ramlistl_next = tail;
      head_arg->ramlistl_next = head_arg->ramlistl_prev = head_arg;

      *tail_arg = tail;
      return RAM_REPLY_OK;
   }
}

ram_reply_t ramlist_hastail(int *result_arg, const ramlist_list_t *list_arg)
{
   RAM_FAIL_NOTNULL(result_arg);
   *result_arg = 0;
   RAM_FAIL_NOTNULL(list_arg);
   RAM_FAIL_EXPECT(RAM_REPLY_DISALLOWED, !RAMLIST_ISNIL(list_arg));

   *result_arg = (list_arg != list_arg->ramlistl_next);
   return RAM_REPLY_OK;
}

ram_reply_t ramlist_next(ramlist_list_t **succ_arg, ramlist_list_t *of_arg)
{
   RAM_FAIL_NOTNULL(succ_arg);
   *succ_arg = 0;
   RAM_FAIL_NOTNULL(of_arg);
   RAM_FAIL_EXPECT(RAM_REPLY_DISALLOWED, !RAMLIST_ISNIL(of_arg));

   *succ_arg = of_arg->ramlistl_next;
   return RAM_REPLY_OK;
}

ram_reply_t ramlist_foreach(ramlist_list_t *begin_arg, ramlist_list_t *end_arg,
   ramlist_foreach_t func_arg, void *context_arg)
{
   ramlist_list_t *i = NULL;
   ram_reply_t e = RAM_REPLY_AGAIN;

   RAM_FAIL_NOTNULL(begin_arg);
   RAM_FAIL_EXPECT(RAM_REPLY_DISALLOWED, !RAMLIST_ISNIL(begin_arg));
   RAM_FAIL_NOTNULL(end_arg);
   RAM_FAIL_EXPECT(RAM_REPLY_DISALLOWED, !RAMLIST_ISNIL(end_arg));
   RAM_FAIL_NOTNULL(func_arg);

   for (i = begin_arg; RAM_REPLY_AGAIN == e && i != end_arg; i = i->ramlistl_next)
      e = func_arg(i, context_arg);

   if (RAM_REPLY_AGAIN == e)
      return RAM_REPLY_OK;
   else
      return e;
}

ram_reply_t ramlist_mknil(ramlist_list_t *list_arg)
{
   RAM_FAIL_NOTNULL(list_arg);

   list_arg->ramlistl_next = NULL;
   list_arg->ramlistl_prev = NULL;

   return RAM_REPLY_OK;
}
