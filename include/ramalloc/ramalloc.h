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

/**
 * @file
 * @brief the main header file.
 */

#ifndef RAMALLOC_H_IS_INCLUDED
#define RAMALLOC_H_IS_INCLUDED

#include <ramalloc/fail.h>
#include <ramalloc/facade.h>

/**
 * @ingroup init
 * @brief initialize @e ramalloc.
 * @details ram_initialize() should be called before calling other
 *    @e ramalloc functions.
 * @param supmalloc_arg
 *    a pointer to the supplementary allocation function. if @c NULL,
 *    <tt>@&malloc</tt> is used.
 * @param supfree_arg
 *    a pointer to the supplementary deallocation function. if @c NULL,
 *    <tt>@&free</tt> is used.
 * @return @c RAM_REPLY_OK - the operation was successful.
 * @remark this function returns a @e reply as described in reply.h.
 *    replies not yet documented here may also be passed up through the
 *    callstack. use a reply wrapper from fail.h to trap unexpected
 *    replies.
 */
ram_reply_t ram_initialize(ram_malloc_t supmalloc_arg,
      ram_free_t supfree_arg);

#endif /* RAMALLOC_H_IS_INCLUDED */

