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

#ifndef __SC_GL_WINDOW_H__
#define __SC_GL_WINDOW_H__

#include <string>
#include <list>
#include <deque>

#include "scout/Runtime/opengl/opengl.h"
#include "scout/Runtime/opengl/glCamera.h"
#include "scout/Runtime/opengl/glManipulator.h"
#include "scout/Runtime/opengl/glRenderable.h"

namespace scout {

  /**
   * For future flexibility we represent both screen coordinates and
   * dimensions using this type -- both measured in terms of pixels.
   */   
  typedef unsigned short ScreenCoord;

  
  /** ----- ScreenPoint
   * A location on the screen measured in pixels. 
   */
  struct ScreenPoint {

    ScreenPoint(ScreenCoord px,
                ScreenCoord py) 
        : x(px),
          y(py) {
    }

    ScreenPoint(const ScreenPoint &other)
        : x(other.x),
          y(other.y) {
    }
    
    ScreenCoord x;
    ScreenCoord y;
  };

  
  /** ----- ScreenSize
   * The width and height of an on-screen element measured in pixels.
   */
  struct ScreenSize {
    
    ScreenSize(ScreenCoord w,
               ScreenCoord h)
        : width(w),
          height(h) {
    }

    ScreenSize(const ScreenSize &other)
        : width(other.width),
          height(other.height) {
    }
    
    ScreenCoord   width;
    ScreenCoord   height;
  };


  /** ----- WindowRect
   * A window rectangle captures both the position and the dimensions
   * of an on-screen region.  As the name implies, it is primarily
   * used to describe a window.
   */
  struct WindowRect {

    WindowRect(ScreenCoord w, ScreenCoord h)
        : origin(0, 0),
          size(w, h) {
    }

    WindowRect(ScreenCoord x, ScreenCoord y,
               ScreenCoord w, ScreenCoord h)
        : origin(x, y),
          size(w, h) {
    }

    WindowRect(const ScreenPoint &P,
               const ScreenSize  &dims)
        : origin(P),
          size(dims) {
    }
    
    ScreenPoint  origin;
    ScreenSize   size;
  };

  
  /** ----- glWindow
   * The base class for an OpenGL-enabled window.  The window is
   * described by a given rectangle region, or an given set of width
   * and height dimensions.  In addition, it is possible to assign an
   * on-screen title and background color for the rendering area of
   * the window.  The overall, platform-centric, details of the window
   * must be provied by subclasses. 
   */
  class glWindow {
    
   public:
    
    /// Create a window of the given width and height.  The window's
    /// position is undetermined and the background color will default
    /// to black.
    glWindow(ScreenCoord width, ScreenCoord height);
    
    /// Create a window with the given location and size (as described
    /// by the given WindowRect).  The background color will default
    /// to black.
    glWindow(const WindowRect &rect);
    
    /// Create a window with the given location, size (as described by
    /// the given WindowRect) and background color.
    glWindow(const WindowRect &rect, const oglColor &color);
    
    virtual ~glWindow(){};
    
    /// Set the window's background color.
    void setBackgroundColor(float red,
                            float green,
                            float blue);
    
    /// Set the window's background color.
    void setBackgroundColor(const oglColor &rgba);
    
    void paint(){};
    void paintMono();
    void paintStereo();
    
    // not sure these should be here and if they make sense for derived classes that are not GLFW, eg macosx
    virtual void swapBuffers() = 0;
    virtual bool processEvent() = 0;
    virtual void eventLoop() = 0;
    
   protected:
    WindowRect      _frame;     /// The position and dimensions of window.
    oglColor        _bgColor;   /// Window's background color.
   
    typedef std::deque<glRenderable*>  RenderableList;
    RenderableList _renderables;
    
    bool           _ignore_events;
    bool           _stereo_mode;
    bool           _save_frames;
    int            _img_seq_num;
    std::string    _frame_basename;
    std::string    _display_text;
    int            _text_x, _text_y;
    glCamera       *_camera;
    glManipulator  *_manipulator;    
    glfloat4       _bg_color;
  };
  
}
#endif
