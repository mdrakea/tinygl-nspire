#ifndef TINYGL_GLES_GL_H
#define TINYGL_GLES_GL_H

#include <stddef.h>
#include <stdint.h>
#include <GLES/glplatform.h>
#include <GL/fixed.h>

#ifdef __cplusplus
extern "C" {
#endif

#define GL_VERSION_ES_CL_1_1 1

typedef void GLvoid;
typedef unsigned int GLenum;
typedef unsigned int GLbitfield;
typedef unsigned char GLboolean;
typedef signed char GLbyte;
typedef short GLshort;
typedef int GLint;
typedef unsigned char GLubyte;
typedef unsigned short GLushort;
typedef unsigned int GLuint;
typedef int GLsizei;
typedef ptrdiff_t GLintptr;
typedef ptrdiff_t GLsizeiptr;
typedef GLfixed GLclampx;

enum {
  GL_FALSE = 0,
  GL_TRUE = 1,

  GL_POINTS = 0x0000,
  GL_LINES = 0x0001,
  GL_LINE_LOOP = 0x0002,
  GL_LINE_STRIP = 0x0003,
  GL_TRIANGLES = 0x0004,
  GL_TRIANGLE_STRIP = 0x0005,
  GL_TRIANGLE_FAN = 0x0006,

  GL_DEPTH_BUFFER_BIT = 0x00000100,
  GL_COLOR_BUFFER_BIT = 0x00004000,
  GL_STENCIL_BUFFER_BIT = 0x00000400,

  GL_NEVER = 0x0200,
  GL_LESS = 0x0201,
  GL_EQUAL = 0x0202,
  GL_LEQUAL = 0x0203,
  GL_GREATER = 0x0204,
  GL_NOTEQUAL = 0x0205,
  GL_GEQUAL = 0x0206,
  GL_ALWAYS = 0x0207,

  GL_ZERO = 0,
  GL_ONE = 1,
  GL_SRC_COLOR = 0x0300,
  GL_ONE_MINUS_SRC_COLOR = 0x0301,
  GL_SRC_ALPHA = 0x0302,
  GL_ONE_MINUS_SRC_ALPHA = 0x0303,
  GL_DST_ALPHA = 0x0304,
  GL_ONE_MINUS_DST_ALPHA = 0x0305,
  GL_DST_COLOR = 0x0306,
  GL_ONE_MINUS_DST_COLOR = 0x0307,
  GL_SRC_ALPHA_SATURATE = 0x0308,

  GL_FRONT = 0x0404,
  GL_BACK = 0x0405,
  GL_FRONT_AND_BACK = 0x0408,

  GL_NO_ERROR = 0,
  GL_INVALID_ENUM = 0x0500,
  GL_INVALID_VALUE = 0x0501,
  GL_INVALID_OPERATION = 0x0502,
  GL_STACK_OVERFLOW = 0x0503,
  GL_STACK_UNDERFLOW = 0x0504,
  GL_OUT_OF_MEMORY = 0x0505,

  GL_EXP = 0x0800,
  GL_EXP2 = 0x0801,
  GL_CW = 0x0900,
  GL_CCW = 0x0901,

  GL_FOG = 0x0B60,
  GL_LIGHTING = 0x0B50,
  GL_TEXTURE_2D = 0x0DE1,
  GL_CULL_FACE = 0x0B44,
  GL_ALPHA_TEST = 0x0BC0,
  GL_BLEND = 0x0BE2,
  GL_COLOR_LOGIC_OP = 0x0BF2,
  GL_DITHER = 0x0BD0,
  GL_STENCIL_TEST = 0x0B90,
  GL_DEPTH_TEST = 0x0B71,
  GL_POINT_SMOOTH = 0x0B10,
  GL_LINE_SMOOTH = 0x0B20,
  GL_SCISSOR_TEST = 0x0C11,
  GL_COLOR_MATERIAL = 0x0B57,
  GL_NORMALIZE = 0x0BA1,
  GL_RESCALE_NORMAL = 0x803A,

  GL_VERTEX_ARRAY = 0x8074,
  GL_NORMAL_ARRAY = 0x8075,
  GL_COLOR_ARRAY = 0x8076,
  GL_TEXTURE_COORD_ARRAY = 0x8078,

  GL_FOG_DENSITY = 0x0B62,
  GL_FOG_START = 0x0B63,
  GL_FOG_END = 0x0B64,
  GL_FOG_MODE = 0x0B65,
  GL_FOG_COLOR = 0x0B66,

  GL_CURRENT_COLOR = 0x0B00,
  GL_CURRENT_NORMAL = 0x0B02,
  GL_CURRENT_TEXTURE_COORDS = 0x0B03,
  GL_POINT_SIZE = 0x0B11,
  GL_POINT_SIZE_MIN = 0x8126,
  GL_POINT_SIZE_MAX = 0x8127,
  GL_SMOOTH_POINT_SIZE_RANGE = 0x0B12,
  GL_LINE_WIDTH = 0x0B21,
  GL_SMOOTH_LINE_WIDTH_RANGE = 0x0B22,
  GL_ALIASED_POINT_SIZE_RANGE = 0x846D,
  GL_ALIASED_LINE_WIDTH_RANGE = 0x846E,
  GL_CULL_FACE_MODE = 0x0B45,
  GL_FRONT_FACE = 0x0B46,
  GL_SHADE_MODEL = 0x0B54,
  GL_DEPTH_RANGE = 0x0B70,
  GL_DEPTH_WRITEMASK = 0x0B72,
  GL_DEPTH_CLEAR_VALUE = 0x0B73,
  GL_DEPTH_FUNC = 0x0B74,
  GL_MATRIX_MODE = 0x0BA0,
  GL_VIEWPORT = 0x0BA2,
  GL_MODELVIEW_STACK_DEPTH = 0x0BA3,
  GL_PROJECTION_STACK_DEPTH = 0x0BA4,
  GL_TEXTURE_STACK_DEPTH = 0x0BA5,
  GL_MODELVIEW_MATRIX = 0x0BA6,
  GL_PROJECTION_MATRIX = 0x0BA7,
  GL_TEXTURE_MATRIX = 0x0BA8,
  GL_COLOR_CLEAR_VALUE = 0x0C22,
  GL_MAX_LIGHTS = 0x0D31,
  GL_MAX_CLIP_PLANES = 0x0D32,
  GL_MAX_TEXTURE_SIZE = 0x0D33,
  GL_MAX_MODELVIEW_STACK_DEPTH = 0x0D36,
  GL_MAX_PROJECTION_STACK_DEPTH = 0x0D38,
  GL_MAX_TEXTURE_STACK_DEPTH = 0x0D39,
  GL_MAX_VIEWPORT_DIMS = 0x0D3A,
  GL_MAX_TEXTURE_UNITS = 0x84E2,
  GL_SUBPIXEL_BITS = 0x0D50,
  GL_RED_BITS = 0x0D52,
  GL_GREEN_BITS = 0x0D53,
  GL_BLUE_BITS = 0x0D54,
  GL_ALPHA_BITS = 0x0D55,
  GL_DEPTH_BITS = 0x0D56,
  GL_POLYGON_OFFSET_UNITS = 0x2A00,
  GL_POLYGON_OFFSET_FILL = 0x8037,
  GL_POLYGON_OFFSET_FACTOR = 0x8038,

  GL_TEXTURE_BINDING_2D = 0x8069,
  GL_VERTEX_ARRAY_SIZE = 0x807A,
  GL_VERTEX_ARRAY_TYPE = 0x807B,
  GL_VERTEX_ARRAY_STRIDE = 0x807C,
  GL_NORMAL_ARRAY_TYPE = 0x807E,
  GL_NORMAL_ARRAY_STRIDE = 0x807F,
  GL_COLOR_ARRAY_SIZE = 0x8081,
  GL_COLOR_ARRAY_TYPE = 0x8082,
  GL_COLOR_ARRAY_STRIDE = 0x8083,
  GL_TEXTURE_COORD_ARRAY_SIZE = 0x8088,
  GL_TEXTURE_COORD_ARRAY_TYPE = 0x8089,
  GL_TEXTURE_COORD_ARRAY_STRIDE = 0x808A,
  GL_VERTEX_ARRAY_POINTER = 0x808E,
  GL_NORMAL_ARRAY_POINTER = 0x808F,
  GL_COLOR_ARRAY_POINTER = 0x8090,
  GL_TEXTURE_COORD_ARRAY_POINTER = 0x8092,

  GL_DONT_CARE = 0x1100,
  GL_FASTEST = 0x1101,
  GL_NICEST = 0x1102,
  GL_PERSPECTIVE_CORRECTION_HINT = 0x0C50,
  GL_POINT_SMOOTH_HINT = 0x0C51,
  GL_LINE_SMOOTH_HINT = 0x0C52,
  GL_FOG_HINT = 0x0C54,
  GL_GENERATE_MIPMAP_HINT = 0x8192,

  GL_LIGHT_MODEL_AMBIENT = 0x0B53,
  GL_LIGHT_MODEL_TWO_SIDE = 0x0B52,
  GL_LIGHT_MODEL_LOCAL_VIEWER = 0x0B51,
  GL_AMBIENT = 0x1200,
  GL_DIFFUSE = 0x1201,
  GL_SPECULAR = 0x1202,
  GL_POSITION = 0x1203,
  GL_SPOT_DIRECTION = 0x1204,
  GL_SPOT_EXPONENT = 0x1205,
  GL_SPOT_CUTOFF = 0x1206,
  GL_CONSTANT_ATTENUATION = 0x1207,
  GL_LINEAR_ATTENUATION = 0x1208,
  GL_QUADRATIC_ATTENUATION = 0x1209,

  GL_BYTE = 0x1400,
  GL_UNSIGNED_BYTE = 0x1401,
  GL_SHORT = 0x1402,
  GL_UNSIGNED_SHORT = 0x1403,
  GL_FLOAT = 0x1406,
  GL_FIXED = 0x140C,
  GL_UNSIGNED_SHORT_4_4_4_4 = 0x8033,
  GL_UNSIGNED_SHORT_5_5_5_1 = 0x8034,
  GL_UNSIGNED_SHORT_5_6_5 = 0x8363,

  GL_EMISSION = 0x1600,
  GL_SHININESS = 0x1601,
  GL_AMBIENT_AND_DIFFUSE = 0x1602,
  GL_MODELVIEW = 0x1700,
  GL_PROJECTION = 0x1701,
  GL_TEXTURE = 0x1702,

  GL_ALPHA = 0x1906,
  GL_RGB = 0x1907,
  GL_RGBA = 0x1908,
  GL_LUMINANCE = 0x1909,
  GL_LUMINANCE_ALPHA = 0x190A,
  GL_UNPACK_ALIGNMENT = 0x0CF5,
  GL_PACK_ALIGNMENT = 0x0D05,

  GL_FLAT = 0x1D00,
  GL_SMOOTH = 0x1D01,
  GL_VENDOR = 0x1F00,
  GL_RENDERER = 0x1F01,
  GL_VERSION = 0x1F02,
  GL_EXTENSIONS = 0x1F03,

  GL_MODULATE = 0x2100,
  GL_DECAL = 0x2101,
  GL_ADD = 0x0104,
  GL_TEXTURE_ENV_MODE = 0x2200,
  GL_TEXTURE_ENV_COLOR = 0x2201,
  GL_TEXTURE_ENV = 0x2300,
  GL_NEAREST = 0x2600,
  GL_LINEAR = 0x2601,
  GL_NEAREST_MIPMAP_NEAREST = 0x2700,
  GL_LINEAR_MIPMAP_NEAREST = 0x2701,
  GL_NEAREST_MIPMAP_LINEAR = 0x2702,
  GL_LINEAR_MIPMAP_LINEAR = 0x2703,
  GL_TEXTURE_MAG_FILTER = 0x2800,
  GL_TEXTURE_MIN_FILTER = 0x2801,
  GL_TEXTURE_WRAP_S = 0x2802,
  GL_TEXTURE_WRAP_T = 0x2803,
  GL_GENERATE_MIPMAP = 0x8191,
  GL_TEXTURE0 = 0x84C0,
  GL_ACTIVE_TEXTURE = 0x84E0,
  GL_CLIENT_ACTIVE_TEXTURE = 0x84E1,
  GL_REPEAT = 0x2901,
  GL_CLAMP_TO_EDGE = 0x812F,

  GL_LIGHT0 = 0x4000,
  GL_LIGHT1 = 0x4001,
  GL_LIGHT2 = 0x4002,
  GL_LIGHT3 = 0x4003,
  GL_LIGHT4 = 0x4004,
  GL_LIGHT5 = 0x4005,
  GL_LIGHT6 = 0x4006,
  GL_LIGHT7 = 0x4007,

  GL_ARRAY_BUFFER = 0x8892,
  GL_ELEMENT_ARRAY_BUFFER = 0x8893,
  GL_ARRAY_BUFFER_BINDING = 0x8894,
  GL_ELEMENT_ARRAY_BUFFER_BINDING = 0x8895,
  GL_VERTEX_ARRAY_BUFFER_BINDING = 0x8896,
  GL_NORMAL_ARRAY_BUFFER_BINDING = 0x8897,
  GL_COLOR_ARRAY_BUFFER_BINDING = 0x8898,
  GL_TEXTURE_COORD_ARRAY_BUFFER_BINDING = 0x889A,
  GL_STATIC_DRAW = 0x88E4,
  GL_DYNAMIC_DRAW = 0x88E8,
  GL_BUFFER_SIZE = 0x8764,
  GL_BUFFER_USAGE = 0x8765
};

GL_API void GL_APIENTRY glBindBuffer(GLenum target, GLuint buffer);
GL_API void GL_APIENTRY glBindTexture(GLenum target, GLuint texture);
GL_API void GL_APIENTRY glBufferData(GLenum target, GLsizeiptr size, const void *data, GLenum usage);
GL_API void GL_APIENTRY glBufferSubData(GLenum target, GLintptr offset, GLsizeiptr size, const void *data);
GL_API void GL_APIENTRY glClear(GLbitfield mask);
GL_API void GL_APIENTRY glClearColorx(GLfixed red, GLfixed green, GLfixed blue, GLfixed alpha);
GL_API void GL_APIENTRY glClearDepthx(GLfixed depth);
GL_API void GL_APIENTRY glColor4x(GLfixed red, GLfixed green, GLfixed blue, GLfixed alpha);
GL_API void GL_APIENTRY glColor4ub(GLubyte red, GLubyte green, GLubyte blue, GLubyte alpha);
GL_API void GL_APIENTRY glColorPointer(GLint size, GLenum type, GLsizei stride, const void *pointer);
GL_API void GL_APIENTRY glCullFace(GLenum mode);
GL_API void GL_APIENTRY glDeleteBuffers(GLsizei n, const GLuint *buffers);
GL_API void GL_APIENTRY glDeleteTextures(GLsizei n, const GLuint *textures);
GL_API void GL_APIENTRY glDisable(GLenum cap);
GL_API void GL_APIENTRY glDisableClientState(GLenum array);
GL_API void GL_APIENTRY glDrawArrays(GLenum mode, GLint first, GLsizei count);
GL_API void GL_APIENTRY glDrawElements(GLenum mode, GLsizei count, GLenum type, const void *indices);
GL_API void GL_APIENTRY glEnable(GLenum cap);
GL_API void GL_APIENTRY glEnableClientState(GLenum array);
GL_API void GL_APIENTRY glFinish(void);
GL_API void GL_APIENTRY glFlush(void);
GL_API void GL_APIENTRY glFrontFace(GLenum mode);
GL_API void GL_APIENTRY glFrustumx(GLfixed left, GLfixed right, GLfixed bottom, GLfixed top, GLfixed near, GLfixed far);
GL_API void GL_APIENTRY glGenBuffers(GLsizei n, GLuint *buffers);
GL_API void GL_APIENTRY glGenTextures(GLsizei n, GLuint *textures);
GL_API GLenum GL_APIENTRY glGetError(void);
GL_API void GL_APIENTRY glGetBufferParameteriv(GLenum target, GLenum pname, GLint *params);
GL_API void GL_APIENTRY glGetFixedv(GLenum pname, GLfixed *params);
GL_API void GL_APIENTRY glGetIntegerv(GLenum pname, GLint *params);
GL_API void GL_APIENTRY glGetPointerv(GLenum pname, void **params);
GL_API const GLubyte *GL_APIENTRY glGetString(GLenum name);
GL_API void GL_APIENTRY glHint(GLenum target, GLenum mode);
GL_API GLboolean GL_APIENTRY glIsBuffer(GLuint buffer);
GL_API GLboolean GL_APIENTRY glIsEnabled(GLenum cap);
GL_API GLboolean GL_APIENTRY glIsTexture(GLuint texture);
GL_API void GL_APIENTRY glLightModelx(GLenum pname, GLfixed param);
GL_API void GL_APIENTRY glLightModelxv(GLenum pname, const GLfixed *params);
GL_API void GL_APIENTRY glLightx(GLenum light, GLenum pname, GLfixed param);
GL_API void GL_APIENTRY glLightxv(GLenum light, GLenum pname, const GLfixed *params);
GL_API void GL_APIENTRY glLoadIdentity(void);
GL_API void GL_APIENTRY glLoadMatrixx(const GLfixed *m);
GL_API void GL_APIENTRY glMaterialx(GLenum face, GLenum pname, GLfixed param);
GL_API void GL_APIENTRY glMaterialxv(GLenum face, GLenum pname, const GLfixed *params);
GL_API void GL_APIENTRY glMatrixMode(GLenum mode);
GL_API void GL_APIENTRY glMultMatrixx(const GLfixed *m);
GL_API void GL_APIENTRY glMultiTexCoord4x(GLenum target, GLfixed s, GLfixed t, GLfixed r, GLfixed q);
GL_API void GL_APIENTRY glNormal3x(GLfixed nx, GLfixed ny, GLfixed nz);
GL_API void GL_APIENTRY glNormalPointer(GLenum type, GLsizei stride, const void *pointer);
GL_API void GL_APIENTRY glOrthox(GLfixed left, GLfixed right, GLfixed bottom, GLfixed top, GLfixed near, GLfixed far);
GL_API void GL_APIENTRY glPixelStorei(GLenum pname, GLint param);
GL_API void GL_APIENTRY glPolygonOffsetx(GLfixed factor, GLfixed units);
GL_API void GL_APIENTRY glPopMatrix(void);
GL_API void GL_APIENTRY glPushMatrix(void);
GL_API void GL_APIENTRY glRotatex(GLfixed angle, GLfixed x, GLfixed y, GLfixed z);
GL_API void GL_APIENTRY glScalex(GLfixed x, GLfixed y, GLfixed z);
GL_API void GL_APIENTRY glShadeModel(GLenum mode);
GL_API void GL_APIENTRY glTexCoordPointer(GLint size, GLenum type, GLsizei stride, const void *pointer);
GL_API void GL_APIENTRY glTexEnvi(GLenum target, GLenum pname, GLint param);
GL_API void GL_APIENTRY glTexEnviv(GLenum target, GLenum pname, const GLint *params);
GL_API void GL_APIENTRY glTexEnvx(GLenum target, GLenum pname, GLfixed param);
GL_API void GL_APIENTRY glTexEnvxv(GLenum target, GLenum pname, const GLfixed *params);
GL_API void GL_APIENTRY glTexImage2D(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const void *pixels);
GL_API void GL_APIENTRY glTexParameteri(GLenum target, GLenum pname, GLint param);
GL_API void GL_APIENTRY glTexParameteriv(GLenum target, GLenum pname, const GLint *params);
GL_API void GL_APIENTRY glTexParameterx(GLenum target, GLenum pname, GLfixed param);
GL_API void GL_APIENTRY glTexParameterxv(GLenum target, GLenum pname, const GLfixed *params);
GL_API void GL_APIENTRY glTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const void *pixels);
GL_API void GL_APIENTRY glTranslatex(GLfixed x, GLfixed y, GLfixed z);
GL_API void GL_APIENTRY glVertexPointer(GLint size, GLenum type, GLsizei stride, const void *pointer);
GL_API void GL_APIENTRY glViewport(GLint x, GLint y, GLsizei width, GLsizei height);

void glInit(void *zbuffer);
void glClose(void);

#ifdef __cplusplus
}
#endif

#endif /* TINYGL_GLES_GL_H */
