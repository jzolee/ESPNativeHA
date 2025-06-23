#include <Arduino.h>
#include "ESPNativeHA.h" // Beillesztjük a saját, új könyvtárunkat!

// Létrehozunk egy objektumot (egy "példányt") a könyvtárunkból.
// Nevezzük el pl. 'ha_client'-nek (Home Assistant kliens).
ESPNativeHA ha_client;

void setup() {
  // Elindítjuk a soros kommunikációt a debug üzenetekhez.
  Serial.begin(115200);
  delay(1000); // Adunk egy kis időt a soros monitornak, hogy elinduljon.

  // Meghívjuk a könyvtárunk inicializáló függvényét a saját adatainkkal.
  // Ezeket később majd a config fájlból fogjuk olvasni!
  ha_client.begin("my-esp32-device", "192.168.1.100");
}

void loop() {
  // Egyelőre a fő ciklus nem csinál semmit.
}