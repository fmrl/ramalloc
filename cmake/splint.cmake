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

# options
# -------

if(WIN32)
	option(WANT_SPLINT 
		"if YES, splint will ne integrated into the build (not yet functioning on Windows)."
		NO)
	if(WANT_SPLINT)
		message(WARNING "splint is not yet functioning on Windows.")
	endif()
else()
	option(WANT_SPLINT 
		"if YES, splint will ne integrated into the build." YES)
endif()

# it would be problematic to have splint stop the build until i've fixed all
# of the warnings it spits out, so i'm going to make the default NO for now.
option(SPLINT_HAS_A_FATAL 
	"if YES, splint warnings will halt the build." NO)
mark_as_advanced(SPLINT_HAS_A_FATAL)

# cache variables
# ---------------

set(SPLINT_COMMAND splint${CMAKE_EXECUTABLE_SUFFIX} 
	CACHE FILEPATH "the location of the splint executable.")
mark_as_advanced(SPLINT_COMMAND)

# SPLINT_FLAGS should be a cache variable but i haven't gotten around to
# making it so yet.	
set(SPLINT_FLAGS -f ${CMAKE_CURRENT_SOURCE_DIR}/splintrc)
if(WIN32)
	set(SPLINT_FLAGS -f ${CMAKE_CURRENT_SOURCE_DIR}/cmake/windows.splintrc)
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Linux")#
	set(SPLINT_FLAGS ${SPLINT_FLAGS} +posixlib -D__linux__)
else()
	message(WARNING 
		"i don't know how to configure splint for this platform.")
endif()
if(CMAKE_COMPILER_IS_GNUCC)
	set(SPLINT_FLAGS ${SPLINT_FLAGS} -D__GNUC__)
elseif(MSVC)
	# TODO: i may have to detect the compiler version and properly set
	# this variable someday.
	set(SPLINT_FLAGS ${SPLINT_FLAGS} -D_MSC_VER)
else()
	message(WARNING 
		"i don't know how to configure splint for this compiler.")
endif()

# functions
# ---------

function(add_splint TARGET)
	if(WANT_SPLINT)
		if(SPLINT_HAS_A_FATAL)
			unset(maybe_short_circuit_errors)
		else()
			set(maybe_short_circuit_errors || true)
		endif()
		get_directory_property(include_dirs INCLUDE_DIRECTORIES)
		foreach(i ${include_dirs})
			list(APPEND include_flags -I${i})
		endforeach()
		add_custom_target(
			splint-${TARGET}
			COMMAND ${SPLINT_COMMAND} ${SPLINT_FLAGS} ${include_flags} ${ARGN} ${maybe_short_circuit_errors}
			DEPENDS ${ARGN}
			WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
			)
		add_dependencies(${TARGET} splint-${TARGET})
	endif()
endfunction()

