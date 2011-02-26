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

#ifndef RAMLIST_H_IS_INCLUDED
#define RAMLIST_H_IS_INCLUDED

#include <ramalloc/fail.h>

struct ramlist_list;

typedef struct ramlist_list
{
   struct ramlist_list *ramlistl_next;
   struct ramlist_list *ramlistl_prev;
} ramlist_list_t;

typedef ramfail_status_t (*ramlist_foreach_t)(ramlist_list_t *node_arg, void *context_arg);

ramfail_status_t ramlist_mklist(ramlist_list_t *list_arg);
ramfail_status_t ramlist_splice(ramlist_list_t *lhs_arg, ramlist_list_t *rhs_arg);
ramfail_status_t ramlist_pop(ramlist_list_t **tail_arg, ramlist_list_t *head_arg);
ramfail_status_t ramlist_chklist(const ramlist_list_t *list_arg);
ramfail_status_t ramlist_hastail(int *result_arg, const ramlist_list_t *list_arg);
ramfail_status_t ramlist_next(ramlist_list_t **succ_arg, ramlist_list_t *of_arg);
ramfail_status_t ramlist_foreach(ramlist_list_t *begin_arg, ramlist_list_t *end_arg,
   ramlist_foreach_t func_arg, void *context_arg); 

#endif /* RAMLIST_H_IS_INCLUDED */
