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
project(scout_test_lldb)

set(SRC_PATH ${CMAKE_SOURCE_DIR}/lldb)

find_package(EXPECT)
find_package(LLDB)

if(EXPECT_FOUND)
  if(LLDB_FOUND)
    set(name scc-breakpoint)
    set(tname lldb_${name})
    add_test(${tname} ${SRC_PATH}/breakpoint.exp ${SRC_PATH} "scc")
  endif()
  if(DEFINED ENV{SC_BUILD_LLDB})
    set(name scc-mesh)
    set(tname lldb_${name})
    add_test(${tname} ${SRC_PATH}/mesh.exp ${SRC_PATH} "scc" "sc")
    #set(name scc-mesh-uq)
    #set(tname lldb_${name})
    #add_test(${tname} ${SRC_PATH}/mesh-uq.exp ${SRC_PATH} "scc" "sc")
    set(name scc-mesh-function)
    set(tname lldb_${name})
    add_test(${tname} ${SRC_PATH}/mesh-function.exp ${SRC_PATH} "scc" "sc" "0")
    #if (SCOUT_ENABLE_LEGION MATCHES "ON") 
     # set(name scc-mesh-legion)
     # set(tname lldb_${name})
     # add_test(${tname} ${SRC_PATH}/mesh-function.exp ${SRC_PATH} "scc" "sc" "1")
    #endif()
    if(OPENGL_VERSION_MAJOR EQUAL 3 OR OPENGL_VERSION_MAJOR GREATER 3)
      if(DEFINED ENV{DISPLAY}) 
        set(name scc-mesh-viz)
        set(tname lldb_${name})
        add_test(${tname} ${SRC_PATH}/mesh-viz.exp ${SRC_PATH} "scc" "sc")
      endif()
    endif()
  endif()
endif()
