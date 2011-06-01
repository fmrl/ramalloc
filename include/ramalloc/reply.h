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
 * @file reply.h
 * @brief specific replies.
 */

#ifndef RAMALLOC_REPLY_H_IS_INCLUDED
#define RAMALLOC_REPLY_H_IS_INCLUDED

/**
 * @brief replies.
 * @section reply wrappers.
 */
typedef enum ram_reply
{
   /** indicates that everything is fine. */
   RAM_REPLY_OK = 0,
   /** indicates that the logic is probably bad. review assumptions the code
    * makes. */
   RAM_REPLY_INSANE,
   /** indicates a C runtime library related failure; check errno. */
   RAM_REPLY_CRTFAIL,
   /** indicates a foreign API returned a failure code. */
   RAM_REPLY_APIFAIL,
   /** indicates a specific, disallowed value was provided as a function
    * argument. */
   RAM_REPLY_DISALLOWED,
   /** indicates a value is out of its permitted range. */
   RAM_REPLY_RANGEFAIL,
   /** indicates a resource (e.g. file) related problem. */
   RAM_REPLY_RESOURCEFAIL,
   /** indicates a search failed. */
   RAM_REPLY_NOTFOUND,
   /** indicates that a specific action or request is unsupported. */
   RAM_REPLY_UNSUPPORTED,
   /** indicates that a complex state is inconsistent. */
   RAM_REPLY_INCONSISTENT,
   /** indicates that an action or request needs to be performed again. */
   RAM_REPLY_AGAIN,
   /** indicates that a data structure failed a runtime check. */
   RAM_REPLY_CORRUPT,
   /** indicates that an accumulator or collection exceeded its maximum. */
   RAM_REPLY_UNDERFLOW,
   /** indicates that an accumulator or collection exceeded its minimum. */
   RAM_REPLY_OVERFLOW,
   /** indicates a user input related problem. */
   RAM_REPLY_INPUTFAIL,
} ram_reply_t;

#endif /* RAMALLOC_REPLY_H_IS_INCLUDED */
