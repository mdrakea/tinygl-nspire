#include "zgl.h"

static void copy_matrix(GLfixed *v, M4 *m)
{
  GLfixed *p = &m->m[0][0];
  int i;

  for (i = 0; i < 4; i++) {
    *v++ = p[0];
    *v++ = p[4];
    *v++ = p[8];
    *v++ = p[12];
    p++;
  }
}

static int matrix_number(GLenum pname)
{
  switch (pname) {
  case GL_MODELVIEW_MATRIX: return 0;
  case GL_PROJECTION_MATRIX: return 1;
  case GL_TEXTURE_MATRIX: return 2;
  default: return -1;
  }
}

void glGetFixedv(GLenum pname, GLfixed *params)
{
  GLContext *c = gl_get_context();
  int mnr;

  if (params == NULL) {
    gl_set_error(c, GL_INVALID_VALUE);
    return;
  }

  mnr = matrix_number(pname);
  if (mnr >= 0) {
    copy_matrix(params, c->matrix_stack_ptr[mnr]);
    return;
  }

  switch (pname) {
  case GL_CURRENT_COLOR:
    params[0] = c->current_color.X;
    params[1] = c->current_color.Y;
    params[2] = c->current_color.Z;
    params[3] = c->current_color.W;
    break;
  case GL_CURRENT_NORMAL:
    params[0] = c->current_normal.X;
    params[1] = c->current_normal.Y;
    params[2] = c->current_normal.Z;
    break;
  case GL_CURRENT_TEXTURE_COORDS:
    params[0] = c->current_tex_coord.X;
    params[1] = c->current_tex_coord.Y;
    params[2] = c->current_tex_coord.Z;
    params[3] = c->current_tex_coord.W;
    break;
  case GL_COLOR_CLEAR_VALUE:
    params[0] = c->clear_color.X;
    params[1] = c->clear_color.Y;
    params[2] = c->clear_color.Z;
    params[3] = c->clear_color.W;
    break;
  case GL_DEPTH_CLEAR_VALUE:
    *params = c->clear_depth;
    break;
  case GL_LINE_WIDTH:
  case GL_POINT_SIZE:
    *params = TGL_FIX_ONE;
    break;
  case GL_ALIASED_LINE_WIDTH_RANGE:
  case GL_SMOOTH_LINE_WIDTH_RANGE:
  case GL_ALIASED_POINT_SIZE_RANGE:
  case GL_SMOOTH_POINT_SIZE_RANGE:
    params[0] = TGL_FIX_ONE;
    params[1] = TGL_FIX_ONE;
    break;
  default:
    gl_set_error(c, GL_INVALID_ENUM);
    break;
  }
}

void glGetIntegerv(GLenum pname, GLint *params)
{
  GLContext *c = gl_get_context();

  if (params == NULL) {
    gl_set_error(c, GL_INVALID_VALUE);
    return;
  }

  switch(pname) {
  case GL_VIEWPORT:
    params[0]=c->viewport.xmin;
    params[1]=c->viewport.ymin;
    params[2]=c->viewport.xsize;
    params[3]=c->viewport.ysize;
    break;
  case GL_MATRIX_MODE:
    *params = c->matrix_mode == 0 ? GL_MODELVIEW :
              c->matrix_mode == 1 ? GL_PROJECTION : GL_TEXTURE;
    break;
  case GL_MODELVIEW_STACK_DEPTH:
    *params = (GLint)(c->matrix_stack_ptr[0] - c->matrix_stack[0] + 1);
    break;
  case GL_PROJECTION_STACK_DEPTH:
    *params = (GLint)(c->matrix_stack_ptr[1] - c->matrix_stack[1] + 1);
    break;
  case GL_TEXTURE_STACK_DEPTH:
    *params = (GLint)(c->matrix_stack_ptr[2] - c->matrix_stack[2] + 1);
    break;
  case GL_MAX_MODELVIEW_STACK_DEPTH:
    *params = MAX_MODELVIEW_STACK_DEPTH;
    break;
  case GL_MAX_PROJECTION_STACK_DEPTH:
    *params = MAX_PROJECTION_STACK_DEPTH;
    break;
  case GL_MAX_TEXTURE_STACK_DEPTH:
    *params = MAX_TEXTURE_STACK_DEPTH;
    break;
  case GL_MAX_LIGHTS:
    *params = MAX_LIGHTS;
    break;
  case GL_MAX_TEXTURE_SIZE:
    *params = 256;
    break;
  case GL_MAX_TEXTURE_UNITS:
    *params = 1;
    break;
  case GL_MAX_VIEWPORT_DIMS:
    params[0] = c->zb->xsize;
    params[1] = c->zb->ysize;
    break;
  case GL_SUBPIXEL_BITS:
    *params = 4;
    break;
  case GL_RED_BITS:
  case GL_GREEN_BITS:
  case GL_BLUE_BITS:
    *params = TGL_FEATURE_RENDER_BITS == 16 ? 5 : 8;
    break;
  case GL_ALPHA_BITS:
    *params = TGL_FEATURE_RENDER_BITS == 32 ? 8 : 0;
    break;
  case GL_DEPTH_BITS:
    *params = ZB_Z_BITS;
    break;
  case GL_TEXTURE_BINDING_2D:
    *params = c->current_texture != NULL ? c->current_texture->handle : 0;
    break;
  case GL_ARRAY_BUFFER_BINDING:
    *params = c->array_buffer_binding != NULL ? (GLint)c->array_buffer_binding->handle : 0;
    break;
  case GL_ELEMENT_ARRAY_BUFFER_BINDING:
    *params = c->element_array_buffer_binding != NULL ? (GLint)c->element_array_buffer_binding->handle : 0;
    break;
  case GL_VERTEX_ARRAY_BUFFER_BINDING:
    *params = c->vertex_array.buffer != NULL ? (GLint)c->vertex_array.buffer->handle : 0;
    break;
  case GL_NORMAL_ARRAY_BUFFER_BINDING:
    *params = c->normal_array.buffer != NULL ? (GLint)c->normal_array.buffer->handle : 0;
    break;
  case GL_COLOR_ARRAY_BUFFER_BINDING:
    *params = c->color_array.buffer != NULL ? (GLint)c->color_array.buffer->handle : 0;
    break;
  case GL_TEXTURE_COORD_ARRAY_BUFFER_BINDING:
    *params = c->texcoord_array.buffer != NULL ? (GLint)c->texcoord_array.buffer->handle : 0;
    break;
  case GL_VERTEX_ARRAY_SIZE:
    *params = c->vertex_array.size;
    break;
  case GL_VERTEX_ARRAY_TYPE:
    *params = c->vertex_array.type;
    break;
  case GL_VERTEX_ARRAY_STRIDE:
    *params = c->vertex_array.stride;
    break;
  case GL_NORMAL_ARRAY_TYPE:
    *params = c->normal_array.type;
    break;
  case GL_NORMAL_ARRAY_STRIDE:
    *params = c->normal_array.stride;
    break;
  case GL_COLOR_ARRAY_SIZE:
    *params = c->color_array.size;
    break;
  case GL_COLOR_ARRAY_TYPE:
    *params = c->color_array.type;
    break;
  case GL_COLOR_ARRAY_STRIDE:
    *params = c->color_array.stride;
    break;
  case GL_TEXTURE_COORD_ARRAY_SIZE:
    *params = c->texcoord_array.size;
    break;
  case GL_TEXTURE_COORD_ARRAY_TYPE:
    *params = c->texcoord_array.type;
    break;
  case GL_TEXTURE_COORD_ARRAY_STRIDE:
    *params = c->texcoord_array.stride;
    break;
  case GL_CULL_FACE_MODE:
    *params = c->current_cull_face;
    break;
  case GL_FRONT_FACE:
    *params = c->current_front_face ? GL_CW : GL_CCW;
    break;
  case GL_SHADE_MODEL:
    *params = c->current_shade_model;
    break;
  case GL_UNPACK_ALIGNMENT:
    *params = c->unpack_alignment;
    break;
  default:
    gl_set_error(c, GL_INVALID_ENUM);
    break;
  }
}

const GLubyte *glGetString(GLenum name)
{
  GLContext *c = gl_get_context();

  switch (name) {
  case GL_VENDOR:
    return (const GLubyte *)"TinyGL";
  case GL_RENDERER:
    return (const GLubyte *)"TinyGL fixed rasterizer";
  case GL_VERSION:
    return (const GLubyte *)"OpenGL ES-CM 1.1 TinyGL Common-Lite";
  case GL_EXTENSIONS:
    return (const GLubyte *)"";
  default:
    gl_set_error(c, GL_INVALID_ENUM);
    return NULL;
  }
}
