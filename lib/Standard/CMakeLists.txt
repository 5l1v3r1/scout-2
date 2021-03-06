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
#   This file is bootstrapped by the main CMake configuration.  In
#   particular the bootstrap stages create a configuration file for
#   the standard library -- this is because the standard library is
#   written in Scout and thus requires 'scc' to be configured and
#   compiled to provdie a sane environment (at least as far as CMake
#   is concerned).  We also need to let 'scc' know that we're building
#   the standard library that allows it avoid searching for the
#   associated headers and library file.
#   

include(${CMAKE_BINARY_DIR}/CMake/Modules/Configuration.cmake)

set(CMAKE_MODULE_PATH
  ${SCOUT_CMAKE_PATH}
  ${CMAKE_MODULE_PATH}
  )

project(scStandardLibrary SCC)
  cmake_minimum_required(VERSION 2.8.12.2)
  set(CMAKE_SCC_FLAGS "-g -noscstdlib")
  set(LINK_FLAGS "-noscstdlib")
  set(CMAKE_INSTALL_PREFIX ${SCOUT_INSTALL_DIR})
  set(LIBRARY_OUTPUT_PATH ${SCOUT_BUILD_DIR}/lib)

  include_directories(
    ${SCOUT_INCLUDE_DIR}
    )

  file(GLOB std_hdrs ${SCOUT_INCLUDE_DIR}/scout/*.sch)
  file(GLOB std_srcs *.sc)

  add_library(scStandard SHARED ${std_srcs})

  if(NOT APPLE)  
    set_target_properties(scStandard PROPERTIES COMPILE_FLAGS "-fPIC")
    set_target_properties(scStandard PROPERTIES LINK_FLAGS "-fPIC -noscstdlib")
  else()
    set_target_properties(scStandard PROPERTIES POSITION_INDEPENDENT_CODE 1
                                                COMPILE_FLAGS "-fPIC -noscstdlib"
						MACOSX_RPATH 1) 
  endif()

  install(TARGETS scStandard
    DESTINATION lib)
