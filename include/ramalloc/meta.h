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
 * @brief metaprogramming tools for C.
 */

#ifndef RAM_META_H_IS_INCLUDED
#define RAM_META_H_IS_INCLUDED

/**
 * @internal
 * @brief a support macro for RAM_META_CATTOK().
 * @details this macro is part of a common pattern for concatenating two
 *    tokens with support for the expansion of special preprocessor symbols
 *    such as @c __FILE__.
 * @param A
 *    the left-hand token in the expression @e A @e + @e B @e => @e @e AB.
 * @param B
 *    the right-hand token in the expression @e A @e + @e B @e => @e @e AB.
 */
#define RAM_META_CATTOK2(A, B) A##B

/**
 * @brief concatenate two tokens.
 * @details this macro is a common pattern for concatenating two tokens
 *    with support for the expansion of special preprocessor symbols such
 *    as @c __FILE__.
 * @param A
 *    the left-hand token in the expression @e A @e + @e B @e => @e @e AB.
 * @param B
 *    the right-hand token in the expression @e A @e + @e B @e => @e @e AB.
 */
#define RAM_META_CATTOK(A, B) RAM_META_CATTOK2(A, B)

/**
 * @internal
 * @brief a token counter.
 * @details use RAM_META_COUNTER to generate unique (or semi-unique) tokens
 *    for use in automatically generated identifiers.
 * @remark do not use this function directly to generate identifiers. use
 *    RAM_META_GENNAME() instead.
 * @todo @c __COUNTER__ isn't supported on all versions of all compilers,
 *    so i need to determine when it isn't and use @c __LINE__ instead.
 * @see RAM_META_GENNAME()
 */
#define RAM_META_COUNTER __COUNTER__

/**
 * @brief generate a unique (or semi-unique) C identifier.
 * @details use RAM_META_GENNAME() to generate a unique (or
 *    semi-unique) C identifier, as best as the compiler can manage.
 * @param Prefix
 *    a C identifier that is to be used as the prefix for the generated
 *    identifier. e.g. if @e Prefix is @c tmp then RAM_META_GENNAME()
 *    will produce an identifier such as @c tmp36.
 */
#define RAM_META_GENNAME(Prefix) \
      RAM_META_CATTOK(Prefix, RAM_META_COUNTER)

/**
 * @brief generate an <em><b>if</b>...<b>then</b></em> statement.
 * @details use RAM_META_IFTHEN() to generate an
 *    <em><b>if</b>...<b>then</b></em> statement. it is intended for use
 *    within other preprocessor macros where it is inconvenient to type out
 *    the <em><b>if</b>...<b>then</b></em>,
 *    <em><b>do</b>...<b>while</b> (0)</em> wrapper, and the requisite
 *    backslashes.
 * @param Condition
 *    a boolean expression to be used as the conditional clause.
 * @param Then
 *    a statement, or series of statements to be used as the @e then
 *    clause.
 */
#define RAM_META_IFTHEN(Condition, Then) \
      do \
      { \
         if (Condition) \
         { \
            Then; \
         } \
      } \
      while (0)

/**
 * @brief generate an empty statement.
 * @details use RAM_META_NOP() to generate code that does nothing. it is
 *    useful in situations where something is needed to satisfy lexical
 *    requirements but no actual processing should occur.
 */
#define RAM_META_NOP() do { } while (0)


#endif /* RAM_META_H_IS_INCLUDED */
