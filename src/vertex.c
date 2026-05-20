#include "zgl.h"

void glopNormal(GLContext * c, GLParam * p)
{
    V3 v;

    v.X = p[1].f;
    v.Y = p[2].f;
    v.Z = p[3].f;

    c->current_normal.X = v.X;
    c->current_normal.Y = v.Y;
    c->current_normal.Z = v.Z;
    c->current_normal.W = 0;
}

void glopTexCoord(GLContext * c, GLParam * p)
{
    c->current_tex_coord.X = p[1].f;
    c->current_tex_coord.Y = p[2].f;
    c->current_tex_coord.Z = p[3].f;
    c->current_tex_coord.W = p[4].f;
}

void glopColor(GLContext * c, GLParam * p)
{

    c->current_color.X = p[1].f;
    c->current_color.Y = p[2].f;
    c->current_color.Z = p[3].f;
    c->current_color.W = p[4].f;
    c->longcurrent_color[0] = p[5].ui;
    c->longcurrent_color[1] = p[6].ui;
    c->longcurrent_color[2] = p[7].ui;

    if (c->color_material_enabled) {
	GLParam q[7];
	q[1].i = c->current_color_material_mode;
	q[2].i = c->current_color_material_type;
	q[3].f = p[1].f;
	q[4].f = p[2].f;
	q[5].f = p[3].f;
	q[6].f = p[4].f;
	glopMaterial(c, q);
    }
}


void gl_eval_viewport(GLContext * c)
{
    GLViewport *v;

    v = &c->viewport;

    v->trans.X = ((TGL_I(v->xsize) - TGL_FIX_HALF) / 2) + TGL_I(v->xmin);
    v->trans.Y = ((TGL_I(v->ysize) - TGL_FIX_HALF) / 2) + TGL_I(v->ymin);
    v->trans.Z = 0;

    v->scale.X = (TGL_I(v->xsize) - TGL_FIX_HALF) / 2;
    v->scale.Y = -((TGL_I(v->ysize) - TGL_FIX_HALF) / 2);
    v->scale.Z = 0;
}

int gl_begin_primitive(GLContext *c, GLenum mode)
{
    GLParam p[2];

    if (c->in_begin) {
        gl_set_error(c, GL_INVALID_OPERATION);
        return 0;
    }
    p[1].i = mode;
    glopBegin(c, p);
    return 1;
}

void gl_submit_vertex(GLContext *c, GLfixed x, GLfixed y, GLfixed z, GLfixed w)
{
    GLParam p[5];
    p[1].f = x;
    p[2].f = y;
    p[3].f = z;
    p[4].f = w;
    glopVertex(c, p);
}

void gl_end_primitive(GLContext *c)
{
    if (!c->in_begin) {
        gl_set_error(c, GL_INVALID_OPERATION);
        return;
    }
    glopEnd(c, NULL);
}

void glopBegin(GLContext * c, GLParam * p)
{
    int type;
    M4 tmp;

    assert(c->in_begin == 0);

    type = p[1].i;
    c->begin_type = type;
    c->in_begin = 1;
    c->vertex_n = 0;
    c->vertex_cnt = 0;

    if (c->matrix_model_projection_updated) {

	if (c->lighting_enabled) {
	    /* precompute inverse modelview */
	    gl_M4_Inv(&tmp, c->matrix_stack_ptr[0]);
	    gl_M4_Transpose(&c->matrix_model_view_inv, &tmp);
	} else {
	    GLfixed *m = &c->matrix_model_projection.m[0][0];
	    /* precompute projection matrix */
	    gl_M4_Mul(&c->matrix_model_projection,
		      c->matrix_stack_ptr[1],
		      c->matrix_stack_ptr[0]);
	    /* test to accelerate computation */
	    c->matrix_model_projection_no_w_transform = 0;
	    if (m[12] == 0 && m[13] == 0 && m[14] == 0)
		c->matrix_model_projection_no_w_transform = 1;
	}

	/* test if the texture matrix is not Identity */
	c->apply_texture_matrix = !gl_M4_IsId(c->matrix_stack_ptr[2]);

	c->matrix_model_projection_updated = 0;
    }
    /*  viewport */
    if (c->viewport.updated) {
	gl_eval_viewport(c);
	c->viewport.updated = 0;
    }
}

/* coords, tranformation , clip code and projection */
/* TODO : handle all cases */
static inline void gl_vertex_transform(GLContext * c, GLVertex * v)
{
    GLfixed *m;
    V4 *n;

    if (c->lighting_enabled) {
	/* eye coordinates needed for lighting */

	m = &c->matrix_stack_ptr[0]->m[0][0];
	v->ec.X = (tgl_fix_mul(v->coord.X, m[0]) + tgl_fix_mul(v->coord.Y, m[1]) +
		   tgl_fix_mul(v->coord.Z, m[2]) + m[3]);
	v->ec.Y = (tgl_fix_mul(v->coord.X, m[4]) + tgl_fix_mul(v->coord.Y, m[5]) +
		   tgl_fix_mul(v->coord.Z, m[6]) + m[7]);
	v->ec.Z = (tgl_fix_mul(v->coord.X, m[8]) + tgl_fix_mul(v->coord.Y, m[9]) +
		   tgl_fix_mul(v->coord.Z, m[10]) + m[11]);
	v->ec.W = (tgl_fix_mul(v->coord.X, m[12]) + tgl_fix_mul(v->coord.Y, m[13]) +
		   tgl_fix_mul(v->coord.Z, m[14]) + m[15]);

	/* projection coordinates */
	m = &c->matrix_stack_ptr[1]->m[0][0];
	v->pc.X = (tgl_fix_mul(v->ec.X, m[0]) + tgl_fix_mul(v->ec.Y, m[1]) +
		   tgl_fix_mul(v->ec.Z, m[2]) + tgl_fix_mul(v->ec.W, m[3]));
	v->pc.Y = (tgl_fix_mul(v->ec.X, m[4]) + tgl_fix_mul(v->ec.Y, m[5]) +
		   tgl_fix_mul(v->ec.Z, m[6]) + tgl_fix_mul(v->ec.W, m[7]));
	v->pc.Z = (tgl_fix_mul(v->ec.X, m[8]) + tgl_fix_mul(v->ec.Y, m[9]) +
		   tgl_fix_mul(v->ec.Z, m[10]) + tgl_fix_mul(v->ec.W, m[11]));
	v->pc.W = (tgl_fix_mul(v->ec.X, m[12]) + tgl_fix_mul(v->ec.Y, m[13]) +
		   tgl_fix_mul(v->ec.Z, m[14]) + tgl_fix_mul(v->ec.W, m[15]));

	m = &c->matrix_model_view_inv.m[0][0];
	n = &c->current_normal;

	v->normal.X = (tgl_fix_mul(n->X, m[0]) + tgl_fix_mul(n->Y, m[1]) + tgl_fix_mul(n->Z, m[2]));
	v->normal.Y = (tgl_fix_mul(n->X, m[4]) + tgl_fix_mul(n->Y, m[5]) + tgl_fix_mul(n->Z, m[6]));
	v->normal.Z = (tgl_fix_mul(n->X, m[8]) + tgl_fix_mul(n->Y, m[9]) + tgl_fix_mul(n->Z, m[10]));

	if (c->normalize_enabled) {
	    gl_V3_Norm(&v->normal);
	}
    } else {
	/* no eye coordinates needed, no normal */
	/* NOTE: W = 1 is assumed */
	m = &c->matrix_model_projection.m[0][0];

	v->pc.X = (tgl_fix_mul(v->coord.X, m[0]) + tgl_fix_mul(v->coord.Y, m[1]) +
		   tgl_fix_mul(v->coord.Z, m[2]) + m[3]);
	v->pc.Y = (tgl_fix_mul(v->coord.X, m[4]) + tgl_fix_mul(v->coord.Y, m[5]) +
		   tgl_fix_mul(v->coord.Z, m[6]) + m[7]);
	v->pc.Z = (tgl_fix_mul(v->coord.X, m[8]) + tgl_fix_mul(v->coord.Y, m[9]) +
		   tgl_fix_mul(v->coord.Z, m[10]) + m[11]);
	if (c->matrix_model_projection_no_w_transform) {
	    v->pc.W = m[15];
	} else {
	    v->pc.W = (tgl_fix_mul(v->coord.X, m[12]) + tgl_fix_mul(v->coord.Y, m[13]) +
		       tgl_fix_mul(v->coord.Z, m[14]) + m[15]);
	}
    }

    v->clip_code = gl_clipcode(v->pc.X, v->pc.Y, v->pc.Z, v->pc.W);
}

void glopVertex(GLContext * c, GLParam * p)
{
    GLVertex *v;
    int n, cnt;

    assert(c->in_begin != 0);

    n = c->vertex_n;
    cnt = c->vertex_cnt;
    cnt++;
    c->vertex_cnt = cnt;

    /* quick fix to avoid crashes on large polygons */
    if (n >= c->vertex_max) {
	GLVertex *newarray;
	c->vertex_max <<= 1;
	newarray = gl_malloc(sizeof(GLVertex) * c->vertex_max);
	if (!newarray) {
	    gl_fatal_error("unable to allocate GLVertex array.\n");
	}
	memcpy(newarray, c->vertex, n * sizeof(GLVertex));
	gl_free(c->vertex);
	c->vertex = newarray;
    }
    /* new vertex entry */
    v = &c->vertex[n];
    n++;

    v->coord.X = p[1].f;
    v->coord.Y = p[2].f;
    v->coord.Z = p[3].f;
    v->coord.W = p[4].f;

    gl_vertex_transform(c, v);

    /* color */

    if (c->lighting_enabled) {
	gl_shade_vertex(c, v);
    } else {
	v->color = c->current_color;
    }

    /* tex coords */

    if (c->texture_2d_enabled) {
	if (c->apply_texture_matrix) {
	    gl_M4_MulV4(&v->tex_coord, c->matrix_stack_ptr[2], &c->current_tex_coord);
	} else {
	    v->tex_coord = c->current_tex_coord;
	}
    }
    /* precompute the mapping to the viewport */
    if (v->clip_code == 0)
	gl_transform_to_viewport(c, v);

    switch (c->begin_type) {
    case GL_POINTS:
	gl_draw_point(c, &c->vertex[0]);
	n = 0;
	break;

    case GL_LINES:
	if (n == 2) {
	    gl_draw_line(c, &c->vertex[0], &c->vertex[1]);
	    n = 0;
	}
	break;
    case GL_LINE_STRIP:
    case GL_LINE_LOOP:
	if (n == 1) {
	    c->vertex[2] = c->vertex[0];
	} else if (n == 2) {
	    gl_draw_line(c, &c->vertex[0], &c->vertex[1]);
	    c->vertex[0] = c->vertex[1];
	    n = 1;
	}
	break;

    case GL_TRIANGLES:
	if (n == 3) {
	    gl_draw_triangle(c, &c->vertex[0], &c->vertex[1], &c->vertex[2]);
	    n = 0;
	}
	break;
    case GL_TRIANGLE_STRIP:
	if (cnt >= 3) {
	    if (n == 3)
		n = 0;
            /* needed to respect triangle orientation */
            switch(cnt & 1) {
            case 0:
      		gl_draw_triangle(c,&c->vertex[2],&c->vertex[1],&c->vertex[0]);
      		break;
            default:
            case 1:
      		gl_draw_triangle(c,&c->vertex[0],&c->vertex[1],&c->vertex[2]);
      		break;
            }
	}
	break;
    case GL_TRIANGLE_FAN:
	if (n == 3) {
	    gl_draw_triangle(c, &c->vertex[0], &c->vertex[1], &c->vertex[2]);
	    c->vertex[1] = c->vertex[2];
	    n = 2;
	}
	break;

    default:
	gl_set_error(c, GL_INVALID_ENUM);
	n = 0;
	break;
    }

    c->vertex_n = n;
}

void glopEnd(GLContext * c, GLParam * param)
{
    assert(c->in_begin == 1);

    if (c->begin_type == GL_LINE_LOOP) {
	if (c->vertex_cnt >= 3) {
	    gl_draw_line(c, &c->vertex[0], &c->vertex[2]);
	}
    }
    c->in_begin = 0;
}
