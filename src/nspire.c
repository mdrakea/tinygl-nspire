/*
 * TinyGL TI-Nspire driver
 * Port using Libndls - https://www.hackspire.org/Libndls/
 *
 * This provides a simple interface for TinyGL on the TI-Nspire calculator.
 * Uses the native RGB565 format for maximum performance.
 */

#include <libndls.h>
#include <GL/oscontext.h>
#include <GL/gl.h>
#include "zgl.h"
#include "zbuffer.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/* TI-Nspire screen dimensions */
#define NSPIRE_SCREEN_WIDTH  320
#define NSPIRE_SCREEN_HEIGHT 240

#ifndef NSPIRE_PROFILER
#define NSPIRE_PROFILER 0
#endif

/* Our private context structure */
typedef struct {
    GLContext *gl_context;
    int xsize, ysize;
    int numbuffers;
    void **framebuffers;  /* Pointers to our pixel buffers */
    ZBuffer **zbs;        /* Z-buffers for each framebuffer */
    scr_type_t screen_type;
    int current_buffer;
} TinyNspireContext;

/* Global state */
static TinyNspireContext *current_ctx = NULL;
static bool lcd_initialized = false;
static unsigned short lcd_framebuffer[NSPIRE_SCREEN_WIDTH * NSPIRE_SCREEN_HEIGHT];
#if NSPIRE_PROFILER
static bool profiler_timer_initialized = false;
static unsigned profiler_timer_saved_load;
static unsigned profiler_timer_saved_control;
static unsigned profiler_timer_saved_bgload;
#endif

/* Forward declarations */
static int nspire_resize_viewport(GLContext *c, int *xsize_ptr, int *ysize_ptr);

#if NSPIRE_PROFILER
static void nspire_profiler_timer_init(void)
{
    volatile unsigned *load;
    volatile unsigned *control;
    volatile unsigned *int_clear;
    volatile unsigned *bgload;

    if (profiler_timer_initialized || is_classic) return;

    /*
     * CX models expose an SP804 dual timer at 0x900D0000. Ndless uses
     * timer 1 for sleep/idle; timer 2 is enough for local frame profiling.
     */
    load = (volatile unsigned *)0x900D0020;
    control = (volatile unsigned *)0x900D0028;
    int_clear = (volatile unsigned *)0x900D002C;
    bgload = (volatile unsigned *)0x900D0038;

    profiler_timer_saved_load = *load;
    profiler_timer_saved_control = *control;
    profiler_timer_saved_bgload = *bgload;

    *control = 0;
    *int_clear = 1;
    *load = 0xffffffff;
    *bgload = 0xffffffff;
    *control = 0x82; /* enabled, 32-bit, no interrupt, free-running */
    profiler_timer_initialized = true;
}

static void nspire_profiler_timer_shutdown(void)
{
    volatile unsigned *load;
    volatile unsigned *control;
    volatile unsigned *int_clear;
    volatile unsigned *bgload;

    if (!profiler_timer_initialized || is_classic) return;

    load = (volatile unsigned *)0x900D0020;
    control = (volatile unsigned *)0x900D0028;
    int_clear = (volatile unsigned *)0x900D002C;
    bgload = (volatile unsigned *)0x900D0038;

    *control = 0;
    *int_clear = 1;
    *load = profiler_timer_saved_load;
    *bgload = profiler_timer_saved_bgload;
    *control = profiler_timer_saved_control;
    profiler_timer_initialized = false;
}

unsigned nspire_timer_ticks(void)
{
    if (is_classic) {
        return *(volatile unsigned *)0x90090000;
    }

    return *(volatile unsigned *)0x900D0024;
}

unsigned nspire_timer_elapsed(unsigned start, unsigned end)
{
    if (is_classic) {
        return end - start;
    }

    return start - end;
}
#endif

/*
 * Create a TinyGL context for TI-Nspire
 * This allocates the Z-buffers and initializes the LCD
 */
ostgl_context *ostgl_create_context(const int xsize, const int ysize,
                                     const int depth, void **framebuffers,
                                     const int numbuffers)
{
    ostgl_context *context;
    TinyNspireContext *ctx;
    int i;
    
    /* Validate parameters */
    if (xsize <= 0 || ysize <= 0 || numbuffers < 1) {
        fprintf(stderr, "ostgl_create_context: invalid parameters\n");
        return NULL;
    }
    
    /* Only support 16-bit depth (RGB565) */
    if (depth != 16) {
        fprintf(stderr, "ostgl_create_context: only 16-bit depth supported, got %d\n", depth);
        /* Continue anyway - we'll use RGB565 */
    }
    
    /* Allocate our context structure */
    ctx = (TinyNspireContext *)gl_malloc(sizeof(TinyNspireContext));
    if (!ctx) {
        fprintf(stderr, "ostgl_create_context: out of memory\n");
        return NULL;
    }
    
    memset(ctx, 0, sizeof(TinyNspireContext));
    
    /* Allocate storage for framebuffer and Z-buffer pointers */
    ctx->framebuffers = (void **)gl_malloc(sizeof(void *) * numbuffers);
    ctx->zbs = (ZBuffer **)gl_malloc(sizeof(ZBuffer *) * numbuffers);
    
    if (!ctx->framebuffers || !ctx->zbs) {
        fprintf(stderr, "ostgl_create_context: out of memory\n");
        if (ctx->framebuffers) gl_free(ctx->framebuffers);
        if (ctx->zbs) gl_free(ctx->zbs);
        gl_free(ctx);
        return NULL;
    }
    
    /* Copy framebuffer pointers */
    for (i = 0; i < numbuffers; i++) {
        ctx->framebuffers[i] = framebuffers[i];
    }
    
    ctx->numbuffers = numbuffers;
    ctx->xsize = xsize;
    ctx->ysize = ysize;
    ctx->current_buffer = 0;
    
    /* Get and initialize the LCD */
    ctx->screen_type = lcd_type();
    lcd_init(ctx->screen_type);
    lcd_initialized = true;
#if NSPIRE_PROFILER
    nspire_profiler_timer_init();
#endif
    
    /* Create Z-buffers for each framebuffer */
    for (i = 0; i < numbuffers; i++) {
        ctx->zbs[i] = ZB_open(xsize, ysize, ZB_MODE_5R6G5B, 0, NULL, NULL, framebuffers[i]);
        if (!ctx->zbs[i]) {
            fprintf(stderr, "ostgl_create_context: ZB_open failed for buffer %d\n", i);
            /* Cleanup already-created buffers */
            for (int j = 0; j < i; j++) {
                ZB_close(ctx->zbs[j]);
            }
            gl_free(ctx->framebuffers);
            gl_free(ctx->zbs);
            gl_free(ctx);
            return NULL;
        }
    }
    
    /* Initialize TinyGL with the first Z-buffer */
    glInit(ctx->zbs[0]);
    ctx->gl_context = gl_get_context();
    ctx->gl_context->opaque = (void *)ctx;
    ctx->gl_context->gl_resize_viewport = nspire_resize_viewport;
    current_ctx = ctx;
    
    /* Force viewport initialization */
    ctx->gl_context->viewport.xsize = -1;
    ctx->gl_context->viewport.ysize = -1;
    
    glViewport(0, 0, xsize, ysize);
    
    /* Create and return the oscontext wrapper */
    context = (ostgl_context *)gl_malloc(sizeof(ostgl_context));
    if (!context) {
        fprintf(stderr, "ostgl_create_context: out of memory for oscontext\n");
        for (i = 0; i < numbuffers; i++) {
            ZB_close(ctx->zbs[i]);
        }
        gl_free(ctx->framebuffers);
        gl_free(ctx->zbs);
        gl_free(ctx);
        return NULL;
    }
    
    context->zbs = (void **)ctx->zbs;
    context->framebuffers = ctx->framebuffers;
    context->numbuffers = numbuffers;
    context->xsize = xsize;
    context->ysize = ysize;
    
    return (ostgl_context *)context;
}

/*
 * Delete a TinyGL context
 */
void ostgl_delete_context(ostgl_context *oscontext)
{
    int i;
    TinyNspireContext *ctx = NULL;
    GLContext *gl_ctx;
    
    if (!oscontext) return;
    
    /* Find our context from the GL context */
    gl_ctx = gl_get_context();
    if (gl_ctx && gl_ctx->opaque) {
        ctx = (TinyNspireContext *)gl_ctx->opaque;
    }
    
    /* Close all Z-buffers */
    for (i = 0; i < oscontext->numbuffers; i++) {
        if (oscontext->zbs && oscontext->zbs[i]) {
            ZB_close((ZBuffer *)oscontext->zbs[i]);
        }
    }
    
    /*
     * oscontext->zbs and oscontext->framebuffers alias the arrays owned by
     * TinyNspireContext, so free them only once through ctx.
     */
    if (ctx) {
        if (ctx->zbs) gl_free(ctx->zbs);
        if (ctx->framebuffers) gl_free(ctx->framebuffers);
        if (current_ctx == ctx) current_ctx = NULL;
        gl_free(ctx);
    }
    
    gl_free(oscontext);
    
    /* Cleanup LCD */
    if (lcd_initialized) {
        lcd_init(SCR_TYPE_INVALID);
        lcd_initialized = false;
    }
#if NSPIRE_PROFILER
    nspire_profiler_timer_shutdown();
#endif
    
    glClose();
}

/*
 * Make a context current (activate a specific buffer)
 */
void ostgl_make_current(ostgl_context *oscontext, const int idx)
{
    GLContext *gl_ctx;
    TinyNspireContext *ctx;
    
    if (!oscontext || idx < 0 || idx >= oscontext->numbuffers) {
        fprintf(stderr, "ostgl_make_current: invalid parameters\n");
        return;
    }
    
    gl_ctx = gl_get_context();
    if (!gl_ctx) {
        fprintf(stderr, "ostgl_make_current: no GL context\n");
        return;
    }
    
    ctx = (TinyNspireContext *)gl_ctx->opaque;
    if (!ctx) {
        fprintf(stderr, "ostgl_make_current: no TinyNspire context\n");
        return;
    }
    
    /* Switch to the requested buffer */
    ctx->current_buffer = idx;
    gl_ctx->zb = (ZBuffer *)oscontext->zbs[idx];
}

/*
 * Resize the framebuffers
 */
void ostgl_resize(ostgl_context *oscontext, const int xsize,
                  const int ysize, void **framebuffers)
{
    int i;
    TinyNspireContext *ctx;
    GLContext *gl_ctx;
    
    if (!oscontext) return;
    
    gl_ctx = gl_get_context();
    if (!gl_ctx) return;
    
    ctx = (TinyNspireContext *)gl_ctx->opaque;
    if (!ctx) return;
    
    /* Ensure dimensions are aligned for Z-buffer */
    int aligned_xsize = xsize & ~3;
    int aligned_ysize = ysize & ~3;
    
    if (aligned_xsize != xsize || aligned_ysize != ysize) {
        fprintf(stderr, "ostgl_resize: dimensions adjusted from %dx%d to %dx%d\n",
                xsize, ysize, aligned_xsize, aligned_ysize);
    }
    
    /* Update framebuffer pointers */
    for (i = 0; i < oscontext->numbuffers; i++) {
        oscontext->framebuffers[i] = framebuffers[i];
        ctx->framebuffers[i] = framebuffers[i];
        
        /* Resize each Z-buffer */
        ZB_resize((ZBuffer *)oscontext->zbs[i], framebuffers[i], 
                  aligned_xsize, aligned_ysize);
    }
    
    ctx->xsize = aligned_xsize;
    ctx->ysize = aligned_ysize;
    oscontext->xsize = aligned_xsize;
    oscontext->ysize = aligned_ysize;
}

/*
 * Internal: resize viewport callback
 */
static int nspire_resize_viewport(GLContext *c, int *xsize_ptr, int *ysize_ptr)
{
    int xsize, ysize;
    TinyNspireContext *ctx;
    
    ctx = (TinyNspireContext *)c->opaque;
    
    xsize = *xsize_ptr;
    ysize = *ysize_ptr;
    
    /* Ensure alignment */
    xsize &= ~3;
    ysize &= ~3;
    
    if (xsize == 0 || ysize == 0) return -1;
    
    *xsize_ptr = xsize;
    *ysize_ptr = ysize;
    
    /* Z-buffer was already resized in ostgl_resize */
    return 0;
}

/*
 * Swap buffers - blit the current buffer to the screen
 * This is the main rendering function users will call
 */
#if NSPIRE_PROFILER
static void nspire_swap_buffers_internal(unsigned *scale_ticks,
                                         unsigned *blit_ticks)
#else
static void nspire_swap_buffers_internal(void)
#endif
{
    GLContext *gl_ctx;
    TinyNspireContext *ctx;
    unsigned short *src;
    unsigned short *dst;
#if NSPIRE_PROFILER
    unsigned t0, t1, t2;
#endif
    int x, y;

#if NSPIRE_PROFILER
    if (scale_ticks) *scale_ticks = 0;
    if (blit_ticks) *blit_ticks = 0;
#endif
    
    gl_ctx = gl_get_context();
    if (!gl_ctx) return;
    
    ctx = (TinyNspireContext *)gl_ctx->opaque;
    if (!ctx) return;

    src = (unsigned short *)ctx->framebuffers[ctx->current_buffer];

    if (ctx->xsize == NSPIRE_SCREEN_WIDTH &&
        ctx->ysize == NSPIRE_SCREEN_HEIGHT) {
#if NSPIRE_PROFILER
        t1 = nspire_timer_ticks();
#endif
        lcd_blit(src, ctx->screen_type);
#if NSPIRE_PROFILER
        t2 = nspire_timer_ticks();
        if (blit_ticks) *blit_ticks = nspire_timer_elapsed(t1, t2);
#endif
        return;
    }

#if NSPIRE_PROFILER
    t0 = nspire_timer_ticks();
#endif

    if (ctx->xsize == NSPIRE_SCREEN_WIDTH / 2 &&
        ctx->ysize == NSPIRE_SCREEN_HEIGHT / 2) {
        for (y = 0; y < ctx->ysize; y++) {
            unsigned short *dst0 = lcd_framebuffer + (y * 2) * NSPIRE_SCREEN_WIDTH;
            unsigned short *dst1 = dst0 + NSPIRE_SCREEN_WIDTH;

            for (x = 0; x < ctx->xsize; x++) {
                unsigned short pixel = src[y * ctx->xsize + x];
                int dx = x * 2;

                dst0[dx] = pixel;
                dst0[dx + 1] = pixel;
                dst1[dx] = pixel;
                dst1[dx + 1] = pixel;
            }
        }

#if NSPIRE_PROFILER
        t1 = nspire_timer_ticks();
#endif
        lcd_blit(lcd_framebuffer, ctx->screen_type);
#if NSPIRE_PROFILER
        t2 = nspire_timer_ticks();
        if (scale_ticks) *scale_ticks = nspire_timer_elapsed(t0, t1);
        if (blit_ticks) *blit_ticks = nspire_timer_elapsed(t1, t2);
#endif
        return;
    }
    
    dst = lcd_framebuffer;
    for (y = 0; y < NSPIRE_SCREEN_HEIGHT; y++) {
        int sy = y * ctx->ysize / NSPIRE_SCREEN_HEIGHT;

        for (x = 0; x < NSPIRE_SCREEN_WIDTH; x++) {
            int sx = x * ctx->xsize / NSPIRE_SCREEN_WIDTH;

            dst[y * NSPIRE_SCREEN_WIDTH + x] = src[sy * ctx->xsize + sx];
        }
    }

#if NSPIRE_PROFILER
    t1 = nspire_timer_ticks();
#endif
    lcd_blit(lcd_framebuffer, ctx->screen_type);
#if NSPIRE_PROFILER
    t2 = nspire_timer_ticks();
    if (scale_ticks) *scale_ticks = nspire_timer_elapsed(t0, t1);
    if (blit_ticks) *blit_ticks = nspire_timer_elapsed(t1, t2);
#endif
}

void nspire_swap_buffers(void)
{
#if NSPIRE_PROFILER
    nspire_swap_buffers_internal(NULL, NULL);
#else
    nspire_swap_buffers_internal();
#endif
}

#if NSPIRE_PROFILER
void nspire_swap_buffers_profiled(unsigned *scale_ticks,
                                  unsigned *blit_ticks)
{
    nspire_swap_buffers_internal(scale_ticks, blit_ticks);
}
#endif

/*
 * Get the current screen type
 */
scr_type_t nspire_get_screen_type(void)
{
    if (current_ctx) {
        return current_ctx->screen_type;
    }
    return lcd_type();
}

/*
 * Check if a specific key is pressed
 * Returns non-zero if pressed, 0 if not
 */
int nspire_is_key_pressed(t_key key)
{
    return isKeyPressed(key) ? 1 : 0;
}

/*
 * Check if any key is pressed
 */
int nspire_any_key_pressed(void)
{
    return any_key_pressed() ? 1 : 0;
}

/*
 * Wait for a key press (blocking)
 */
void nspire_wait_key(void)
{
    wait_key_pressed();
}

/*
 * Wait for all keys to be released
 */
void nspire_wait_no_key(void)
{
    wait_no_key_pressed();
}

/*
 * Sleep for specified milliseconds
 */
void nspire_msleep(unsigned int ms)
{
    msleep(ms);
}
