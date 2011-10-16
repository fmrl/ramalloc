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

#include <ramalloc/foot.h>
#include <ramalloc/mem.h>
#include <assert.h>
#include <memory.h>

static uintptr_t ramfoot_alignright(uintptr_t value_arg, uintptr_t length_arg, uintptr_t alignment_arg);
static ram_reply_t ramfoot_footeraddr(char **footer_arg,
      const ramfoot_spec_t *spec_arg, void *ptr_arg);
static ram_reply_t ramfoot_getfooter(char **result_arg, const ramfoot_spec_t *spec_arg, void *ptr_arg);

#define RAMFOOT_ALIGNLEFT(Value, Alignment) ((Value) & (~((Alignment) - 1)))
#define RAMFOOT_ISSPECINIT(Spec) ((Spec)->footer_offset != 0)

uintptr_t ramfoot_alignright(uintptr_t value_arg, uintptr_t length_arg, uintptr_t alignment_arg)
{
   return RAMFOOT_ALIGNLEFT(value_arg, alignment_arg) + (alignment_arg - length_arg);
}

ram_reply_t ramfoot_mkspec(ramfoot_spec_t *spec_arg, size_t writezn_arg, 
   size_t footsz_arg, size_t footalign_arg, size_t storofs_arg, const char *sig_arg)
{
   ramfoot_spec_t stage = {0};
   uintptr_t n = 0;
   size_t pgsz = 0;

   RAM_FAIL_NOTNULL(spec_arg);
   memset(spec_arg, 0, sizeof(*spec_arg));
   RAM_FAIL_NOTZERO(writezn_arg);
   RAM_FAIL_NOTZERO(footsz_arg);
   RAM_FAIL_NOTZERO(footalign_arg);
   RAM_FAIL_NOTNULL(sig_arg);

   RAM_FAIL_TRAP(rammem_pagesize(&pgsz));
   RAM_FAIL_EXPECT(RAM_REPLY_RANGEFAIL, writezn_arg <= pgsz);
   stage.writable_zone = writezn_arg;
   stage.storage_offset = storofs_arg;
   stage.footer_size = footsz_arg;
   stage.footer_alignment = footalign_arg;

   /* i calculate the offset from a page'stage address to the footer'stage address. */
   n = ramfoot_alignright(0, footsz_arg, writezn_arg);
   stage.footer_offset = RAMFOOT_ALIGNLEFT(n, footalign_arg);

   RAM_FAIL_TRAP(ramsig_init(&stage.master_signature, sig_arg));

   *spec_arg = stage;
   return RAM_REPLY_OK;
}

ram_reply_t ramfoot_footeraddr(char **footer_arg,
      const ramfoot_spec_t *spec_arg, void *ptr_arg)
{
   char *p = NULL;

   assert(spec_arg != NULL);
   assert(RAMFOOT_ISSPECINIT(spec_arg));

   RAM_FAIL_TRAP(rammem_getpage(&p, ptr_arg));
   *footer_arg = p + spec_arg->footer_offset;

   return RAM_REPLY_OK;
}

ram_reply_t ramfoot_getfooter(char **result_arg,
      const ramfoot_spec_t *spec_arg, void *ptr_arg)
{
   char *p = NULL;

   RAM_FAIL_NOTNULL(result_arg);
   *result_arg = NULL;
   RAM_FAIL_NOTNULL(spec_arg);
   RAM_FAIL_NOTNULL(ptr_arg);

   RAM_FAIL_TRAP(ramfoot_footeraddr(&p, spec_arg, ptr_arg));

   /* the first four bytes of the footer should always match the master signature. this is considered
    * an expected error, however, since it is used to detect whether an alternate solution should be
    * used. */
   if (0 == RAMSIG_CMP(*((const ramsig_signature_t *)p), spec_arg->master_signature))
   {
      *result_arg = p;
      return RAM_REPLY_OK;
   }
   else
      return RAM_REPLY_NOTFOUND;
}

ram_reply_t ramfoot_mkfooter(void **storage_arg, const ramfoot_spec_t *spec_arg, void *page_arg)
{
   char *p = NULL;
   int ispage = 0;

   RAM_FAIL_NOTNULL(storage_arg);
   *storage_arg = NULL;
   RAM_FAIL_NOTNULL(spec_arg);
   RAM_FAIL_EXPECT(RAM_REPLY_INCONSISTENT, RAMFOOT_ISSPECINIT(spec_arg));
   RAM_FAIL_NOTNULL(page_arg);
   RAM_FAIL_TRAP(rammem_ispage(&ispage, page_arg));
   RAM_FAIL_EXPECT(RAM_REPLY_DISALLOWED, ispage);

   RAM_FAIL_TRAP(ramfoot_footeraddr(&p, spec_arg, page_arg));
   *((ramsig_signature_t *)p) = spec_arg->master_signature;

   *storage_arg = p + spec_arg->storage_offset;
   return RAM_REPLY_OK;
}

ram_reply_t ramfoot_getstorage(void **result_arg,
      const ramfoot_spec_t *spec_arg, void *ptr_arg)
{
   char *p = NULL;
   ram_reply_t e = RAM_REPLY_INSANE;

   e = ramfoot_getfooter(&p, spec_arg, ptr_arg);
   switch (e)
   {
   default:
      RAM_FAIL_TRAP(e);
      /* i shouldn't ever get here. */
      return RAM_REPLY_INSANE;
   case RAM_REPLY_NOTFOUND:
      return e;
   case RAM_REPLY_OK:
      break;
   }

   *result_arg = p + spec_arg->storage_offset;
   return RAM_REPLY_OK;
}


