#include <PubSubClient.h>
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>
#include "env.h"


const char WIFI_SSID[] = "realme C3";
const char WIFI_PASSWORD[] = "1234567890";


const char THINGNAME[] = "challan_esp_8266";

// MQTT broker host address from AWS
const char MQTT_HOST[] = "a86jsgdgk88wy-ats.iot.eu-north-1.amazonaws.com";

// MQTT topics
const char AWS_IOT_PUBLISH_TOPIC[] = "esp8266/pub";
const char AWS_IOT_SUBSCRIBE_TOPIC[] = "esp8266/sub";


const long interval = 5000;

const int8_t TIME_ZONE = -5;

// Last time message was sent
unsigned long lastMillis = 0;


WiFiClientSecure net;


BearSSL::X509List cert(cacert);


BearSSL::X509List client_crt(client_cert);


BearSSL::PrivateKey key(privkey);


PubSubClient client(net);

// Function to connect to NTP server and set time
void NTPConnect() {
  // Set time using SNTP
  Serial.print("Setting time using SNTP");
  configTime(TIME_ZONE * 3600, 0, "pool.ntp.org", "time.nist.gov");
  time_t now = time(nullptr);
  while (now < 1510592825) { // January 13, 2018
    delay(500);
    Serial.print(".");
    now = time(nullptr);
  }
  Serial.println("done!");
}

void messageReceived(char *topic, byte *payload, unsigned int length) {
  Serial.print("Received [");
  Serial.print(topic);
  Serial.print("]: ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}


void connectAWS() {
  delay(3000);
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  Serial.println(String("Attempting to connect to SSID: ") + String(WIFI_SSID));

  // Wait for WiFi connection
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(1000);
  }

  NTPConnect();


  net.setTrustAnchors(&cert);
  net.setClientRSACert(&client_crt, &key);


  client.setServer(MQTT_HOST, 8883);
  client.setCallback(messageReceived);

  Serial.println("Connecting to AWS IoT");

  while (!client.connect(THINGNAME)) {
    Serial.print(".");
    delay(1000);
  }


  if (!client.connected()) {
    Serial.println("AWS IoT Timeout!");
    return;
  }


  client.subscribe(AWS_IOT_SUBSCRIBE_TOPIC);
  Serial.println("AWS IoT Connected!");
}

void publishMessage() {
  const char* body = "{\"latitude\":\"26.747394\",\"longitude\":\"94.246704\",\"carNumber\":\"AS2343R533\",\"violatorName\":\"Rahul Rouchan Gogoi\",\"speedData\":\"100 km/h\",\"challanAmount\":\"5000 /-\",\"minorDetails\":\"'Expired registration','Illegal parking','Speeding in a school zone',\",\"phone_number\":\"+916001012501\",\"email\":\"rrouchangogoi@gmail.com\",\"Challan_no\":\"MTPCCC180019354\",\"Payment_Status\":\"Pending\",\"Vehicle_name\":\"Tata Tigor\",\"Vehicle_type\":\"Light motor vehicle\"}";
  client.publish(AWS_IOT_PUBLISH_TOPIC, body);
}


void setup() {
  Serial.begin(9600);
  connectAWS();
  
}

bool messagePublished = false;

void loop() {
  if (!messagePublished && millis() - lastMillis > interval) {
    lastMillis = millis();
    if (client.connected()) {
      publishMessage();
      messagePublished = true; // Set the flag to true once message is published
    }
  }
  client.loop();
}
