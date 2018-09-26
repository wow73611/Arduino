#include "DHT.h"

#define DHTTYPE DHT22 // DHT 22 (AM2302), AM2321

DHT* dhtSensors[20];
// Declare what GPIO will be enabled 16,5,4,0,2,14,12,13,15,3,1,10,9
int enabled_pin_list[] = {5, 4, 0, 2};
const int enabled_pin_count = sizeof(enabled_pin_list) / sizeof(int);

void setup() {
  Serial.begin(115200);
  // Wait for serial to initialize.
  while (!Serial) { };

  Serial.println("GPIO Enabled Count:" + String(enabled_pin_count));
  for (int i = 0; i < enabled_pin_count; i++) {
    Serial.println("Enabled GPIO:" + String(enabled_pin_list[i]));
    dhtSensors[i] = new DHT(enabled_pin_list[i], DHTTYPE);
  }
  int pinCount = sizeof(dhtSensors) / sizeof(DHT);
  
  Serial.println("Running DHT Sensor... Count:" + String(pinCount));
  for (int pinIndex = 0; pinIndex < pinCount; pinIndex++) {
    dhtSensors[pinIndex]->begin();
  }
}

void check_dht(DHT &dht) {
  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float h = dht.readHumidity();
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();
  // Read temperature as Fahrenheit (isFahrenheit = true)
  float f = dht.readTemperature(true);

  // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(t) || isnan(f)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  // Compute heat index in Fahrenheit (the default)
  float hif = dht.computeHeatIndex(f, h);
  // Compute heat index in Celsius (isFahreheit = false)
  float hic = dht.computeHeatIndex(t, h, false);

  Serial.print("Humidity: ");
  Serial.print(h);
  Serial.print(" %\t");
  Serial.print("Temperature: ");
  Serial.print(t);
  Serial.print(" *C ");
  Serial.print(f);
  Serial.print(" *F\t");
  Serial.print("Heat index: ");
  Serial.print(hic);
  Serial.print(" *C ");
  Serial.print(hif);
  Serial.println(" *F");
}

void loop() {
  int pinCount = sizeof(dhtSensors) / sizeof(DHT);

  for (int i = 0; i < pinCount; i++) {
    Serial.println();
    Serial.println("DHT Sensor. GPIO:" + String(enabled_pin_list[i]));
    check_dht(*dhtSensors[i]);
  }
  delay(5000);
}

