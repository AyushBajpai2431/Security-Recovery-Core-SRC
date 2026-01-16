/**
 * Logging Implementation
 * Stores logs in tamper-resistant storage
 */

#include "logging.h"
#include "platform.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#define MAX_LOG_ENTRIES 1000
#define MAX_LOG_SIZE 256

typedef struct {
    uint32_t timestamp;
    char message[MAX_LOG_SIZE];
} log_entry_t;

static log_entry_t log_buffer[MAX_LOG_ENTRIES];
static uint32_t log_index = 0;
static bool logging_enabled = true;

bool logging_init(void) {
    memset(log_buffer, 0, sizeof(log_buffer));
    log_index = 0;
    return true;
}

void src_log(const char *format, ...) {
    if (!logging_enabled) {
        return;
    }
    
    if (log_index >= MAX_LOG_ENTRIES) {
        log_index = 0;  // Wrap around
    }
    
    log_entry_t *entry = &log_buffer[log_index];
    entry->timestamp = platform_get_timestamp();
    
    va_list args;
    va_start(args, format);
    vsnprintf(entry->message, MAX_LOG_SIZE, format, args);
    va_end(args);
    
    /* Also output to platform debug interface if available */
    platform_debug_log(entry->message);
    
    log_index++;
}

bool logging_read(uint8_t *buffer, size_t *size) {
    if (!buffer || !size) {
        return false;
    }
    
    /* Serialize log entries */
    size_t available = sizeof(log_buffer);
    if (*size < available) {
        return false;
    }
    
    memcpy(buffer, log_buffer, sizeof(log_buffer));
    *size = sizeof(log_buffer);
    return true;
}

bool logging_clear(void) {
    /* Requires authentication - platform-specific */
    if (!platform_authenticate()) {
        return false;
    }
    
    memset(log_buffer, 0, sizeof(log_buffer));
    log_index = 0;
    return true;
}
