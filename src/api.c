#include "zgl.h"

static int valid_light(GLenum light)
{
  return light >= GL_LIGHT0 && light < GL_LIGHT0 + MAX_LIGHTS;
}

static int valid_face(GLenum face)
{
  return face == GL_FRONT || face == GL_BACK || face == GL_FRONT_AND_BACK;
}

static int valid_material_pname(GLenum pname)
{
  switch (pname) {
  case GL_EMISSION:
  case GL_AMBIENT:
  case GL_DIFFUSE:
  case GL_SPECULAR:
  case GL_SHININESS:
  case GL_AMBIENT_AND_DIFFUSE:
    return 1;
  default:
    return 0;
  }
}

static int valid_light_pname(GLenum pname)
{
  switch (pname) {
  case GL_AMBIENT:
  case GL_DIFFUSE:
  case GL_SPECULAR:
  case GL_POSITION:
  case GL_SPOT_DIRECTION:
  case GL_SPOT_EXPONENT:
  case GL_SPOT_CUTOFF:
  case GL_CONSTANT_ATTENUATION:
  case GL_LINEAR_ATTENUATION:
  case GL_QUADRATIC_ATTENUATION:
    return 1;
  default:
    return 0;
  }
}

static void call_color(GLContext *c, GLfixed r, GLfixed g, GLfixed b, GLfixed a)
{
  GLParam p[8];
  p[1].f = r;
  p[2].f = g;
  p[3].f = b;
  p[4].f = a;
  p[5].ui = (unsigned int)tgl_fix_to_range(r, ZB_POINT_RED_MIN, ZB_POINT_RED_MAX);
  p[6].ui = (unsigned int)tgl_fix_to_range(g, ZB_POINT_GREEN_MIN, ZB_POINT_GREEN_MAX);
  p[7].ui = (unsigned int)tgl_fix_to_range(b, ZB_POINT_BLUE_MIN, ZB_POINT_BLUE_MAX);
  glopColor(c, p);
}

void glColor4x(GLfixed red, GLfixed green, GLfixed blue, GLfixed alpha)
{
  call_color(gl_get_context(), red, green, blue, alpha);
}

void glColor4ub(GLubyte red, GLubyte green, GLubyte blue, GLubyte alpha)
{
  call_color(gl_get_context(), TGL_FRAC(red, 255), TGL_FRAC(green, 255),
             TGL_FRAC(blue, 255), TGL_FRAC(alpha, 255));
}

void glNormal3x(GLfixed nx, GLfixed ny, GLfixed nz)
{
  GLParam p[4];
  GLContext *c = gl_get_context();
  p[1].f = nx;
  p[2].f = ny;
  p[3].f = nz;
  glopNormal(c, p);
}

void glMultiTexCoord4x(GLenum target, GLfixed s, GLfixed t, GLfixed r, GLfixed q)
{
  GLParam p[5];
  GLContext *c = gl_get_context();

  if (target != GL_TEXTURE0) {
    gl_set_error(c, GL_INVALID_ENUM);
    return;
  }
  p[1].f = s;
  p[2].f = t;
  p[3].f = r;
  p[4].f = q;
  glopTexCoord(c, p);
}

void glShadeModel(GLenum mode)
{
  GLParam p[2];
  GLContext *c = gl_get_context();

  if (mode != GL_FLAT && mode != GL_SMOOTH) {
    gl_set_error(c, GL_INVALID_ENUM);
    return;
  }
  p[1].i = mode;
  glopShadeModel(c, p);
}

void glCullFace(GLenum mode)
{
  GLParam p[2];
  GLContext *c = gl_get_context();

  if (mode != GL_BACK && mode != GL_FRONT && mode != GL_FRONT_AND_BACK) {
    gl_set_error(c, GL_INVALID_ENUM);
    return;
  }
  p[1].i = mode;
  glopCullFace(c, p);
}

void glFrontFace(GLenum mode)
{
  GLParam p[2];
  GLContext *c = gl_get_context();

  if (mode != GL_CCW && mode != GL_CW) {
    gl_set_error(c, GL_INVALID_ENUM);
    return;
  }
  p[1].i = (mode != GL_CCW);
  glopFrontFace(c, p);
}

static int enable_supported(GLenum cap)
{
  return cap == GL_CULL_FACE ||
         cap == GL_LIGHTING ||
         cap == GL_COLOR_MATERIAL ||
         cap == GL_TEXTURE_2D ||
         cap == GL_NORMALIZE ||
         cap == GL_DEPTH_TEST ||
         cap == GL_POLYGON_OFFSET_FILL ||
         cap == GL_DITHER ||
         valid_light(cap);
}

static void enable_disable(GLenum cap, int enabled)
{
  GLParam p[3];
  GLContext *c = gl_get_context();

  if (!enable_supported(cap)) {
    gl_set_error(c, GL_INVALID_ENUM);
    return;
  }
  if (cap == GL_DITHER) return;
  p[1].i = cap;
  p[2].i = enabled;
  glopEnableDisable(c, p);
}

void glEnable(GLenum cap)
{
  enable_disable(cap, 1);
}

void glDisable(GLenum cap)
{
  enable_disable(cap, 0);
}

GLboolean glIsEnabled(GLenum cap)
{
  GLContext *c = gl_get_context();

  switch (cap) {
  case GL_CULL_FACE: return c->cull_face_enabled ? GL_TRUE : GL_FALSE;
  case GL_LIGHTING: return c->lighting_enabled ? GL_TRUE : GL_FALSE;
  case GL_COLOR_MATERIAL: return c->color_material_enabled ? GL_TRUE : GL_FALSE;
  case GL_TEXTURE_2D: return c->texture_2d_enabled ? GL_TRUE : GL_FALSE;
  case GL_NORMALIZE: return c->normalize_enabled ? GL_TRUE : GL_FALSE;
  case GL_DEPTH_TEST: return c->depth_test ? GL_TRUE : GL_FALSE;
  case GL_POLYGON_OFFSET_FILL: return (c->offset_states & TGL_OFFSET_FILL) ? GL_TRUE : GL_FALSE;
  case GL_DITHER: return GL_FALSE;
  default:
    if (valid_light(cap)) return c->lights[cap - GL_LIGHT0].enabled ? GL_TRUE : GL_FALSE;
    gl_set_error(c, GL_INVALID_ENUM);
    return GL_FALSE;
  }
}

void glMatrixMode(GLenum mode)
{
  GLParam p[2];
  GLContext *c = gl_get_context();

  if (mode != GL_MODELVIEW && mode != GL_PROJECTION && mode != GL_TEXTURE) {
    gl_set_error(c, GL_INVALID_ENUM);
    return;
  }
  p[1].i = mode;
  glopMatrixMode(c, p);
}

void glLoadMatrixx(const GLfixed *m)
{
  GLParam p[17];
  GLContext *c = gl_get_context();
  int i;

  if (m == NULL) {
    gl_set_error(c, GL_INVALID_VALUE);
    return;
  }
  for (i = 0; i < 16; i++) p[i + 1].f = m[i];
  glopLoadMatrix(c, p);
}

void glLoadIdentity(void)
{
  GLContext *c = gl_get_context();
  glopLoadIdentity(c, NULL);
}

void glMultMatrixx(const GLfixed *m)
{
  GLParam p[17];
  GLContext *c = gl_get_context();
  int i;

  if (m == NULL) {
    gl_set_error(c, GL_INVALID_VALUE);
    return;
  }
  for (i = 0; i < 16; i++) p[i + 1].f = m[i];
  glopMultMatrix(c, p);
}

void glPushMatrix(void)
{
  GLContext *c = gl_get_context();
  int n = c->matrix_mode;

  if ((c->matrix_stack_ptr[n] - c->matrix_stack[n] + 1) >= c->matrix_stack_depth_max[n]) {
    gl_set_error(c, GL_STACK_OVERFLOW);
    return;
  }
  glopPushMatrix(c, NULL);
}

void glPopMatrix(void)
{
  GLContext *c = gl_get_context();
  int n = c->matrix_mode;

  if (c->matrix_stack_ptr[n] <= c->matrix_stack[n]) {
    gl_set_error(c, GL_STACK_UNDERFLOW);
    return;
  }
  glopPopMatrix(c, NULL);
}

void glRotatex(GLfixed angle, GLfixed x, GLfixed y, GLfixed z)
{
  GLParam p[5];
  GLContext *c = gl_get_context();
  p[1].f = angle;
  p[2].f = x;
  p[3].f = y;
  p[4].f = z;
  glopRotate(c, p);
}

void glTranslatex(GLfixed x, GLfixed y, GLfixed z)
{
  GLParam p[4];
  GLContext *c = gl_get_context();
  p[1].f = x;
  p[2].f = y;
  p[3].f = z;
  glopTranslate(c, p);
}

void glScalex(GLfixed x, GLfixed y, GLfixed z)
{
  GLParam p[4];
  GLContext *c = gl_get_context();
  p[1].f = x;
  p[2].f = y;
  p[3].f = z;
  glopScale(c, p);
}

void glViewport(GLint x, GLint y, GLsizei width, GLsizei height)
{
  GLParam p[5];
  GLContext *c = gl_get_context();

  if (width <= 0 || height <= 0) {
    gl_set_error(c, GL_INVALID_VALUE);
    return;
  }
  p[1].i = x;
  p[2].i = y;
  p[3].i = width;
  p[4].i = height;
  glopViewport(c, p);
}

void glFrustumx(GLfixed left, GLfixed right, GLfixed bottom, GLfixed top,
                GLfixed near, GLfixed farv)
{
  GLParam p[7];
  GLContext *c = gl_get_context();

  if (near <= 0 || farv <= 0 || left == right || bottom == top || near == farv) {
    gl_set_error(c, GL_INVALID_VALUE);
    return;
  }
  p[1].f = left;
  p[2].f = right;
  p[3].f = bottom;
  p[4].f = top;
  p[5].f = near;
  p[6].f = farv;
  glopFrustum(c, p);
}

void glOrthox(GLfixed left, GLfixed right, GLfixed bottom, GLfixed top,
              GLfixed near, GLfixed farv)
{
  GLfixed m[16];

  if (left == right || bottom == top || near == farv) {
    gl_set_error(gl_get_context(), GL_INVALID_VALUE);
    return;
  }

  m[0] = tgl_fix_div(TGL_I(2), right - left);
  m[1] = 0;
  m[2] = 0;
  m[3] = 0;
  m[4] = 0;
  m[5] = tgl_fix_div(TGL_I(2), top - bottom);
  m[6] = 0;
  m[7] = 0;
  m[8] = 0;
  m[9] = 0;
  m[10] = -tgl_fix_div(TGL_I(2), farv - near);
  m[11] = 0;
  m[12] = -tgl_fix_div(right + left, right - left);
  m[13] = -tgl_fix_div(top + bottom, top - bottom);
  m[14] = -tgl_fix_div(farv + near, farv - near);
  m[15] = TGL_FIX_ONE;
  glMultMatrixx(m);
}

static void call_material(GLContext *c, GLenum face, GLenum pname, const GLfixed *params)
{
  GLParam p[7];
  int i, n = pname == GL_SHININESS ? 1 : 4;
  p[1].i = face;
  p[2].i = pname;
  for (i = 0; i < n; i++) p[3 + i].f = params[i];
  for (; i < 4; i++) p[3 + i].f = 0;
  glopMaterial(c, p);
}

void glMaterialxv(GLenum face, GLenum pname, const GLfixed *params)
{
  GLContext *c = gl_get_context();

  if (!valid_face(face)) {
    gl_set_error(c, GL_INVALID_ENUM);
    return;
  }
  if (!valid_material_pname(pname)) {
    gl_set_error(c, GL_INVALID_ENUM);
    return;
  }
  if (params == NULL) {
    gl_set_error(c, GL_INVALID_VALUE);
    return;
  }
  if (pname == GL_SHININESS && (params[0] < 0 || params[0] > TGL_I(128))) {
    gl_set_error(c, GL_INVALID_VALUE);
    return;
  }
  call_material(c, face, pname, params);
}

void glMaterialx(GLenum face, GLenum pname, GLfixed param)
{
  glMaterialxv(face, pname, &param);
}

void glColorMaterial(GLenum face, GLenum pname)
{
  GLParam p[3];
  GLContext *c = gl_get_context();

  if (!valid_face(face) || !valid_material_pname(pname) || pname == GL_SHININESS) {
    gl_set_error(c, GL_INVALID_ENUM);
    return;
  }
  p[1].i = face;
  p[2].i = pname;
  glopColorMaterial(c, p);
}

static void call_light(GLContext *c, GLenum light, GLenum pname, const GLfixed *params)
{
  GLParam p[7];
  int i;
  p[1].i = light;
  p[2].i = pname;
  for (i = 0; i < 4; i++) p[3 + i].f = params[i];
  glopLight(c, p);
}

void glLightxv(GLenum light, GLenum pname, const GLfixed *params)
{
  GLContext *c = gl_get_context();

  if (!valid_light(light)) {
    gl_set_error(c, GL_INVALID_ENUM);
    return;
  }
  if (!valid_light_pname(pname)) {
    gl_set_error(c, GL_INVALID_ENUM);
    return;
  }
  if (params == NULL) {
    gl_set_error(c, GL_INVALID_VALUE);
    return;
  }
  if (pname == GL_SPOT_CUTOFF &&
      !(params[0] == TGL_I(180) || (params[0] >= 0 && params[0] <= TGL_I(90)))) {
    gl_set_error(c, GL_INVALID_VALUE);
    return;
  }
  if (pname == GL_SPOT_EXPONENT && (params[0] < 0 || params[0] > TGL_I(128))) {
    gl_set_error(c, GL_INVALID_VALUE);
    return;
  }
  if ((pname == GL_CONSTANT_ATTENUATION ||
       pname == GL_LINEAR_ATTENUATION ||
       pname == GL_QUADRATIC_ATTENUATION) && params[0] < 0) {
    gl_set_error(c, GL_INVALID_VALUE);
    return;
  }
  call_light(c, light, pname, params);
}

void glLightx(GLenum light, GLenum pname, GLfixed param)
{
  GLfixed p[4] = {param, 0, 0, 0};
  glLightxv(light, pname, p);
}

void glLightModelxv(GLenum pname, const GLfixed *params)
{
  GLParam p[6];
  GLContext *c = gl_get_context();
  int i;

  if (pname != GL_LIGHT_MODEL_AMBIENT &&
      pname != GL_LIGHT_MODEL_TWO_SIDE &&
      pname != GL_LIGHT_MODEL_LOCAL_VIEWER) {
    gl_set_error(c, GL_INVALID_ENUM);
    return;
  }
  if (params == NULL) {
    gl_set_error(c, GL_INVALID_VALUE);
    return;
  }
  p[1].i = pname;
  for (i = 0; i < 4; i++) p[2 + i].f = (pname == GL_LIGHT_MODEL_AMBIENT) ? params[i] : params[0];
  glopLightModel(c, p);
}

void glLightModelx(GLenum pname, GLfixed param)
{
  glLightModelxv(pname, &param);
}

void glClear(GLbitfield mask)
{
  GLParam p[2];
  GLContext *c = gl_get_context();

  if ((mask & ~(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT)) != 0) {
    gl_set_error(c, GL_INVALID_VALUE);
    return;
  }
  p[1].i = (int)mask;
  glopClear(c, p);
}

void glClearColorx(GLfixed r, GLfixed g, GLfixed b, GLfixed a)
{
  GLParam p[5];
  GLContext *c = gl_get_context();
  p[1].f = r;
  p[2].f = g;
  p[3].f = b;
  p[4].f = a;
  glopClearColor(c, p);
}

void glClearDepthx(GLfixed depth)
{
  GLParam p[2];
  GLContext *c = gl_get_context();
  p[1].f = depth;
  glopClearDepth(c, p);
}

void glBindTexture(GLenum target, GLuint texture)
{
  GLParam p[3];
  GLContext *c = gl_get_context();
  p[1].i = target;
  p[2].i = (int)texture;
  glopBindTexture(c, p);
}

void glTexImage2D(GLenum target, GLint level, GLint internalformat,
                  GLsizei width, GLsizei height, GLint border,
                  GLenum format, GLenum type, const GLvoid *pixels)
{
  GLParam p[10];
  GLContext *c = gl_get_context();
  p[1].i = target;
  p[2].i = level;
  p[3].i = internalformat;
  p[4].i = width;
  p[5].i = height;
  p[6].i = border;
  p[7].i = format;
  p[8].i = type;
  p[9].p = (void *)pixels;
  glopTexImage2D(c, p);
}

void glTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset,
                     GLsizei width, GLsizei height, GLenum format, GLenum type,
                     const GLvoid *pixels)
{
  GLParam p[10];
  GLContext *c = gl_get_context();
  p[1].i = target;
  p[2].i = level;
  p[3].i = xoffset;
  p[4].i = yoffset;
  p[5].i = width;
  p[6].i = height;
  p[7].i = format;
  p[8].i = type;
  p[9].p = (void *)pixels;
  glopTexSubImage2D(c, p);
}

static void call_tex_env(GLenum target, GLenum pname, GLint param)
{
  GLParam p[8];
  GLContext *c = gl_get_context();
  p[1].i = target;
  p[2].i = pname;
  p[3].i = param;
  p[4].f = p[5].f = p[6].f = p[7].f = 0;
  glopTexEnv(c, p);
}

void glTexEnvi(GLenum target, GLenum pname, GLint param)
{
  call_tex_env(target, pname, param);
}

void glTexEnviv(GLenum target, GLenum pname, const GLint *params)
{
  GLContext *c = gl_get_context();
  if (params == NULL) {
    gl_set_error(c, GL_INVALID_VALUE);
    return;
  }
  call_tex_env(target, pname, params[0]);
}

void glTexEnvx(GLenum target, GLenum pname, GLfixed param)
{
  call_tex_env(target, pname, (GLint)param);
}

void glTexEnvxv(GLenum target, GLenum pname, const GLfixed *params)
{
  GLContext *c = gl_get_context();
  if (params == NULL) {
    gl_set_error(c, GL_INVALID_VALUE);
    return;
  }
  call_tex_env(target, pname, (GLint)params[0]);
}

static void call_tex_parameter(GLenum target, GLenum pname, GLint param)
{
  GLParam p[8];
  GLContext *c = gl_get_context();
  p[1].i = target;
  p[2].i = pname;
  p[3].i = param;
  p[4].f = p[5].f = p[6].f = p[7].f = 0;
  glopTexParameter(c, p);
}

void glTexParameteri(GLenum target, GLenum pname, GLint param)
{
  call_tex_parameter(target, pname, param);
}

void glTexParameteriv(GLenum target, GLenum pname, const GLint *params)
{
  GLContext *c = gl_get_context();
  if (params == NULL) {
    gl_set_error(c, GL_INVALID_VALUE);
    return;
  }
  call_tex_parameter(target, pname, params[0]);
}

void glTexParameterx(GLenum target, GLenum pname, GLfixed param)
{
  call_tex_parameter(target, pname, (GLint)param);
}

void glTexParameterxv(GLenum target, GLenum pname, const GLfixed *params)
{
  GLContext *c = gl_get_context();
  if (params == NULL) {
    gl_set_error(c, GL_INVALID_VALUE);
    return;
  }
  call_tex_parameter(target, pname, (GLint)params[0]);
}

void glPixelStorei(GLenum pname, GLint param)
{
  GLParam p[3];
  GLContext *c = gl_get_context();
  p[1].i = pname;
  p[2].i = param;
  glopPixelStore(c, p);
}

void glPolygonOffsetx(GLfixed factor, GLfixed units)
{
  GLParam p[3];
  GLContext *c = gl_get_context();
  p[1].f = factor;
  p[2].f = units;
  glopPolygonOffset(c, p);
}

void glFlush(void)
{
}

void glFinish(void)
{
}

void glHint(GLenum target, GLenum mode)
{
  GLContext *c = gl_get_context();

  switch (target) {
  case GL_PERSPECTIVE_CORRECTION_HINT:
  case GL_POINT_SMOOTH_HINT:
  case GL_LINE_SMOOTH_HINT:
  case GL_FOG_HINT:
  case GL_GENERATE_MIPMAP_HINT:
    break;
  default:
    gl_set_error(c, GL_INVALID_ENUM);
    return;
  }

  if (mode != GL_FASTEST && mode != GL_NICEST && mode != GL_DONT_CARE) {
    gl_set_error(c, GL_INVALID_ENUM);
  }
}
