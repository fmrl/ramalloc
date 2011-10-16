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
 * @addtogroup default
 * @{
 * @file
 * @brief the default allocator
 * @details the default module simply provides access to a global
 *    parallelized pool.
 * @todo
 *    the default allocator needs to be able to be parameterized through
 *    runtime options, presumably passed through ram_default_initialize().
 */

#ifndef RAMALLOC_DEFAULT_H_IS_INCLUDED
#define RAMALLOC_DEFAULT_H_IS_INCLUDED

#include <ramalloc/fail.h>

/**
 * @internal
 * @ingroup init
 * @brief initialize the default allocator.
 * @details ram_default_initialize() should be called before calling other
 *    functions from the @c ram_default namespace.
 * @return @c RAM_REPLY_OK - the operation was successful.
 * @remark this function returns a @e reply as described in reply.h.
 *    replies not yet documented here may also be passed up through the
 *    callstack. use a reply wrapper from fail.h to trap unexpected
 *    replies.
 * @remark it should not be necessary to call ram_default_initialize()
 *    directly. normally, ram_initialize() invokes this function for you.
 */
ram_reply_t ram_default_initialize();

/**
 * @brief acquire a quantity of memory.
 * @details ram_default_acquire() acquires a quantity of memory from the
 *    default pool.
 * @param newptr_arg
 *    the address of a pointer that will reference the newly  allocated
 *    memory. this address cannot be @c NULL.
 * @param size_arg
 *    the minimum quantity of memory, in bytes, that is desired. this
 *    quantity cannot be 0.
 * @return @c RAM_REPLY_OK - the operation was successful.
 * @return @c RAM_REPLY_DISALLOWED (unanticipated) - an argument contained
 *    a disallowed value.
 * @return @c RAM_REPLY_RANGEFAIL - the pool cannot accommodate the specific
 *    size requested.
 * @par performance
 *    this function completes in amortized constant time.
 * @remark this function performs the @e acquire operation and the
 *    @e reclaim operation with the default reclamation goal.
 * @remark this function returns a @e reply as described in reply.h.
 *    replies not yet documented here may also be passed up through the
 *    callstack. use a reply wrapper from fail.h to trap unexpected
 *    replies.
 */
ram_reply_t ram_default_acquire(void **newptr_arg, size_t size_arg);

/**
 * @brief discard memory.
 * @details ram_default_discard() informs an allocator that memory is no
 *    longer in use.
 * @param ptr_arg
 *    the address of the pointer that is no longer in use. this address
 *    cannot be NULL.
 * @return @c RAM_REPLY_OK - the operation was successful.
 * @return @c RAM_REPLY_DISALLOWED (unanticipated) - an argument contained
 *    a disallowed value.
 * @return @c RAM_REPLY_NOTFOUND (unanticipated) - the memory described
 *    by @e ptr_arg was not acquired from the default allocator. filter
 *    your calls to ram_default_discard() with ram_default_query() if you
 *    wish to anticipate this reply.
 * @par performance
 *    this function completes in constant time.
 * @remark this function performs the @e discard operation.
 * @remark this function returns a @e reply as described in reply.h.
 *    replies not yet documented here may also be passed up through the
 *    callstack. use a reply wrapper from fail.h to trap unexpected
 *    replies.
 * @warning attempts to use the memory referenced by @c ptr_arg following a
 *    call to ram_default_discard() are likely to crash the process at a
 *    later time.
 */
ram_reply_t ram_default_discard(void *ptr_arg);

/**
 * @brief reclaim discarded memory.
 * @details ram_default_reclaim() pulls a specified number of discarded
 *    pointers off of the current thread's @e trash and releases them to
 *    the current thread's allocator.
 * @param count_arg
 *    the address of a variable where the number of pointers successfully
 *    reclaimed should be deposited. this address cannot be @c NULL.
 * @param goal_arg
 *    the number of pointers to reclaim before stopping. if there are not
 *    enough pointers to satisfy the goal, the operation is still considered
 *    a success. this value cannot be 0.
 * @return @c RAM_REPLY_OK - the operation was successful.
 * @return @c RAM_REPLY_DISALLOWED (unanticipated) - an argument contained
 *    a disallowed value.
 * @par performance
 *    this function completes in linear time, bounded by the value of
 *    @e goal_arg.
 * @remark this function performs the @e reclaim operation.
 * @remark this function returns a @e reply as described in reply.h.
 *    replies not yet documented here may also be passed up through the
 *    callstack. use a reply wrapper from fail.h to trap unexpected
 *    replies.
 * @warning generally, ram_default_acquire() performs this operation for
 *    you. in a parallelized system, however, asymmetric allocation
 *    patterns require the regular use of ram_default_reclaim() on each
 *    thread during idle processing. otherwise, the thread may withhold
 *    discarded memory from the host (and from other threads) for an
 *    unreasonable period of time.
 */
ram_reply_t ram_default_reclaim(size_t *count_arg, size_t goal_arg);

/**
 * @brief reclaim discarded memory.
 * @details ram_default_flush() reclaims all pointers known to be discarded
 *    on the current thread.
 * @return @c RAM_REPLY_OK - the operation was successful.
 * @par performance
 *    this function completes in linear time, bounded by number of pointers
 *    in the trash at the moment of the call to ram_default_flush().
 * @remark this function performs the @e reclaim operation.
 * @remark this function returns a @e reply as described in reply.h.
 *    replies not yet documented here may also be passed up through the
 *    callstack. use a reply wrapper from fail.h to trap unexpected
 *    replies.
 * @warning use this function with caution; it will defeat the incremental
 *    nature of the allocation algorithm.
 * @todo should this function report the number of pointers reclaimed?
 */
ram_reply_t ram_default_flush();

/**
 * @brief inquire about an allocation.
 * @details ram_default_query() reports whether an address was allocated
 *    with the default allocator. if it was, the actual size of the
 *    allocation is also reported.
 * @param size_arg
 *    the address of a variable intended to hold the size, in bytes, of the
 *    allocation in question. note that the reported size might be larger
 *    than the original requested quantity. this address cannot be @c NULL.
 * @param ptr_arg
 *    the address in question.
 * @return @c RAM_REPLY_OK - the operation was successful.
 * @return @c RAM_REPLY_DISALLOWED (unanticipated) - an argument contained
 *    a disallowed value.
 * @return @c RAM_REPLY_NOTFOUND - the address provided was not acquired
 *    from the default allocator.
 * @par performance
 *    this function completes in constant time.
 * @remark this function performs the @e query operation.
 * @remark this function returns a @e reply as described in reply.h.
 *    replies not yet documented here may also be passed up through the
 *    callstack. use a reply wrapper from fail.h to trap unexpected
 *    replies.
 * @todo it just occurred to me that this might not check to be certain
 *    the address is a base address for an allocated object.
 * @todo should @e ptr_arg be @b const?
 */
ram_reply_t ram_default_query(size_t *size_arg, void *ptr_arg);

/**
 * @ingroup test
 * @brief perform diagnostics on the default allocator.
 * @details ram_default_check() will perform integrity checks on the default
 *    allocator's state.
 * @return @c RAM_REPLY_OK - the operation was successful.
 * @return @c RAM_REPLY_INCONSISTENT (unanticipated) - the allocator state
 *    is inconsistent.
 * @return @c RAM_REPLY_CORRUPT (unanticipated) - the allocator state
 *    is corrupt.
 * @return @c RAM_REPLY_INSANE (unanticipated) - a logic error was detected.
 * @par performance
 *    assume that this function should not be used in performance-sensitive
 *    applications.
 * @remark this function performs the @e reclaim operation.
 * @remark this function returns a @e reply as described in reply.h.
 *    replies not yet documented here may also be passed up through the
 *    callstack. use a reply wrapper from fail.h to trap unexpected
 *    replies.
 */
ram_reply_t ram_default_check();

#endif /* RAMALLOC_DEFAULT_H_IS_INCLUDED */
