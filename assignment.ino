//Firebase modules
#include <Arduino.h>
#include <Firebase_ESP_Client.h>
#define API_KEY "AIzaSyD3kkHqfGoEcl-njGv686743J9NfCHIrU4"
#define DATABASE_URL "https://airaware-d3ab4-default-rtdb.europe-west1.firebasedatabase.app/"
#define USER_EMAIL "meshwildias@gmail.com"
#define USER_PASS "MESHdias"

FirebaseData fbdo;      //
FirebaseAuth auth;      //
FirebaseConfig config;  //
FirebaseJson json;      //

unsigned long sendDataPrevMillis = 0;
int count = 0;
bool signupOk = false;

//WiFi
#include <WiFi.h>
#define SSID "MAD"
#define WIFI_PWD "88888888"

//DHT
#include "DHT.h"
#define DHTPIN 17
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

//PM2.5
#include "Adafruit_PM25AQI.h"
Adafruit_PM25AQI aqi = Adafruit_PM25AQI();

//Twilio
// #include "twilio.hpp"
// // Values from Twilio (find them on the dashboard)
// static const char *account_sid = "AC9782d8bc25daa6b8ed4a1c909d7733c3";
// static const char *auth_token = "c57120e3acc2476f3a6246a859848087";
// // Phone number should start with "+<countrycode>"
// static const char *from_number = "+447893938729";

// static char *to_number = "";

// char *getNumber() {
//   Firebase.RTDB.getString(&fbdo, "PhoneNumber") {
//     if (fbdo.dataTypeEnum() == firebase_rtdb_data_type_string) {
//       Serial.print(fbdo.to());
//     } else {
//       Serial.println(fbdo.errorReason());
//     }
//   }
// }

void setup() {
  Serial.begin(115200);

  //WiFi configuration
  WiFi.begin(SSID, WIFI_PWD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(300);
  }

  //Firebase configuration
  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASS;

  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
  Firebase.RTDB.setMaxRetry(&fbdo, 3);
  Firebase.RTDB.setMaxErrorQueue(&fbdo, 30);
  Firebase.RTDB.enableClassicRequest(&fbdo, true);

  fbdo.setBSSLBufferSize(4096, 1024);
  fbdo.setResponseSize(2048);

  //DHT initiation
  dht.begin();

  //PM2.5 sensor
  //Wait one second for sensor to boot up!
  delay(1000);
  aqi.begin_I2C();
}

int counter = 0;
int temperature = 0;
int humidity = 0;

void loop() {
  counter++;

  if (Firebase.ready()) {
    //DHT
    float h = dht.readHumidity();
    float t = dht.readTemperature();

    if (isnan(h) || isnan(t)) {
      Serial.println(F("Failed to read from DHT sensor!"));
      return;
    }

    //PM2.5
    PM25_AQI_Data data;

    //Temperature value insertion
    temperature += t;
    if (Firebase.RTDB.setInt(&fbdo, "SensorValues/Temperature", temperature / counter)) {
      Serial.println("Temperature value add successful");
    } else {
      Serial.println("Temperature failed! " + fbdo.errorReason());
    }

    //Humidity value insertion
    humidity += h;
    if (Firebase.RTDB.setInt(&fbdo, "SensorValues/Humidity", humidity / counter)) {
      Serial.println("Humidity value add successful");
    } else {
      Serial.println("Humidity failed! " + fbdo.errorReason());
    }

    //PM2.5 sensor (Standard values)
    aqi.read(&data);
    if (Firebase.RTDB.setInt(&fbdo, "SensorValues/ParticleSensor/StandardConcentration/PM10", data.pm10_standard) && Firebase.RTDB.setInt(&fbdo, "SensorValues/ParticleSensor/StandardConcentration/PM25", data.pm25_standard) && Firebase.RTDB.setInt(&fbdo, "SensorValues/ParticleSensor/StandardConcentration/PM100", data.pm100_standard)) {
      Serial.println("Particle sensor value registration successful");
    } else {
      Serial.println("Particle sensor standard values registration unsuccessful");
    }

    //(Environmental values)
    if (Firebase.RTDB.setInt(&fbdo, "SensorValues/ParticleSensor/EnvironmentalConcentration/PM10", data.pm10_env) && Firebase.RTDB.setInt(&fbdo, "SensorValues/ParticleSensor/EnvironmentalConcentration/PM25", data.pm25_env) && Firebase.RTDB.setInt(&fbdo, "SensorValues/ParticleSensor/EnvironmentalConcentration/PM100", data.pm100_env)) {
      Serial.println("Particle sensor environmental values registration successful");
    } else {
      Serial.println("Particle sensor environmental values registration unsuccessful");
    }

    //(Particle size)
    // if (Firebase.RTDB.setInt(&fbdo, "SensorValues/ParticleSensor/ParticleSize/3", data.particles_03um) && Firebase.RTDB.setInt(&fbdo, "SensorValues/ParticleSensor/ParticleSize/5", data.particles_05um) && Firebase.RTDB.setInt(&fbdo, "SensorValues/ParticleSensor/ParticleSize/10", data.particles_10um) && Firebase.RTDB.setInt(&fbdo, "SensorValues/ParticleSensor/ParticleSize/25um", data.particles_25um) && Firebase.RTDB.setInt(&fbdo, "SensorValues/ParticleSensor/ParticleSize/50um", data.particles_50um) && Firebase.RTDB.setInt(&fbdo, "SensorValues/ParticleSensor/ParticleSize/100um", data.particles_100um)) {
    //   Serial.println("Particle sensor particle size values registration successful");
    // } else {
    //   Serial.println("Particle sensor particle size values registration unsuccessful");
    // }
  }

  Serial.println();

  delay(10000);
}
