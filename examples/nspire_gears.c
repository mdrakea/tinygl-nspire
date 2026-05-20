/*
 * TinyGL TI-Nspire example: 3-D gear wheels
 * Ported from the classic gears.c demo
 *
 * Build with TI-Nspire toolchain:
 *   nspire-gcc -o gears.tns gears.c nspire.c -lndls -lTinyGL
 *
 * Key mappings for TI-Nspire:
 *   Up/Down/Left/Right: Rotate view
 *   ESC: Exit
 *   +: Speed up rotation
 *   -: Slow down rotation
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>

#include <GLES/gl.h>
#include <GL/math.h>
#include <GL/oscontext.h>
#include <GL/nspire.h>
#include "nspire_utils.h"

/* TI-Nspire screen dimensions */
#define SCREEN_WIDTH  320
#define SCREEN_HEIGHT 240

#ifndef NSPIRE_RENDER_SCALE
#ifdef NSPIRE_RELEASE_BUILD
#define NSPIRE_RENDER_SCALE 2
#else
#define NSPIRE_RENDER_SCALE 1
#endif
#endif

#define RENDER_WIDTH  (SCREEN_WIDTH / NSPIRE_RENDER_SCALE)
#define RENDER_HEIGHT (SCREEN_HEIGHT / NSPIRE_RENDER_SCALE)

#ifndef NSPIRE_PROFILER
#define NSPIRE_PROFILER 0
#endif

typedef struct GearMesh {
    GLfixed *vertices;
    GLfixed *normals;
    GLushort *indices;
    GLint vertex_count;
    GLint index_count;
    GLint vertex_capacity;
    GLint index_capacity;
    GLfixed color[4];
} GearMesh;

typedef struct GearBuilder {
    GearMesh *mesh;
    GLfixed normal[3];
} GearBuilder;

static inline GLfixed gear_angle(GLint i, GLint teeth)
{
    return (TGL_FIX_2PI * i) / teeth;
}

static inline GLfixed gear_x(GLfixed r, GLfixed angle)
{
    return tgl_fix_mul(r, cos(angle));
}

static inline GLfixed gear_y(GLfixed r, GLfixed angle)
{
    return tgl_fix_mul(r, sin(angle));
}

static void mesh_reserve(GearMesh *mesh, GLint extra_vertices, GLint extra_indices)
{
    GLint needed_vertices = mesh->vertex_count + extra_vertices;
    GLint needed_indices = mesh->index_count + extra_indices;

    if (needed_vertices > mesh->vertex_capacity) {
        GLint new_capacity = mesh->vertex_capacity == 0 ? 256 : mesh->vertex_capacity;
        while (new_capacity < needed_vertices) new_capacity *= 2;
        mesh->vertices = realloc(mesh->vertices, (size_t)new_capacity * 3 * sizeof(GLfixed));
        mesh->normals = realloc(mesh->normals, (size_t)new_capacity * 3 * sizeof(GLfixed));
        mesh->vertex_capacity = new_capacity;
    }
    if (needed_indices > mesh->index_capacity) {
        GLint new_capacity = mesh->index_capacity == 0 ? 512 : mesh->index_capacity;
        while (new_capacity < needed_indices) new_capacity *= 2;
        mesh->indices = realloc(mesh->indices, (size_t)new_capacity * sizeof(GLushort));
        mesh->index_capacity = new_capacity;
    }
}

static GLushort mesh_vertex(GearBuilder *builder, GLfixed x, GLfixed y, GLfixed z)
{
    GearMesh *mesh = builder->mesh;
    GLint n;

    mesh_reserve(mesh, 1, 0);
    n = mesh->vertex_count++;
    mesh->vertices[n * 3 + 0] = x;
    mesh->vertices[n * 3 + 1] = y;
    mesh->vertices[n * 3 + 2] = z;
    mesh->normals[n * 3 + 0] = builder->normal[0];
    mesh->normals[n * 3 + 1] = builder->normal[1];
    mesh->normals[n * 3 + 2] = builder->normal[2];
    return (GLushort)n;
}

static GLushort mesh_polar_vertex(GearBuilder *builder, GLfixed r, GLfixed angle,
                                  GLfixed z)
{
    return mesh_vertex(builder, gear_x(r, angle), gear_y(r, angle), z);
}

static void mesh_triangle(GearBuilder *builder, GLushort a, GLushort b, GLushort c)
{
    GearMesh *mesh = builder->mesh;
    mesh_reserve(mesh, 0, 3);
    mesh->indices[mesh->index_count++] = a;
    mesh->indices[mesh->index_count++] = b;
    mesh->indices[mesh->index_count++] = c;
}

static void mesh_quad(GearBuilder *builder,
                      GLfixed x0, GLfixed y0, GLfixed z0,
                      GLfixed x1, GLfixed y1, GLfixed z1,
                      GLfixed x2, GLfixed y2, GLfixed z2,
                      GLfixed x3, GLfixed y3, GLfixed z3)
{
    GLushort a, b, c, d;

    a = mesh_vertex(builder, x0, y0, z0);
    b = mesh_vertex(builder, x1, y1, z1);
    c = mesh_vertex(builder, x2, y2, z2);
    d = mesh_vertex(builder, x3, y3, z3);
    mesh_triangle(builder, a, b, c);
    mesh_triangle(builder, a, c, d);
}

static void mesh_polar_quad(GearBuilder *builder,
                            GLfixed r0, GLfixed a0, GLfixed z0,
                            GLfixed r1, GLfixed a1, GLfixed z1,
                            GLfixed r2, GLfixed a2, GLfixed z2,
                            GLfixed r3, GLfixed a3, GLfixed z3)
{
    mesh_quad(builder,
              gear_x(r0, a0), gear_y(r0, a0), z0,
              gear_x(r1, a1), gear_y(r1, a1), z1,
              gear_x(r2, a2), gear_y(r2, a2), z2,
              gear_x(r3, a3), gear_y(r3, a3), z3);
}

static void builder_normal(GearBuilder *builder, GLfixed x, GLfixed y, GLfixed z)
{
    builder->normal[0] = x;
    builder->normal[1] = y;
    builder->normal[2] = z;
}

static void builder_flank_normal(GearBuilder *builder,
                                 GLfixed x0, GLfixed y0, GLfixed x1, GLfixed y1)
{
    GLfixed u = x1 - x0;
    GLfixed v = y1 - y0;
    GLfixed len = tgl_fix_sqrt(tgl_fix_mul(u, u) + tgl_fix_mul(v, v));

    if (len != 0) {
        u = tgl_fix_div(u, len);
        v = tgl_fix_div(v, len);
    }
    builder_normal(builder, v, -u, 0);
}

static void build_gear_mesh(GearMesh *mesh, GLfixed inner_radius,
                            GLfixed outer_radius, GLfixed width,
                            GLint teeth, GLfixed tooth_depth,
                            const GLfixed color[4])
{
    GearBuilder builder;
    GLint i;
    GLfixed r0, r1, r2;
    GLfixed da, half_width;

    memset(mesh, 0, sizeof(*mesh));
    memcpy(mesh->color, color, sizeof(mesh->color));
    builder.mesh = mesh;

    r0 = inner_radius;
    r1 = outer_radius - tooth_depth / 2;
    r2 = outer_radius + tooth_depth / 2;
    half_width = width / 2;
    da = TGL_FIX_2PI / teeth / 4;

    builder_normal(&builder, 0, 0, TGL_FIX_ONE);
    for (i = 0; i < teeth; i++) {
        GLfixed a0 = gear_angle(i, teeth);
        GLfixed a1 = a0 + da;
        GLfixed a2 = a0 + 2 * da;
        GLfixed a3 = a0 + 3 * da;
        GLfixed a4 = gear_angle(i + 1, teeth);

        mesh_polar_quad(&builder, r0, a0, half_width,
                        r1, a0, half_width,
                        r1, a3, half_width,
                        r0, a3, half_width);
        mesh_polar_quad(&builder, r0, a3, half_width,
                        r1, a3, half_width,
                        r1, a4, half_width,
                        r0, a4, half_width);
        mesh_polar_quad(&builder, r1, a0, half_width,
                        r2, a1, half_width,
                        r2, a2, half_width,
                        r1, a3, half_width);
    }

    builder_normal(&builder, 0, 0, -TGL_FIX_ONE);
    for (i = 0; i < teeth; i++) {
        GLfixed a0 = gear_angle(i, teeth);
        GLfixed a1 = a0 + da;
        GLfixed a2 = a0 + 2 * da;
        GLfixed a3 = a0 + 3 * da;
        GLfixed a4 = gear_angle(i + 1, teeth);

        mesh_polar_quad(&builder, r1, a3, -half_width,
                        r1, a0, -half_width,
                        r0, a0, -half_width,
                        r0, a3, -half_width);
        mesh_polar_quad(&builder, r1, a4, -half_width,
                        r1, a3, -half_width,
                        r0, a3, -half_width,
                        r0, a4, -half_width);
        mesh_polar_quad(&builder, r1, a3, -half_width,
                        r2, a2, -half_width,
                        r2, a1, -half_width,
                        r1, a0, -half_width);
    }

    for (i = 0; i < teeth; i++) {
        GLfixed a0 = gear_angle(i, teeth);
        GLfixed a1 = a0 + da;
        GLfixed a2 = a0 + 2 * da;
        GLfixed a3 = a0 + 3 * da;
        GLfixed a4 = gear_angle(i + 1, teeth);

        builder_flank_normal(&builder, gear_x(r1, a0), gear_y(r1, a0),
                             gear_x(r2, a1), gear_y(r2, a1));
        mesh_polar_quad(&builder, r1, a0, half_width,
                        r1, a0, -half_width,
                        r2, a1, -half_width,
                        r2, a1, half_width);

        builder_normal(&builder, cos(a0), sin(a0), 0);
        mesh_polar_quad(&builder, r2, a1, half_width,
                        r2, a1, -half_width,
                        r2, a2, -half_width,
                        r2, a2, half_width);

        builder_flank_normal(&builder, gear_x(r2, a2), gear_y(r2, a2),
                             gear_x(r1, a3), gear_y(r1, a3));
        mesh_polar_quad(&builder, r2, a2, half_width,
                        r2, a2, -half_width,
                        r1, a3, -half_width,
                        r1, a3, half_width);

        builder_normal(&builder, cos(a3), sin(a3), 0);
        mesh_polar_quad(&builder, r1, a3, half_width,
                        r1, a3, -half_width,
                        r1, a4, -half_width,
                        r1, a4, half_width);
    }

    for (i = 0; i < teeth; i++) {
        GLfixed a0 = gear_angle(i, teeth);
        GLfixed a4 = gear_angle(i + 1, teeth);
        GLfixed mid = (a0 + a4) / 2;

        builder_normal(&builder, -cos(mid), -sin(mid), 0);
        mesh_polar_quad(&builder, r0, a4, -half_width,
                        r0, a0, -half_width,
                        r0, a0, half_width,
                        r0, a4, half_width);
    }
}


/* Global state */
static GLfixed view_rotx = TGL_I(20), view_roty = TGL_I(30), view_rotz = 0;
static GearMesh gear1, gear2, gear3;
static GLfixed angle = 0;
static GLfixed speed = TGL_I(2);
static int running = 1;
static int fps_value = 0;
static int fps_frames = 0;
static long fps_last_second = 0;

#if NSPIRE_PROFILER
typedef struct FrameProfile {
    unsigned input;
    unsigned clear;
    unsigned scene;
    unsigned scene_setup;
    unsigned gear1;
    unsigned gear2;
    unsigned gear3;
    unsigned overlay;
    unsigned scale;
    unsigned blit;
    unsigned total;
} FrameProfile;

static FrameProfile profiler_display;
static int profiler_has_sample = 0;
#endif

/* Our two framebuffers */
static unsigned short framebuffer1[RENDER_WIDTH * RENDER_HEIGHT];
static unsigned short framebuffer2[RENDER_WIDTH * RENDER_HEIGHT];
static void *framebuffers[2];
static ostgl_context *context;

static unsigned short rgb565(unsigned int r, unsigned int g, unsigned int b)
{
    return (unsigned short)(((r & 0xf8) << 8) |
                            ((g & 0xfc) << 3) |
                            ((b & 0xf8) >> 3));
}

static long current_second(void)
{
    struct timeval tv;

    gettimeofday(&tv, NULL);
    return (long)tv.tv_sec;
}

static void update_fps_counter(void)
{
    long now = current_second();

    if (fps_last_second == 0) {
        fps_last_second = now;
    }

    fps_frames++;
    if (now != fps_last_second) {
        fps_value = fps_frames;
        fps_frames = 0;
        fps_last_second = now;
    }
}

static const unsigned char *glyph_for(char ch)
{
    static const unsigned char glyph_0[5] = {7, 5, 5, 5, 7};
    static const unsigned char glyph_1[5] = {2, 6, 2, 2, 7};
    static const unsigned char glyph_2[5] = {7, 1, 7, 4, 7};
    static const unsigned char glyph_3[5] = {7, 1, 7, 1, 7};
    static const unsigned char glyph_4[5] = {5, 5, 7, 1, 1};
    static const unsigned char glyph_5[5] = {7, 4, 7, 1, 7};
    static const unsigned char glyph_6[5] = {7, 4, 7, 5, 7};
    static const unsigned char glyph_7[5] = {7, 1, 2, 2, 2};
    static const unsigned char glyph_8[5] = {7, 5, 7, 5, 7};
    static const unsigned char glyph_9[5] = {7, 5, 7, 1, 7};
    static const unsigned char glyph_b[5] = {6, 5, 6, 5, 6};
    static const unsigned char glyph_c[5] = {7, 4, 4, 4, 7};
    static const unsigned char glyph_d[5] = {6, 5, 5, 5, 6};
    static const unsigned char glyph_f[5] = {7, 4, 6, 4, 4};
    static const unsigned char glyph_i[5] = {7, 2, 2, 2, 7};
    static const unsigned char glyph_m[5] = {5, 7, 7, 5, 5};
    static const unsigned char glyph_o[5] = {7, 5, 5, 5, 7};
    static const unsigned char glyph_p[5] = {6, 5, 6, 4, 4};
    static const unsigned char glyph_s[5] = {7, 4, 7, 1, 7};
    static const unsigned char glyph_colon[5] = {0, 2, 0, 2, 0};
    static const unsigned char glyph_space[5] = {0, 0, 0, 0, 0};

    switch (ch) {
    case '0': return glyph_0;
    case '1': return glyph_1;
    case '2': return glyph_2;
    case '3': return glyph_3;
    case '4': return glyph_4;
    case '5': return glyph_5;
    case '6': return glyph_6;
    case '7': return glyph_7;
    case '8': return glyph_8;
    case '9': return glyph_9;
    case 'B': return glyph_b;
    case 'C': return glyph_c;
    case 'D': return glyph_d;
    case 'F': return glyph_f;
    case 'I': return glyph_i;
    case 'M': return glyph_m;
    case 'O': return glyph_o;
    case 'P': return glyph_p;
    case 'S': return glyph_s;
    case ':': return glyph_colon;
    default: return glyph_space;
    }
}

static void fill_rect(unsigned short *buffer, int x, int y, int w, int h,
                      unsigned short color)
{
    int px, py;

    for (py = y; py < y + h; py++) {
        if (py < 0 || py >= RENDER_HEIGHT) continue;
        for (px = x; px < x + w; px++) {
            if (px < 0 || px >= RENDER_WIDTH) continue;
            buffer[py * RENDER_WIDTH + px] = color;
        }
    }
}

static void draw_char(unsigned short *buffer, int x, int y, char ch,
                      unsigned short color)
{
    const unsigned char *glyph = glyph_for(ch);
    int row, col, sx, sy;
    const int scale = 2;

    for (row = 0; row < 5; row++) {
        for (col = 0; col < 3; col++) {
            if ((glyph[row] & (1 << (2 - col))) == 0) continue;
            for (sy = 0; sy < scale; sy++) {
                for (sx = 0; sx < scale; sx++) {
                    int px = x + col * scale + sx;
                    int py = y + row * scale + sy;
                    if (px >= 0 && px < RENDER_WIDTH &&
                        py >= 0 && py < RENDER_HEIGHT) {
                        buffer[py * RENDER_WIDTH + px] = color;
                    }
                }
            }
        }
    }
}

static void draw_text(unsigned short *buffer, int x, int y, const char *text,
                      unsigned short color)
{
    while (*text) {
        draw_char(buffer, x, y, *text, color);
        x += 8;
        text++;
    }
}

static void fps_label(char *label, int fps)
{
    char digits[5];
    int count = 0;
    int value = fps;
    int i;

    strcpy(label, "FPS: ");
    if (value <= 0) {
        strcat(label, "0");
        return;
    }

    if (value > 9999) value = 9999;
    while (value > 0) {
        digits[count++] = (char)('0' + (value % 10));
        value /= 10;
    }

    for (i = 0; i < count; i++) {
        label[5 + i] = digits[count - i - 1];
    }
    label[5 + count] = '\0';
}

static void draw_fps_overlay(unsigned short *buffer)
{
    char label[11];

    fps_label(label, fps_value);
    fill_rect(buffer, 2, 2, 76, 16, rgb565(0, 0, 0));
    draw_text(buffer, 5, 5, label, rgb565(255, 255, 255));
}

#if NSPIRE_PROFILER
static void profiler_commit(const FrameProfile *sample)
{
    if (sample->total == 0) return;

    if (!profiler_has_sample) {
        profiler_display = *sample;
        profiler_has_sample = 1;
        return;
    }

    profiler_display.input = (profiler_display.input * 3 + sample->input) / 4;
    profiler_display.clear = (profiler_display.clear * 3 + sample->clear) / 4;
    profiler_display.scene = (profiler_display.scene * 3 + sample->scene) / 4;
    profiler_display.scene_setup = (profiler_display.scene_setup * 3 + sample->scene_setup) / 4;
    profiler_display.gear1 = (profiler_display.gear1 * 3 + sample->gear1) / 4;
    profiler_display.gear2 = (profiler_display.gear2 * 3 + sample->gear2) / 4;
    profiler_display.gear3 = (profiler_display.gear3 * 3 + sample->gear3) / 4;
    profiler_display.overlay = (profiler_display.overlay * 3 + sample->overlay) / 4;
    profiler_display.scale = (profiler_display.scale * 3 + sample->scale) / 4;
    profiler_display.blit = (profiler_display.blit * 3 + sample->blit) / 4;
    profiler_display.total = (profiler_display.total * 3 + sample->total) / 4;
}

static unsigned profiler_misc_ticks(const FrameProfile *profile)
{
    unsigned known = profile->input + profile->clear + profile->scene +
                     profile->overlay + profile->scale + profile->blit;

    if (profile->total <= known) return 0;
    return profile->total - known;
}

static unsigned profiler_scene_misc_ticks(const FrameProfile *profile)
{
    unsigned known = profile->scene_setup + profile->gear1 +
                     profile->gear2 + profile->gear3;

    if (profile->scene <= known) return 0;
    return profile->scene - known;
}

static int profiler_bar_width(unsigned ticks, unsigned total, int full_width)
{
    int width;

    if (ticks == 0 || total == 0) return 0;

    width = (int)(((unsigned long long)ticks * full_width + total / 2) / total);
    if (width == 0) width = 1;
    return width;
}

static void draw_segment_bar(unsigned short *buffer, int x, int y,
                             int full_width, int height,
                             const unsigned *ticks,
                             const unsigned short *colors,
                             int count, unsigned total)
{
    int i, used = 0;

    for (i = 0; i < count; i++) {
        int width = profiler_bar_width(ticks[i], total, full_width);

        if (used + width > full_width) {
            width = full_width - used;
        }
        if (width <= 0) continue;

        fill_rect(buffer, x + used, y, width, height, colors[i]);
        used += width;
        if (used >= full_width) break;
    }
}

static void draw_profile_overlay(unsigned short *buffer)
{
    static const char labels[7] = {'I', 'C', 'D', 'O', 'S', 'B', 'M'};
    unsigned short colors[7];
    unsigned ticks[7];
    unsigned short scene_colors[5];
    unsigned scene_ticks[5];
    unsigned total;
    int x, y, i;

    if (!profiler_has_sample) return;

    colors[0] = rgb565(255, 220, 0);
    colors[1] = rgb565(0, 140, 255);
    colors[2] = rgb565(255, 60, 0);
    colors[3] = rgb565(255, 255, 255);
    colors[4] = rgb565(0, 220, 80);
    colors[5] = rgb565(255, 0, 220);
    colors[6] = rgb565(96, 96, 96);

    ticks[0] = profiler_display.input;
    ticks[1] = profiler_display.clear;
    ticks[2] = profiler_display.scene;
    ticks[3] = profiler_display.overlay;
    ticks[4] = profiler_display.scale;
    ticks[5] = profiler_display.blit;
    ticks[6] = profiler_misc_ticks(&profiler_display);

    total = profiler_display.total;
    if (total == 0) return;

    scene_colors[0] = rgb565(255, 220, 0);
    scene_colors[1] = rgb565(255, 60, 0);
    scene_colors[2] = rgb565(0, 220, 80);
    scene_colors[3] = rgb565(0, 140, 255);
    scene_colors[4] = rgb565(96, 96, 96);

    scene_ticks[0] = profiler_display.scene_setup;
    scene_ticks[1] = profiler_display.gear1;
    scene_ticks[2] = profiler_display.gear2;
    scene_ticks[3] = profiler_display.gear3;
    scene_ticks[4] = profiler_scene_misc_ticks(&profiler_display);

    y = RENDER_HEIGHT - 39;
    fill_rect(buffer, 2, y - 1, RENDER_WIDTH - 4, 38, rgb565(0, 0, 0));

    x = 4;
    for (i = 0; i < 7; i++) {
        fill_rect(buffer, x, y + 3, 5, 5, colors[i]);
        draw_char(buffer, x + 7, y, labels[i], colors[i]);
        x += 22;
    }

    draw_segment_bar(buffer, 2, y + 13, RENDER_WIDTH - 4, 5,
                     ticks, colors, 7, total);

    x = 4;
    y += 19;
    draw_char(buffer, x, y, 'D', colors[2]);
    draw_char(buffer, x + 8, y, ':', colors[2]);
    x += 20;

    for (i = 0; i < 5; i++) {
        char label = (i == 4) ? 'M' : (char)('0' + i);

        fill_rect(buffer, x, y + 3, 5, 5, scene_colors[i]);
        draw_char(buffer, x + 7, y, label, scene_colors[i]);
        x += 22;
    }

    draw_segment_bar(buffer, 2, y + 13, RENDER_WIDTH - 4, 5,
                     scene_ticks, scene_colors, 5, profiler_display.scene);
}
#endif

static void draw_gear(const GearMesh *mesh)
{
    glMaterialxv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, mesh->color);
    glVertexPointer(3, GL_FIXED, 0, mesh->vertices);
    glNormalPointer(GL_FIXED, 0, mesh->normals);
    glDrawElements(GL_TRIANGLES, mesh->index_count, GL_UNSIGNED_SHORT,
                   mesh->indices);
}

static void free_gear_mesh(GearMesh *mesh)
{
    free(mesh->vertices);
    free(mesh->normals);
    free(mesh->indices);
    memset(mesh, 0, sizeof(*mesh));
}


/*
 * Main render function
 */
#if NSPIRE_PROFILER
static void draw(FrameProfile *profile)
{
    unsigned t0, t1, scene_start;

    t0 = nspire_timer_ticks();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    t1 = nspire_timer_ticks();
    profile->clear = nspire_timer_elapsed(t0, t1);

    t0 = t1;
    scene_start = t0;
    glPushMatrix();
    glRotatex(view_rotx, TGL_FIX_ONE, 0, 0);
    glRotatex(view_roty, 0, TGL_FIX_ONE, 0);
    glRotatex(view_rotz, 0, 0, TGL_FIX_ONE);
    t1 = nspire_timer_ticks();
    profile->scene_setup = nspire_timer_elapsed(t0, t1);

    t0 = t1;
    glPushMatrix();
    glTranslatex(-TGL_I(3), -TGL_I(2), 0);
    glRotatex(angle, 0, 0, TGL_FIX_ONE);
    draw_gear(&gear1);
    glPopMatrix();
    t1 = nspire_timer_ticks();
    profile->gear1 = nspire_timer_elapsed(t0, t1);

    t0 = t1;
    glPushMatrix();
    glTranslatex(TGL_FRAC(31,10), -TGL_I(2), 0);
    glRotatex(-(2 * angle) - TGL_I(9), 0, 0, TGL_FIX_ONE);
    draw_gear(&gear2);
    glPopMatrix();
    t1 = nspire_timer_ticks();
    profile->gear2 = nspire_timer_elapsed(t0, t1);

    t0 = t1;
    glPushMatrix();
    glTranslatex(-TGL_FRAC(31,10), TGL_FRAC(42,10), 0);
    glRotatex(-(2 * angle) - TGL_I(25), 0, 0, TGL_FIX_ONE);
    draw_gear(&gear3);
    glPopMatrix();
    t1 = nspire_timer_ticks();
    profile->gear3 = nspire_timer_elapsed(t0, t1);

    t0 = t1;
    glPopMatrix();
    t1 = nspire_timer_ticks();
    profile->scene_setup += nspire_timer_elapsed(t0, t1);
    profile->scene = nspire_timer_elapsed(scene_start, t1);

    t0 = t1;
    draw_fps_overlay(framebuffer1);
    draw_profile_overlay(framebuffer1);
    t1 = nspire_timer_ticks();
    profile->overlay = nspire_timer_elapsed(t0, t1);

    /* Swap buffers - blit to TI-Nspire screen */
    nspire_swap_buffers_profiled(&profile->scale, &profile->blit);
}
#else
static void draw(void)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glPushMatrix();
    glRotatex(view_rotx, TGL_FIX_ONE, 0, 0);
    glRotatex(view_roty, 0, TGL_FIX_ONE, 0);
    glRotatex(view_rotz, 0, 0, TGL_FIX_ONE);

    glPushMatrix();
    glTranslatex(-TGL_I(3), -TGL_I(2), 0);
    glRotatex(angle, 0, 0, TGL_FIX_ONE);
    draw_gear(&gear1);
    glPopMatrix();

    glPushMatrix();
    glTranslatex(TGL_FRAC(31,10), -TGL_I(2), 0);
    glRotatex(-(2 * angle) - TGL_I(9), 0, 0, TGL_FIX_ONE);
    draw_gear(&gear2);
    glPopMatrix();

    glPushMatrix();
    glTranslatex(-TGL_FRAC(31,10), TGL_FRAC(42,10), 0);
    glRotatex(-(2 * angle) - TGL_I(25), 0, 0, TGL_FIX_ONE);
    draw_gear(&gear3);
    glPopMatrix();

    glPopMatrix();

    draw_fps_overlay(framebuffer1);
    nspire_swap_buffers();
}
#endif


/*
 * Idle/update function - called every frame
 */
#if NSPIRE_PROFILER
static void idle_guh(FrameProfile *profile)
{
    angle += speed;
    draw(profile);
    update_fps_counter();
}
#else
static void idle_guh(void)
{
    angle += speed;
    draw();
    update_fps_counter();
}
#endif


/*
 * Handle keyboard input
 */
static void handle_input(void)
{
    /* Check for arrow keys */
    if (nspire_is_key_pressed(KEY_NSPIRE_UP)) {
        view_rotx += TGL_I(2);
    }
    if (nspire_is_key_pressed(KEY_NSPIRE_DOWN)) {
        view_rotx -= TGL_I(2);
    }
    if (nspire_is_key_pressed(KEY_NSPIRE_LEFT)) {
        view_roty += TGL_I(2);
    }
    if (nspire_is_key_pressed(KEY_NSPIRE_RIGHT)) {
        view_roty -= TGL_I(2);
    }

    /* Speed control */
    if (nspire_is_key_pressed(KEY_NSPIRE_PLUS)) {
        speed += TGL_FRAC(1,10);
    }
    if (nspire_is_key_pressed(KEY_NSPIRE_MINUS)) {
        speed -= TGL_FRAC(1,10);
        if (speed < 0) speed = 0;
    }

    /* Exit on ESC (caps key on TI-Nspire) */
    if (nspire_is_key_pressed(KEY_NSPIRE_ESC)) {
        running = 0;
    }
}


/*
 * Reshape callback
 */
static void reshape(int width, int height)
{
    GLfixed h = tgl_fix_div(TGL_I(height), TGL_I(width));

    glViewport(0, 0, (GLint)width, (GLint)height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glFrustumx(-TGL_FIX_ONE, TGL_FIX_ONE, -h, h, TGL_I(5), TGL_I(60));
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatex(0, 0, -TGL_I(40));
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}


/*
 * Initialize GL scene
 */
static void init(void)
{
    static GLfixed pos[4] = {TGL_I(5), TGL_I(5), TGL_I(10), 0};
    static GLfixed red[4] = {TGL_FRAC(4,5), TGL_FRAC(1,10), 0, TGL_FIX_ONE};
    static GLfixed green[4] = {0, TGL_FRAC(4,5), TGL_FRAC(1,5), TGL_FIX_ONE};
    static GLfixed blue[4] = {TGL_FRAC(1,5), TGL_FRAC(1,5), TGL_FIX_ONE, TGL_FIX_ONE};

    glLightxv(GL_LIGHT0, GL_POSITION, pos);
    glEnable(GL_CULL_FACE);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_DEPTH_TEST);
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_NORMAL_ARRAY);

    build_gear_mesh(&gear1, TGL_FIX_ONE, TGL_I(4), TGL_FIX_ONE, 20,
                    TGL_FRAC(7,10), red);
    build_gear_mesh(&gear2, TGL_FIX_HALF, TGL_I(2), TGL_I(2), 10,
                    TGL_FRAC(7,10), green);
    build_gear_mesh(&gear3, TGL_FRAC(13,10), TGL_I(2), TGL_FIX_HALF, 10,
                    TGL_FRAC(7,10), blue);
}


/*
 * Main entry point
 */
int main(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    puts("TinyGL TI-Nspire Demo");
    puts("Press arrow keys to rotate view");
    puts("+/- to adjust speed, ESC to exit");

    /* Setup framebuffers */
    framebuffers[0] = framebuffer1;
    framebuffers[1] = framebuffer2;

    /* Create TinyGL context */
    context = ostgl_create_context(RENDER_WIDTH, RENDER_HEIGHT, 16,
                                    framebuffers, 2);
    if (!context) {
        fiprintf(stderr, "Failed to create TinyGL context\n");
        return 1;
    }

#if NSPIRE_PROFILER
    nspire_profiler_init();
#endif

    /* Make first buffer current */
    ostgl_make_current(context, 0);

    /* Initialize GL */
    init();
    reshape(RENDER_WIDTH, RENDER_HEIGHT);

    /* Main loop */
    while (running) {
#if NSPIRE_PROFILER
        FrameProfile profile;
        unsigned frame_start, t0, t1;

        memset(&profile, 0, sizeof(profile));
        frame_start = nspire_timer_ticks();
        t0 = frame_start;

        handle_input();
        t1 = nspire_timer_ticks();
        profile.input = nspire_timer_elapsed(t0, t1);

        idle_guh(&profile);
        profile.total = nspire_timer_elapsed(frame_start, nspire_timer_ticks());
        profiler_commit(&profile);
#else
        handle_input();
        idle_guh();
#endif
    }

    /* Cleanup */
    free_gear_mesh(&gear1);
    free_gear_mesh(&gear2);
    free_gear_mesh(&gear3);
#if NSPIRE_PROFILER
    nspire_profiler_shutdown();
#endif
    ostgl_delete_context(context);

    puts("Goodbye!");
    return 0;
}
