void breachedThreshold(int temperature, int humidity, int pm25) {
  String message = "";
  bool transmitMessage = false;
  int minTemperature = 0, maxTemperature = 30, minHumidity = 0, maxHumidity = 30;

  if (Firebase.RTDB.getString(&fbdo, "/Thresholds/Min_Humidity")) {
    if (fbdo.dataTypeEnum() == firebase_rtdb_data_type_integer)
      minHumidity = fbdo.to<int>();
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
      message += ", which has breached the threshold. Close the windows or turn on the heater. ";
    } else if (temperature > maxTemperature) {
      message += ", which has breached the threshold. Open the windows or turn off the heater. ";
    }
  }

  // Humidity block
  if (humidity < minHumidity || humidity > maxHumidity) {
    message += "Humidity is " + String(humidity) + "%";
    transmitMessage = true;
    if (humidity < minHumidity) {
      message += ", which has breached the threshold. Please open the window.";
    } else if (humidity > maxHumidity) {
      message += ", which has breached the threshold. Please open the window.";
    }
  }

  // PM2.5 block
  if (pm25 > 10) {
    message += "Particulate matter (i.e. 2.5μg) has reached to " + String(pm25) + " data points, which is of ";
    else if (pm25 > 10 && pm25 < 25)
      message += " moderate ";
    else if (pm25 >= 35 && pm25 < 55)
      message += " increased ";
    else
      message += " serious ";
    message += "concern. It is recommended to turn on your air purifier.";
  }

  unsigned long currentTime = millis();

  if (transmitMessage && currentTime - lastExecutionTime >= 60000) {



    String response;
    twilio->send_message(to_number, from_number, message.c_str(), response);
    lastExecutionTime = currentTime;
  }
}