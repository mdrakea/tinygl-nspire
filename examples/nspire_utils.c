#include "nspire_utils.h"

#include <stdbool.h>
#include <GL/nspire.h>

#if NSPIRE_PROFILER
static bool profiler_timer_initialized = false;
static unsigned profiler_timer_saved_load;
static unsigned profiler_timer_saved_control;
static unsigned profiler_timer_saved_bgload;

void nspire_profiler_init(void)
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

void nspire_profiler_shutdown(void)
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

void nspire_swap_buffers_profiled(unsigned *scale_ticks,
                                  unsigned *blit_ticks)
{
    unsigned start;
    unsigned end;

    if (scale_ticks) *scale_ticks = 0;
    if (blit_ticks) *blit_ticks = 0;

    start = nspire_timer_ticks();
    nspire_swap_buffers();
    end = nspire_timer_ticks();

    /* Core TinyGL exposes only presentation, so demos time it as one unit. */
    if (blit_ticks) *blit_ticks = nspire_timer_elapsed(start, end);
}
#endif

scr_type_t nspire_get_screen_type(void)
{
    return lcd_type();
}

int nspire_is_key_pressed(t_key key)
{
    return isKeyPressed(key) ? 1 : 0;
}

int nspire_any_key_pressed(void)
{
    return any_key_pressed() ? 1 : 0;
}

void nspire_wait_key(void)
{
    wait_key_pressed();
}

void nspire_wait_no_key(void)
{
    wait_no_key_pressed();
}

void nspire_msleep(unsigned int ms)
{
    msleep(ms);
}

void nspire_breakpoint(void)
{
#ifdef DEBUG
    bkpt();
#endif
}
