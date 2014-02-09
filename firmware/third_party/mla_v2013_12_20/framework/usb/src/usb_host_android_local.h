
#include <stdint.h>
#include <stdbool.h>

//If the user hasn't specified a timeout period, make one up for them
#ifndef ANDROID_DEVICE_ATTACH_TIMEOUT
    #define ANDROID_DEVICE_ATTACH_TIMEOUT 1500
#endif

uint8_t AndroidAppWrite_Pv2(void* handle, uint8_t* data, uint32_t size);
bool AndroidAppIsWriteComplete_Pv2(void* handle, uint8_t* errorCode, uint32_t* size);
uint8_t AndroidAppRead_Pv2(void* handle, uint8_t* data, uint32_t size);
bool AndroidAppIsReadComplete_Pv2(void* handle, uint8_t* errorCode, uint32_t* size);
void* AndroidInitialize_Pv2 ( uint8_t address, uint32_t flags, uint8_t clientDriverID );
void AndroidTasks_Pv2(void);
bool AndroidAppEventHandler_Pv2( uint8_t address, USB_EVENT event, void *data, uint32_t size );
bool AndroidAppDataEventHandler_Pv2( uint8_t address, USB_EVENT event, void *data, uint32_t size );
void AndroidAppStart_Pv2(void);
