/**
 * @file ioConfig.c
 * @brief Configure serial ports on beaglebone
 *
 * @author Jeremy Ruhland <jeremy ( a t ) goopypanther.org>
 *
 * @license GPL 3.0
 * @version1.0
 * @since Sep 14, 2015
 */


#include "includes.h"

// Defines
#define TARGET_RELEASE "3.8.13-bone70"
#define SLOT_PATH "/sys/devices/bone_capemgr.8/slots"

// Function prototypes
void ioConfigInit(void);

// Static variables

/**
 * Enable UART 2 & 4
 */
void ioConfigInit(void) {
    struct utsname unameData;
    int32_t isBeagleBone;

    uname(&unameData); // Get kernel data

    isBeagleBone = strncmp(TARGET_RELEASE, unameData.release, sizeof(TARGET_RELEASE));

    // Check if kernel release is matched
    if (isBeagleBone == 0) {
        system("echo BB-UART2 >" SLOT_PATH);
        system("echo BB-UART4 >" SLOT_PATH);
    } else {}

}