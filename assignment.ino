#include <string>
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
#include "twilio.hpp"
// Values from Twilio (find them on the dashboard)
static const char *account_sid = "AC8107c6642757d0a153882680fdec311f";
static const char *auth_token = "c57120e3acc2476f3a6246a859848087";
static const char *from_number = "+447401056606";
static char *to_number = "+447865076487";
unsigned long lastExecutionTime = 0;

Twilio *twilio = new Twilio(account_sid, auth_token);

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

void breachedThreshold(int temperature, int humidity, int pm25) {
  Serial.println(String(humidity));

  String message = "";
  bool transmitMessage = false;
  int minTemperature = 0, maxTemperature = 30, minHumidity = 0, maxHumidity = 30;

  if (Firebase.RTDB.getString(&fbdo, "/Thresholds/Min_Humidity")) {
    if (fbdo.dataTypeEnum() == firebase_rtdb_data_type_integer) {
      minHumidity = fbdo.to<int>();
    }
    else
      Serial.println(fbdo.errorReason());
  }
  if (Firebase.RTDB.getString(&fbdo, "/Thresholds/Max_Humidity")) {
    if (fbdo.dataTypeEnum() == firebase_rtdb_data_type_integer)
      maxHumidity = fbdo.to<int>();
    else
      Serial.println(fbdo.errorReason());
  }
  if (Firebase.RTDB.getString(&fbdo, "/Thresholds/Min_Temperature")) {
    if (fbdo.dataTypeEnum() == firebase_rtdb_data_type_integer)
      minTemperature = fbdo.to<int>();
    else
      Serial.println(fbdo.errorReason());
  }
  if (Firebase.RTDB.getString(&fbdo, "/Thresholds/Max_Temperature")) {
    if (fbdo.dataTypeEnum() == firebase_rtdb_data_type_integer)
      maxTemperature = fbdo.to<int>();
    else
      Serial.println(fbdo.errorReason());
  }

  // Temperature block
  if (temperature < minTemperature || temperature > maxTemperature) {
    message += "Temperature is " + String(temperature) + "°C";
    transmitMessage = true;
    if (temperature < minTemperature) {
      message += ", which has breached the lower threshold. Close the windows or turn on the heater. ";
    } else if (temperature > maxTemperature) {
      message += ", which has breached the upper threshold. Open the windows or turn off the heater. ";
    }
  }

  // Humidity block
  if (humidity < minHumidity || humidity > maxHumidity) {
    message += "Humidity is " + String(humidity) + "%";
    transmitMessage = true;
    if (humidity < minHumidity) {
      message += ", which has breached the lower threshold. Please open the window.";
    } else if (humidity > maxHumidity) {
      message += ", which has breached the upper threshold. Please open the window.";
    }
  }

  // PM2.5 block
  if (pm25 > 10) {
    message += "Particulate matter (i.e. 2.5μg) has reached " + String(pm25) + " data points, ";
    if (pm25 >= 10 && pm25 < 25)
      message += "which is of moderate concern. ";
    else if (pm25 >= 25 && pm25 < 35)
      message += "which is of increased concern. ";
    else
      message += "which is of serious concern. ";
    message += "It is recommended to turn on your air purifier.";
    transmitMessage = true;
  }

  unsigned long currentTime = millis();

  if (transmitMessage && currentTime - lastExecutionTime >= 60000) {
    if (Firebase.RTDB.getString(&fbdo, "ClientData/phoneNumber")) {
      if (fbdo.dataTypeEnum() == firebase_rtdb_data_type_string) {
        from_number = fbdo.to<String>().c_str();
      } else {
        Serial.println(fbdo.errorReason());
      }
    } else {
      Serial.println("Failed to get phone number from Firebase");
    }
    String response;
    Serial.println(message);
    twilio->send_message(to_number, from_number, message.c_str(), response);
    lastExecutionTime = currentTime;
  }
}

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

    //Read the data from the sensor
    aqi.read(&data);

    //(Environmental values)
    if (Firebase.RTDB.setInt(&fbdo, "SensorValues/PM_25", data.pm25_env)) {
      Serial.println("Particle sensor environmental values registration successful");
    } else {
      Serial.println("Particle sensor environmental values registration unsuccessful");
    }

    breachedThreshold(t, h, data.pm25_env);
  }

  Serial.println();

  delay(10000);
}
