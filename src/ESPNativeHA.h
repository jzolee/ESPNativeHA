#ifndef ESP_NATIVE_HA_H
#define ESP_NATIVE_HA_H

#include <Arduino.h>

// Ez az "include guard". Biztosítja, hogy a fordító csak egyszer olvassa be ezt a fájlt.
class ESPNativeHA {
public:
  // Konstruktor: Ez a függvény fut le, amikor létrehozunk egy objektumot ebből az osztályból.
  ESPNativeHA();

  // Inicializáló függvény: Itt adjuk meg a legfontosabb paramétereket.
  void begin(const char* deviceName, const char* haIpAddress);

private:
  // Privát változók: Ezeket csak a könyvtáron belülről lehet elérni.
  String _deviceName;
  String _haIpAddress;
};

#endif // ESP_NATIVE_HA_H