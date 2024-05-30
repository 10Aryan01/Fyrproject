#include <ESP8266HTTPClient.h>

#include <ESP8266WiFi.h>
#include <WiFiClientSecureBearSSL.h>
#include <ArduinoJson.h>

const char* rootCACertificate = R"EOF(
-----BEGIN CERTIFICATE-----
MIIF+TCCBOGgAwIBAgIQCC7ds4hedMm30HiPYuFzZDANBgkqhkiG9w0BAQsFADA8
MQswCQYDVQQGEwJVUzEPMA0GA1UEChMGQW1hem9uMRwwGgYDVQQDExNBbWF6b24g
UlNBIDIwNDggTTAyMB4XDTI0MDUyMTAwMDAwMFoXDTI1MDYxOTIzNTk1OVowMTEv
MC0GA1UEAwwmKi5leGVjdXRlLWFwaS5ldS1ub3J0aC0xLmFtYXpvbmF3cy5jb20w
ggEiMA0GCSqGSIb3DQEBAQUAA4IBDwAwggEKAoIBAQDp1Hwpg8CWNscV7kUGh0rN
Wr8Mz43eDk65oHDBjO2ayRx0nXVIBHvjYRPWCguREfv2yGdhoD6htkf8RnfoQGC6
uZILTm7oVakUMUQO3+76wvUr3eivoidOz37sfa+BwM0J9VnxmnYafcSaXMm760wf
7Mqwoblyd98zrnXJST4nJ086X7ThtxbMHGiYEvKKP1kqObMjBzBcRFMSDQkKKDGd
0u/0SWLY1N1KUtv/LGj8RrvYAsKDoedrjMdY6I+yp5T3T0dO3ZY8ERGjLcYDdpwK
byXNoGO6dCOCpTORSaqJsZZ8uGszuqjZrG8mR5+ATyOyPTUXnR63gSwHH7lUv9dx
AgMBAAGjggMAMIIC/DAfBgNVHSMEGDAWgBTAMVLNWlDDgnx0cc7L6Zz5euuC4jAd
BgNVHQ4EFgQUpipOW7Wvmf1v2Hwfp29yMZtwplAwMQYDVR0RBCowKIImKi5leGVj
dXRlLWFwaS5ldS1ub3J0aC0xLmFtYXpvbmF3cy5jb20wEwYDVR0gBAwwCjAIBgZn
gQwBAgEwDgYDVR0PAQH/BAQDAgWgMB0GA1UdJQQWMBQGCCsGAQUFBwMBBggrBgEF
BQcDAjA7BgNVHR8ENDAyMDCgLqAshipodHRwOi8vY3JsLnIybTAyLmFtYXpvbnRy
dXN0LmNvbS9yMm0wMi5jcmwwdQYIKwYBBQUHAQEEaTBnMC0GCCsGAQUFBzABhiFo
dHRwOi8vb2NzcC5yMm0wMi5hbWF6b250cnVzdC5jb20wNgYIKwYBBQUHMAKGKmh0
dHA6Ly9jcnQucjJtMDIuYW1hem9udHJ1c3QuY29tL3IybTAyLmNlcjAMBgNVHRMB
Af8EAjAAMIIBfwYKKwYBBAHWeQIEAgSCAW8EggFrAWkAdwBOdaMnXJoQwzhbbNTf
P1LrHfDgjhuNacCx+mSxYpo53wAAAY+YhwWZAAAEAwBIMEYCIQDP0cGAO2GAMdqQ
B4/xPbqMs3H0IVjRtGVGajxcI9griQIhANyzej68w9HD6AmC5t6wxE13eQZOx1nT
Qm3wOGqOIzTbAHUAfVkeEuF4KnscYWd8Xv340IdcFKBOlZ65Ay/ZDowuebgAAAGP
mIcFmQAABAMARjBEAiBDlRnzeCFr4wETY2N9naL655LV0YVvqngIo+Y1MLUm/QIg
Ys4PM3ph8awz74VQMkB+rau90hLvRiwTbnUyZWzb058AdwDm0jFjQHeMwRBBBtdx
uc7B0kD2loSG+7qHMh39HjeOUAAAAY+YhwWwAAAEAwBIMEYCIQDdRb1QqgcwzIpX
RtoF1/MNgKA4Ryn15p1Q8We+Cd4r8QIhAIMf8hQFhAxFY+Bb7BU3vw9nnGb88RPz
orECu+uDL9wtMA0GCSqGSIb3DQEBCwUAA4IBAQA9Ur+14Ya7Wefnvh80/l4+qi+i
0A7yYzs3VqM3L5TESe3jHqSYLoe0VZjIbJZuX4gOpu+PLn3MLCxJwdI4iOLGmNjt
dddmqJA7kiriS0kUDppQRdccBw1O3ViAnpUp139Q7ISa64iDX1BX+Z7R9R4QB6XT
iPWIrFEIbDoO6ff+Vaqg0cH7kBsKxmf+/SI+krRAnn1WPPMJZ/kI0jnOLLui6HQ9
zUm25OB6zEXyWvIMhqvAyO/5JfZry6P4bhrabVzb3at/yN0QDESMFT+QxGxzrw1Y
YXJQRKfeacd8+T5nG07dKYWtCL2u0e7DnVBJeMO7WLpu9+fh2NKWL3aYHAud
-----END CERTIFICATE-----
)EOF";

const char* WIFI_SSID = "Crazyhead";
const char* WIFI_PASSWORD = "995464450556";

const char* host = "9ycawm1u61.execute-api.eu-north-1.amazonaws.com";
const int httpsPort = 443;

const char* url = "/V3/";

const long interval = 5000;

unsigned long lastMillis = 0;

BearSSL::WiFiClientSecure client;


void connectWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  Serial.println("Connecting to WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Connected to WiFi");
}

void setClock() {
  configTime(3 * 3600, 0, "pool.ntp.org", "time.nist.gov");

  Serial.print("Waiting for NTP time sync: ");
  time_t now = time(nullptr);
  while (now < 8 * 3600 * 2) {
    delay(500);
    Serial.print(".");
    now = time(nullptr);
  }
  Serial.println("");
  struct tm timeinfo;
  gmtime_r(&now, &timeinfo);
  Serial.print("Current time: ");
  Serial.print(asctime(&timeinfo));
}





void publishMessage(const String& data) {
  Serial.println("The incoming data: ");
  Serial.println(data);
  
  BearSSL::X509List list(rootCACertificate);
  client.setTrustAnchors(&list);

  HTTPClient http;

  DynamicJsonDocument doc(1024);
  DeserializationError error = deserializeJson(doc, data);
  if (error) {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.f_str());
    client.stop();  
    return;
  }

  const String  latitude = doc["latitude"];
  const String  longitude = doc["longitude"];
  const String speed = doc["speed"];

  Serial.printf("Lat",latitude);
  Serial.printf("Longitude",longitude);
  Serial.printf("speed",speed);

  Serial.println("Attempting to connect to server...");
  if (http.begin(client, "https://9ycawm1u61.execute-api.eu-north-1.amazonaws.com/V3/")) {
    Serial.println("Connected to server");

    String body = "{\"latitude\":\"" + latitude +  
               "\",\"longitude\":\"" + longitude +  
               "\",\"carNumber\":\"AS2343R533\"," + 
               "\"violatorName\":\"Sudarshan Bhuyan\"," + 
               "\"speedData\":\"" + speed + " km/h\"," + 
               "\"challanAmount\":\"5000 /-\"," + 
               "\"minorDetails\":\"Speeding in a school zone\"," + 
               "\"phone_number\":\"+916001012501\"," + 
               "\"email\":\"rrouchangogoi@gmail.com\"," + 
               "\"Challan_no\":\"MTPCCC180019354\"," + 
               "\"Payment_Status\":\"Pending\"," + 
               "\"Vehicle_name\":\"Tata Tigor\"," + 
               "\"Vehicle_type\":\"Light motor vehicle\"}";

    http.setTimeout(60000);
    int httpCode = http.POST(body);
    http.setTimeout(60000);

    Serial.printf("This is the httpcode %d\n ",httpCode);
    if (httpCode > 0) {
      Serial.printf("HTTP response code: %d\n", httpCode);
      if (httpCode == HTTP_CODE_OK) {
        String payload = http.getString();
        Serial.println("Response:");
        Serial.println(payload);
      }
    } else {
      Serial.printf("HTTP request failed: %s\n", http.errorToString(httpCode).c_str());
    }

    http.end();
  } else {
    Serial.println("Failed to connect to server");
  }
}


void setup() {
  Serial.begin(9600);
  connectWiFi();
  setClock();
}

bool messagePublished = false;
unsigned long lastMessageTime = 0;

void loop() {
  
  if (!messagePublished && Serial.available() > 0 && (millis() - lastMessageTime > interval)) {
    String data = Serial.readStringUntil('\n');
    Serial.printf("\nRaw data : ",data);
    if (!data.isEmpty() && data != "\n" && data != "\r\n") {
      publishMessage(data);
      messagePublished = true;
      lastMessageTime = millis();
    }
  }
  delay(100);
}


