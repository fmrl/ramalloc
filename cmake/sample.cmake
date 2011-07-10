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

# functions
# ---------

function(add_sample TARGET SAMPLE_SOURCE_DIR)
	set(sample_target sample)
	set(sample_binary_dir ${CMAKE_CURRENT_BINARY_DIR}/sample)
	set(staging_dir ${sample_binary_dir}/${${TARGET}-prefix})
	file(MAKE_DIRECTORY ${sample_binary_dir})
	add_custom_target(
		${sample_target}
		COMMAND 
			${CMAKE_COMMAND} --build ${CMAKE_CURRENT_BINARY_DIR} --target install -- DESTDIR=${staging_dir} 
			&& ${CMAKE_COMMAND} ${SAMPLE_SOURCE_DIR} -DCMAKE_LIBRARY_PATH:path=${staging_dir}/${CMAKE_INSTALL_PREFIX}/lib
			&& ${CMAKE_COMMAND} --build ${sample_binary_dir}
		WORKING_DIRECTORY ${sample_binary_dir}
		)
	add_dependencies(${sample_target} ${TARGET})
	add_test(${sample_target} ${sample_binary_dir}/sample)
endfunction()

