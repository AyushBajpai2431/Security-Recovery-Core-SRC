/**
 * Tamper-Resistant Logging System
 */

#ifndef LOGGING_H
#define LOGGING_H

#include <stdint.h>
#include <stdbool.h>

/* Log message (tamper-resistant) */
void src_log(const char *format, ...);

/* Initialize logging system */
bool logging_init(void);

/* Read log entries */
bool logging_read(uint8_t *buffer, size_t *size);

/* Clear logs (requires authentication) */
bool logging_clear(void);

#endif /* LOGGING_H */
