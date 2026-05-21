#include <stdarg.h>
#include "zgl.h"

int vfiprintf(FILE *stream, const char *format, va_list ap);

void gl_fatal_error(char *format, ...)
{
  va_list ap;

  va_start(ap,format);

  fiprintf(stderr,"TinyGL: fatal error: ");
  vfiprintf(stderr,format,ap);
  fputc('\n', stderr);
  exit(1);

  va_end(ap);
}

void gl_set_error(GLContext *c, GLenum error)
{
  if (c != NULL && c->error == GL_NO_ERROR) {
    c->error = error;
  }
}

GLenum glGetError(void)
{
  GLContext *c = gl_get_context();
  GLenum error;

  if (c == NULL) return GL_INVALID_OPERATION;
  error = c->error;
  c->error = GL_NO_ERROR;
  return error;
}
