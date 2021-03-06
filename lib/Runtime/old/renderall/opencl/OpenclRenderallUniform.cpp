/*
 * ###########################################################################
 * Copyright (c) 2010, Los Alamos National Security, LLC.
 * All rights reserved.
 * 
 *  Copyright 2010. Los Alamos National Security, LLC. This software was
 *  produced under U.S. Government contract DE-AC52-06NA25396 for Los
 *  Alamos National Laboratory (LANL), which is operated by Los Alamos
 *  National Security, LLC for the U.S. Department of Energy. The
 *  U.S. Government has rights to use, reproduce, and distribute this
 *  software.  NEITHER THE GOVERNMENT NOR LOS ALAMOS NATIONAL SECURITY,
 *  LLC MAKES ANY WARRANTY, EXPRESS OR IMPLIED, OR ASSUMES ANY LIABILITY
 *  FOR THE USE OF THIS SOFTWARE.  If software is modified to produce
 *  derivative works, such modified software should be clearly marked,
 *  so as not to confuse it with the version available from LANL.
 *
 *  Additionally, redistribution and use in source and binary forms,
 *  with or without modification, are permitted provided that the
 *  following conditions are met:
 *
 *    * Redistributions of source code must retain the above copyright
 *      notice, this list of conditions and the following disclaimer.
 * 
 *    * Redistributions in binary form must reproduce the above
 *      copyright notice, this list of conditions and the following
 *      disclaimer in the documentation and/or other materials provided 
 *      with the distribution.
 *
 *    * Neither the name of Los Alamos National Security, LLC, Los
 *      Alamos National Laboratory, LANL, the U.S. Government, nor the
 *      names of its contributors may be used to endorse or promote
 *      products derived from this software without specific prior
 *      written permission.
 * 
 *  THIS SOFTWARE IS PROVIDED BY LOS ALAMOS NATIONAL SECURITY, LLC AND
 *  CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
 *  INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 *  MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 *  DISCLAIMED. IN NO EVENT SHALL LOS ALAMOS NATIONAL SECURITY, LLC OR
 *  CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF
 *  USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 *  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 *  OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 *  OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 *  SUCH DAMAGE.
 * ########################################################################### 
 * 
 * Notes
 *
 * ##### 
 */ 

#include <cassert>
#include "scout/Runtime/base_types.h"
#include "scout/Runtime/opengl/glSDL.h"
#include "scout/Runtime/opengl/glQuadRenderableVA.h"

#include "scout/Runtime/opencl/scout_opencl.h"
#include "scout/Runtime/renderall/renderall_uniform_.h"

using namespace scout;
cl_mem __scrt_renderall_uniform_opencl_device;

namespace scout{

  void renderall_uniform_rt_::registerPbo(GLuint pbo){
    cl_int ret;
    if(__sc_opencl) {
      __scrt_renderall_uniform_opencl_device =
          clCreateFromGLBuffer(__sc_opencl_context,
              CL_MEM_WRITE_ONLY,
              pbo,
              &ret);
      assert(ret == CL_SUCCESS);
    }
  }

  void renderall_uniform_rt_::mapGpuResources(void) {
    if(__sc_opencl) {
      glFinish();
      clEnqueueAcquireGLObjects(__sc_opencl_command_queue,
          1,
          &__scrt_renderall_uniform_opencl_device,
          0,
          NULL,
          NULL);
    } else {
      __scrt_renderall_uniform_colors =_renderable->map_colors();
    }
  }

  void renderall_uniform_rt_::unmapGpuResources(void) {
    if(__sc_opencl) {
      clEnqueueReleaseGLObjects(__sc_opencl_command_queue,
          1,
          &__scrt_renderall_uniform_opencl_device,
          0,
          NULL,
          NULL);
      clFinish(__sc_opencl_command_queue);
    } else {
      _renderable->unmap_colors();
    }
  }
}

