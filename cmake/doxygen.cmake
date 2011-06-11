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

find_package(Doxygen)

option(WANT_DOCS 
	"if YES, i'll automatically generate documentation (requires doxygen)." 
	${DOXYGEN_FOUND})
option(WANT_INTERNAL_DOCS 
	"if YES, doxygen will generate internal documentation (INTERNAL_DOCS)." 
	NO)

# TODO: all of these should be cache variables.
set(DOXYGEN_CONFIG ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)
set(DOXYGEN_FLAGS ${DOXYGEN_CONFIG})
set(DOXYGEN_CONFIG_TEMPLATE ${CMAKE_CURRENT_SOURCE_DIR}/cmake/Doxyfile.in)

# TODO: all of these should be cache variables.
set(DOXYGEN_OUTPUT_DIRECTORY "\"${CMAKE_CURRENT_BINARY_DIR}/doxygen\"")
set(DOXYGEN_GENERATE_HTML YES)
set(DOXYGEN_GENERATE_LATEX NO)
set(DOXYGEN_FULL_PATH_NAMES NO)
set(DOXYGEN_OPTIMIZE_OUTPUT_FOR_C YES)
set(DOXYGEN_SOURCE_BROWSER NO)
set(DOXYGEN_INLINE_SOURCES YES)
set(DOXYGEN_WARN_NO_PARAMDOC YES)
set(DOXYGEN_SORT_MEMBER_DOCS YES)
set(DOXYGEN_SORT_BRIEF_DOCS YES)
set(DOXYGEN_HTML_FOOTER ${CMAKE_CURRENT_SOURCE_DIR}/src/doc/footer.html.in)
if(WANT_INTERNAL_DOCS)
	set(DOXYGEN_INTERNAL_DOCS YES)
else()
	set(DOXYGEN_INTERNAL_DOCS NO)
endif()

# TODO: project name should probably have a keyword.
function(add_doxygen TARGET DOXYGEN_PROJECT_NAME)

	if (NOT DOXYGEN_FOUND)
		message(WARNING 
			"i will be unable to generate documentation because doxygen could not be found.")
		return()
	endif()

	foreach(i ${ARGN})
		set(DOXYGEN_INPUT 
			"${DOXYGEN_INPUT} \"${CMAKE_CURRENT_SOURCE_DIR}/${i}\""
			)
	endforeach()
	configure_file(${DOXYGEN_CONFIG_TEMPLATE} ${DOXYGEN_CONFIG})
	add_custom_target(
		doxygen
		COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_FLAGS}
		DEPENDS ${ARGN} ${DOXYGEN_CONFIG}
		)
	if(WANT_DOCS)
		add_dependencies(${TARGET} doxygen)
	endif()

endfunction()


