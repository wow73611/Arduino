#include <OneWire.h>

// DS18S20 Temperature chip i/o
OneWire ds(4);

void setup(void) {
  // initialize inputs/outputs
  // start serial port
  Serial.begin(115200);
}

void loop(void) {
  byte i;
  byte present = 0;
  byte data[12];
  byte addr[8];
  float celsius, fahrenheit;
    
  //ds.reset_search();
  if ( !ds.search(addr)) {
      Serial.println("No more addresses.\n");
      ds.reset_search();
      delay(250);
      return;
  }

  Serial.print("ROM=");
  for( i = 0; i < 8; i++) {
    Serial.print(addr[i], HEX);
    Serial.print(" ");
  }

  if ( OneWire::crc8( addr, 7) != addr[7]) {
      Serial.print("CRC is not valid!\n");
      return;
  }

  switch (addr[0]) {
    case 0x10:
      Serial.print("Device is a DS18S20 family device.\n");
      break;
    case 0x28:
      Serial.print("Device is a DS18B20 family device.\n");
      break;
    case 0x22:
      Serial.print("Device is a DS1822 family device.\n");
      break;
    default:
      Serial.print("Device family is not recognized: 0x");
      Serial.println(addr[0], HEX);
      return;
  }

  ds.reset();
  ds.select(addr);
  ds.write(0x44,1);         // start conversion, with parasite power on at the end

  delay(1000);     // maybe 750ms is enough, maybe not
  // we might do a ds.depower() here, but the reset will take care of it.

  present = ds.reset();
  ds.select(addr);    
  ds.write(0xBE);         // Read Scratchpad

  Serial.print("P=");
  Serial.print(present, HEX);
  Serial.print(" ");
  for ( i = 0; i < 9; i++) {           // we need 9 bytes
    data[i] = ds.read();
    Serial.print(data[i], HEX);
    Serial.print(" ");
  }
  Serial.print(" CRC=");
  Serial.print(OneWire::crc8(data, 8), HEX);
  Serial.println();

  // Convert the data to actual temperature
  unsigned int raw = (data[1] << 8) | data[0];
  
  if (addr[0] == 0x10) {
    raw = raw << 3; // 9 bit resolution default
    if (data[7] == 0x10){
      raw = (raw & 0xFFF0) + 12 - data[6];
    }
  } else {
    byte cfg = (data[4] & 0x60);
    // at lower res, the low bits are undefined, so let's zero them
    if (cfg == 0x00) raw = raw & ~7; // 9 bit resolution, 93.75 ms
    else if (cfg == 0x20) raw = raw & ~3; // 10 bit res, 187.5 ms
    else if (cfg == 0x40) raw = raw & ~1; // 11 bit res, 375 ms
    // default is 12 bit resolution, 750 ms conversion time
  }

  celsius = (float)raw / 16.0;
  fahrenheit = celsius * 1.8 + 32.0;
  Serial.print("Temperature = ");
  Serial.print(celsius);
  Serial.print(" Celsius, ");
  Serial.print(fahrenheit);
  Serial.println(" Fahrenheit");
  Serial.println();
}