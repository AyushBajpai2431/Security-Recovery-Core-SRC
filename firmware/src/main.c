/**
 * Security Recovery Core - Main Entry Point
 * 
 * This is the firmware entry point that runs before BIOS/UEFI
 */

#include "recovery_core.h"
#include "platform.h"
#include <stdint.h>

/**
 * Firmware entry point
 * Called by bootloader or directly from reset vector
 */
void main(void) {
    /* Initialize platform first */
    platform_init();
    
    /* Initialize Recovery Core */
    src_init();
    
    /* Main loop */
    while (1) {
        src_main_loop();
        
        /* Platform-specific delay/yield */
        platform_delay_ms(100);
    }
}
