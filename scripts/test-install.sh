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

set -e

# this is a rudimentary script that tests the package on Linux.

# TODO: it seems that it would be a better strategy to test the install
# command, passing an alternate CMAKE_INSTALL_DIR value into CMake.
# TODO; this script needs to be generalized. is there a way to port it to
# CMake?

# usage:
#	test-install.sh <package> <install-prefix> <sample-source-dir>

PACKAGE_PATH=`readlink -e $1`
PREFIX=`readlink -f $2`
PACKAGE_PREFIX=$PREFIX/pkg
SAMPLE_SOURCE_DIR=`readlink -e $3`
SAMPLE_BINARY_DIR=$PREFIX/sample

UNCOMPRESS=gunzip

make package
mkdir -p $PACKAGE_PREFIX
cd $PACKAGE_PREFIX 
tar -I $UNCOMPRESS --strip-components=1 -xf $PACKAGE_PATH  
mkdir -p $SAMPLE_BINARY_DIR 
cd $SAMPLE_BINARY_DIR 
cmake $SAMPLE_SOURCE_DIR -DCMAKE_LIBRARY_PATH:path=$PACKAGE_PREFIX/lib
make
./sample




