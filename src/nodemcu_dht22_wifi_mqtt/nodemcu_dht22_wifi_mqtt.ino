#include "DHT.h"
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

#define DHTPIN 4       // what digital pin the DHT is conected to(GPIO4=D2)
#define DHTTYPE DHT22  // DHT 22 (AM2302), AM2321

// Replace with your network details.
#define WIFI_SSID "CloudCube_TPE_Office"
#define WIFI_PASS "cloudcube54729082"
 
#define MQTT_SERVER "192.168.123.158"
#define MQTT_PORT 1883
#define MQTT_USER "mqtt_username"
#define MQTT_PASS "mqtt_password"
#define MQTT_TOPIC "esp/dht22"
//const char* mqtt_topic="dht_topic";

DHT dht(DHTPIN, DHTTYPE);
//WiFiServer server(80);
WiFiClient espClient;
PubSubClient mqttClient(espClient);

long lastMsg = 0;

void setup() {
  Serial.begin(115200);
  Serial.setTimeout(2000);

  // Wait for serial to initialize.
  while (!Serial) { }
  setup_wifi();
   
  Serial.println("Running DHT Sensor ...");
  dht.begin();
  //Serial.println("Running ESP Web Server ...");
  //espServer.begin();
  
  mqttClient.setServer(MQTT_SERVER, MQTT_PORT);
  mqttClient.setCallback(callback);
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Reconnecting to wifi...");
    setup_wifi();
  }
  if (!mqttClient.connected()) {
    Serial.println("Reconnecting to MQTT ...");     
    setup_mqtt();
  }
  mqttClient.loop();

  // read DHT22 sensor every ? seconds
  long now = millis();
  if (now - lastMsg > 1000 * 10) {
    lastMsg = now;
    check_dht(dht);
  }
}

void setup_wifi() {
  delay(100);
  // We start by connecting to a WiFi network
  Serial.print("Connecting to wifi: " + String(WIFI_SSID));
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: "+WiFi.localIP());
}

// recieve Topic message handler , I don't implements it 
void callback(char* topic, byte* payload, unsigned int length) {  
}

void setup_mqtt() {
  // Loop until we're reconnected
  while (!mqttClient.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    //if you MQTT broker has clientID,username and password
    //please change following line to
    // if (client.connect(clientId,MQTT_USER,MQTT_PASS))
    if (mqttClient.connect(clientId.c_str())) {
      Serial.println("connected");
     //once connected to MQTT broker, subscribe command if any
      //mqttClient.subscribe(topic);
    } else {
      Serial.print("failed, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" try again in 3 seconds");
      delay(3000);
    }
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

  //publish sensor data to MQTT broker
  String payload = String(h)+","+String(t);
  char message[payload.length()];
  payload.toCharArray(message, payload.length());
  mqttClient.publish(MQTT_TOPIC, message);
  Serial.println("Publish to MQTT Topic: "+ String(MQTT_TOPIC));
  Serial.println(payload);
  Serial.println();
}
