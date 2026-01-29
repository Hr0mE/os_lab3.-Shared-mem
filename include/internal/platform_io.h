#pragma once

/**
 * Poll stdin for input.
 *
 * timeout_ms:
 *   < 0  — block
 *   = 0  — non-blocking
 *   > 0  — timeout in milliseconds
 *
 * Returns:
 *   1 — input available
 *   0 — timeout
 *  -1 — error
 */
int platform_poll_stdin(int timeout_ms);

