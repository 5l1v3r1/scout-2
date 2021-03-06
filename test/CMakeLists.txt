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
include(${CMAKE_BINARY_DIR}/CMake/Modules/Configuration.cmake)

set(CMAKE_MODULE_PATH
  ${SCOUT_CMAKE_PATH}
  ${CMAKE_MODULE_PATH}
)


project(scout_test_scc C SCC)

  cmake_minimum_required(VERSION 2.8.12.2)
  set(DART_TESTING_TIMEOUT "300" CACHE STRING "" FORCE)
  include(CTest)
  
  include(../cmake/Modules/ScoutConfig.cmake)

  set(SCOUT_REGRESSION_DIR ${CMAKE_CURRENT_BINARY_DIR}/../../test)
  set(SCOUT_BINARY_REGRESSION_DIR ${CMAKE_CURRENT_BINARY_DIR})
  set(TEST_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/bin)

  add_definitions(-DSC_REG_DATA_PATH=${SCOUT_REGRESSION_DIR}/data)
  add_definitions(-DSC_REG_BIN_PATH=${SCOUT_BINARY_REGRESSION_DIR})

  set(CMAKE_VERBOSE_MAKEFILE ON)

  if(DEFINED ENV{SDLDIR})
    set(CMAKE_SCC_LINK_FLAGS -L$ENV{SDLDIR}/lib)
  endif()

  add_subdirectory(scc)
  add_subdirectory(runtime)
  if(DEFINED ENV{SC_DEBUG_TESTS}) 
    add_subdirectory(gdb)
    add_subdirectory(lldb)
  endif()
