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

#include <ramalloc/tra.h>
#include <string.h>

typedef struct ramtra_foreachadaptor
{
   ramtra_foreach_t ramtrafea_function;
   void *ramtrafea_context;
} ramtra_foreachadaptor_t;

static ramfail_status_t ramtra_mktrash2(ramtra_trash_t *trash_arg);
static ramfail_status_t ramtra_foreachadaptor(ramslst_slist_t *node_arg, void *context_arg);

ramfail_status_t ramtra_mktrash(ramtra_trash_t *trash_arg)
{
   ramfail_status_t e = RAMFAIL_INSANE;

   RAMFAIL_DISALLOWZ(trash_arg);

   e = ramtra_mktrash2(trash_arg);
   if (RAMFAIL_OK == e)
      return RAMFAIL_OK;
   else
   {
      memset(trash_arg, 0, sizeof(*trash_arg));
      return e;
   }
}

ramfail_status_t ramtra_mktrash2(ramtra_trash_t *trash_arg)
{
   assert(trash_arg);

   RAMFAIL_RETURN(rammtx_mkmutex(&trash_arg->ramtrat_mutex));
   RAMFAIL_RETURN(ramslst_mklist(&trash_arg->ramtrat_items));
   trash_arg->ramtrat_size = 0;

   return RAMFAIL_OK;
}

ramfail_status_t ramtra_push(ramtra_trash_t *trash_arg, void *ptr_arg)
{
   ramfail_status_t e = RAMFAIL_INSANE;

   RAMFAIL_DISALLOWZ(trash_arg);
   RAMFAIL_DISALLOWZ(ptr_arg);

   RAMFAIL_RETURN(rammtx_wait(&trash_arg->ramtrat_mutex));
   e = ramslst_insert((ramslst_slist_t *)ptr_arg, &trash_arg->ramtrat_items);
   trash_arg->ramtrat_size += (RAMFAIL_OK == e);
   /* if i fail to quit the mutex, the process can't continue meaningfully. */
   RAMFAIL_EPICFAIL(rammtx_quit(&trash_arg->ramtrat_mutex));
   RAMFAIL_RETURN(e);

   return RAMFAIL_OK;
}

ramfail_status_t ramtra_pop(void **ptr_arg, ramtra_trash_t *trash_arg)
{
   void *p = NULL;
   ramfail_status_t e = RAMFAIL_INSANE;

   RAMFAIL_DISALLOWZ(ptr_arg);
   *ptr_arg = NULL;
   RAMFAIL_DISALLOWZ(trash_arg);

   RAMFAIL_RETURN(rammtx_wait(&trash_arg->ramtrat_mutex));
   p = RAMSLST_NEXT(&trash_arg->ramtrat_items);
   e = ramslst_remove(&trash_arg->ramtrat_items);
   trash_arg->ramtrat_size -= (RAMFAIL_OK == e);
   /* if i fail to quit the mutex, the process can't continue meaningfully. */
   RAMFAIL_EPICFAIL(rammtx_quit(&trash_arg->ramtrat_mutex));
   if (RAMFAIL_OK == e || RAMFAIL_NOTFOUND == e)
   {
      *ptr_arg = p;
      return e;
   }
   else
   {
      RAMFAIL_RETURN(e);
      /* i shouldn't be able to get here, since 'e' is known to not be RAMFAIL_OK. */
      return RAMFAIL_INSANE;
   }
}

ramfail_status_t ramtra_rmtrash(ramtra_trash_t *trash_arg)
{
   RAMFAIL_DISALLOWZ(trash_arg);

   /* TODO: what do i do if the trash isn't empty yet? */
   RAMFAIL_RETURN(rammtx_rmmutex(&trash_arg->ramtrat_mutex));
   memset(trash_arg, 0, sizeof(*trash_arg));

   return RAMFAIL_OK;
}

ramfail_status_t ramtra_size(size_t *size_arg, ramtra_trash_t *trash_arg)
{
   size_t sz = 0;

   RAMFAIL_DISALLOWZ(size_arg);
   *size_arg = 0;
   RAMFAIL_DISALLOWZ(trash_arg);

   RAMFAIL_RETURN(rammtx_wait(&trash_arg->ramtrat_mutex));
   sz = trash_arg->ramtrat_size;
   /* if i fail to quit the mutex, the process can't continue meaningfully. */
   RAMFAIL_EPICFAIL(rammtx_quit(&trash_arg->ramtrat_mutex));

   *size_arg = sz;
   return RAMFAIL_OK;
}

ramfail_status_t ramtra_foreach(ramtra_trash_t *trash_arg, ramtra_foreach_t func_arg, void *context_arg)
{
   ramfail_status_t e = RAMFAIL_INSANE;
   ramtra_foreachadaptor_t fea = {0};

   RAMFAIL_DISALLOWZ(trash_arg);
   RAMFAIL_DISALLOWZ(func_arg);

   fea.ramtrafea_function = func_arg;
   fea.ramtrafea_context = context_arg;

   RAMFAIL_RETURN(rammtx_wait(&trash_arg->ramtrat_mutex));
   /* i don't include the first element in 'ramtrat_items' because it is a sentinel and doesn't
    * actually hold a trashed pointer. */
   e = ramslst_foreach(RAMSLST_NEXT(&trash_arg->ramtrat_items), &ramtra_foreachadaptor, &fea);
   /* if i fail to quit the mutex, the process can't continue meaningfully. */
   RAMFAIL_EPICFAIL(rammtx_quit(&trash_arg->ramtrat_mutex));
   RAMFAIL_RETURN(e);

   return RAMFAIL_OK;
}

ramfail_status_t ramtra_foreachadaptor(ramslst_slist_t *node_arg, void *context_arg)
{
   ramtra_foreachadaptor_t *fea = NULL;

   assert(node_arg);
   assert(context_arg);

   fea = (ramtra_foreachadaptor_t *)context_arg;
   assert(fea->ramtrafea_function);
   return fea->ramtrafea_function(node_arg, fea->ramtrafea_context);
}
