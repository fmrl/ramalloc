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
 * @addtogroup facade
 * @{
 * @file
 * @brief convenience façade
 */

#ifndef RAMALLOC_FACADE_H_IS_INCLUDED
#define RAMALLOC_FACADE_H_IS_INCLUDED

#include <ramalloc/default.h>
#include <ramalloc/compat.h>
#include <ramalloc/mem.h>

/**
 * @brief supplementary allocation function signature (façade).
 * @see rammem_malloc_t
 * @remark this identifier is a @e façade, meaning it aliases another
 *    identifier for convenience. please see the documentation for the
 *    aliased identifier for detailed information about its use.
 */
typedef rammem_malloc_t ram_malloc_t;

/**
 * @brief supplementary deallocation function signature (façade).
 * @see rammem_free_t
 * @remark this identifier is a @e façade, meaning it aliases another
 *    identifier for convenience. please see the documentation for the
 *    aliased identifier for detailed information about its use.
 */
typedef rammem_free_t ram_free_t;

/**
 * @brief acquire memory (façade).
 * @see ram_default_acquire
 * @remark this identifier is a @e façade, meaning it aliases another
 *    identifier for convenience. please see the documentation for the
 *    aliased identifier for detailed information about its use.
 */
#define ram_acquire ram_default_acquire

/**
 * @brief discard memory (façade).
 * @see ram_default_discard
 * @remark this identifier is a @e façade, meaning it aliases another
 *    identifier for convenience. please see the documentation for the
 *    aliased identifier for detailed information about its use.
 */
#define ram_discard ram_default_discard

/**
 * @brief reclaim memory (façade).
 * @see ram_default_reclaim
 * @remark this identifier is a @e façade, meaning it aliases another
 *    identifier for convenience. please see the documentation for the
 *    aliased identifier for detailed information about its use.
 */
#define ram_reclaim ram_default_reclaim

/**
 * @brief flush the calling thread's allocator (façade).
 * @see ram_default_flush
 * @remark this identifier is a @e façade, meaning it aliases another
 *    identifier for convenience. please see the documentation for the
 *    aliased identifier for detailed information about its use.
 */
#define ram_flush ram_default_flush

/**
 * @brief inquire about a pointer (façade).
 * @see ram_default_query
 * @remark this identifier is a @e façade, meaning it aliases another
 *    identifier for convenience. please see the documentation for the
 *    aliased identifier for detailed information about its use.
 */
#define ram_query ram_default_query

#endif /* RAMALLOC_FACADE_H_IS_INCLUDED */

/**
 * @}
 */
