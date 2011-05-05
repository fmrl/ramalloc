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

#ifndef RAMOPT_H_IS_INCLUDED
#define RAMOPT_H_IS_INCLUDED

#include <stdlib.h>

/* 'RAMOPT_DEBUG' specifies whether ramalloc should include debugging oriented
 * features and code. if no preference is specified, a sensible value will
 * be determined based on whether NDEBUG is defined or not. */
#ifndef RAMOPT_DEBUG
#  ifdef NDEBUG
#     define RAMOPT_DEBUG 0
#else
#     define RAMOPT_DEBUG 1
#  endif
#endif

/* #define RAMOPT_MARKFREED=1 before the preprocessor gets here if you want 
 * ramalloc to mark memory that's been freed but not returned to the system.
 * likewise, #define TMPOT_MARKFREED=0, if you don't want this behavior. 
 * if no preference is specified, ramalloc will do this only if RAMOPT_DEBUG 
 * is /true/. */
#ifndef RAMOPT_MARKFREED
#  if RAMOPT_DEBUG
#     define RAMOPT_MARKFREED 0xbe
#else
#     define RAMOPT_MARKFREED 0
#  endif
#endif

/* TODO: i'm still on the fence regarding whether i should take
 * responsibility for zeroing out memory that's been allocated. i'll leave
 * it off by default for the moment, partially because it's not implemented
 * uniformly for all pools. */
#ifndef RAMOPT_ZEROMEM
#  define RAMOPT_ZEROMEM 0
#endif

/* #define RAMOPT_COMPACT=1 before the preprocessor gets here if you want 
 * ramalloc to use the smallest possible types in the interest of conserving
 * memory. there may be small a performance penalty and the memory savings
 * is likely to be small. */
#ifndef RAMOPT_COMPACT
#  define RAMOPT_COMPACT 0
#endif

/* the minimum page capacity places a constraint on the number of objects
 * to allocate on a pool. it also determines the size of the largest object 
 * ramalloc will attempt to pool. 
 * 
 * for example, 2 objects on a page might defeats the purpose of pooling but
 * 10 might be worth it. */
#ifndef RAMOPT_MINDENSITY
#  define RAMOPT_MINDENSITY 10
#endif

typedef enum rampg_appetites
{
   RAMOPT_FRUGAL,
   RAMOPT_GREEDY,
} ramopt_appetite_t;

#ifndef RAMOPT_DEFAULTAPPETITE
#  define RAMOPT_DEFAULTAPPETITE RAMOPT_FRUGAL
#endif

/* the default reclaimation goal is the number of trashed pointers a lazy pool should
 * reclaim per allocation request. */
#ifndef RAMOPT_DEFAULTRECLAIMGOAL
#  define RAMOPT_DEFAULTRECLAIMGOAL 3
#endif

/* use *RAMOPT_BARRIERDEADLOCK* to demonstrate a non-deterministic deadlock
 * i encountered in the NLPT implementation of pthread_barrier_wait(). */
#ifndef RAMOPT_BARRIERDEADLOCK
#  define RAMOPT_BARRIERDEADLOCK 0
#endif

typedef void * (*ramopt_malloc_t)(size_t);
typedef void (*ramopt_free_t)(void *);

#ifndef RAMOPT_DEBUGUNUSEDARGS
#  define RAMOPT_DEBUGUNUSEDARGS 0
#endif

#endif /* RAMOPT_H_IS_INCLUDED */

