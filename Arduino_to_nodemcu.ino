#include <SoftwareSerial.h>
#include <TinyGPS++.h>
#include <LiquidCrystal_I2C.h>
#include <ArduinoJson.h>

// Define GPS serial connection pins
#define GPS_TX_PIN 3
#define GPS_RX_PIN 4

// Define variables for GPS data and the GPS library
SoftwareSerial gpsSerial(GPS_TX_PIN, GPS_RX_PIN);
TinyGPSPlus gps;

LiquidCrystal_I2C lcd(0x27, 16, 2);

// JEC Gate (latitude and longitude)
const float YOUR_LATITUDE = 26.745951519770458;
const float YOUR_LONGITUDE = 94.24827269898316;

// Define region parameters
float REGION_RADIUS = 300.0; // Radius of the circular region in meters
float APPROACH_DISTANCE = 400.0; // Distance to show approach message in meters
const float SPEED_LIMIT = 30; // Speed limit in km/h

// Track region entry time
unsigned long regionEntryTime = 0;

// Track if data has been sent
bool dataSent = false;

// Track if 7-second threshold has been crossed
bool sevenSecondsCrossed = false;

// Track if vehicle is approaching the region
bool approachingRegion = false;

// Track if vehicle is inside the region
bool insideRegion = false;

// Track the highest speed recorded inside the region
float highestSpeed = 0.0;

void setup() {
    // Initialize serial communication for debugging
    Serial.begin(9600);

    // Initialize software serial connection with GPS module
    gpsSerial.begin(9600);

    // Wait for GPS module to initialize
    delay(1000);

    // Initialize LCD
    lcd.init();
    lcd.backlight();
    lcd.clear();
    pinMode(13, OUTPUT);
}

void loop() {
    // Read GPS data
    while (gpsSerial.available() > 0) {
        if (gps.encode(gpsSerial.read())) {
            if (gps.location.isValid()) {
                // Parse latitude and longitude from GPS data
                float currentLatitude = gps.location.lat();
                float currentLongitude = gps.location.lng();
                float distanceToRegion = calculateDistance(currentLatitude, currentLongitude, YOUR_LATITUDE, YOUR_LONGITUDE);
                float speed = gps.speed.kmph();

                // Print current location and distance from predefined location
                // Serial.print("Current Latitude: ");
                // Serial.println(currentLatitude, 6);
                // Serial.print("Current Longitude: ");
                // Serial.println(currentLongitude, 6);
                // Serial.print("Distance to Your Location: ");
                // Serial.print(distanceToRegion);
                // Serial.println(" meters\n");
                // Serial.print("Current Speed: ");
                // Serial.print(speed);
                // Serial.println(" km/h");

                if (distanceToRegion <= REGION_RADIUS) {
                    if (!insideRegion) {
                        lcd.clear();
                        insideRegion = true;
                        highestSpeed = 0.0; // Reset highest speed when entering the region
                    }
                    lcd.setCursor(0, 0);
                    lcd.print("Inside REGION!");
                    lcd.setCursor(0, 1);
                    lcd.print("Speed: ");
                    lcd.print(speed);
                    lcd.print(" km/h");

                    digitalWrite(13, HIGH);
                    if (!dataSent) {
                        if (regionEntryTime == 0) {
                            regionEntryTime = millis();
                        } else if ((millis() - regionEntryTime) >= 7000) {
                            sevenSecondsCrossed = true;
                        }
                    }

                    // Update highest speed
                    if (speed > highestSpeed) {
                        highestSpeed = speed;
                    }

                    if (highestSpeed > SPEED_LIMIT && sevenSecondsCrossed && !dataSent) {
                        sendGPSData(currentLatitude, currentLongitude, highestSpeed);
                        dataSent = true;
                        lcd.setCursor(0, 1);
                        lcd.print("Challan Imposed!");
                        delay(4000);
                        lcd.clear();
                        lcd.setCursor(0, 0);
                        lcd.print("Inside REGION!");
                        lcd.setCursor(0, 1);
                        lcd.print("Speed: ");
                        lcd.print(speed);
                        lcd.print(" km/h");
                    }
                } else {
                    if (insideRegion) {
                        lcd.clear();
                        insideRegion = false;
                        regionEntryTime = 0;
                        sevenSecondsCrossed = false;
                        dataSent = false;
                        highestSpeed = 0.0; // Reset highest speed when leaving the region
                    }
                    if (distanceToRegion <= APPROACH_DISTANCE && !approachingRegion) {
                        lcd.clear();
                        lcd.setCursor(0, 0);
                        lcd.print("Approaching...");
                        lcd.setCursor(0, 1);
                        lcd.print("Speed Limit: ");
                        lcd.print(SPEED_LIMIT);
                        approachingRegion = true;
                        delay(500);
                    } else if (distanceToRegion > APPROACH_DISTANCE) {
                        approachingRegion = false;
                        lcd.clear();
                        lcd.setCursor(0, 0);
                        lcd.print("Not in Region");
                        lcd.setCursor(0, 1);
                        lcd.print("Speed: ");
                        lcd.print(speed);
                        lcd.print(" km/h");
                    }
                }
            } else {
                // Serial.println("Invalid GPS data");
                lcd.clear();
                lcd.setCursor(0, 0);
                lcd.print("Not Available!");
            }
        }
    }
}

// Function to calculate distance between two points using Haversine formula
float calculateDistance(float lat1, float lon1, float lat2, float lon2) {
    // Convert latitude and longitude from degrees to radians
    lat1 = radians(lat1);
    lon1 = radians(lon1);
    lat2 = radians(lat2);
    lon2 = radians(lon2);

    // Haversine formula
    float dlon = lon2 - lon1;
    float dlat = lat2 - lat1;
    float a = pow(sin(dlat / 2), 2) + cos(lat1) * cos(lat2) * pow(sin(dlon / 2), 2);
    float c = 2 * atan2(sqrt(a), sqrt(1 - a));
    float distance = 6371000 * c; // Radius of the Earth in meters

    return distance;
}

void sendGPSData(float latitude, float longitude, float speed) {
    String speedStr = String(speed, 2);
    String latitudeStr = String(latitude, 6);
    String longitudeStr = String(longitude, 6);

    // Create JSON object to store GPS data
    StaticJsonDocument<200> jsonDoc;
    JsonObject root = jsonDoc.to<JsonObject>();
    root["latitude"] = latitudeStr;
    root["longitude"] = longitudeStr;
    root["speed"] = speedStr;

    // Serialize JSON object to string
    char jsonBuffer[200];
    serializeJson(root, jsonBuffer);

    // Send JSON data to NodeMCU
    Serial.println(jsonBuffer);
}
