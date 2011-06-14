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

find_package(Doxygen)
# currently, threads are required to compile ramalloc.
find_package(Threads REQUIRED)

option(RAMALLOC_DOWNLOAD 
	"indicates that i should download and build ramalloc, if i can't find it."
	NO)
option(RAMALLOC_RUN_TESTS 
	"indicates whether i should run tests on ramalloc after building." 
	YES)
option(RAMALLOC_WANT_DOCS
	"indicates whether i should build the ramalloc documentation; requires doxygen." 
	${DOXYGEN_FOUND})

if(RAMALLOC_DOWNLOAD)
	find_package(Git)
endif()

set(RAMALLOC_URL IGNORE
	CACHE STRING
	"a URL describing where to download an installation archive from. set to IGNORE if you wish to fetch with git.")
set(RAMALLOC_GIT_REPOSITORY "git://github.com/fmrl/ramalloc.git" 
	CACHE STRING 
	"a URL describing where the ramalloc git repository is located.")
set(RAMALLOC_GIT_TAG "install"
	CACHE STRING 
	"the git branch, tag, or commit it to retrieve.")

set(RAMALLOC_STATIC_LIBRARY_NAME libramalloc.a)
set(RAMALLOC_STATIC_LIBRARY_DOC "path to the ramalloc static library.")
set(RAMALLOC_CMAKE_ARGS )
set(RAMALLOC_BINDIR_PREFIX ${CMAKE_CURRENT_BINARY_DIR}/ramalloc-prefix)

find_library(RAMALLOC_LIBRARY libramalloc.a DOC ${RAMALLOC_STATIC_LIBRARY_DOC})

if(NOT RAMALLOC_LIBRARY AND NOT RAMALLOC_DOWNLOAD)
	message(SEND_ERROR "i couldn't find ramalloc on your system. set RAMALLOC_DOWNLOAD=1 if you would like me to download and install it for you.")
elseif(RAMALLOC_LIBRARY STREQUAL ${RAMALLOC_BINDIR_PREFIX}/lib/${RAMALLOC_STATIC_LIBRARY_NAME}
		OR NOT RAMALLOC_LIBRARY)
	if(NOT RAMALLOC_IS_A_TARGET AND RAMALLOC_DOWNLOAD)
		if(RAMALLOC_URL)
			set(download_args 
				URL ${RAMALLOC_URL}
				)
		elseif(GIT_FOUND)
			set(download_args 
				GIT_TAG ${RAMALLOC_GIT_TAG}
				GIT_REPOSITORY ${RAMALLOC_GIT_REPOSITORY}
				)
		else()
			message(SEND_ERROR 
				"if you want me to download ramalloc, you must either install git or provide the URL of an installation archive.")
		endif() 

		include(ExternalProject)
		ExternalProject_Add(ramalloc
			PREFIX ${RAMALLOC_BINDIR_PREFIX}
			${download_args}
			CMAKE_ARGS 
				-DCMAKE_INSTALL_PREFIX:path=${RAMALLOC_BINDIR_PREFIX}
				-DTRIO_DOWNLOAD:bool=YES
				-DWANT_SPLINT:bool=NO
				-DWANT_DOCS:bool=${RAMALLOC_WANT_DOCS}
				${RAMALLOC_CMAKE_ARGS}
   			TEST_BEFORE_INSTALL ${RAMALLOC_RUN_TESTS}
			)	
		set(RAMALLOC_LIBRARY 
			${RAMALLOC_BINDIR_PREFIX}/lib/${RAMALLOC_STATIC_LIBRARY_NAME} 
			CACHE FILEPATH ${RAMALLOC_STATIC_LIBRARY_DOC} FORCE
			)
		set(RAMALLOC_IS_A_TARGET YES)
	endif()
endif()

get_filename_component(RAMALLOC_PREFIX ${RAMALLOC_LIBRARY} PATH)
get_filename_component(RAMALLOC_PREFIX ${RAMALLOC_PREFIX}/.. ABSOLUTE)

include_directories(${RAMALLOC_PREFIX}/include)

function(add_dependency_on_ramalloc TARGET)
	set(RAMALLOC_LIBRARIES 
		${RAMALLOC_LIBRARY} 
		${CMAKE_THREAD_LIBS_INIT}
		)
	if(RAMALLOC_LIBRARY STREQUAL ${RAMALLOC_PREFIX}/lib/${RAMALLOC_STATIC_LIBRARY_NAME})
		add_dependencies(${TARGET} ramalloc)
	endif()
	get_target_property(Type ${TARGET} TYPE)
	if(Type STREQUAL EXECUTABLE OR Type STREQUAL SHARED_LIBRARY)
		target_link_libraries(${TARGET} ${RAMALLOC_LIBRARIES})
	endif()
endfunction()

