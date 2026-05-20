/*
 * TinyGL TI-Nspire framebuffer presentation helpers.
 * https://www.hackspire.org/Libndls/
 */

#ifndef TINYGL_NSPIRE_H
#define TINYGL_NSPIRE_H

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Swap buffers - blit the current framebuffer to the LCD screen
 * Call this after rendering each frame
 */
void nspire_swap_buffers(void);

#ifdef __cplusplus
}
#endif

#endif /* TINYGL_NSPIRE_H */
