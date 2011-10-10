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

# CTest needs to be included in order to support add_test() commands.
include(CTest)

include(cmake/splint.cmake)
include(cmake/cache.cmake)
include(cmake/detect.cmake)

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

# compiler configuration
# ----------------------
if(CMAKE_COMPILER_IS_GNUCC)
	# -Wmissing-braces and -Wmissing-field-initializers appears
	# to interfere with C89-defined behavior.
	set(DEFAULT_CFLAGS "-Wall -Wextra -Wno-missing-braces -Wno-missing-field-initializers")
elseif(MSVC)
	# C4127 pops up every time a "do {} while (0)" is used.
	# C4200 is a good warning but i feel zero-indexed arrays are necessary
	# in this particular library.
	# C4131 warns about old-style declarators, used by getopt().
	set(DEFAULT_CFLAGS "/W4 /wd4127 /wd4200 /wd4131")
else()
	message(WARNING "i don't know how to configure CMAKE_C_FLAGS for this compiler.")
endif()
set_default_cflags(${DEFAULT_CFLAGS} ${DEFAULT_CFLAGS})

# threads
# -------
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

# pstdint
# -------
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

# ramalloc
# --------
# setup include paths and configure where include files will be installed.
include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}/include
	${CMAKE_CURRENT_BINARY_DIR}/include
	)
install(DIRECTORY
		${CMAKE_CURRENT_SOURCE_DIR}/include/ramalloc
		${CMAKE_CURRENT_BINARY_DIR}/include/ramalloc
	DESTINATION include)

# set up cache variable defaults for ramalloc compile-time options.
option(WANT_FEEDBACK
	"enabled (or disables) compile-time option feedback."
	NO)
mark_as_advanced(WANT_FEEDBACK)
optional_cache_string(WANT_DEBUG
	"enables (or disables) debugging features (YES, NO, or DEFAULT).")
optional_cache_string(WANT_MARK_FREED
	"specifies a byte to mark unused memory with (a hexidecimal number or DEFAULT).")
mark_as_advanced(WANT_MARK_FREED)
optional_cache_string(WANT_ZEROED_MEMORY
	"enables (or disables) the zeroing of newly allocated memory (YES, NO, or DEFAULT).")
optional_cache_string(WANT_COMPACT
	"enables (or disables) compact mode, which is not yet tested (YES, NO, or DEFAULT).")
mark_as_advanced(WANT_COMPACT)
optional_cache_string(WANT_COMPACT
	"enables (or disables) compact mode, which is not yet tested (YES, NO, or DEFAULT).")
optional_cache_string(WANT_MINIMUM_PAGE_CAPACITY
	"specifies a minimum page capacity (a number >1 or DEFAULT).")
mark_as_advanced(WANT_MINIMUM_PAGE_CAPACITY)
optional_cache_string(WANT_DEFAULT_RECLAIM_GOAL
	"specifies a default reclaimation goal (a number >=1 or DEFAULT).")
mark_as_advanced(WANT_DEFAULT_RECLAIM_GOAL)
optional_cache_string(WANT_DEFAULT_APPETITE
	"specifies a default appetite (RAMOPT_FRUGAL, RAMOPT_GREEDY, or DEFAULT).")
mark_as_advanced(WANT_DEFAULT_APPETITE)
option(WANT_NPTL_DEADLOCK
	"enables (or disables) the demonstration of a deadlock in NPTL."
	NO)
option(WANT_NO_UNUSED_ARGS
	"enables (or disables) help finding incorrect unused argument annotations."
	NO)
mark_as_advanced(WANT_NO_UNUSED_ARGS)
option(WANT_OVERCONFIDENT
	"enables (or disables) overconfident mode (unsupported)."
	NO)
mark_as_advanced(WANT_OVERCONFIDENT)
configure_file(src/lib/config.h.in
	${CMAKE_CURRENT_BINARY_DIR}/include/ramalloc/config.h @ONLY)

# specify source files.
set(RAMALLOC_HEADERS
	include/ramalloc/algn.h
	include/ramalloc/barrier.h
	include/ramalloc/cast.h
	include/ramalloc/compat.h
	include/ramalloc/default.h
	include/ramalloc/facade.h
	include/ramalloc/fail.h
	include/ramalloc/foot.h
	include/ramalloc/lazy.h
	include/ramalloc/list.h
	include/ramalloc/mem.h
	include/ramalloc/meta.h
	include/ramalloc/misc.h
	include/ramalloc/mtx.h
	include/ramalloc/mux.h
	include/ramalloc/para.h
	include/ramalloc/pg.h
	include/ramalloc/ramalloc.h
	include/ramalloc/reply.h
	include/ramalloc/sig.h
	include/ramalloc/slot.h
	include/ramalloc/slst.h
	include/ramalloc/stdint.h
	include/ramalloc/sys.h
	include/ramalloc/sys/detect.h
	include/ramalloc/sys/gcc.h
	include/ramalloc/sys/linux.h
	include/ramalloc/sys/msvc.h
	include/ramalloc/sys/posix.h
	include/ramalloc/sys/pthreads.h
	include/ramalloc/sys/types.h
	include/ramalloc/sys/win.h
	include/ramalloc/thread.h
	include/ramalloc/tls.h
	include/ramalloc/tra.h
	include/ramalloc/vec.h
	include/ramalloc/want.h
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
add_splint(ramalloc ${RAMALLOC_SOURCES} ${RAMALLOC_HEADERS})
# TODO: i'd like to cannonize this code.
if(WIN32)
	detect_multi_configuration_build(IS_MULTI_CONFIG_BUILD)
	if(IS_MULTI_CONFIG_BUILD)
		foreach(i ${CMAKE_CONFIGURATION_TYPES})
			install(TARGETS ramalloc DESTINATION lib/${i} CONFIGURATIONS ${i})
		endforeach()
	else()
		install(TARGETS ramalloc DESTINATION lib/${CMAKE_BUILD_TYPE})
	endif()
else()
	install(TARGETS ramalloc DESTINATION lib)
endif()

# trio
# ----
# the tests need a portable implementation of fprintf(), so they depend
# on trio.
add_subdirectory(dep/trio)
include_directories(${TRIO_INCLUDE_DIRS})

# tests
# -----

set(TEST_RAMALLOC_SOURCES
	src/test/shared/parseargs.c
	src/test/shared/test.c
	)
set(TEST_RAMALLOC_HEADERS
	src/test/shared/parseargs.h
	src/test/shared/test.h
	)
add_library(testramalloc
	${TEST_RAMALLOC_SOURCES}
	${TEST_RAMALLOC_HEADERS}
	${RAMALLOC_WINDOWS_GETOPT_SOURCES}
	)
target_link_libraries(testramalloc ramalloc trio ${CMAKE_THREAD_LIBS_INIT})
add_splint(testramalloc
	${TEST_RAMALLOC_SOURCES}
	${TEST_RAMALLOC_HEADERS}
	${RAMALLOC_WINDOWS_GETOPT_SOURCES}
	)

set(CASTTEST_SOURCES src/test/casttest.c)
add_executable(casttest ${CASTTEST_SOURCES})
add_splint(casttest ${CASTTEST_SOURCES})
target_link_libraries(casttest testramalloc)
add_test(casttest ${EXECUTABLE_OUTPUT_PATH}/casttest)

set(SLOTTEST_SOURCES src/test/slottest.c)
add_executable(slottest ${SLOTTEST_SOURCES})
add_splint(slottest ${SLOTTEST_SOURCES})
target_link_libraries(slottest testramalloc)
add_test(slottest ${EXECUTABLE_OUTPUT_PATH}/slottest
	--rng-seed=1564740528)

set(PGTEST_SOURCES src/test/pgtest.c)
add_executable(pgtest ${PGTEST_SOURCES})
add_splint(pgtest ${PGTEST_SOURCES})
target_link_libraries(pgtest testramalloc)
add_test(pgtest ${EXECUTABLE_OUTPUT_PATH}/pgtest
	--allocations=10240 --rng-seed=1342375263)

set(ALGNTEST_SOURCES src/test/algntest.c)
add_executable(algntest ${ALGNTEST_SOURCES})
add_splint(algntest ${ALGNTEST_SOURCES})
target_link_libraries(algntest testramalloc)
add_test(algntest-8 ${EXECUTABLE_OUTPUT_PATH}/algntest
	--smallest=8 --largest=8
	--rng-seed=2426525316)
add_test(algntest-128 ${EXECUTABLE_OUTPUT_PATH}/algntest
	--smallest=128 --largest=128
	--rng-seed=2694545209)

set(MUXTEST_SOURCES src/test/muxtest.c)
add_executable(muxtest ${MUXTEST_SOURCES})
add_splint(muxtest ${MUXTEST_SOURCES})
target_link_libraries(muxtest testramalloc)
add_test(muxtest ${EXECUTABLE_OUTPUT_PATH}/muxtest
	--rng-seed=2596644741)

set(LAZYTEST_SOURCES src/test/lazytest.c)
add_executable(lazytest ${LAZYTEST_SOURCES})
add_splint(lazytest ${LAZYTEST_SOURCES})
target_link_libraries(lazytest testramalloc)
add_test(lazytest-serial ${EXECUTABLE_OUTPUT_PATH}/lazytest --parallelize=1
	--rng-seed=342795249)
add_test(lazytest-parallel ${EXECUTABLE_OUTPUT_PATH}/lazytest)

set(PARATEST_SOURCES src/test/paratest.c)
add_executable(paratest ${PARATEST_SOURCES})
add_splint(paratest ${PARATEST_SOURCES})
target_link_libraries(paratest testramalloc)
add_test(paratest-serial ${EXECUTABLE_OUTPUT_PATH}/paratest --parallelize=1
	--rng-seed=2374434648)
add_test(paratest-parallel ${EXECUTABLE_OUTPUT_PATH}/paratest)

set(DEFAULTTEST_SOURCES src/test/defaulttest.c)
add_executable(defaulttest ${DEFAULTTEST_SOURCES})
add_splint(defaulttest ${DEFAULTTEST_SOURCES})
target_link_libraries(defaulttest testramalloc)
add_test(defaulttest-serial ${EXECUTABLE_OUTPUT_PATH}/defaulttest
	--parallelize=1	--rng-seed=2828559559)
add_test(defaulttest-parallel ${EXECUTABLE_OUTPUT_PATH}/defaulttest)

set(COMPATTEST_SOURCES src/test/compattest.c)
add_executable(compattest ${COMPATTEST_SOURCES})
add_splint(compattest ${COMPATTEST_SOURCES})
target_link_libraries(compattest testramalloc)
add_test(compattest ${EXECUTABLE_OUTPUT_PATH}/compattest)

# install the README and LICENSE files.
if(UNIX)
	install(FILES LICENSE README.md	ROFLME.md
		DESTINATION share/ramalloc
		)
else(UNIX)
	install(FILES LICENSE README.md ROFLME.md DESTINATION .)
endif(UNIX)

# documentation
# -------------
include(cmake/doxygen.cmake)
add_doxygen(ramalloc ramalloc
	src/doc/groups.h ${RAMALLOC_HEADERS} ${RAMALLOC_SOURCES})
# TODO: i don't yet how to force the generation of documentation
# if "package" is the target.
#add_dependencies(package doxygen)
# TODO: i can't use ${DOXYGEN_OUTPUT_DIRECTORY} because there's quotes in
# it. i'll have to refactor doxygen.cmake to offer versions without quotes
# and versions with quotes to substitute into Doxyfile.
if(UNIX)
	install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/doxygen/html
		DESTINATION share/doc/ramalloc
		)
else()
	install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/doxygen/html
		DESTINATION doc
		)
endif()
