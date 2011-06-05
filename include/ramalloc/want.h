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
 * @addtogroup config
 * @{
 * @file
 * @brief compile-time configuration options.
 */

#ifndef RAMALLOC_WANT_H_IS_INCLUDED
#define RAMALLOC_WANT_H_IS_INCLUDED

#include <ramalloc/config.h>
#include <ramalloc/compiler.h>
#include <stdlib.h>

/**
 * @def RAM_WANT_FEEDBACK
 * @brief enable or disable configuration feedback.
 * @details @c RAM_WANT_FEEDBACK=1 indicates that the compiler should give
 *    feedback about compile-time configuration options. if no preference
 *    is specified, then feedback will be provided. note that some messages
 *    cannot be suppressed, since they have information the user should be
 *    aware of (e.g. unsupported modes).
 * @remark you can customize this option using the CMake cache variable
 *    @c WANT_FEEDBACK.
 * @remark the feedback is spammy, since it's repeated for every compilation
 *    unit. i recommend you restrict its use for when you are unsure about
 *    the configuration or if you are debugging the configuration code.
 */
#ifndef RAM_WANT_FEEDBACK
#  define RAM_WANT_FEEDBACK 0
#endif

/**
 * @def RAM_WANT_DEBUG
 * @brief enable or disable debugging features.
 * @details @c RAM_WANT_DEBUG=1 specifies whether @e ramalloc should include
 *    debugging oriented features and code. if no preference is specified,
 *    a sensible value will be determined based on whether NDEBUG is
 *    defined or not.
 * @remark you can customize this option using the CMake cache variable
 *    @c WANT_DEBUG.
 */
#ifndef RAM_WANT_DEBUG
#  ifdef NDEBUG
#     define RAM_WANT_DEBUG 0
#else
#     define RAM_WANT_DEBUG 1
#  endif
#endif
#if RAM_WANT_FEEDBACK && RAM_WANT_DEBUG
   RAMSYS_MESSAGE(debug mode is enabled.)
#endif

/**
 * @def RAM_WANT_MARKFREED
 * @brief mark freed memory.
 * @details <tt>#define RAM_WANT_MARKFREED=<em>byte-pattern</em></tt> if you
 *    want @e ramalloc to mark memory that's been freed but not returned to
 *    the system. a byte pattern of <tt>0</tt> indicates you don't want
 *    memory to be marked at all. if no preference is specified, @e ramalloc
 *    mark memory with a predefined byte pattern only if @c RAM_WANT_DEBUG is
 *    @e true.
 * @remark you can customize this option using the CMake cache variable
 *    @c WANT_MARK_FREED.
 */
#ifndef RAM_WANT_MARKFREED
#  if RAM_WANT_DEBUG
#     define RAM_WANT_MARKFREED 0xbe
#else
#     define RAM_WANT_MARKFREED 0
#  endif
#endif
#if RAM_WANT_FEEDBACK && RAM_WANT_MARKFREED
   RAMSYS_MESSAGE(unused memory will be marked with RAM_WANT_MARKFREED.)
#endif

/**
 * @def RAM_WANT_ZEROMEM
 * @brief zero newly allocated memory.
 * @details @c RAM_WANT_ZEROMEM=1 indicates that you wish for newly allocated
 *    memory to be zeroed out before handed off to the caller.
 * @todo i'm still on the fence regarding whether i should take
 *    responsibility for zeroing out memory that's been allocated.
 *    i'll leave it off by default for the moment, partially because it's
 *    not implemented uniformly for all pools.
 * @remark you can customize this option using the CMake cache variable
 *    @c WANT_ZEROED_MEMORY.
 */
#ifndef RAM_WANT_ZEROMEM
#  define RAM_WANT_ZEROMEM 0
#endif
#if RAM_WANT_FEEDBACK && RAM_WANT_ZEROMEM
   RAMSYS_MESSAGE(newly allocated memory will be zeroed out.)
#endif

/**
 * @def RAM_WANT_COMPACT
 * @brief compact mode.
 * @details @c RAM_WANT_COMPACT=1 specifies that @e ramalloc should use the
 *    smallest possible data types in the interest of conserving memory.
 *    there may be small a performance penalty and it is not yet known if
 *    the memory savings is significant.
 * @remark you can customize this option using the CMake cache variable
 *    @c WANT_COMPACT.
 * @todo this feature isn't completely implemented yet. it should be a
 *    consideration during comprehensive code review.
 * @todo i need to implement metrics before i can determine the significance
 *    of the memory savings.
 */
#ifndef RAM_WANT_COMPACT
#  define RAM_WANT_COMPACT 0
#endif
#if RAM_WANT_COMPACT
   RAMSYS_MESSAGE(compact mode is enabled. please keep in mind that this is not fully functional yet.)
#endif

/**
 * @def RAM_WANT_MINPAGECAPACITY
 * @brief specifies the minimum page capacity.
 * @details the <b>minimum page capacity</b> places a constraint on the
 *    number of objects to allocate on a hardware page. it also influences
 *    the size of the largest object @e ramalloc will attempt to pool.
 * @remark you can customize this option using the CMake cache variable
 *    @c WANT_MINIMUM_PAGE_CAPACITY.
 * @todo
 *    a single object on a page would defeat the purpose of the
 *    pool. as few as 10 might be considered worthwhile. the best choice
 *    for this option might be known once i've implemented metrics and
 *    benchmarking.
 */
#ifndef RAM_WANT_MINPAGECAPACITY
#  define RAM_WANT_MINPAGECAPACITY 10
#endif
#if RAM_WANT_MINPAGECAPACITY < 2
#  error the minimum page capacity cannot be less than 2.
#elif RAM_WANT_FEEDBACK
   RAMSYS_MESSAGE(the minimum page capacity is RAM_WANT_MINPAGECAPACITY objects.)
#endif
/**
 * @def RAM_WANT_DEFAULTAPPETITE
 * @brief the default appetite.
 * @see rampg_appetite_t
 * @remark you can customize this option using the CMake cache variable
 *    @c WANT_DEFAULT_APPETITE.
 */
#ifndef RAM_WANT_DEFAULTAPPETITE
#  define RAM_WANT_DEFAULTAPPETITE RAMOPT_FRUGAL
#endif
#if RAM_WANT_FEEDBACK
   RAMSYS_MESSAGE(the default appetite is RAM_WANT_DEFAULTAPPETITE.)
#endif
/**
 * @def RAM_WANT_DEFAULTRECLAIMGOAL
 * @brief the default reclamation goal.
 * @details the <b>default reclamation goal</b> is the number of trashed
 *    pointers a lazy pool should reclaim per allocation request.
 * @remark you can customize this option using the CMake cache variable
 *    @c WANT_DEFAULT_RECLAIM_GOAL.
 */
#ifndef RAM_WANT_DEFAULTRECLAIMGOAL
#  define RAM_WANT_DEFAULTRECLAIMGOAL 3
#endif
#if RAM_WANT_DEFAULTRECLAIMGOAL < 1
#  error the default reclamation goal cannot be less than 1.
#elif RAM_WANT_FEEDBACK
   RAMSYS_MESSAGE(the default reclamation goal is RAM_WANT_DEFAULTRECLAIMGOAL objects.)
#endif

/**
 * @def RAM_WANT_NPTLDEADLOCK
 * @brief demonstrates a deadlock discovered in NPTL.
 * @details use @c RAM_WANT_NPTLDEADLOCK to demonstrate a non-deterministic
 *    deadlock i encountered in the NPTL implementation of
 *    pthread_barrier_wait().
 * @remark you can customize this option using the CMake cache variable
 *    @c WANT_NPTL_DEADLOCK.
 * @see ramuix_waitonbarrier()
 * @see ramuix_startroutine()
 */
#ifndef RAM_WANT_NPTLDEADLOCK
#  define RAM_WANT_NPTLDEADLOCK 0
#endif
#if RAM_WANT_NPTLDEADLOCK
   RAMSYS_MESSAGE(i will demonstrate a deadlock in NPTL.)
#endif


/**
 * @internal
 * @def RAM_WANT_NOUNUSEDARGS
 * @brief helps find obsolete uses of RAMANNOTATE_UNUSEDARGS()
 * @details use @c RAM_WANT_NOUNUSEDARGS to disable its mechanism so you
 *    can match uses of RAMANNOTATE_UNUSEDARGS() with the compiler warning
 *    that its intended to suppress.
 * @see RAMANNOTATE_UNUSEDARGS()
 * @remark you can customize this option using the CMake cache variable
 *    @c WANT_NO_UNUSED_ARGS.
 */
#ifndef RAM_WANT_NOUNUSEDARGS
#  define RAM_WANT_NOUNUSEDARGS 0
#endif
#if RAM_WANT_FEEDBACK && RAM_WANT_NOUNUSEDARGS
   RAMSYS_MESSAGE(i will not suppress unused argument warnings.)
#endif

/**
 * @def RAM_WANT_OVERCONFIDENT
 * @brief disables unanticipated reply detection (unsupported).
 * @details use @c RAM_WANT_OVERCONFIDENT to eliminate branching
 *    associated with detecting unanticipated replies. this has the effect
 *    of improving performance under the (usually mistaken) assumption that
 *    everything that could go wrong has gone wrong.
 * @remark you can customize this option using the CMake cache variable
 *    @c WANT_OVERCONFIDENT.
 * @warning i cannot support problems encountered while error checking is
 *    disabled.
 * @see fail
 */
#ifndef RAM_WANT_OVERCONFIDENT
#  define RAM_WANT_OVERCONFIDENT 0
#endif
#if RAM_WANT_OVERCONFIDENT
   RAMSYS_MESSAGE(overconfident mode is enabled. note that this is unsupported!)
#endif

#endif /* RAMALLOC_WANT_H_IS_INCLUDED */

/**
 * @}
 */
