#include <TinyGsmClient.h>
#include <HardwareSerial.h>
#include "modem.h"

HardwareSerial SerialAT(1);
TinyGsm modem(SerialAT);
TinyGsmClient client(modem);

void initModemSerial() {
  SerialAT.begin(9600, SERIAL_8N1, 26, 27); // RX, TX pins
}
