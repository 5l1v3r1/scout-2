#
###########################################################################
# Copyright (c) 2010, Los Alamos National Security, LLC.
# All rights reserved.
# 
#  Copyright 2010. Los Alamos National Security, LLC. This software was
#  produced under U.S. Government contract DE-AC52-06NA25396 for Los
#  Alamos National Laboratory (LANL), which is operated by Los Alamos
#  National Security, LLC for the U.S. Department of Energy. The
#  U.S. Government has rights to use, reproduce, and distribute this
#  software.  NEITHER THE GOVERNMENT NOR LOS ALAMOS NATIONAL SECURITY,
#  LLC MAKES ANY WARRANTY, EXPRESS OR IMPLIED, OR ASSUMES ANY LIABILITY
#  FOR THE USE OF THIS SOFTWARE.  If software is modified to produce
#  derivative works, such modified software should be clearly marked,
#  so as not to confuse it with the version available from LANL.
#
#  Additionally, redistribution and use in source and binary forms,
#  with or without modification, are permitted provided that the
#  following conditions are met:
#
#    * Redistributions of source code must retain the above copyright
#      notice, this list of conditions and the following disclaimer.
#
#    * Redistributions in binary form must reproduce the above
#      copyright notice, this list of conditions and the following
#      disclaimer in the documentation and/or other materials provided 
#      with the distribution.  
#
#    * Neither the name of Los Alamos National Security, LLC, Los
#      Alamos National Laboratory, LANL, the U.S. Government, nor the
#      names of its contributors may be used to endorse or promote
#      products derived from this software without specific prior
#      written permission.
#
#  THIS SOFTWARE IS PROVIDED BY LOS ALAMOS NATIONAL SECURITY, LLC AND
#  CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
#  INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
#  MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
#  DISCLAIMED. IN NO EVENT SHALL LOS ALAMOS NATIONAL SECURITY, LLC OR
#  CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
#  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
#  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF
#  USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
#  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
#  OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
#  OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
#  SUCH DAMAGE.
#
###########################################################################
#
# Notes
# 
#   - The primary job of this Makefile is to run CMake for you.  If you
#     prefer to run CMake manually you are free to do so (see details in
#     the ReadMe.md file).
#
# Enviornment variables:
# 
#   - SC_BUILD_NTHREADS: Controls the number of threads in a parallel
#     build (enabled if set).
#
#   - SC_BUILD_TYPE: Controls the build type (e.g. debug or release):
#
#	export SC_BUILD_TYPE=[DEBUG|RELEASE] (default is debug)
#   
#####

.PHONY : default
.PHONY : build 
.PHONY : test


##### PARALLEL BUILD CONFIGURATION 
#
ifdef SC_BUILD_NTHREADS
  make_flags := --jobs=$(SC_BUILD_NTHREADS)
endif
#
#####


##### BUILD TYPE SELECTION 
#
ifdef SC_BUILD_TYPE
  build_type  := $(SC_BUILD_TYPE)
else
  build_type  := DEBUG
endif

#
#####

##### SOURCE LOCATION 
# 
src_dir := ${CURDIR}

##### BUILD LOCATION 
# 
# We strongly advice against doing an in-source build with CMake...
# 
ifdef SC_BUILD_DIR
  build_dir := $(SC_BUILD_DIR)
else
  build_dir := $(CURDIR)/build
endif

stdlib_dir       := lib/Standard
stdlib_build_dir := $(build_dir)/$(stdlib_dir)
stdlib_src_dir   := $(src_dir)/$(stdlib_dir)
stdlib_flags     :=

runtime_dir := lib/Runtime
runtime_build_dir := $(build_dir)/$(runtime_dir)
runtime_flags := -DCMAKE_SCC_RUNTIME_ONLY=ON 
runtime_remove_flags := -UCMAKE_SCC_RUNTIME_ONLY

test_dir := test
test_build_dir := $(build_dir)/$(test_dir)
test_src_dir := $(src_dir)/$(test_dir)

docs_build_dir := docs/_build
#
#####

##### gcc in module
ifdef SC_GCC_INSTALL_PREFIX
  extra_cmake_flags := -DGCC_INSTALL_PREFIX="$(SC_GCC_INSTALL_PREFIX)" \
                       -DCMAKE_CXX_LINK_FLAGS="-L$(SC_GCC_INSTALL_PREFIX)/lib64 -Wl,-rpath,$(SC_GCC_INSTALL_PREFIX)/lib64" 
endif


cmake_flags := -DCMAKE_BUILD_TYPE=$(build_type) \
               -DCMAKE_INSTALL_PREFIX=$(build_dir) \
               -DCMAKE_SOURCE_DIR=$(src_dir) \
               -DCMAKE_PREFIX_PATH="/usr/local/Qt/5.5/clang_64/;/usr/local/Qt/5.5/gcc_64/;/usr/local/Qt;$(SC_QT_DIR)"\
               $(SC_BUILD_CMAKE_FLAGS) \
               -DDEBUG_REPORT_URL="https://github.com/losalamos/scout"\
               -DCLANG_VENDOR_UTI="gov.lanl"\
               -Wno-dev

uname := $(shell uname)

ifeq ($(uname), Darwin)
  cmake_flags += -DCMAKE_CXX_FLAGS="-stdlib=libc++ -std=c++11" -DCMAKE_SHARED_LINKER_FLAGS="-stdlib=libc++"
  cmake_flags += -DLLVM_ENABLE_LIBCXX=ON 
else
# These flags must be enabled in order to build LLDB.
  cmake_flags += -DLLVM_ENABLE_CXX11=ON -DLLVM_REQUIRES_RTTI=1 -DCMAKE_CXX_FLAGS="-std=c++11"
endif

cmake_flags += $(extra_cmake_flags)

all: $(build_dir)/Makefile toolchain stdlib
.PHONY: all 

$(build_dir)/Makefile: CMakeLists.txt
	@echo "*** Scout source directory: $(src_dir)"
	@((test -d $(build_dir)) || (mkdir $(build_dir)))
	@echo "*** Creating Scout build directory: $(build_dir)"
	@(cd $(build_dir); cmake $(cmake_flags) $(src_dir))

legion-rt: $(build_dir) $(build_dir)/Makefile
	@ \
	echo "*** Building Legion Runtime" && \
	cd $(build_dir) && \
	CC=$(build_dir)/bin/clang  \
	CXX=$(build_dir)/bin/clang++ \
	LG_RT_DIR=$(src_dir)/legion/runtime \
	make $(make_flags) -C $(src_dir)/legion/liblsci && \
	LSCI_PREFIX=$(build_dir) make install -C $(src_dir)/legion/liblsci

.PHONY: clean-legion-rt
clean-legion-rt:
	LG_RT_DIR=$(src_dir)/legion/runtime \
	make vclean -C $(src_dir)/legion/liblsci

toolchain: $(build_dir) $(build_dir)/Makefile
	@(cd $(build_dir); make $(make_flags))

.PHONY: scc
scc: $(build_dir)/Makefile toolchain 
	@(cd $(build_dir); make $(make_flags))
	cp $(build_dir)/tools/clang/scc/scc $(build_dir)/scout/bin/scc
	cp $(build_dir)/tools/clang/scc-rewrite/scc-rewrite $(build_dir)/scout/bin/scc-rewrite


.PHONY: stdlib
stdlib: $(build_dir)/Makefile 
	@echo "*** Creating standard library build directory: $(stdlib_build_dir)"
	@((test -d $(stdlib_build_dir)) || (mkdir $(stdlib_build_dir)))
	@(cd $(stdlib_build_dir); cmake $(cmake_flags) --debug-trycompile --debug-output -trace $(stdlib_flags) $(stdlib_src_dir))
	@(cd $(stdlib_build_dir); make $(make_flags))

.PHONY: test
test: 
	@((test -d $(test_build_dir)) || (mkdir $(test_build_dir)))
	@(cd $(test_build_dir); cmake $(cmake_flags) $(test_src_dir))
	@(cd $(test_build_dir); make $(make_flags))
	@(cd $(test_build_dir); ARGS="-D ExperimentalTest --no-compress-output" make test; cp Testing/`head -n 1 Testing/TAG`/Test.xml ./CTestResults.xml)

.PHONY: testclean
testclean: 
	-@/bin/rm -rf $(test_build_dir)
	@(cd $(build_dir); cmake $(cmake_flags) $(src_dir))

#run llvm/cmake tests
.PHONY: check
check: 
	@(cd $(build_dir); make check $(make_flags))
	@(cd $(build_dir); make check-clang $(make_flags))

.PHONY: runtime
runtime: 
	@((test -d $(runtime_build_dir)) || (mkdir $(runtime_build_dir)))
	@(cd $(build_dir); cmake $(cmake_flags) $(runtime_flags) $(src_dir))
	@(cd $(runtime_build_dir); make $(make_flags) install)
	@(cd $(build_dir); cmake $(cmake_flags) $(runtime_remove_flags) $(src_dir))

.PHONY: runtimeclean
runtimeclean:
	-@/bin/rm -rf $(runtime_build_dir)

.PHONY: cacheclean
cacheclean:
	-@/bin/rm -rf $(build_dir)/CMakeCache.txt

.PHONY: xcode
xcode:;
	@((test -d xcode) || (mkdir xcode))
	@(cd xcode; cmake -G Xcode $(src_dir))

.PHONY: clean
clean: 
	-@/bin/rm -rf $(build_dir)
	-@/bin/rm -rf $(docs_build_dir)
	-@(if test -d scout-local/sandbox; then /usr/bin/find ./scout-local/sandbox -name build -exec rm -rf {} \; ;fi)
	-@/usr/bin/find . -name '*~' -exec rm -f {} \;
	-@/usr/bin/find . -name '._*' -exec rm -f {} \;
	-@/usr/bin/find . -name '.DS_Store' -exec rm -f {} \;
