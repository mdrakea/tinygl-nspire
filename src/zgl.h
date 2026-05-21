#ifndef _tgl_zgl_h_
#define _tgl_zgl_h_

#include <stdlib.h>
#include <stdio.h>
#include <GL/math.h>
#include <assert.h>
#include <string.h>
#include <GL/gl.h>
#include "zbuffer.h"
#include "zmath.h"
#include "zfeatures.h"

/* #define DEBUG  */
/* #define NDEBUG */

/* initially # of allocated GLVertexes (will grow when necessary) */
#define POLYGON_MAX_VERTEX 16

/* Max # of specular light pow buffers */
#define MAX_SPECULAR_BUFFERS 8
/* # of entries in specular buffer */
#define SPECULAR_BUFFER_SIZE 1024
/* specular buffer granularity */
#define SPECULAR_BUFFER_RESOLUTION 1024


#define MAX_MODELVIEW_STACK_DEPTH  32
#define MAX_PROJECTION_STACK_DEPTH 8
#define MAX_TEXTURE_STACK_DEPTH    8
#define MAX_TEXTURE_LEVELS         11
#define MAX_LIGHTS                 16

#define TGL_OFFSET_FILL    0x1


typedef struct GLSpecBuf {
  int shininess_i;
  int last_used;
  GLfixed buf[SPECULAR_BUFFER_SIZE+1];
  struct GLSpecBuf *next;
} GLSpecBuf;

typedef struct GLLight {
  V4 ambient;
  V4 diffuse;
  V4 specular;
  V4 position;	
  V3 spot_direction;
  GLfixed spot_exponent;
  GLfixed spot_cutoff;
  GLfixed attenuation[3];
  /* precomputed values */
  GLfixed cos_spot_cutoff;
  V3 norm_spot_direction;
  V3 norm_position;
  /* we use a linked list to know which are the enabled lights */
  int enabled;
  struct GLLight *next,*prev;
} GLLight;

typedef struct GLMaterial {
  V4 emission;
  V4 ambient;
  V4 diffuse;
  V4 specular;
  GLfixed shininess;

  /* computed values */
  int shininess_i;
} GLMaterial;


typedef struct GLViewport {
  int xmin,ymin,xsize,ysize;
  V3 scale;
  V3 trans;
  int updated;
} GLViewport;

typedef union {
  GLfixed f;
  int i;
  unsigned int ui;
  void *p;
} GLParam;

typedef struct GLVertex {
  V3 normal;
  V4 coord;
  V4 tex_coord;
  V4 color;
  
  /* computed values */
  V4 ec;                /* eye coordinates */
  V4 pc;                /* coordinates in the normalized volume */
  int clip_code;        /* clip code */
  ZBufferPoint zp;      /* integer coordinates for the rasterization */
} GLVertex;

typedef struct GLImage {
  void *pixmap;
  unsigned char *rgb;
  int xsize,ysize;
} GLImage;

/* textures */

#define TEXTURE_HASH_TABLE_SIZE 256

typedef struct GLTexture {
  GLImage images[MAX_TEXTURE_LEVELS];
  int handle;
  int wrap_s,wrap_t;
  int min_filter,mag_filter;
  int env_mode;
  struct GLTexture *next,*prev;
} GLTexture;

typedef struct GLBuffer {
  unsigned int handle;
  unsigned char *data;
  GLsizeiptr size;
  GLenum usage;
  struct GLBuffer *next;
} GLBuffer;

typedef struct GLArray {
  int enabled;
  int size;
  GLenum type;
  GLsizei stride;
  const void *pointer;
  GLBuffer *buffer;
} GLArray;


/* shared state */

typedef struct GLSharedState {
  GLTexture **texture_hash_table;
} GLSharedState;

/* display context */

typedef struct GLContext {
  /* Z buffer */
  ZBuffer *zb;

  /* lights */
  GLLight lights[MAX_LIGHTS];
  GLLight *first_light;
  V4 ambient_light_model;
  int local_light_model;
  int lighting_enabled;
  int light_model_two_side;

  /* materials */
  GLMaterial materials[2];
  int color_material_enabled;
  int current_color_material_mode;
  int current_color_material_type;

  /* textures */
  GLTexture *current_texture;
  int texture_2d_enabled;
  int unpack_alignment;

  /* shared state */
  GLSharedState shared_state;

  /* matrix */

  int matrix_mode;
  M4 *matrix_stack[3];
  M4 *matrix_stack_ptr[3];
  int matrix_stack_depth_max[3];

  M4 matrix_model_view_inv;
  M4 matrix_model_projection;
  int matrix_model_projection_updated;
  int matrix_model_projection_no_w_transform; 
  int apply_texture_matrix;

  /* viewport */
  GLViewport viewport;

  /* current state */
  int current_front_face;
  int current_shade_model;
  int current_cull_face;
  int cull_face_enabled;
  int normalize_enabled;

  GLenum error;

  /* clear */
  GLfixed clear_depth;
  V4 clear_color;

  /* current vertex state */
  V4 current_color;
  unsigned int longcurrent_color[3]; /* precomputed integer color */
  V4 current_normal;
  V4 current_tex_coord;

  /* Internal primitive assembly fed by glDrawArrays/glDrawElements. */
  int in_begin;
  int begin_type;
  int vertex_n,vertex_cnt;
  int vertex_max;
  GLVertex *vertex;

  GLArray vertex_array;
  GLArray normal_array;
  GLArray color_array;
  GLArray texcoord_array;
  GLBuffer *buffers;
  GLBuffer *array_buffer_binding;
  GLBuffer *element_array_buffer_binding;
  unsigned int next_buffer_handle;
  
  /* opengl 1.1 polygon offset */
  GLfixed offset_factor;
  GLfixed offset_units;
  int offset_states;
  
  /* specular buffer. could probably be shared between contexts, 
    but that wouldn't be 100% thread safe */
  GLSpecBuf *specbuf_first;
  int specbuf_used_counter;
  int specbuf_num_buffers;

  /* opaque structure for user's use */
  void *opaque;
  /* resize viewport function */
  int (*gl_resize_viewport)(struct GLContext *c,int *xsize,int *ysize);

  /* depth test */
  int depth_test;
} GLContext;

extern GLContext *gl_ctx;

void gl_set_error(GLContext *c, GLenum error);
void gl_submit_vertex(GLContext *c, GLfixed x, GLfixed y, GLfixed z, GLfixed w);
int gl_begin_primitive(GLContext *c, GLenum mode);
void gl_end_primitive(GLContext *c);

/* clip.c */
void gl_transform_to_viewport(GLContext *c,GLVertex *v);
void gl_draw_triangle(GLContext *c,GLVertex *p0,GLVertex *p1,GLVertex *p2);
void gl_draw_line(GLContext *c,GLVertex *p0,GLVertex *p1);
void gl_draw_point(GLContext *c,GLVertex *p0);

void gl_draw_triangle_fill(GLContext *c,
                           GLVertex *p0,GLVertex *p1,GLVertex *p2);

/* matrix.c */
void gl_print_matrix(const GLfixed *m);
/*
void glopLoadIdentity(GLContext *c,GLParam *p);
void glopTranslate(GLContext *c,GLParam *p);*/

/* light.c */
void gl_enable_disable_light(GLContext *c,int light,int v);
void gl_shade_vertex(GLContext *c,GLVertex *v);

void glInitTextures(GLContext *c);
GLTexture *alloc_texture(GLContext *c,int h);

/* image_util.c */
void gl_convertRGB_to_5R6G5B(unsigned short *pixmap,unsigned char *rgb,
                             int xsize,int ysize);
void gl_convertRGB_to_8A8R8G8B(unsigned int *pixmap, unsigned char *rgb,
                               int xsize, int ysize);
void gl_resizeImage(unsigned char *dest,int xsize_dest,int ysize_dest,
                    unsigned char *src,int xsize_src,int ysize_src);
void gl_resizeImageNoInterpolate(unsigned char *dest,int xsize_dest,int ysize_dest,
                                 unsigned char *src,int xsize_src,int ysize_src);

GLContext *gl_get_context(void);

void gl_fatal_error(char *format, ...);


/* specular buffer "api" */
GLSpecBuf *specbuf_get_buffer(GLContext *c, const int shininess_i, 
                              const GLfixed shininess);

#ifdef __BEOS__
void dprintf(const char *, ...);

#else /* !BEOS */

#ifdef DEBUG

#define dprintf(format, args...)  \
  fiprintf(stderr,"In '%s': " format "\n",__FUNCTION__, ##args);

#else

#define dprintf(format, args...)

#endif
#endif /* !BEOS */

/* Internal state mutators shared by the GLES entry points. */
void glopColor(GLContext *c, GLParam *p);
void glopTexCoord(GLContext *c, GLParam *p);
void glopNormal(GLContext *c, GLParam *p);
void glopBegin(GLContext *c, GLParam *p);
void glopVertex(GLContext *c, GLParam *p);
void glopEnd(GLContext *c, GLParam *p);
void glopEnableDisable(GLContext *c, GLParam *p);
void glopMatrixMode(GLContext *c, GLParam *p);
void glopLoadMatrix(GLContext *c, GLParam *p);
void glopLoadIdentity(GLContext *c, GLParam *p);
void glopMultMatrix(GLContext *c, GLParam *p);
void glopPushMatrix(GLContext *c, GLParam *p);
void glopPopMatrix(GLContext *c, GLParam *p);
void glopRotate(GLContext *c, GLParam *p);
void glopTranslate(GLContext *c, GLParam *p);
void glopScale(GLContext *c, GLParam *p);
void glopViewport(GLContext *c, GLParam *p);
void glopFrustum(GLContext *c, GLParam *p);
void glopMaterial(GLContext *c, GLParam *p);
void glopColorMaterial(GLContext *c, GLParam *p);
void glopLight(GLContext *c, GLParam *p);
void glopLightModel(GLContext *c, GLParam *p);
void glopClear(GLContext *c, GLParam *p);
void glopClearColor(GLContext *c, GLParam *p);
void glopClearDepth(GLContext *c, GLParam *p);
void glopTexImage2D(GLContext *c, GLParam *p);
void glopTexSubImage2D(GLContext *c, GLParam *p);
void glopBindTexture(GLContext *c, GLParam *p);
void glopTexEnv(GLContext *c, GLParam *p);
void glopTexParameter(GLContext *c, GLParam *p);
void glopPixelStore(GLContext *c, GLParam *p);
void glopShadeModel(GLContext *c, GLParam *p);
void glopCullFace(GLContext *c, GLParam *p);
void glopFrontFace(GLContext *c, GLParam *p);
void glopPolygonOffset(GLContext *c, GLParam *p);

/* this clip epsilon is needed to avoid some rounding errors after
   several clipping stages */

#define CLIP_EPSILON TGL_FRAC(1, 100000)

static inline int gl_clipcode(GLfixed x,GLfixed y,GLfixed z,GLfixed w1)
{
  GLfixed w;

  w=tgl_fix_mul(w1, TGL_FIX_ONE + CLIP_EPSILON);
  return (x<-w) |
    ((x>w)<<1) |
    ((y<-w)<<2) |
    ((y>w)<<3) |
    ((z<-w)<<4) | 
    ((z>w)<<5) ;
}

#endif /* _tgl_zgl_h_ */
