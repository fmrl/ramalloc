# This file is part of the *ramalloc* project at <http://fmrl.org>.
# Copyright (c) 2011, Michael Lowell Roberts.
# All rights reserved. 
#
# Redistribution and use in source and binary forms, with or without 
# modification, are permitted provided that the following conditions are 
# met: 
#
#  * Redistributions of source code must retain the above copyright 
#  notice, this list of conditions and the following disclaimer. 
#
#  * Redistributions in binary form must reproduce the above copyright 
#  notice, this list of conditions and the following disclaimer in the 
#  documentation and/or other materials provided with the distribution.
# 
#  * Neither the name of the copyright holder nor the names of 
#  contributors may be used to endorse or promote products derived 
#  from this software without specific prior written permission. 
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS 
# IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED 
# TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A 
# PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER 
# OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
# SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED 
# TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR 
# PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF 
# LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING 
# NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS 
# SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 

# i use CMake 2.8, so that's what i know works. it might work
# with earlier versions, however (especially 2.6).
cmake_minimum_required(VERSION 2.8)
project(ramalloc)

include(CTest)

# the following code sets a default for cache variables (such as 
# CMAKE_C_FLAGS) that can be changed by the user.
IF(NOT RAMALLOC_IS_CONFIGURED)
   # Change default values.
   # TODO: this won't work for windows.
   set(CMAKE_C_FLAGS -Wall)

   # save default value in cache.
   SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}" CACHE STRING
       "contains options always passed to the C compiler." FORCE)

   # there's no longer any need to generate defaults.
   SET(RAMALLOC_IS_CONFIGURED 1 CACHE INTERNAL
       "indicates that ramalloc has been configured at least once.")
ENDIF(NOT RAMALLOC_IS_CONFIGURED)

# currently, threads are required to compile ramalloc.
#
# TODO: if threads aren't available, it's still possible to produce
# a library but the following changes need to be made:
#
#	* the default module should use a mux pool, instead of a 
#	parallelized pool.
#	* the modules that depend on threads should not be excluded
#	from the build.
find_package(Threads REQUIRED)

# for Microsoft platforms, i use pstdint to emulate <stdint.h>.
# new versions can be fetched from 
# <http://www.azillionmonkeys.com/qed/pstdint.h>.
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/dep/pstdint)

# ramalloc's headers.
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

add_library(ramalloc
	include/ramalloc/algn.h
	include/ramalloc/barrier.h
	include/ramalloc/compat.h
	include/ramalloc/default.h
	include/ramalloc/fail.h
	include/ramalloc/foot.h
	include/ramalloc/lazy.h
	include/ramalloc/list.h
	include/ramalloc/mem.h
	include/ramalloc/meta.h
	include/ramalloc/misc.h
	include/ramalloc/mtx.h
	include/ramalloc/mux.h
	include/ramalloc/opt.h
	include/ramalloc/para.h
	include/ramalloc/pg.h
	include/ramalloc/ramalloc.h
	include/ramalloc/sig.h
	include/ramalloc/slot.h
	include/ramalloc/slst.h
	include/ramalloc/stdint.h
	include/ramalloc/sys.h
	include/ramalloc/sys/detect.h
	include/ramalloc/sys/linux.h
	include/ramalloc/sys/posix.h
	include/ramalloc/sys/pthreads.h
	include/ramalloc/sys/types.h
	include/ramalloc/sys/win.h
	include/ramalloc/thread.h
	include/ramalloc/tls.h
	include/ramalloc/tra.h
	include/ramalloc/vec.h
	src/lib/algn.c
	src/lib/barrier.c
	src/lib/compat.c
	src/lib/default.c
	src/lib/fail.c
	src/lib/foot.c
	src/lib/lazy.c
	src/lib/list.c
	src/lib/mem.c
	src/lib/misc.c
	src/lib/mtx.c
	src/lib/mux.c
	src/lib/para.c
	src/lib/pg.c
	src/lib/ramalloc.c
	src/lib/sig.c
	src/lib/slot.c
	src/lib/slst.c
	src/lib/sys/linux.c
	src/lib/sys/posix.c
	src/lib/sys/pthreads.c
	src/lib/sys/win.c
	src/lib/thread.c
	src/lib/tls.c
	src/lib/tra.c
	src/lib/vec.c
	)
	
set(RAMALLOC_LIBRARIES ramalloc ${CMAKE_THREAD_LIBS_INIT})

add_executable(slottest
	src/test/slottest.c
	src/test/shared/test.c
	src/test/shared/test.h
	)
target_link_libraries(slottest ${RAMALLOC_LIBRARIES})
add_test(slottest ${EXECUTABLE_OUTPUT_PATH}/slottest)

add_executable(pgtest
	src/test/pgtest.c
	src/test/shared/test.c
	src/test/shared/test.h
	)
target_link_libraries(pgtest ${RAMALLOC_LIBRARIES})
add_test(pgtest ${EXECUTABLE_OUTPUT_PATH}/pgtest)

add_executable(algntest
	src/test/algntest.c
	src/test/shared/test.c
	src/test/shared/test.h
	)
target_link_libraries(algntest ${RAMALLOC_LIBRARIES})
add_test(algntest ${EXECUTABLE_OUTPUT_PATH}/algntest)

add_executable(muxtest
	src/test/muxtest.c
	src/test/shared/test.c
	src/test/shared/test.h
	)
target_link_libraries(muxtest ${RAMALLOC_LIBRARIES})
add_test(muxtest ${EXECUTABLE_OUTPUT_PATH}/muxtest)

add_executable(lazytest
	src/test/lazytest.c
	src/test/shared/parseargs.c
	src/test/shared/parseargs.h
	src/test/shared/getopt.h
	src/test/shared/test.c
	src/test/shared/test.h
	)
target_link_libraries(lazytest ${RAMALLOC_LIBRARIES})
add_test(lazytest ${EXECUTABLE_OUTPUT_PATH}/lazytest)

add_executable(paratest
	src/test/paratest.c
	src/test/shared/test.c
	src/test/shared/test.h
	)
target_link_libraries(paratest ${RAMALLOC_LIBRARIES})
add_test(paratest ${EXECUTABLE_OUTPUT_PATH}/paratest)

add_executable(defaulttest
	src/test/defaulttest.c
	src/test/shared/test.c
	src/test/shared/test.h
	)
target_link_libraries(defaulttest ${RAMALLOC_LIBRARIES})
add_test(defaulttest ${EXECUTABLE_OUTPUT_PATH}/defaulttest)

add_executable(compattest
	src/test/compattest.c
	)
target_link_libraries(compattest ${RAMALLOC_LIBRARIES})
add_test(compattest ${EXECUTABLE_OUTPUT_PATH}/compattest)

