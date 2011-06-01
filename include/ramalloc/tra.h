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

#ifndef RAMTRA_H_IS_INCLUDED
#define RAMTRA_H_IS_INCLUDED

#include <ramalloc/fail.h>
#include <ramalloc/mtx.h>
#include <ramalloc/slst.h>

typedef struct ramtra_trash
{
   rammtx_mutex_t ramtrat_mutex;
   ramslst_slist_t ramtrat_items;
   size_t ramtrat_size;
} ramtra_trash_t;

typedef ram_reply_t (*ramtra_foreach_t)(void *ptr_arg, void *context_arg);

ram_reply_t ramtra_mktrash(ramtra_trash_t *trash_arg);
ram_reply_t ramtra_rmtrash(ramtra_trash_t *trash_arg);
ram_reply_t ramtra_push(ramtra_trash_t *trash_arg, void *ptr_arg);
ram_reply_t ramtra_pop(void **ptr_arg, ramtra_trash_t *trash_arg);
ram_reply_t ramtra_size(size_t *size_arg, ramtra_trash_t *trash_arg);
ram_reply_t ramtra_foreach(ramtra_trash_t *trash_arg, ramtra_foreach_t func_arg, void *context_arg);

#endif /* RAMTRA_H_IS_INCLUDED */
