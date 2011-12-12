/*
 * -----  Scout Programming Language -----
 *
 * This file is distributed under an open source license by Los Alamos
 * National Security, LCC.  See the file License.txt (located in the
 * top level of the source distribution) for details.
 *
 *-----
 *
 */


#include "runtime/types.h"
#include "runtime/opengl/uniform_renderall.h"
#include "runtime/opengl/uniform_renderall_private.h"
#include "runtime/scout_gpu.h"

namespace scout
{

  // ----- __sc_fill_vbo
  //
  static void __sc_fill_vbo(glVertexBuffer* vbo, float x0, float y0, float x1, float y1)
  {
    float* verts = (float*)vbo->mapForWrite();

    verts[0] = x0;
    verts[1] = y0;
    verts[2] = 0.0f;

    verts[3] = x1;
    verts[4] = y0;
    verts[5] = 0.f;

    verts[6] = x1;
    verts[7] = y1;
    verts[8] = 0.0f;

    verts[9] = x0;
    verts[10] = y1;
    verts[11] = 0.0f;

    vbo->unmap();
  }


  // ----- __sc_fill_tcbo
  //
  static void __sc_fill_tcbo_2d(glTexCoordBuffer* tcbo, float x0, float y0, float x1, float y1)
  {
    float* coords = (float*)tcbo->mapForWrite();

    coords[0] = x0;
    coords[1] = y0;

    coords[2] = x1;
    coords[3] = y0;

    coords[4] = x1;
    coords[5] = y1;

    coords[6] = x0;
    coords[7] = y1;

    tcbo->unmap();
  }


  // ----- __sc_fill_tcbo_1d
  //
  static void __sc_fill_tcbo_1d(glTexCoordBuffer* tcbo, float start, float end)
  {
    float* coords = (float*)tcbo->mapForWrite();

    coords[0] = start;
    coords[1] = end;
    coords[2] = end;
    coords[3] = start;

    tcbo->unmap();
  }

  void _map_gpu_resources() {
    assert(cuGraphicsMapResources(1, &_scout_device_resource, 0) == CUDA_SUCCESS);

    size_t bytes;
    assert(cuGraphicsResourceGetMappedPointer(&_scout_device_pixels, &bytes, _scout_device_resource) == CUDA_SUCCESS);
  }

  void _unmap_gpu_resources(uniform_renderall_t *info) {
    assert(cuGraphicsUnmapResources(1, &_scout_device_resource, 0) == CUDA_SUCCESS);

    if(!info) return;

    info->pbo->bind();
    info->tex->initialize(0);
    info->pbo->release();
  }

  void _register_gpu_pbo(GLuint pbo, unsigned int flags){
    assert(cuGraphicsGLRegisterBuffer(&_scout_device_resource, pbo, flags) ==
           CUDA_SUCCESS);
  }

  // ----- __sc_init_uniform_renderall
  //
  uniform_renderall_t* __sc_init_uniform_renderall(dim_t xdim)
  {
    uniform_renderall_t* info = new uniform_renderall_t;

    info->ntexcoords = 1;
    info->tex = new glTexture1D(xdim);
    info->tex->addParameter(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    info->tex->addParameter(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    info->tex->addParameter(GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    info->pbo = new glTextureBuffer;
    info->pbo->bind();
    info->pbo->alloc(sizeof(float) * 4 * xdim, GL_STREAM_DRAW_ARB);
    info->pbo->release();

    info->vbo = new glVertexBuffer;
    info->vbo->bind();
    info->vbo->alloc(sizeof(float) * 3 * 4, GL_STREAM_DRAW_ARB);  // we use a quad for 1D meshes...
    __sc_fill_vbo(info->vbo, 0.0f, 0.0f, float(xdim), float(xdim));
    info->vbo->release();
    info->nverts = 4;

    info->tcbo = new glTexCoordBuffer;
    info->tcbo->bind();
    info->tcbo->alloc(sizeof(float) * 4, GL_STREAM_DRAW_ARB);  // one-dimensional texture coordinates.
    __sc_fill_tcbo_1d(info->tcbo, 0.0f, 1.0f);
    info->tcbo->release();

    OpenGLErrorCheck();

    if(_scout_gpu)
      _register_gpu_pbo(info->pbo->id(),
        CU_GRAPHICS_REGISTER_FLAGS_WRITE_DISCARD);

    return info;
  }


  // ----- __sc_init_uniform_renderall
  //
  uniform_renderall_t* __sc_init_uniform_renderall(dim_t xdim, dim_t ydim)
  {
    uniform_renderall_t* info = new uniform_renderall_t;

    info->ntexcoords = 2;
    info->tex = new glTexture2D(xdim, ydim);
    info->tex->addParameter(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    info->tex->addParameter(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    info->tex->addParameter(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    //info->tex->initialize(0);

    info->pbo = new glTextureBuffer;
    info->pbo->bind();
    info->pbo->alloc(sizeof(float) * 4 * xdim * ydim, GL_STREAM_DRAW_ARB);
    info->pbo->release();

    info->vbo = new glVertexBuffer;
    info->vbo->bind();
    info->vbo->alloc(sizeof(float) * 3 * 4, GL_STREAM_DRAW_ARB);
    __sc_fill_vbo(info->vbo, 0.0f, 0.0f, float(xdim), float(ydim));
    info->vbo->release();
    info->nverts = 4;

    info->tcbo = new glTexCoordBuffer;
    info->tcbo->bind();
    info->tcbo->alloc(sizeof(float) * 8, GL_STREAM_DRAW_ARB);  // two-dimensional texture coordinates.
    __sc_fill_tcbo_2d(info->tcbo, 0.0f, 0.0f, 1.0f, 1.0f);
    info->tcbo->release();

    OpenGLErrorCheck();

    if(_scout_gpu)
      _register_gpu_pbo(info->pbo->id(), 
        CU_GRAPHICS_REGISTER_FLAGS_WRITE_DISCARD);

    return info;
  }


  // ----- __sc_destroy_renderall
  //
  void __sc_destroy_uniform_renderall(uniform_renderall_t* info)
  {

    //
    // TODO; need to add cuda interop support here...
    //

    delete info->pbo;
    delete info->vbo;
    delete info->tcbo;
    delete info->tex;
    delete info;
  }


  // ----- __sc_map_uniform_colors
  //
  float4* __sc_map_uniform_colors(uniform_renderall_t* info)
  {
    //
    // TODO; need to add cuda interop support here...
    //

    if (info)
      return (float4*)info->pbo->mapForWrite();
    else
      return 0;
  }


  // ----- __sc_unmap_uniform_colors
  //
  void __sc_unmap_uniform_colors(uniform_renderall_t* info)
  {
    //
    // TODO; need to add cuda interop support here...
    //

    if (info){
      info->pbo->unmap();
      info->pbo->bind();
      info->tex->initialize(0);
      info->pbo->release();
    }

  }


  // ----- __sc_exec_uniform_renderall
  //
  void __sc_exec_uniform_renderall(uniform_renderall_t* info)
  {
    info->pbo->bind();
    info->tex->enable();
    info->tex->update(0);
    info->pbo->release();

    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    info->tcbo->bind();
    glTexCoordPointer(info->ntexcoords, GL_FLOAT, 0, 0);

    glEnableClientState(GL_VERTEX_ARRAY);
    info->vbo->bind();
    glVertexPointer(3, GL_FLOAT, 0, 0);

    glDrawArrays(GL_POLYGON, 0, info->nverts);

    glDisableClientState(GL_VERTEX_ARRAY);
    info->vbo->release();

    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    info->tcbo->release();

    info->tex->disable();
  }

} // end namespace scout
