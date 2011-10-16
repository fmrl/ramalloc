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
project(trio)

# CTest needs to be included in order to support add_test() commands.
include(CTest)

set(TRIO_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR} PARENT_SCOPE)
include_directories(${TRIO_INCLUDE_DIRS})

set(TRIO_SOURCES
	triostr.c
	trio.c
	trionan.c
	)
set(TRIO_HEADERS
	trio.h
	triop.h
	triodef.h
	trionan.h
	triostr.h
	)
add_library(trio ${TRIO_SOURCES} ${TRIO_HEADERS})
if(UNIX)
	target_link_libraries(trio -lm)
endif()

install(
	FILES ${TRIO_HEADERS}
	DESTINATION include
	)
# if this is a single-configuration build, then the library is installed into
# the *lib* directory. otherwise, i need to install the library into the
# appropriate subdirectory to ensure that they are all available after a
# batch build.
# TODO: i'd like to cannonize this code.
if(WIN32)
	if(DEFINED CMAKE_CONFIGURATION_TYPES)
		foreach(i ${CMAKE_CONFIGURATION_TYPES})
			install(TARGETS trio DESTINATION lib/${i} CONFIGURATIONS ${i})
		endforeach()
	else()
		install(TARGETS trio DESTINATION lib/${CMAKE_BUILD_TYPE})
	endif()
else()
	install(TARGETS trio DESTINATION lib)
endif()

add_executable(trio-regression regression.c)
target_link_libraries(trio-regression trio)
# [todo] on windows vista, the regression test crashes if you don't run it 
# with elevated privledges, so i have to disable this test for windows in 
# general until i can learn how to detect vista.
if(NOT WIN32)
	add_test(trio-regression ${EXECUTABLE_OUTPUT_PATH}/trio-regression)
endif()
