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

include(cmake/splint.cmake)

# CPack integration
# -----------------
# TODO: i'm not yet sure how to write the following include file but i'm
# interested...
#include(InstallRequiredSystemLibraries)
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "a parallelized, amortized-constant time allocator for objects smaller than a hardware page.")
set(CPACK_PACKAGE_DESCRIPTION "ramalloc is a parallelized, amortized-constant time allocator for objects smaller than a hardware page. it exhibits deterministic performance characteristics, which is necessary for soft real-time applications. it also reduces memory fragmentation, which is important for long-running processes. ramalloc is designed for game and interpreter development but should be useful for other applications as well. ramalloc is also parallelized; there is no global resource that requires serialized access.")
set(CPACK_PACKAGE_VENDOR "fmrl")
set(CPACK_PACKAGE_DESCRIPTION_FILE 
	"${CMAKE_CURRENT_SOURCE_DIR}/README.md")
set(CPACK_RESOURCE_FILE_LICENSE
	"${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
# TODO: the following should probably be set in a variable and perhaps in
# a separate file.
set(CPACK_PACKAGE_VERSION_MAJOR "0")
set(CPACK_PACKAGE_VERSION_MINOR "0")
set(CPACK_PACKAGE_VERSION_PATCH "0")
if(UNIX)
	set(CPACK_GENERATOR "TGZ")
else(UNIX)
	set(CPACK_GENERATOR "ZIP")
endif(UNIX)
include(CPack)

# the following code sets a default for cache variables (such as 
# CMAKE_C_FLAGS) that can be changed by the user.
if(NOT RAMALLOC_IS_CONFIGURED)
	# CMake doesn't enable any warnings by default.
	if(CMAKE_COMPILER_IS_GNUCC)
		# -Wmissing-braces and -Wmissing-field-initializers appears
		# to interfere with C89-defined behavior.
		set(CMAKE_C_FLAGS "-Wall -Wextra -Wno-missing-braces -Wno-missing-field-initializers")
	elseif(MSVC)
		set(CMAKE_C_FLAGS "/W4")
	endif()
	# save defaults to cache.
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}" CACHE STRING
	   "contains options always passed to the C compiler." FORCE)
	# there's no longer any need to generate defaults.
	set(RAMALLOC_IS_CONFIGURED 1 CACHE INTERNAL
	   "indicates that ramalloc has been configured at least once.")
endif(NOT RAMALLOC_IS_CONFIGURED)

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

# i use pstdint to emulate <stdint.h> when necessary (splint and Windows).
# the latest version can be fetched from 
# <http://www.azillionmonkeys.com/qed/pstdint.h>.
# TODO: this may not be necessary for VS2010 and later.
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/dep/pstdint)
install(FILES dep/pstdint/pstdint.h DESTINATION include)

# on Windows platforms, i use the BSD implementation of getopt.
# TODO: i should probably make this a separate library.
if(WIN32)
	include_directories(${CMAKE_CURRENT_SOURCE_DIR}/dep/freebsd/include)
	install(FILES dep/freebsd/include/getopt.h DESTINATION include)
	set(RAMALLOC_WINDOWS_GETOPT_SOURCES 
		dep/freebsd/include/getopt.h
		dep/freebsd/src/getopt.c
		dep/freebsd/src/getopt_long.c
		)
endif(WIN32)

# ramalloc's headers.
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/ramalloc 
	DESTINATION include)

set(RAMALLOC_HEADERS
	include/ramalloc/algn.h
	include/ramalloc/barrier.h
	include/ramalloc/cast.h
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
	)
set(RAMALLOC_SOURCES
	src/lib/algn.c
	src/lib/barrier.c
	src/lib/cast.c
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

add_library(ramalloc ${RAMALLOC_SOURCES} ${RAMALLOC_HEADERS})
install(TARGETS ramalloc DESTINATION lib)

# tests
# -----

set(RAMALLOC_LIBRARIES ramalloc ${CMAKE_THREAD_LIBS_INIT})
set(RAMALLOC_SHARED_TEST_SOURCES
	src/test/shared/parseargs.c
	src/test/shared/test.c
	)
set(RAMALLOC_SHARED_TEST_HEADERS
	src/test/shared/parseargs.h
	src/test/shared/test.h
	)

add_executable(slottest
	src/test/slottest.c
	${RAMALLOC_SHARED_TEST_SOURCES}
	${RAMALLOC_SHARED_TEST_HEADERS}
	${RAMALLOC_WINDOWS_GETOPT_SOURCES}
	)
target_link_libraries(slottest ${RAMALLOC_LIBRARIES})
add_test(slottest ${EXECUTABLE_OUTPUT_PATH}/slottest
	--rng-seed=1564740528)

add_executable(pgtest
	src/test/pgtest.c
	${RAMALLOC_SHARED_TEST_SOURCES}
	${RAMALLOC_SHARED_TEST_HEADERS}
	${RAMALLOC_WINDOWS_GETOPT_SOURCES}
	)
target_link_libraries(pgtest ${RAMALLOC_LIBRARIES})
add_test(pgtest ${EXECUTABLE_OUTPUT_PATH}/pgtest
	--allocations=10240 --rng-seed=1342375263)

add_executable(algntest
	src/test/algntest.c
	${RAMALLOC_SHARED_TEST_SOURCES}
	${RAMALLOC_SHARED_TEST_HEADERS}
	${RAMALLOC_WINDOWS_GETOPT_SOURCES}
	)
target_link_libraries(algntest ${RAMALLOC_LIBRARIES})
add_test(algntest-8 ${EXECUTABLE_OUTPUT_PATH}/algntest
	--smallest=8 --largest=8
	--rng-seed=2426525316)
add_test(algntest-128 ${EXECUTABLE_OUTPUT_PATH}/algntest
	--smallest=128 --largest=128
	--rng-seed=2694545209)

add_executable(muxtest
	src/test/muxtest.c
	${RAMALLOC_SHARED_TEST_SOURCES}
	${RAMALLOC_SHARED_TEST_HEADERS}
	${RAMALLOC_WINDOWS_GETOPT_SOURCES}
	)
target_link_libraries(muxtest ${RAMALLOC_LIBRARIES})
add_test(muxtest ${EXECUTABLE_OUTPUT_PATH}/muxtest
	--rng-seed=2596644741)

add_executable(lazytest
	src/test/lazytest.c
	${RAMALLOC_SHARED_TEST_SOURCES}
	${RAMALLOC_SHARED_TEST_HEADERS}
	${RAMALLOC_WINDOWS_GETOPT_SOURCES}
	)
target_link_libraries(lazytest ${RAMALLOC_LIBRARIES})
add_test(lazytest-serial ${EXECUTABLE_OUTPUT_PATH}/lazytest --parallelize=1 
	--rng-seed=342795249)
add_test(lazytest-parallel ${EXECUTABLE_OUTPUT_PATH}/lazytest)

add_executable(paratest
	src/test/paratest.c
	${RAMALLOC_SHARED_TEST_SOURCES}
	${RAMALLOC_SHARED_TEST_HEADERS}
	${RAMALLOC_WINDOWS_GETOPT_SOURCES}
	)
target_link_libraries(paratest ${RAMALLOC_LIBRARIES})
add_test(paratest-serial ${EXECUTABLE_OUTPUT_PATH}/paratest --parallelize=1 
	--rng-seed=2374434648)
add_test(paratest-parallel ${EXECUTABLE_OUTPUT_PATH}/paratest)

add_executable(defaulttest
	src/test/defaulttest.c
	${RAMALLOC_SHARED_TEST_SOURCES}
	${RAMALLOC_SHARED_TEST_HEADERS}
	${RAMALLOC_WINDOWS_GETOPT_SOURCES}
	)
target_link_libraries(defaulttest ${RAMALLOC_LIBRARIES})
add_test(defaulttest-serial ${EXECUTABLE_OUTPUT_PATH}/defaulttest 
	--parallelize=1	--rng-seed=2828559559)
add_test(defaulttest-parallel ${EXECUTABLE_OUTPUT_PATH}/defaulttest)

add_executable(compattest
	src/test/compattest.c
	)
target_link_libraries(compattest ${RAMALLOC_LIBRARIES})
add_test(compattest ${EXECUTABLE_OUTPUT_PATH}/compattest)

# install the README and LICENSE files.
if(UNIX)
	install(FILES LICENSE README.md	ROFLME.md 
		DESTINATION share/ramalloc
		)
else(UNIX)
	install(FILES LICENSE README.md ROFLME.md DESTINATION .)
endif(UNIX)

# splint
# ------
if(WIN32)
	option(WANT_SPLINT 
		"integrate splint into the build (not yet functioning on Windows)."
		NO
		)
else()
	option(WANT_SPLINT "integrate splint into the build." YES)
endif()
if(WANT_SPLINT)
	if(WIN32)
		message(WARNING "splint is not yet functioning on Windows.")
	endif()
	add_splint(
		${RAMALLOC_SOURCES}
		${RAMALLOC_SHARED_TEST_SOURCES}
		src/test/slottest.c
		src/test/pgtest.c
		src/test/algntest.c
		src/test/muxtest.c
		src/test/lazytest.c
		src/test/paratest.c
		src/test/defaulttest.c
		src/test/compattest.c
		)
endif()

# documentation
# -------------
include(cmake/doxygen.cmake)
add_doxygen(ramalloc ${RAMALLOC_HEADERS} ${RAMALLOC_SOURCES})

