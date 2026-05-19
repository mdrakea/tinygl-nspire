/*
 * TinyGL TI-Nspire example: 3-D gear wheels
 * Ported from the classic gears.c demo
 *
 * Build with TI-Nspire toolchain:
 *   nspire-gcc -o gears.tns gears.c nspire.c -lndls -lm -lTinyGL
 *
 * Key mappings for TI-Nspire:
 *   Up/Down/Left/Right: Rotate view
 *   ESC: Exit
 *   +: Speed up rotation
 *   -: Slow down rotation
 */

#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>

#include <GL/gl.h>
#include <GL/oscontext.h>
#include <GL/nspire.h>

#ifndef M_PI
#define M_PI 3.14159265
#endif

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

/*
 * Draw a gear wheel
 */
static void gear(GLfloat inner_radius, GLfloat outer_radius, GLfloat width,
                GLint teeth, GLfloat tooth_depth)
{
    GLint i;
    GLfloat r0, r1, r2;
    GLfloat angle, da;
    GLfloat u, v, len;

    r0 = inner_radius;
    r1 = outer_radius - tooth_depth / 2.0;
    r2 = outer_radius + tooth_depth / 2.0;

    da = 2.0 * M_PI / teeth / 4.0;

    glShadeModel(GL_FLAT);
    glNormal3f(0.0, 0.0, 1.0);

    /* Front face */
    glBegin(GL_QUAD_STRIP);
    for (i = 0; i <= teeth; i++) {
        angle = i * 2.0 * M_PI / teeth;
        glVertex3f(r0 * cos(angle), r0 * sin(angle), width * 0.5);
        glVertex3f(r1 * cos(angle), r1 * sin(angle), width * 0.5);
        glVertex3f(r0 * cos(angle), r0 * sin(angle), width * 0.5);
        glVertex3f(r1 * cos(angle + 3 * da), r1 * sin(angle + 3 * da), width * 0.5);
    }
    glEnd();

    /* Front sides of teeth */
    glBegin(GL_QUADS);
    da = 2.0 * M_PI / teeth / 4.0;
    for (i = 0; i < teeth; i++) {
        angle = i * 2.0 * M_PI / teeth;
        glVertex3f(r1 * cos(angle), r1 * sin(angle), width * 0.5);
        glVertex3f(r2 * cos(angle + da), r2 * sin(angle + da), width * 0.5);
        glVertex3f(r2 * cos(angle + 2 * da), r2 * sin(angle + 2 * da), width * 0.5);
        glVertex3f(r1 * cos(angle + 3 * da), r1 * sin(angle + 3 * da), width * 0.5);
    }
    glEnd();

    glNormal3f(0.0, 0.0, -1.0);

    /* Back face */
    glBegin(GL_QUAD_STRIP);
    for (i = 0; i <= teeth; i++) {
        angle = i * 2.0 * M_PI / teeth;
        glVertex3f(r1 * cos(angle), r1 * sin(angle), -width * 0.5);
        glVertex3f(r0 * cos(angle), r0 * sin(angle), -width * 0.5);
        glVertex3f(r1 * cos(angle + 3 * da), r1 * sin(angle + 3 * da), -width * 0.5);
        glVertex3f(r0 * cos(angle), r0 * sin(angle), -width * 0.5);
    }
    glEnd();

    /* Back sides of teeth */
    glBegin(GL_QUADS);
    da = 2.0 * M_PI / teeth / 4.0;
    for (i = 0; i < teeth; i++) {
        angle = i * 2.0 * M_PI / teeth;
        glVertex3f(r1 * cos(angle + 3 * da), r1 * sin(angle + 3 * da), -width * 0.5);
        glVertex3f(r2 * cos(angle + 2 * da), r2 * sin(angle + 2 * da), -width * 0.5);
        glVertex3f(r2 * cos(angle + da), r2 * sin(angle + da), -width * 0.5);
        glVertex3f(r1 * cos(angle), r1 * sin(angle), -width * 0.5);
    }
    glEnd();

    /* Outward faces of teeth */
    glBegin(GL_QUAD_STRIP);
    for (i = 0; i < teeth; i++) {
        angle = i * 2.0 * M_PI / teeth;
        glVertex3f(r1 * cos(angle), r1 * sin(angle), width * 0.5);
        glVertex3f(r1 * cos(angle), r1 * sin(angle), -width * 0.5);
        u = r2 * cos(angle + da) - r1 * cos(angle);
        v = r2 * sin(angle + da) - r1 * sin(angle);
        len = sqrt(u * u + v * v);
        u /= len;
        v /= len;
        glNormal3f(v, -u, 0.0);
        glVertex3f(r2 * cos(angle + da), r2 * sin(angle + da), width * 0.5);
        glVertex3f(r2 * cos(angle + da), r2 * sin(angle + da), -width * 0.5);
        glNormal3f(cos(angle), sin(angle), 0.0);
        glVertex3f(r2 * cos(angle + 2 * da), r2 * sin(angle + 2 * da), width * 0.5);
        glVertex3f(r2 * cos(angle + 2 * da), r2 * sin(angle + 2 * da), -width * 0.5);
        u = r1 * cos(angle + 3 * da) - r2 * cos(angle + 2 * da);
        v = r1 * sin(angle + 3 * da) - r2 * sin(angle + 2 * da);
        len = sqrt(u * u + v * v);
        u /= len;
        v /= len;
        glNormal3f(v, -u, 0.0);
        glVertex3f(r1 * cos(angle + 3 * da), r1 * sin(angle + 3 * da), width * 0.5);
        glVertex3f(r1 * cos(angle + 3 * da), r1 * sin(angle + 3 * da), -width * 0.5);
        glNormal3f(cos(angle), sin(angle), 0.0);
    }
    glVertex3f(r1 * cos(0), r1 * sin(0), width * 0.5);
    glVertex3f(r1 * cos(0), r1 * sin(0), -width * 0.5);
    glEnd();

    glShadeModel(GL_SMOOTH);

    /* Inside radius cylinder */
    glBegin(GL_QUAD_STRIP);
    for (i = 0; i <= teeth; i++) {
        angle = i * 2.0 * M_PI / teeth;
        glNormal3f(-cos(angle), -sin(angle), 0.0);
        glVertex3f(r0 * cos(angle), r0 * sin(angle), -width * 0.5);
        glVertex3f(r0 * cos(angle), r0 * sin(angle), width * 0.5);
    }
    glEnd();
}


/* Global state */
static GLfloat view_rotx = 20.0, view_roty = 30.0, view_rotz = 0.0;
static GLint gear1, gear2, gear3;
static GLfloat angle = 0.0;
static GLfloat speed = 2.0;
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

/* Our framebuffers (double-buffered) */
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
    glRotatef(view_rotx, 1.0, 0.0, 0.0);
    glRotatef(view_roty, 0.0, 1.0, 0.0);
    glRotatef(view_rotz, 0.0, 0.0, 1.0);
    t1 = nspire_timer_ticks();
    profile->scene_setup = nspire_timer_elapsed(t0, t1);

    t0 = t1;
    glPushMatrix();
    glTranslatef(-3.0, -2.0, 0.0);
    glRotatef(angle, 0.0, 0.0, 1.0);
    glCallList(gear1);
    glPopMatrix();
    t1 = nspire_timer_ticks();
    profile->gear1 = nspire_timer_elapsed(t0, t1);

    t0 = t1;
    glPushMatrix();
    glTranslatef(3.1, -2.0, 0.0);
    glRotatef(-2.0 * angle - 9.0, 0.0, 0.0, 1.0);
    glCallList(gear2);
    glPopMatrix();
    t1 = nspire_timer_ticks();
    profile->gear2 = nspire_timer_elapsed(t0, t1);

    t0 = t1;
    glPushMatrix();
    glTranslatef(-3.1, 4.2, 0.0);
    glRotatef(-2.0 * angle - 25.0, 0.0, 0.0, 1.0);
    glCallList(gear3);
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
    glRotatef(view_rotx, 1.0, 0.0, 0.0);
    glRotatef(view_roty, 0.0, 1.0, 0.0);
    glRotatef(view_rotz, 0.0, 0.0, 1.0);

    glPushMatrix();
    glTranslatef(-3.0, -2.0, 0.0);
    glRotatef(angle, 0.0, 0.0, 1.0);
    glCallList(gear1);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(3.1, -2.0, 0.0);
    glRotatef(-2.0 * angle - 9.0, 0.0, 0.0, 1.0);
    glCallList(gear2);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(-3.1, 4.2, 0.0);
    glRotatef(-2.0 * angle - 25.0, 0.0, 0.0, 1.0);
    glCallList(gear3);
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
        view_rotx += 2.0;
    }
    if (nspire_is_key_pressed(KEY_NSPIRE_DOWN)) {
        view_rotx -= 2.0;
    }
    if (nspire_is_key_pressed(KEY_NSPIRE_LEFT)) {
        view_roty += 2.0;
    }
    if (nspire_is_key_pressed(KEY_NSPIRE_RIGHT)) {
        view_roty -= 2.0;
    }

    /* Speed control */
    if (nspire_is_key_pressed(KEY_NSPIRE_PLUS)) {
        speed += 0.1;
    }
    if (nspire_is_key_pressed(KEY_NSPIRE_MINUS)) {
        speed -= 0.1;
        if (speed < 0.0) speed = 0.0;
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
    GLfloat h = (GLfloat)height / (GLfloat)width;

    glViewport(0, 0, (GLint)width, (GLint)height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glFrustum(-1.0, 1.0, -h, h, 5.0, 60.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(0.0, 0.0, -40.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}


/*
 * Initialize GL scene
 */
static void init(void)
{
    static GLfloat pos[4] = {5.0, 5.0, 10.0, 0.0};
    static GLfloat red[4] = {0.8, 0.1, 0.0, 1.0};
    static GLfloat green[4] = {0.0, 0.8, 0.2, 1.0};
    static GLfloat blue[4] = {0.2, 0.2, 1.0, 1.0};

    glLightfv(GL_LIGHT0, GL_POSITION, pos);
    glEnable(GL_CULL_FACE);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_DEPTH_TEST);

    /* Create display lists for gears */
    gear1 = glGenLists(1);
    glNewList(gear1, GL_COMPILE);
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, red);
    gear(1.0, 4.0, 1.0, 20, 0.7);
    glEndList();

    gear2 = glGenLists(1);
    glNewList(gear2, GL_COMPILE);
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, green);
    gear(0.5, 2.0, 2.0, 10, 0.7);
    glEndList();

    gear3 = glGenLists(1);
    glNewList(gear3, GL_COMPILE);
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, blue);
    gear(1.3, 2.0, 0.5, 10, 0.7);
    glEndList();

    /* Normals generated above are already unit length. */
}


/*
 * Main entry point
 */
int main(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    printf("TinyGL TI-Nspire Demo\n");
    printf("Press arrow keys to rotate view\n");
    printf("+/- to adjust speed, ESC to exit\n");

    /* Setup framebuffers */
    framebuffers[0] = framebuffer1;
    framebuffers[1] = framebuffer2;

    /* Create TinyGL context */
    context = ostgl_create_context(RENDER_WIDTH, RENDER_HEIGHT, 16,
                                    framebuffers, 2);
    if (!context) {
        fprintf(stderr, "Failed to create TinyGL context\n");
        return 1;
    }

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
        
#ifndef NSPIRE_RELEASE_BUILD
        /* Small delay to not consume all CPU */
        nspire_msleep(10);
#endif
    }

    /* Cleanup */
    ostgl_delete_context(context);

    printf("Goodbye!\n");
    return 0;
}
