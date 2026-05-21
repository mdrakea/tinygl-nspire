/*
 * TI-Nspire example utilities.
 *
 * These helpers are intentionally kept out of TinyGL's public Nspire header.
 * They exist for demos and local diagnostics, not for the core renderer API.
 */

#ifndef TINYGL_NSPIRE_UTILS_H
#define TINYGL_NSPIRE_UTILS_H

#include <libndls.h>

#ifndef NSPIRE_PROFILER
#define NSPIRE_PROFILER 0
#endif

#ifdef __cplusplus
extern "C" {
#endif

scr_type_t nspire_get_screen_type(void);

int nspire_is_key_pressed(t_key key);
int nspire_any_key_pressed(void);
void nspire_wait_key(void);
void nspire_wait_no_key(void);

void nspire_msleep(unsigned int ms);
void nspire_breakpoint(void);

#if NSPIRE_PROFILER
void nspire_profiler_init(void);
void nspire_profiler_shutdown(void);

unsigned nspire_timer_ticks(void);
unsigned nspire_timer_elapsed(unsigned start, unsigned end);

void nspire_swap_buffers_profiled(unsigned *scale_ticks,
                                  unsigned *blit_ticks);
#endif

#ifdef __cplusplus
}
#endif

#endif /* TINYGL_NSPIRE_UTILS_H */
