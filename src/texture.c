/*
 * Texture Manager
 */

#include "zgl.h"

static GLTexture *find_texture(GLContext *c,int h)
{
  GLTexture *t;

  t=c->shared_state.texture_hash_table[h % TEXTURE_HASH_TABLE_SIZE];
  while (t!=NULL) {
    if (t->handle == h) return t;
    t=t->next;
  }
  return NULL;
}

static void free_texture(GLContext *c,int h)
{
  GLTexture *t,**ht;
  GLImage *im;
  int i;

  t=find_texture(c,h);
  if (t->prev==NULL) {
    ht=&c->shared_state.texture_hash_table
      [t->handle % TEXTURE_HASH_TABLE_SIZE];
    *ht=t->next;
  } else {
    t->prev->next=t->next;
  }
  if (t->next!=NULL) t->next->prev=t->prev;

  for(i=0;i<MAX_TEXTURE_LEVELS;i++) {
    im=&t->images[i];
    if (im->pixmap != NULL) gl_free(im->pixmap);
    if (im->rgb != NULL) gl_free(im->rgb);
  }

  gl_free(t);
}

GLTexture *alloc_texture(GLContext *c,int h)
{
  GLTexture *t,**ht;
  
  t=gl_zalloc(sizeof(GLTexture));

  ht=&c->shared_state.texture_hash_table[h % TEXTURE_HASH_TABLE_SIZE];

  t->next=*ht;
  t->prev=NULL;
  if (t->next != NULL) t->next->prev=t;
  *ht=t;

  t->handle=h;
  t->wrap_s=GL_REPEAT;
  t->wrap_t=GL_REPEAT;
  t->min_filter=GL_NEAREST;
  t->mag_filter=GL_NEAREST;
  t->env_mode=GL_DECAL;
  
  return t;
}


void glInitTextures(GLContext *c)
{
  /* textures */

  c->texture_2d_enabled=0;
  c->current_texture=find_texture(c,0);
}

void glGenTextures(int n, unsigned int *textures)
{
  GLContext *c=gl_get_context();
  int max,i;
  GLTexture *t;

  if (n < 0) {
    gl_set_error(c, GL_INVALID_VALUE);
    return;
  }
  if (textures == NULL) {
    gl_set_error(c, GL_INVALID_VALUE);
    return;
  }

  max=0;
  for(i=0;i<TEXTURE_HASH_TABLE_SIZE;i++) {
    t=c->shared_state.texture_hash_table[i];
    while (t!=NULL) {
      if (t->handle>max) max=t->handle;
      t=t->next;
    }

  }
  for(i=0;i<n;i++) {
    textures[i]=max+i+1;
  }
}


void glDeleteTextures(int n, const unsigned int *textures)
{
  GLContext *c=gl_get_context();
  int i;
  GLTexture *t;

  if (n < 0) {
    gl_set_error(c, GL_INVALID_VALUE);
    return;
  }
  if (textures == NULL) return;

  for(i=0;i<n;i++) {
    t=find_texture(c,textures[i]);
    if (t!=NULL && textures[i] != 0) {
      if (t==c->current_texture) {
	glBindTexture(GL_TEXTURE_2D,0);
      }
      free_texture(c,textures[i]);
    }
  }
}


void glopBindTexture(GLContext *c,GLParam *p)
{
  int target=p[1].i;
  int texture=p[2].i;
  GLTexture *t;

  if (target != GL_TEXTURE_2D) {
    gl_set_error(c, GL_INVALID_ENUM);
    return;
  }
  if (texture < 0) {
    gl_set_error(c, GL_INVALID_VALUE);
    return;
  }

  t=find_texture(c,texture);
  if (t==NULL) {
    t=alloc_texture(c,texture);
  }
  c->current_texture=t;
}

static int pixel_type_size(GLenum format, GLenum type)
{
  if (type == GL_UNSIGNED_BYTE) {
    switch (format) {
    case GL_ALPHA: return 1;
    case GL_LUMINANCE: return 1;
    case GL_LUMINANCE_ALPHA: return 2;
    case GL_RGB: return 3;
    case GL_RGBA: return 4;
    default: return 0;
    }
  }
  if (type == GL_UNSIGNED_SHORT_5_6_5 && format == GL_RGB) return 2;
  if ((type == GL_UNSIGNED_SHORT_4_4_4_4 ||
       type == GL_UNSIGNED_SHORT_5_5_5_1) && format == GL_RGBA) return 2;
  return 0;
}

static int aligned_row_bytes(int bytes, int alignment)
{
  int rem;

  if (alignment <= 1) return bytes;
  rem = bytes % alignment;
  return rem == 0 ? bytes : bytes + alignment - rem;
}

static unsigned char expand_bits(unsigned int v, int bits)
{
  unsigned int max = (1u << bits) - 1u;
  return (unsigned char)((v * 255u + max / 2u) / max);
}

static void decode_pixel(unsigned char *dst, const unsigned char *src,
                         GLenum format, GLenum type)
{
  if (type == GL_UNSIGNED_BYTE) {
    switch (format) {
    case GL_ALPHA:
      dst[0] = dst[1] = dst[2] = src[0];
      break;
    case GL_LUMINANCE:
    case GL_LUMINANCE_ALPHA:
      dst[0] = dst[1] = dst[2] = src[0];
      break;
    case GL_RGB:
      dst[0] = src[0];
      dst[1] = src[1];
      dst[2] = src[2];
      break;
    case GL_RGBA:
      dst[0] = src[0];
      dst[1] = src[1];
      dst[2] = src[2];
      break;
    }
  } else {
    unsigned int p = (unsigned int)((const GLushort *)src)[0];
    if (type == GL_UNSIGNED_SHORT_5_6_5) {
      dst[0] = expand_bits((p >> 11) & 0x1f, 5);
      dst[1] = expand_bits((p >> 5) & 0x3f, 6);
      dst[2] = expand_bits(p & 0x1f, 5);
    } else if (type == GL_UNSIGNED_SHORT_4_4_4_4) {
      dst[0] = expand_bits((p >> 12) & 0x0f, 4);
      dst[1] = expand_bits((p >> 8) & 0x0f, 4);
      dst[2] = expand_bits((p >> 4) & 0x0f, 4);
    } else {
      dst[0] = expand_bits((p >> 11) & 0x1f, 5);
      dst[1] = expand_bits((p >> 6) & 0x1f, 5);
      dst[2] = expand_bits((p >> 1) & 0x1f, 5);
    }
  }
}

static int convert_pixels_to_rgb(GLContext *c, unsigned char *rgb, int width, int height,
                                 GLenum format, GLenum type, const void *pixels)
{
  int bpp = pixel_type_size(format, type);
  int row_bytes;
  int src_row_bytes;
  const unsigned char *src = (const unsigned char *)pixels;
  int x,y;

  if (bpp == 0) {
    gl_set_error(c, GL_INVALID_ENUM);
    return 0;
  }
  if (pixels == NULL) {
    memset(rgb, 0, (size_t)width * (size_t)height * 3);
    return 1;
  }

  row_bytes = width * bpp;
  src_row_bytes = aligned_row_bytes(row_bytes, c->unpack_alignment);
  for (y = 0; y < height; y++) {
    const unsigned char *row = src + (size_t)y * (size_t)src_row_bytes;
    for (x = 0; x < width; x++) {
      decode_pixel(rgb + ((size_t)y * width + x) * 3,
                   row + (size_t)x * bpp,
                   format, type);
    }
  }
  return 1;
}

static int rebuild_pixmap(GLContext *c, GLImage *im)
{
  if (im->pixmap != NULL) {
    gl_free(im->pixmap);
    im->pixmap = NULL;
  }

#if TGL_FEATURE_RENDER_BITS == 24
  im->pixmap = gl_malloc(im->xsize * im->ysize * 3);
  if (im->pixmap != NULL) memcpy(im->pixmap, im->rgb, im->xsize * im->ysize * 3);
#elif TGL_FEATURE_RENDER_BITS == 32
  im->pixmap = gl_malloc(im->xsize * im->ysize * 4);
  if (im->pixmap != NULL) gl_convertRGB_to_8A8R8G8B(im->pixmap, im->rgb, im->xsize, im->ysize);
#elif TGL_FEATURE_RENDER_BITS == 16
  im->pixmap = gl_malloc(im->xsize * im->ysize * 2);
  if (im->pixmap != NULL) gl_convertRGB_to_5R6G5B(im->pixmap, im->rgb, im->xsize, im->ysize);
#else
#error TODO
#endif
  if (im->pixmap == NULL) {
    gl_set_error(c, GL_OUT_OF_MEMORY);
    return 0;
  }
  return 1;
}

static int upload_rgb_image(GLContext *c, int level, int width, int height,
                            unsigned char *src_rgb)
{
  GLImage *im = &c->current_texture->images[level];
  unsigned char *rgb;
  int dst_width = width;
  int dst_height = height;

  if (dst_width != 256 || dst_height != 256) {
    rgb = gl_malloc(256 * 256 * 3);
    if (rgb == NULL) {
      gl_set_error(c, GL_OUT_OF_MEMORY);
      return 0;
    }
    gl_resizeImageNoInterpolate(rgb, 256, 256, src_rgb, width, height);
    dst_width = 256;
    dst_height = 256;
  } else {
    rgb = gl_malloc(width * height * 3);
    if (rgb == NULL) {
      gl_set_error(c, GL_OUT_OF_MEMORY);
      return 0;
    }
    memcpy(rgb, src_rgb, (size_t)width * (size_t)height * 3);
  }

  if (im->rgb != NULL) gl_free(im->rgb);
  im->rgb = rgb;
  im->xsize = dst_width;
  im->ysize = dst_height;
  return rebuild_pixmap(c, im);
}

void glopTexImage2D(GLContext *c,GLParam *p)
{
  int target=p[1].i;
  int level=p[2].i;
  int internalformat=p[3].i;
  int width=p[4].i;
  int height=p[5].i;
  int border=p[6].i;
  int format=p[7].i;
  int type=p[8].i;
  void *pixels=p[9].p;
  unsigned char *rgb;

  if (target != GL_TEXTURE_2D) {
    gl_set_error(c, GL_INVALID_ENUM);
    return;
  }
  if (level < 0 || level >= MAX_TEXTURE_LEVELS || width <= 0 || height <= 0 || border != 0) {
    gl_set_error(c, GL_INVALID_VALUE);
    return;
  }
  if (internalformat != format || pixel_type_size(format, type) == 0) {
    gl_set_error(c, GL_INVALID_ENUM);
    return;
  }

  rgb = gl_malloc(width * height * 3);
  if (rgb == NULL) {
    gl_set_error(c, GL_OUT_OF_MEMORY);
    return;
  }
  if (convert_pixels_to_rgb(c, rgb, width, height, format, type, pixels)) {
    upload_rgb_image(c, level, width, height, rgb);
  }
  gl_free(rgb);
}

void glopTexSubImage2D(GLContext *c, GLParam *p)
{
  int target=p[1].i;
  int level=p[2].i;
  int xoffset=p[3].i;
  int yoffset=p[4].i;
  int width=p[5].i;
  int height=p[6].i;
  int format=p[7].i;
  int type=p[8].i;
  void *pixels=p[9].p;
  GLImage *im;
  unsigned char *rgb;
  int y;

  if (target != GL_TEXTURE_2D) {
    gl_set_error(c, GL_INVALID_ENUM);
    return;
  }
  if (level < 0 || level >= MAX_TEXTURE_LEVELS || xoffset < 0 || yoffset < 0 ||
      width < 0 || height < 0) {
    gl_set_error(c, GL_INVALID_VALUE);
    return;
  }
  if (pixel_type_size(format, type) == 0) {
    gl_set_error(c, GL_INVALID_ENUM);
    return;
  }
  im = &c->current_texture->images[level];
  if (im->rgb == NULL) {
    gl_set_error(c, GL_INVALID_OPERATION);
    return;
  }
  if (xoffset + width > im->xsize || yoffset + height > im->ysize) {
    gl_set_error(c, GL_INVALID_VALUE);
    return;
  }
  if (width == 0 || height == 0) return;
  if (pixels == NULL) {
    gl_set_error(c, GL_INVALID_VALUE);
    return;
  }

  rgb = gl_malloc(width * height * 3);
  if (rgb == NULL) {
    gl_set_error(c, GL_OUT_OF_MEMORY);
    return;
  }
  if (!convert_pixels_to_rgb(c, rgb, width, height, format, type, pixels)) {
    gl_free(rgb);
    return;
  }
  for (y = 0; y < height; y++) {
    memcpy(im->rgb + (((size_t)yoffset + y) * im->xsize + xoffset) * 3,
           rgb + (size_t)y * width * 3,
           (size_t)width * 3);
  }
  gl_free(rgb);
  rebuild_pixmap(c, im);
}


/* TODO: not all tests are done */
void glopTexEnv(GLContext *c,GLParam *p)
{
  int target=p[1].i;
  int pname=p[2].i;
  int param=p[3].i;

  if (target != GL_TEXTURE_ENV) {
    gl_set_error(c, GL_INVALID_ENUM);
    return;
  }

  if (pname != GL_TEXTURE_ENV_MODE) {
    gl_set_error(c, GL_INVALID_ENUM);
    return;
  }

  if (param != GL_DECAL && param != GL_MODULATE && param != GL_ADD) {
    gl_set_error(c, GL_INVALID_ENUM);
    return;
  }
  c->current_texture->env_mode = param;
}

/* TODO: not all tests are done */
void glopTexParameter(GLContext *c,GLParam *p)
{
  int target=p[1].i;
  int pname=p[2].i;
  int param=p[3].i;
  
  if (target != GL_TEXTURE_2D) {
    gl_set_error(c, GL_INVALID_ENUM);
    return;
  }

  switch(pname) {
  case GL_TEXTURE_WRAP_S:
    if (param != GL_REPEAT && param != GL_CLAMP_TO_EDGE) {
      gl_set_error(c, GL_INVALID_ENUM);
      return;
    }
    c->current_texture->wrap_s = param;
    break;
  case GL_TEXTURE_WRAP_T:
    if (param != GL_REPEAT && param != GL_CLAMP_TO_EDGE) {
      gl_set_error(c, GL_INVALID_ENUM);
      return;
    }
    c->current_texture->wrap_t = param;
    break;
  case GL_TEXTURE_MIN_FILTER:
    if (param != GL_NEAREST && param != GL_LINEAR &&
        param != GL_NEAREST_MIPMAP_NEAREST &&
        param != GL_NEAREST_MIPMAP_LINEAR &&
        param != GL_LINEAR_MIPMAP_NEAREST &&
        param != GL_LINEAR_MIPMAP_LINEAR) {
      gl_set_error(c, GL_INVALID_ENUM);
      return;
    }
    c->current_texture->min_filter = param;
    break;
  case GL_TEXTURE_MAG_FILTER:
    if (param != GL_NEAREST && param != GL_LINEAR) {
      gl_set_error(c, GL_INVALID_ENUM);
      return;
    }
    c->current_texture->mag_filter = param;
    break;
  default:
    gl_set_error(c, GL_INVALID_ENUM);
    return;
  }
}

void glopPixelStore(GLContext *c,GLParam *p)
{
  int pname=p[1].i;
  int param=p[2].i;

  if (pname != GL_UNPACK_ALIGNMENT) {
    gl_set_error(c, GL_INVALID_ENUM);
    return;
  }
  if (param != 1 && param != 2 && param != 4 && param != 8) {
    gl_set_error(c, GL_INVALID_VALUE);
    return;
  }
  c->unpack_alignment = param;
}

GLboolean glIsTexture(GLuint texture)
{
  GLContext *c = gl_get_context();
  if (texture == 0) return GL_FALSE;
  return find_texture(c, (int)texture) != NULL ? GL_TRUE : GL_FALSE;
}
