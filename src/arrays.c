#include "zgl.h"
#include <stdint.h>

static GLArray *array_for_name(GLContext *c, GLenum array)
{
  switch (array) {
  case GL_VERTEX_ARRAY: return &c->vertex_array;
  case GL_NORMAL_ARRAY: return &c->normal_array;
  case GL_COLOR_ARRAY: return &c->color_array;
  case GL_TEXTURE_COORD_ARRAY: return &c->texcoord_array;
  default: return NULL;
  }
}

static int component_size(GLenum type)
{
  switch (type) {
  case GL_BYTE: return sizeof(GLbyte);
  case GL_UNSIGNED_BYTE: return sizeof(GLubyte);
  case GL_SHORT: return sizeof(GLshort);
  case GL_UNSIGNED_SHORT: return sizeof(GLushort);
  case GL_FIXED: return sizeof(GLfixed);
  default: return 0;
  }
}

static int fixed_array_type(GLenum type)
{
  return type == GL_BYTE || type == GL_SHORT || type == GL_FIXED;
}

static GLfixed read_component(const unsigned char *p, GLenum type, int normalized)
{
  switch (type) {
  case GL_BYTE:
    return TGL_I(*(const GLbyte *)p);
  case GL_SHORT:
    return TGL_I(*(const GLshort *)p);
  case GL_FIXED:
    return *(const GLfixed *)p;
  case GL_UNSIGNED_BYTE:
    if (normalized) return TGL_FRAC(*(const GLubyte *)p, 255);
    return TGL_I(*(const GLubyte *)p);
  case GL_UNSIGNED_SHORT:
    return TGL_I(*(const GLushort *)p);
  default:
    return 0;
  }
}

static int array_stride(const GLArray *array)
{
  if (array->stride != 0) return array->stride;
  return array->size * component_size(array->type);
}

static const unsigned char *array_element_ptr(const GLArray *array, GLint index)
{
  uintptr_t offset;
  const unsigned char *base;
  int stride;
  int size;
  uint64_t needed;

  size = component_size(array->type);
  if (size == 0) return NULL;
  if (array->buffer != NULL) {
    offset = (uintptr_t)array->pointer;
    stride = array_stride(array);
    needed = (uint64_t)offset + (uint64_t)index * (uint64_t)stride +
      (uint64_t)array->size * (uint64_t)size;
    if (array->buffer->data == NULL || needed > (uint64_t)array->buffer->size) return NULL;
    base = array->buffer->data + offset;
    return base + (uintptr_t)index * (uintptr_t)stride;
  } else {
    if (array->pointer == NULL) return NULL;
    base = (const unsigned char *)array->pointer;
  }

  stride = array_stride(array);
  return base + (uintptr_t)index * (uintptr_t)stride;
}

static int fetch_array_values(const GLArray *array, GLint index, GLfixed *v, int normalized)
{
  const unsigned char *p = array_element_ptr(array, index);
  int size = component_size(array->type);
  int i;

  if (p == NULL || size == 0) return 0;
  for (i = 0; i < array->size; i++) {
    v[i] = read_component(p + i * size, array->type, normalized);
  }
  return 1;
}

static int apply_array_element(GLContext *c, GLint index)
{
  GLfixed v[4];

  if (index < 0) {
    gl_set_error(c, GL_INVALID_VALUE);
    return 0;
  }

  if (c->color_array.enabled) {
    GLParam p[8];
    if (!fetch_array_values(&c->color_array, index, v, c->color_array.type == GL_UNSIGNED_BYTE)) {
      gl_set_error(c, GL_INVALID_OPERATION);
      return 0;
    }
    p[1].f = v[0];
    p[2].f = v[1];
    p[3].f = v[2];
    p[4].f = c->color_array.size > 3 ? v[3] : TGL_FIX_ONE;
    p[5].ui = (unsigned int)tgl_fix_to_range(p[1].f, ZB_POINT_RED_MIN, ZB_POINT_RED_MAX);
    p[6].ui = (unsigned int)tgl_fix_to_range(p[2].f, ZB_POINT_GREEN_MIN, ZB_POINT_GREEN_MAX);
    p[7].ui = (unsigned int)tgl_fix_to_range(p[3].f, ZB_POINT_BLUE_MIN, ZB_POINT_BLUE_MAX);
    glopColor(c, p);
  }

  if (c->normal_array.enabled) {
    GLParam p[4];
    if (!fetch_array_values(&c->normal_array, index, v, 0)) {
      gl_set_error(c, GL_INVALID_OPERATION);
      return 0;
    }
    p[1].f = v[0];
    p[2].f = v[1];
    p[3].f = v[2];
    glopNormal(c, p);
  }

  if (c->texcoord_array.enabled) {
    GLParam p[5];
    if (!fetch_array_values(&c->texcoord_array, index, v, 0)) {
      gl_set_error(c, GL_INVALID_OPERATION);
      return 0;
    }
    p[1].f = v[0];
    p[2].f = c->texcoord_array.size > 1 ? v[1] : 0;
    p[3].f = c->texcoord_array.size > 2 ? v[2] : 0;
    p[4].f = c->texcoord_array.size > 3 ? v[3] : TGL_FIX_ONE;
    glopTexCoord(c, p);
  }

  if (!c->vertex_array.enabled) {
    gl_set_error(c, GL_INVALID_OPERATION);
    return 0;
  }
  if (!fetch_array_values(&c->vertex_array, index, v, 0)) {
    gl_set_error(c, GL_INVALID_OPERATION);
    return 0;
  }
  gl_submit_vertex(c,
                   v[0],
                   c->vertex_array.size > 1 ? v[1] : 0,
                   c->vertex_array.size > 2 ? v[2] : 0,
                   c->vertex_array.size > 3 ? v[3] : TGL_FIX_ONE);
  return 1;
}

static int valid_draw_mode(GLenum mode)
{
  switch (mode) {
  case GL_POINTS:
  case GL_LINES:
  case GL_LINE_LOOP:
  case GL_LINE_STRIP:
  case GL_TRIANGLES:
  case GL_TRIANGLE_STRIP:
  case GL_TRIANGLE_FAN:
    return 1;
  default:
    return 0;
  }
}

void glArrayElement(GLint i)
{
  GLContext *c = gl_get_context();
  apply_array_element(c, i);
}

void glEnableClientState(GLenum array)
{
  GLContext *c = gl_get_context();
  GLArray *a = array_for_name(c, array);

  if (a == NULL) {
    gl_set_error(c, GL_INVALID_ENUM);
    return;
  }
  a->enabled = 1;
}

void glDisableClientState(GLenum array)
{
  GLContext *c = gl_get_context();
  GLArray *a = array_for_name(c, array);

  if (a == NULL) {
    gl_set_error(c, GL_INVALID_ENUM);
    return;
  }
  a->enabled = 0;
}

static void set_pointer(GLContext *c, GLArray *array, GLint size, GLenum type,
                        GLsizei stride, const void *pointer)
{
  array->size = size;
  array->type = type;
  array->stride = stride;
  array->pointer = pointer;
  array->buffer = c->array_buffer_binding;
}

void glVertexPointer(GLint size, GLenum type, GLsizei stride, const GLvoid *pointer)
{
  GLContext *c = gl_get_context();

  if (size < 2 || size > 4) {
    gl_set_error(c, GL_INVALID_VALUE);
    return;
  }
  if (stride < 0) {
    gl_set_error(c, GL_INVALID_VALUE);
    return;
  }
  if (!fixed_array_type(type)) {
    gl_set_error(c, GL_INVALID_ENUM);
    return;
  }
  set_pointer(c, &c->vertex_array, size, type, stride, pointer);
}

void glColorPointer(GLint size, GLenum type, GLsizei stride, const GLvoid *pointer)
{
  GLContext *c = gl_get_context();

  if (size != 4) {
    gl_set_error(c, GL_INVALID_VALUE);
    return;
  }
  if (stride < 0) {
    gl_set_error(c, GL_INVALID_VALUE);
    return;
  }
  if (type != GL_FIXED && type != GL_UNSIGNED_BYTE) {
    gl_set_error(c, GL_INVALID_ENUM);
    return;
  }
  set_pointer(c, &c->color_array, size, type, stride, pointer);
}

void glNormalPointer(GLenum type, GLsizei stride, const GLvoid *pointer)
{
  GLContext *c = gl_get_context();

  if (stride < 0) {
    gl_set_error(c, GL_INVALID_VALUE);
    return;
  }
  if (!fixed_array_type(type)) {
    gl_set_error(c, GL_INVALID_ENUM);
    return;
  }
  set_pointer(c, &c->normal_array, 3, type, stride, pointer);
}

void glTexCoordPointer(GLint size, GLenum type, GLsizei stride, const GLvoid *pointer)
{
  GLContext *c = gl_get_context();

  if (size < 2 || size > 4) {
    gl_set_error(c, GL_INVALID_VALUE);
    return;
  }
  if (stride < 0) {
    gl_set_error(c, GL_INVALID_VALUE);
    return;
  }
  if (!fixed_array_type(type)) {
    gl_set_error(c, GL_INVALID_ENUM);
    return;
  }
  set_pointer(c, &c->texcoord_array, size, type, stride, pointer);
}

void glDrawArrays(GLenum mode, GLint first, GLsizei count)
{
  GLContext *c = gl_get_context();
  GLint i;

  if (!valid_draw_mode(mode)) {
    gl_set_error(c, GL_INVALID_ENUM);
    return;
  }
  if (first < 0 || count < 0) {
    gl_set_error(c, GL_INVALID_VALUE);
    return;
  }
  if (count == 0) return;
  if (!gl_begin_primitive(c, mode)) return;
  for (i = 0; i < count; i++) {
    if (!apply_array_element(c, first + i)) break;
  }
  gl_end_primitive(c);
}

static const unsigned char *element_index_ptr(GLContext *c, GLsizei count,
                                              int index_size, const void *indices)
{
  uintptr_t offset;
  uint64_t needed;

  if (c->element_array_buffer_binding != NULL) {
    offset = (uintptr_t)indices;
    needed = (uint64_t)offset + (uint64_t)count * (uint64_t)index_size;
    if (c->element_array_buffer_binding->data == NULL ||
        needed > (uint64_t)c->element_array_buffer_binding->size) {
      return NULL;
    }
    return c->element_array_buffer_binding->data + offset;
  }
  return (const unsigned char *)indices;
}

void glDrawElements(GLenum mode, GLsizei count, GLenum type, const GLvoid *indices)
{
  GLContext *c = gl_get_context();
  const unsigned char *p;
  int size;
  GLsizei i;

  if (!valid_draw_mode(mode)) {
    gl_set_error(c, GL_INVALID_ENUM);
    return;
  }
  if (count < 0) {
    gl_set_error(c, GL_INVALID_VALUE);
    return;
  }
  if (type != GL_UNSIGNED_BYTE && type != GL_UNSIGNED_SHORT) {
    gl_set_error(c, GL_INVALID_ENUM);
    return;
  }
  if (count == 0) return;

  size = component_size(type);
  p = element_index_ptr(c, count, size, indices);
  if (p == NULL) {
    gl_set_error(c, GL_INVALID_OPERATION);
    return;
  }

  if (!gl_begin_primitive(c, mode)) return;
  for (i = 0; i < count; i++) {
    GLint index = (type == GL_UNSIGNED_BYTE) ?
      (GLint)*(const GLubyte *)(p + i * size) :
      (GLint)*(const GLushort *)(p + i * size);
    if (!apply_array_element(c, index)) break;
  }
  gl_end_primitive(c);
}

void glGetPointerv(GLenum pname, void **params)
{
  GLContext *c = gl_get_context();
  GLArray *array = NULL;

  if (params == NULL) {
    gl_set_error(c, GL_INVALID_VALUE);
    return;
  }

  switch (pname) {
  case GL_VERTEX_ARRAY_POINTER: array = &c->vertex_array; break;
  case GL_NORMAL_ARRAY_POINTER: array = &c->normal_array; break;
  case GL_COLOR_ARRAY_POINTER: array = &c->color_array; break;
  case GL_TEXTURE_COORD_ARRAY_POINTER: array = &c->texcoord_array; break;
  default:
    gl_set_error(c, GL_INVALID_ENUM);
    return;
  }
  *params = (void *)array->pointer;
}

static GLBuffer *find_buffer(GLContext *c, GLuint handle)
{
  GLBuffer *b;
  for (b = c->buffers; b != NULL; b = b->next) {
    if (b->handle == handle) return b;
  }
  return NULL;
}

static GLBuffer *alloc_buffer(GLContext *c, GLuint handle)
{
  GLBuffer *b = gl_zalloc(sizeof(GLBuffer));
  if (b == NULL) {
    gl_set_error(c, GL_OUT_OF_MEMORY);
    return NULL;
  }
  b->handle = handle;
  b->usage = GL_STATIC_DRAW;
  b->next = c->buffers;
  c->buffers = b;
  if (handle >= c->next_buffer_handle) c->next_buffer_handle = handle + 1;
  return b;
}

void glGenBuffers(GLsizei n, GLuint *buffers)
{
  GLContext *c = gl_get_context();
  GLsizei i;

  if (n < 0) {
    gl_set_error(c, GL_INVALID_VALUE);
    return;
  }
  if (buffers == NULL) {
    gl_set_error(c, GL_INVALID_VALUE);
    return;
  }
  for (i = 0; i < n; i++) {
    GLuint handle = c->next_buffer_handle++;
    alloc_buffer(c, handle);
    buffers[i] = handle;
  }
}

void glBindBuffer(GLenum target, GLuint buffer)
{
  GLContext *c = gl_get_context();
  GLBuffer *b = NULL;

  if (target != GL_ARRAY_BUFFER && target != GL_ELEMENT_ARRAY_BUFFER) {
    gl_set_error(c, GL_INVALID_ENUM);
    return;
  }
  if (buffer != 0) {
    b = find_buffer(c, buffer);
    if (b == NULL) b = alloc_buffer(c, buffer);
    if (b == NULL) return;
  }
  if (target == GL_ARRAY_BUFFER) c->array_buffer_binding = b;
  else c->element_array_buffer_binding = b;
}

static GLBuffer *bound_buffer(GLContext *c, GLenum target)
{
  switch (target) {
  case GL_ARRAY_BUFFER: return c->array_buffer_binding;
  case GL_ELEMENT_ARRAY_BUFFER: return c->element_array_buffer_binding;
  default: return NULL;
  }
}

void glBufferData(GLenum target, GLsizeiptr size, const void *data, GLenum usage)
{
  GLContext *c = gl_get_context();
  GLBuffer *b;
  unsigned char *new_data = NULL;

  if (target != GL_ARRAY_BUFFER && target != GL_ELEMENT_ARRAY_BUFFER) {
    gl_set_error(c, GL_INVALID_ENUM);
    return;
  }
  if (usage != GL_STATIC_DRAW && usage != GL_DYNAMIC_DRAW) {
    gl_set_error(c, GL_INVALID_ENUM);
    return;
  }
  if (size < 0) {
    gl_set_error(c, GL_INVALID_VALUE);
    return;
  }
  b = bound_buffer(c, target);
  if (b == NULL) {
    gl_set_error(c, GL_INVALID_OPERATION);
    return;
  }
  if (size > 0) {
    new_data = gl_malloc((int)size);
    if (new_data == NULL) {
      gl_set_error(c, GL_OUT_OF_MEMORY);
      return;
    }
    if (data != NULL) memcpy(new_data, data, (size_t)size);
  }
  if (b->data != NULL) gl_free(b->data);
  b->data = new_data;
  b->size = size;
  b->usage = usage;
}

void glBufferSubData(GLenum target, GLintptr offset, GLsizeiptr size, const void *data)
{
  GLContext *c = gl_get_context();
  GLBuffer *b;

  if (target != GL_ARRAY_BUFFER && target != GL_ELEMENT_ARRAY_BUFFER) {
    gl_set_error(c, GL_INVALID_ENUM);
    return;
  }
  if (offset < 0 || size < 0 || data == NULL) {
    gl_set_error(c, GL_INVALID_VALUE);
    return;
  }
  b = bound_buffer(c, target);
  if (b == NULL) {
    gl_set_error(c, GL_INVALID_OPERATION);
    return;
  }
  if (offset + size > b->size) {
    gl_set_error(c, GL_INVALID_VALUE);
    return;
  }
  memcpy(b->data + offset, data, (size_t)size);
}

void glDeleteBuffers(GLsizei n, const GLuint *buffers)
{
  GLContext *c = gl_get_context();
  GLsizei i;

  if (n < 0) {
    gl_set_error(c, GL_INVALID_VALUE);
    return;
  }
  if (buffers == NULL) return;
  for (i = 0; i < n; i++) {
    GLuint handle = buffers[i];
    GLBuffer **pp = &c->buffers;
    while (*pp != NULL) {
      GLBuffer *b = *pp;
      if (b->handle == handle) {
        *pp = b->next;
        if (c->array_buffer_binding == b) c->array_buffer_binding = NULL;
        if (c->element_array_buffer_binding == b) c->element_array_buffer_binding = NULL;
        if (c->vertex_array.buffer == b) c->vertex_array.buffer = NULL;
        if (c->normal_array.buffer == b) c->normal_array.buffer = NULL;
        if (c->color_array.buffer == b) c->color_array.buffer = NULL;
        if (c->texcoord_array.buffer == b) c->texcoord_array.buffer = NULL;
        if (b->data != NULL) gl_free(b->data);
        gl_free(b);
        break;
      }
      pp = &b->next;
    }
  }
}

GLboolean glIsBuffer(GLuint buffer)
{
  GLContext *c = gl_get_context();
  if (buffer == 0) return GL_FALSE;
  return find_buffer(c, buffer) != NULL ? GL_TRUE : GL_FALSE;
}

void glGetBufferParameteriv(GLenum target, GLenum pname, GLint *params)
{
  GLContext *c = gl_get_context();
  GLBuffer *b;

  if (params == NULL) {
    gl_set_error(c, GL_INVALID_VALUE);
    return;
  }
  if (target != GL_ARRAY_BUFFER && target != GL_ELEMENT_ARRAY_BUFFER) {
    gl_set_error(c, GL_INVALID_ENUM);
    return;
  }
  b = bound_buffer(c, target);
  if (b == NULL) {
    gl_set_error(c, GL_INVALID_OPERATION);
    return;
  }
  switch (pname) {
  case GL_BUFFER_SIZE:
    *params = (GLint)b->size;
    break;
  case GL_BUFFER_USAGE:
    *params = (GLint)b->usage;
    break;
  default:
    gl_set_error(c, GL_INVALID_ENUM);
    break;
  }
}
