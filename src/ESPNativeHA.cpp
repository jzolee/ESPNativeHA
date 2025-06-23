#include "ESPNativeHA.h"

// A konstruktor implementációja. Egyelőre nem csinál semmit.
ESPNativeHA::ESPNativeHA() {
}

// A begin() függvény implementációja.
void ESPNativeHA::begin(const char* deviceName, const char* haIpAddress) {
  // Elmentjük a kapott paramétereket a privát változókba.
  _deviceName = deviceName;
  _haIpAddress = haIpAddress;

  // Kiírunk egy üzenetet a Soros Monitorra, hogy lássuk, működik.
  Serial.println("ESPNativeHA Library Initialized!");
  Serial.print("Device Name: ");
  Serial.println(_deviceName);
  Serial.print("Home Assistant IP: ");
  Serial.println(_haIpAddress);
}