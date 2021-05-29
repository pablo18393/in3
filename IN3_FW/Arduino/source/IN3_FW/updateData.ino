int updateData() {
  watchdogReload();
  GPRSISR();
  if (millis() - lastDebugUpdate > debugUpdatePeriod) {
    logln("[SENSORS] -> Current consumption is: " + String (currentConsumption) + " Amps");
    logln("[SENSORS] -> baby temperature: " + String(temperature[babyNTC]) + "ºC");
    logln("[SENSORS] -> Floor temperature: " + String(temperature[digitalTempSensor]) + "ºC");
    logln("[SENSORS] -> Humidity: " + String(humidity) + "%");
    lastDebugUpdate = millis();
  }
  if (millis() - lastGraphicSensorsUpdate > sensorsUpdatePeriod) {
    updateRoomSensor();
    if (page == advancedModePage || page == actuatorsProgressPage) {
      updateDisplaySensors();
      if (page == actuatorsProgressPage) {
        printStatus();
      }
    }
    lastGraphicSensorsUpdate = millis();
  }
  if ((page == mainMenuPage || page == advancedModePage) && !enableSet) {
    checkSetMessage();
  }
}

void updateDisplaySensors() {
  if (page == advancedModePage || (page == actuatorsProgressPage && controlTemperature)) {
    drawBabyTemperature();
    drawHumidity();
  }
  if (page == actuatorsProgressPage) {
    tft.setTextColor(COLOR_MENU_TEXT);
    if (controlTemperature) {
      float previousTemperaturePercentage = temperaturePercentage;
      if (displayProcessPercentage) {
        drawRightNumber(temperaturePercentage, tft.width() / 2, temperatureY);
      }
      temperaturePercentage = 100 - ((desiredSkinTemp - temperature[babyNTC]) * 100 / (desiredSkinTemp - temperatureAtStart));
      if (temperaturePercentage > 99) {
        temperaturePercentage = 100;
      }
      if (temperaturePercentage < 0) {
        temperaturePercentage = 0;
      }
      updateLoadingTemperatureBar(int(previousTemperaturePercentage), int(temperaturePercentage));
    }
    if (controlHumidity) {
      float previousHumidityPercentage = humidityPercentage;
      if (displayProcessPercentage) {
        drawRightNumber(humidityPercentage, tft.width() / 2, humidityY);
      }
      humidityPercentage = 100 - ((desiredRoomHum - humidity) * 100 / (desiredRoomHum - humidityAtStart));
      if (humidityPercentage > 99) {
        humidityPercentage = 100;
      }
      if (humidityPercentage < 0) {
        humidityPercentage = 0;
      }
      updateLoadingHumidityBar(int(previousHumidityPercentage), int(humidityPercentage));
    }
  }
}



void printStatus() {
  //log(millis() / 1000);
  //log(";");
  //log(desiredSkinTemp);
  //log(";");
  //log(maxHeaterTemp);
  //log(";");
  for (int i = 0; i < numTempSensors; i++) {
    //log(temperature[i]);
    //log(";");
  }
  //log(desiredHeaterTemp);
  //log(";");
  //log(humidity);
  //log(";");
  //log(desiredRoomHum);
  //log(";");
  //log(heaterPower);
  //log(";");
  //consumptionMeanSamples(String(PIDOutput[heaterNTC]));
}

void logln(String dataString) {
  log(String(millis()/1000) + ": " + dataString + '\r' + '\n');
}

void log(String dataString) {
  if (SDCard) {
    digitalWrite(SD_CS, LOW);
    digitalWrite(TFT_CS, HIGH);
    // if the file is available, write to it:
    File dataFile = SD.open(logFile, FILE_WRITE);
    if (dataFile) {
      dataFile.print(day());
      dataFile.print("/");
      dataFile.print(month());
      dataFile.print("/");
      dataFile.print(year());
      dataFile.print("-");
      dataFile.print(hour());
      dataFile.print(":");
      dataFile.print(minute());
      dataFile.print(":");
      dataFile.print(second());
      dataFile.print(";");
      dataFile.print(dataString);
      dataFile.close();
      // print to the serial port too:
    }
    digitalWrite(SD_CS, HIGH);
    digitalWrite(TFT_CS, LOW);
  }
  if (SerialDebug) {
    Serial4.print(dataString);
  }
}

void GPRSLocalLog() {
  if (SDCard) {
    digitalWrite(SD_CS, LOW);
    digitalWrite(TFT_CS, HIGH);
    String dataString;
    dataString += temperature[babyNTC];
    dataString += ";";
    dataString += humidity;
    dataString += ";";
    // if the file is available, write to it:
    File dataFile = SD.open(GPRSFile, FILE_WRITE);
    if (dataFile) {
      dataFile.print(day());
      dataFile.print("/");
      dataFile.print(month());
      dataFile.print("/");
      dataFile.print(year());
      dataFile.print("-");
      dataFile.print(hour());
      dataFile.print(":");
      dataFile.print(minute());
      dataFile.print(":");
      dataFile.print(second());
      dataFile.print(";");
      dataFile.println(dataString);
      dataFile.close();
      // print to the serial port too:
      //logln(dataString);
    }
    digitalWrite(SD_CS, HIGH);
    digitalWrite(TFT_CS, LOW);
  }
}
