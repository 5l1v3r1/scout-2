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

#include <string.h>
#include "scout/Runtime/opengl/glTexture2D.h"

using namespace scout;


// ..... glTexture2D
//
glTexture2D::glTexture2D(GLsizei w, GLsizei h)  
   :glTexture(GL_TEXTURE_2D, GL_RGBA32F_ARB, GL_RGBA, GL_FLOAT)
{
  const GLubyte *str;
  str = glGetString(GL_EXTENSIONS); 
  // if we don't have texture_float then change iformat
  if(strstr((const char *)str, "GL_ARB_texture_float") == NULL) _iformat = GL_RGBA;
  
  _width   = w;
  _height  = h;
}


// ---- ~glTexture2D
//
glTexture2D::~glTexture2D()
{ /* no-op -- base class destroys */ }


// ---- initialize
//
void glTexture2D::initialize(const float* p_data)
{
  enable();
  setParameters();
  //glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 32.0f);
  
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  glTexImage2D(target(), 0, internalFormat(), width(), height(), 0, pixelFormat(), type(), (void*)p_data);
  oglErrorCheck();      
}


// ---- canDownload
//
bool glTexture2D::canDownload() const 
{
  glTexImage2D(GL_PROXY_TEXTURE_2D, 0, internalFormat(), width(), height(), 0, pixelFormat(), type(), 0);
  GLsizei w;
  glGetTexLevelParameteriv(GL_PROXY_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &w);
  oglErrorCheck();      
  return(w != 0);
}


// ---- update 
// 
void glTexture2D::update(const float* p_data)
{
  glTexSubImage2D(target(), 0, 0, 0, width(), height(), internalFormat(), type(), (void*)p_data);
  oglErrorCheck();
}


// ---- update
//
void glTexture2D::update(const float* p_data, GLsizei x_offset, GLsizei y_offset, GLsizei subwidth, GLsizei subheight)
{
  glTexSubImage2D(target(), 0, x_offset, y_offset, subwidth, subheight, pixelFormat(), type(), (void*)p_data);
  oglErrorCheck();
}


// ---- read
// Readback the texture data to address specified by 'p_data'.
void glTexture2D::read(float* p_data) const
{
  glGetTexImage(target(), 0, _format, _type, p_data);
  oglErrorCheck();
}
