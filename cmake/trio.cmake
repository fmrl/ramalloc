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

find_package(CVS)
include(ExternalProject)

set(TRIO_LIBRARY_DOC "path to the trio library.")

function(install_trio)
	if(NOT TRIO_IS_A_TARGET)
		set(TRIO_COMMON_EXTERNALPROJECT_OPTIONS
			CONFIGURE_COMMAND ${CMAKE_CURRENT_BINARY_DIR}/trio-prefix/src/trio/configure --prefix=${CMAKE_CURRENT_BINARY_DIR}/trio-prefix
			TEST_BEFORE_INSTALL 1
			)
		if(CVS_FOUND)
			set(TRIO_CVS_REPOSITORY 
				":pserver:anonymous@ctrio.cvs.sourceforge.net:/cvsroot/ctrio"
				CACHE STRING
				"the CVSROOT of the trio repository."
				) 
			set(TRIO_CVS_USERNAME anonymous
				CACHE STRING
				"the CVS username to use when fetching trio."
				) 
			set(TRIO_CVS_MODULE trio 
				CACHE STRING
				"trio's CVS module name."
				)
			set(TRIO_CVS_TAG trio-1_14 
				CACHE STRING
				"the CVS tag to fetch."
				)
			ExternalProject_Add(trio
				CVS_REPOSITORY ${TRIO_CVS_REPOSITORY}
				CVS_USERNAME ${TRIO_CVS_USERNAME}
				CVS_MODULE ${TRIO_CVS_MODULE}
				CVS_TAG ${TRIO_CVS_TAG}
				${TRIO_COMMON_EXTERNALPROJECT_OPTIONS}
				)	
		else()
			set(TRIO_DOWNLOAD_URL "http://ncu.dl.sourceforge.net/project/ctrio/trio/1.14/trio-1.14.tar.gz" 
				CACHE STRING 
				"a URL describing where trio can be fetched; only necessary if CVS is unavilable.")
			set(TRIO_MD5SUM 0278513e365675ca62bacb6f257b5045
				CACHE STRING 
			"the MD5 sum of the file at TRIO_DOWNLOAD_URL; only necessary if CVS is unavailable.")
			# TODO: i can't seem to find an example of how URL_MD5 is supposed 
			# to be used, and the naive approach doesn't.
			ExternalProject_Add(trio
				URL ${TRIO_DOWNLOAD_URL}
				#URL_MD5 ${TRIO_MD5SUM}
				${TRIO_COMMON_EXTERNALPROJECT_OPTIONS}
				)	
		endif()
		set(TRIO_LIBRARY 
			${CMAKE_CURRENT_BINARY_DIR}/trio-prefix/lib/libtrio.a 
			CACHE FILEPATH ${TRIO_LIBRARY_DOC} FORCE
			)
		set(TRIO_IS_A_TARGET YES PARENT_SCOPE)
		include_directories(${CMAKE_CURRENT_BINARY_DIR}/trio-prefix/include)
	endif()
endfunction()

function(target_link_trio TARGET)
	find_library(TRIO_LIBRARY libtrio.a DOC ${TRIO_LIBRARY_DOC})
	if(NOT TRIO_LIBRARY OR TRIO_LIBRARY STREQUAL ${CMAKE_CURRENT_BINARY_DIR}/trio-prefix/lib/libtrio.a)
		install_trio()
		add_dependencies(${TARGET} trio)
	endif()
	target_link_libraries(${TARGET} ${TRIO_LIBRARY} m)
	set(TRIO_IS_A_TARGET ${TRIO_IS_A_TARGET} PARENT_SCOPE)	
endfunction()

