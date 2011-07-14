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

# TODO: the CMake documentation for ExternalProjects says that the build
# steps have to be split up to be compatible with submitting to the 
# dashboard.

include(ExternalProject)

option(TRIO_DOWNLOAD 
	"indicates that i should download and build trio, if i can't find it."
	NO)
option(TRIO_RUN_TESTS 
	"indicates whether i should run tests on trio after building." 
	YES)
	
set(TRIO_DOWNLOAD_URL "http://ncu.dl.sourceforge.net/project/ctrio/trio/1.14/trio-1.14.tar.gz" 
	CACHE STRING 
	"a URL describing where trio can be fetched.")
set(TRIO_MD5SUM 0278513e365675ca62bacb6f257b5045
	CACHE STRING
	"the MD5 sum of the file at TRIO_DOWNLOAD_URL.")

set(TRIO_LIBRARY_DOC "path(s) to the trio library.")
set(TRIO_DEFAULT_PREFIX ${CMAKE_CURRENT_BINARY_DIR}/trio-prefix)

if(WIN32)
	set(TRIO_LIBRARY_NAME trio.lib)
else()
	set(TRIO_LIBRARY_NAME libtrio.a)
endif()

if(${CMAKE_CONFIGURATION_TYPES})
	# there's no official binary distribution for Windows. given the need to
	# support multi-configuration builds and without any convention for 
	# discovering where libraries for each configuration can be found, i'm
	# not going to try to find the library. feel free to set TRIO_LIBRARY
	# if you know where it is.

	# i can't make the following message an error that stops configuration.
	# if i did, it would prevent other options from being added to the
	# cache and would force the user to hit the configuration button more
	# than is really necessary.
	if(NOT TRIO_LIBRARY AND NOT TRIO_DOWNLOAD)
		message(WARNING "i don't know where trio is on your system. please put the path to trio.lib in TRIO_LIBRARY. or, if you prefer, you can set TRIO_DOWNLOAD to 1 if you would like me to download and build it for you.")
	endif()
else()
	find_library(TRIO_LIBRARY ${TRIO_LIBRARY_NAME} DOC ${TRIO_LIBRARY_DOC})
	# i can't make the following message an error that stops configuration.
	# if i did, it would prevent other options from being added to the
	# cache and would force the user to hit the configuration button more
	# than is really necessary.
	if(NOT TRIO_LIBRARY AND NOT TRIO_DOWNLOAD)
		message(WARNING "i couldn't find trio on your system. please put the path to libtrio.a in TRIO_LIBRARY. or, if you prefer, you can set TRIO_DOWNLOAD to 1 if you would like me to download and build it for you.")
	endif()
endif()
	
if(NOT TRIO_IS_A_TARGET AND TRIO_DOWNLOAD)
	if(UNIX)
		ExternalProject_Add(trio
			PREFIX ${TRIO_DEFAULT_PREFIX}
			URL ${TRIO_DOWNLOAD_URL}
			URL_MD5 ${TRIO_MD5SUM}
			CONFIGURE_COMMAND ${TRIO_DEFAULT_PREFIX}/src/trio/configure --prefix=${TRIO_DEFAULT_PREFIX}
			TEST_BEFORE_INSTALL ${TRIO_RUN_TESTS}	
			)
	elseif(WIN32)
		# trio doesn't provide any capability to build on Windows 
		# out-of-the-box, so i use the patch stage to add a CMakeLists.txt
		# file into the project before building.
		# TODO: there's a bug in CMake <=3.8.5 that prevents ExternalProject_Add()
		# from installing trio into ${TRIO_DEFAULT_PREFIX}. i need to add
		# a version check here once i know which release fixes it.
		file(TO_NATIVE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/trio/CMakeLists.txt" src)
		file(TO_NATIVE_PATH "${TRIO_DEFAULT_PREFIX}/src/trio/" dest)
		ExternalProject_Add(trio
			PREFIX ${TRIO_DEFAULT_PREFIX}
			URL ${TRIO_DOWNLOAD_URL}
			URL_MD5 ${TRIO_MD5SUM}
			# BUG: trio's regression tests appear to fail on Windows. it's
			# related to a specifier that i don't use (%m), so i'm not going
			# worry too much about it. when i'm aware that a fix has been 
			# released, i'll reenable tests.
			#TEST_BEFORE_INSTALL ${TRIO_RUN_TESTS}
			# WORKAROUND: the following option needs to be last, as CMake
			# appears to confuse arguments that follow for part of the
			# command.
			PATCH_COMMAND copy "${src}" "${dest}"
			)
	else()
		message(FATAL_ERROR "i don't know how to compile an external project on this platform.")
	endif()
	if(${CMAKE_CONFIGURATION_TYPES})
		foreach(i ${CMAKE_CONFIGURATION_TYPES})
			# WORKAROUND: target_link_libraries() doesn't recognize configuration
			# names. instead, it recognizes keywords which cover categories
			# of configurations. i elected to use Debug for *debug* and
			# RelWithDebugInfo for everything else. unfortunately, this is 
			# going to cause ExternalProject.cmake to compile unnecessary
			# versions of trio for other configurations.
			if(i STREQUAL Debug)
				list(APPEND trio_libs debug)
				list(APPEND trio_libs "${TRIO_DEFAULT_PREFIX}/lib/${i}/${TRIO_LIBRARY_NAME}")
			elseif(i STREQUAL RelWithDebugInfo)
				list(APPEND trio_libs general)
				list(APPEND trio_libs "${TRIO_DEFAULT_PREFIX}/lib/${i}/${TRIO_LIBRARY_NAME}")
			endif()
		endforeach()
		set(TRIO_LIBRARY 
			${trio_libs} 
			CACHE STRING ${TRIO_LIBRARY_DOC} FORCE
			)
	else()
		set(TRIO_LIBRARY 
			${TRIO_DEFAULT_PREFIX}/lib/${TRIO_LIBRARY_NAME} 
			CACHE FILEPATH ${TRIO_LIBRARY_DOC} FORCE
			)
	endif()
	set(TRIO_IS_A_TARGET YES)
endif()

get_filename_component(TRIO_PREFIX "${TRIO_LIBRARY}" PATH)
# TODO: i want to cannonize this test into a function.
if(${CMAKE_CONFIGURATION_TYPES})
	get_filename_component(TRIO_PREFIX "${TRIO_PREFIX}/../.." ABSOLUTE)
else()
	get_filename_component(TRIO_PREFIX "${TRIO_PREFIX}/.." ABSOLUTE)
endif()

include_directories("${TRIO_PREFIX}/include")

function(add_dependency_on_trio TARGET)
	set(TRIO_LIBRARIES 
		${TRIO_LIBRARY}
		)
	if(TRIO_IS_A_TARGET)
		add_dependencies(${TARGET} trio)
	endif()
	get_target_property(Type ${TARGET} TYPE)
	if(Type STREQUAL EXECUTABLE OR Type STREQUAL SHARED_LIBRARY)
		if(UNIX)
			set(dependencies m)
		endif()
		target_link_libraries(${TARGET} ${TRIO_LIBRARY} ${dependencies})
	endif()
endfunction()

