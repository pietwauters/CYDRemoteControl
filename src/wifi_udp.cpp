#include "wifi_udp.h"
#include "esp_wifi.h"
#include <AsyncUDP.h>
#include <Preferences.h>
#include <WiFi.h>

// WiFi credentials - update these with your access point details
const char *WIFI_SSID = "Piste_001";
const char *WIFI_PASSWORD = "01041967";

// UDP configuration
const char *UDP_TARGET_IP = "192.168.4.1"; // Update with your target IP
const uint16_t UDP_TARGET_PORT = 1234;     // Update with your target port

// Static IP address for UDP target (parsed once)
static IPAddress targetIP;
static bool targetIPInitialized = false;

// AsyncUDP instance
AsyncUDP udp;

// WiFi connection status
bool wifiConnected = false;
static unsigned long lastWiFiStateChange = 0;
const unsigned long WIFI_STATE_DEBOUNCE =
    3000; // 3 seconds debounce for state changes

unsigned long lastReconnectAttempt = 0;
const unsigned long reconnectInterval =
    5000; // Try to reconnect every 5 seconds

// WiFi event handler for connection monitoring
void WiFiEvent(WiFiEvent_t event) {
  unsigned long currentMillis = millis();

  switch (event) {
  case ARDUINO_EVENT_WIFI_STA_CONNECTED:
    Serial.printf("[%lu] WiFi Event: Connected to AP\n", currentMillis);
    break;
  case ARDUINO_EVENT_WIFI_STA_GOT_IP:
    // Allow immediate connection (good news is fast)
    Serial.printf("[%lu] WiFi Event: Got IP address: %s\n", currentMillis,
                  WiFi.localIP().toString().c_str());
    Serial.printf("[%lu]   wifiConnected: %d -> true, lastChange: %lu\n",
                  currentMillis, wifiConnected, lastWiFiStateChange);
    wifiConnected = true;
    lastWiFiStateChange = currentMillis;
    break;
  case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
    Serial.printf("[%lu] WiFi Event: Disconnected, wifiConnected=%d, "
                  "timeSinceChange=%lu\n",
                  currentMillis, wifiConnected,
                  currentMillis - lastWiFiStateChange);
    // Debounce disconnection - only change state after debounce period
    if (wifiConnected &&
        currentMillis - lastWiFiStateChange >= WIFI_STATE_DEBOUNCE) {
      Serial.printf("[%lu]   ACCEPTED: wifiConnected: true -> false\n",
                    currentMillis);
      wifiConnected = false;
      lastWiFiStateChange = currentMillis;
    } else {
      Serial.printf("[%lu]   IGNORED: debounce active\n", currentMillis);
    }
    break;
  default:
    break;
  }
}

// Initialize WiFi connection
void initWiFi() {
  Serial.println("WiFi: Initializing...");

  // Parse the target IP address once
  if (!targetIPInitialized) {
    if (targetIP.fromString(UDP_TARGET_IP)) {
      targetIPInitialized = true;
      Serial.printf("UDP: Target IP configured: %s:%d\n", UDP_TARGET_IP,
                    UDP_TARGET_PORT);
    } else {
      Serial.println("UDP: ERROR - Invalid target IP address!");
    }
  }

  // Register event handler for async monitoring
  WiFi.onEvent(WiFiEvent);

  // Set WiFi mode to both station and access point (AP+STA)
  WiFi.mode(WIFI_AP_STA);

  // Configure and start SoftAP with fixed SSID/password
  // Use a different subnet for the SoftAP to avoid conflicting with the
  // station network (prevents 192.168.4.1 target IP colliding with local AP)
  IPAddress apIP(192, 168, 5, 1);
  IPAddress apGateway(192, 168, 5, 1);
  IPAddress apSubnet(255, 255, 255, 0);
  WiFi.softAPConfig(apIP, apGateway, apSubnet);
  WiFi.softAP("RemoteControl", "01041967");

  // Start station connection
  Preferences networkpreferences;
  networkpreferences.begin("network");

  String storedSSID;
  storedSSID = networkpreferences.getString("Piste", WIFI_SSID);
  networkpreferences.end();

  // Use DHCP for station by default (avoid forcing a static IP here)
  Serial.println("WiFi: Using DHCP for station interface");
  WiFi.begin(storedSSID.c_str(), WIFI_PASSWORD);

  Serial.print("WiFi: Connecting to ");
  Serial.println(storedSSID.c_str());
  // This puts the radio into sleep unless sending UDP packets
  WiFi.setSleep(true);
  esp_wifi_set_ps(WIFI_PS_MIN_MODEM);
}

void SetPiste(int PisteNr) {
  char strPiste[32];
  sprintf(strPiste, "Piste_%.3d", PisteNr);
  Preferences networkpreferences;
  networkpreferences.begin("network", false);
  networkpreferences.putString("Piste", strPiste);
  networkpreferences.end();
  WiFi.disconnect();
  WiFi.begin(strPiste, WIFI_PASSWORD);
}

// Check WiFi status and attempt reconnection if needed
// Returns false if connection is lost, true otherwise
bool checkWiFiConnection() {
  // If connected, return true
  if (wifiConnected) {
    return true;
  }

  unsigned long currentMillis = millis();

  // If not connected and enough time has passed since last attempt
  if (currentMillis - lastReconnectAttempt >= reconnectInterval) {
    lastReconnectAttempt = currentMillis;

    if (WiFi.status() != WL_CONNECTED) {
      Serial.println("WiFi: Connection lost. Attempting to reconnect...");
      // TEST: Comment out WiFi.begin to see if it's causing the flickers
      // WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    }
  }

  // Return actual connection status
  return wifiConnected;
}

// Send a 32-bit word via UDP
bool sendUDP32(uint32_t value) {
  if (!wifiConnected) {
    Serial.println("UDP: Cannot send - WiFi not connected");
    return false;
  }

  if (!targetIPInitialized) {
    Serial.println("UDP: Cannot send - Target IP not initialized");
    return false;
  }

  // Create a packet with the 32-bit value (little-endian)
  uint8_t packet[4];
  packet[0] = (value >> 0) & 0xFF;
  packet[1] = (value >> 8) & 0xFF;
  packet[2] = (value >> 16) & 0xFF;
  packet[3] = (value >> 24) & 0xFF;

  // Debug info
  Serial.printf("UDP: Sending 0x%08X to %s:%d\n", value, UDP_TARGET_IP,
                UDP_TARGET_PORT);
  Serial.printf("UDP: From IP: %s\n", WiFi.localIP().toString().c_str());

  // Send the packet
  size_t sent = udp.writeTo(packet, sizeof(packet), targetIP, UDP_TARGET_PORT);

  if (sent == sizeof(packet)) {
    Serial.printf("UDP: Successfully sent %d bytes\n", sent);
    return true;
  } else {
    Serial.printf("UDP: Failed to send packet (sent %d of %d bytes)\n", sent,
                  sizeof(packet));
    return false;
  }
}

// Send multiple 32-bit words via UDP
bool sendUDP32Array(uint32_t *values, size_t count) {
  if (!wifiConnected) {
    Serial.println("UDP: Cannot send - WiFi not connected");
    return false;
  }

  if (!targetIPInitialized) {
    Serial.println("UDP: Cannot send - Target IP not initialized");
    return false;
  }

  // Create a packet with all 32-bit values
  size_t packetSize = count * 4;
  uint8_t *packet = new uint8_t[packetSize];

  // Pack all values into the packet (little-endian)
  for (size_t i = 0; i < count; i++) {
    packet[i * 4 + 0] = (values[i] >> 0) & 0xFF;
    packet[i * 4 + 1] = (values[i] >> 8) & 0xFF;
    packet[i * 4 + 2] = (values[i] >> 16) & 0xFF;
    packet[i * 4 + 3] = (values[i] >> 24) & 0xFF;
  }

  // Send the packet
  size_t sent = udp.writeTo(packet, packetSize, targetIP, UDP_TARGET_PORT);

  delete[] packet;

  if (sent == packetSize) {
    Serial.printf("UDP: Sent %d words (%d bytes) to %s:%d\n", count, sent,
                  UDP_TARGET_IP, UDP_TARGET_PORT);
    return true;
  } else {
    Serial.printf("UDP: Failed to send packet (sent %d of %d bytes)\n", sent,
                  packetSize);
    return false;
  }
}
