/*
 * TinyGL TI-Nspire specific functions
 * https://www.hackspire.org/Libndls/
 */

#ifndef TINYGL_NSPIRE_H
#define TINYGL_NSPIRE_H

#include <libndls.h>

#ifndef NSPIRE_PROFILER
#define NSPIRE_PROFILER 0
#endif

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Swap buffers - blit the current framebuffer to the LCD screen
 * Call this after rendering each frame
 */
void nspire_swap_buffers(void);

#if NSPIRE_PROFILER
void nspire_swap_buffers_profiled(unsigned *scale_ticks,
                                  unsigned *blit_ticks);

/*
 * Low-overhead timer helpers for visual profiling.
 * Values are raw hardware timer ticks; use nspire_timer_elapsed() to subtract.
 */
unsigned nspire_timer_ticks(void);
unsigned nspire_timer_elapsed(unsigned start, unsigned end);
#endif

/*
 * Get the current screen type
 */
scr_type_t nspire_get_screen_type(void);

/*
 * Keyboard functions
 */

/* Check if a specific key is pressed */
int nspire_is_key_pressed(t_key key);

/* Check if any key is pressed */
int nspire_any_key_pressed(void);

/* Wait for a key press (blocking) */
void nspire_wait_key(void);

/* Wait for all keys to be released */
void nspire_wait_no_key(void);

/*
 * Time functions
 */

/* Sleep for specified milliseconds */
void nspire_msleep(unsigned int ms);

#ifdef __cplusplus
}
#endif

#endif /* TINYGL_NSPIRE_H */
