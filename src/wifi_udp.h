#ifndef WIFI_UDP_H
#define WIFI_UDP_H

#include <Arduino.h>
#include <stddef.h>
#include <stdint.h>

// WiFi credentials - update these with your access point details
extern const char *WIFI_SSID;
extern const char *WIFI_PASSWORD;

// UDP configuration
extern const char *UDP_TARGET_IP;
extern const uint16_t UDP_TARGET_PORT;

// WiFi connection status
extern bool wifiConnected;

// Initialize WiFi connection
void initWiFi();

// Check WiFi status and attempt reconnection if needed
// Returns false if connection is lost, true otherwise
bool checkWiFiConnection();

// C linkage for functions called from C files (ui_events.c)
#ifdef __cplusplus
extern "C" {
#endif

// Send a 32-bit word via UDP
bool sendUDP32(uint32_t value);

// Send multiple 32-bit words via UDP
bool sendUDP32Array(uint32_t *values, size_t count);

#ifdef __cplusplus
}
#endif

#endif // WIFI_UDP_H
