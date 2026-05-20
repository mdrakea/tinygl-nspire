#include "zgl.h"

GLContext *gl_ctx;


void initSharedState(GLContext *c)
{
  GLSharedState *s=&c->shared_state;
  s->texture_hash_table=
      gl_zalloc(sizeof(GLTexture *) * TEXTURE_HASH_TABLE_SIZE);

  alloc_texture(c,0);
}

void endSharedState(GLContext *c)
{
  GLSharedState *s=&c->shared_state;

  gl_free(s->texture_hash_table);
}

GLContext *gl_get_context(void)
{
  return gl_ctx;
}


void glInit(void *zbuffer1)
{
  ZBuffer *zbuffer=(ZBuffer *)zbuffer1;
  GLContext *c;
  GLViewport *v;
  int i;

  c=gl_zalloc(sizeof(GLContext));
  gl_ctx=c;

  c->zb=zbuffer;

  /* allocate GLVertex array */
  c->vertex_max = POLYGON_MAX_VERTEX;
  c->vertex = gl_malloc(POLYGON_MAX_VERTEX*sizeof(GLVertex));
  
  /* viewport */
  v=&c->viewport;
  v->xmin=0;
  v->ymin=0;
  v->xsize=zbuffer->xsize;
  v->ysize=zbuffer->ysize;
  v->updated=1;

  /* shared state */
  initSharedState(c);

  c->in_begin=0;

  /* lights */
  for(i=0;i<MAX_LIGHTS;i++) {
    GLLight *l=&c->lights[i];
    l->ambient=gl_V4_New(0,0,0,TGL_FIX_ONE);
    l->diffuse=gl_V4_New(TGL_FIX_ONE,TGL_FIX_ONE,TGL_FIX_ONE,TGL_FIX_ONE);
    l->specular=gl_V4_New(TGL_FIX_ONE,TGL_FIX_ONE,TGL_FIX_ONE,TGL_FIX_ONE);
    l->position=gl_V4_New(0,0,TGL_FIX_ONE,0);
    l->norm_position=gl_V3_New(0,0,TGL_FIX_ONE);
    l->spot_direction=gl_V3_New(0,0,-TGL_FIX_ONE);
    l->norm_spot_direction=gl_V3_New(0,0,-TGL_FIX_ONE);
    l->spot_exponent=0;
    l->spot_cutoff=TGL_I(180);
    l->attenuation[0]=TGL_FIX_ONE;
    l->attenuation[1]=0;
    l->attenuation[2]=0;
    l->enabled=0;
  }
  c->first_light=NULL;
  c->ambient_light_model=gl_V4_New(TGL_FRAC(1,5),TGL_FRAC(1,5),TGL_FRAC(1,5),TGL_FIX_ONE);
  c->local_light_model=0;
  c->lighting_enabled=0;
  c->light_model_two_side = 0;

  /* default materials */
  for(i=0;i<2;i++) {
    GLMaterial *m=&c->materials[i];
    m->emission=gl_V4_New(0,0,0,TGL_FIX_ONE);
    m->ambient=gl_V4_New(TGL_FRAC(1,5),TGL_FRAC(1,5),TGL_FRAC(1,5),TGL_FIX_ONE);
    m->diffuse=gl_V4_New(TGL_FRAC(4,5),TGL_FRAC(4,5),TGL_FRAC(4,5),TGL_FIX_ONE);
    m->specular=gl_V4_New(0,0,0,TGL_FIX_ONE);
    m->shininess=0;
  }
  c->current_color_material_mode=GL_FRONT_AND_BACK;
  c->current_color_material_type=GL_AMBIENT_AND_DIFFUSE;
  c->color_material_enabled=0;

  /* textures */
  glInitTextures(c);
  c->unpack_alignment = 4;

  /* default state */
  c->current_color.X=TGL_FIX_ONE;
  c->current_color.Y=TGL_FIX_ONE;
  c->current_color.Z=TGL_FIX_ONE;
  c->current_color.W=TGL_FIX_ONE;
  c->longcurrent_color[0] = 65535;
  c->longcurrent_color[1] = 65535;
  c->longcurrent_color[2] = 65535;

  c->current_normal.X=TGL_FIX_ONE;
  c->current_normal.Y=0;
  c->current_normal.Z=0;
  c->current_normal.W=0;

  c->current_tex_coord.X=0;
  c->current_tex_coord.Y=0;
  c->current_tex_coord.Z=0;
  c->current_tex_coord.W=TGL_FIX_ONE;

  c->current_front_face=0; /* 0 = GL_CCW  1 = GL_CW */
  c->current_cull_face=GL_BACK;
  c->current_shade_model=GL_SMOOTH;
  c->cull_face_enabled=0;
  
  /* clear */
  c->clear_color.v[0]=0;
  c->clear_color.v[1]=0;
  c->clear_color.v[2]=0;
  c->clear_color.v[3]=0;
  c->clear_depth=0;

  c->error=GL_NO_ERROR;

  /* matrix */
  c->matrix_mode=0;
  
  c->matrix_stack_depth_max[0]=MAX_MODELVIEW_STACK_DEPTH;
  c->matrix_stack_depth_max[1]=MAX_PROJECTION_STACK_DEPTH;
  c->matrix_stack_depth_max[2]=MAX_TEXTURE_STACK_DEPTH;

  for(i=0;i<3;i++) {
    c->matrix_stack[i]=gl_zalloc(c->matrix_stack_depth_max[i] * sizeof(M4));
    c->matrix_stack_ptr[i]=c->matrix_stack[i];
  }

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glMatrixMode(GL_TEXTURE);
  glLoadIdentity();
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  c->matrix_model_projection_updated=1;

  c->vertex_array.enabled = 0;
  c->vertex_array.size = 4;
  c->vertex_array.type = GL_FIXED;
  c->vertex_array.stride = 0;
  c->vertex_array.pointer = NULL;
  c->vertex_array.buffer = NULL;

  c->normal_array.enabled = 0;
  c->normal_array.size = 3;
  c->normal_array.type = GL_FIXED;
  c->normal_array.stride = 0;
  c->normal_array.pointer = NULL;
  c->normal_array.buffer = NULL;

  c->color_array.enabled = 0;
  c->color_array.size = 4;
  c->color_array.type = GL_FIXED;
  c->color_array.stride = 0;
  c->color_array.pointer = NULL;
  c->color_array.buffer = NULL;

  c->texcoord_array.enabled = 0;
  c->texcoord_array.size = 4;
  c->texcoord_array.type = GL_FIXED;
  c->texcoord_array.stride = 0;
  c->texcoord_array.pointer = NULL;
  c->texcoord_array.buffer = NULL;

  c->buffers = NULL;
  c->array_buffer_binding = NULL;
  c->element_array_buffer_binding = NULL;
  c->next_buffer_handle = 1;
  
  /* opengl 1.1 polygon offset */
  c->offset_states = 0;
  
  /* clear the resize callback function pointer */
  c->gl_resize_viewport = NULL;
  
  /* specular buffer */
  c->specbuf_first = NULL;
  c->specbuf_used_counter = 0;
  c->specbuf_num_buffers = 0;

  /* depth test */
  c->depth_test = 0;
}

void glClose(void)
{
  GLContext *c=gl_get_context();
  GLBuffer *b,*next;
  for (b = c->buffers; b != NULL; b = next) {
    next = b->next;
    if (b->data != NULL) gl_free(b->data);
    gl_free(b);
  }
  endSharedState(c);
  if (c->vertex != NULL) gl_free(c->vertex);
  gl_free(c);
  gl_ctx = NULL;
}
