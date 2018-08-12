void processPage() {
  byte  numWords = 2;
  temperatureAtStart = temperature[cornerNTC];
  processPercentage = 0;
  page = 1;
  tft.setTextSize(1);
  print_text = 0;
  rectangles = numWords;
  drawGraphicInterface();
  drawHeading();
  printLoadingBar();
  print_text = 1; //cambiar en todos
  tft.setTextColor(COLOR_MENU_TEXT);
  if (language) {
    tft.drawCentreString("Temperatura", tft.width() / 2, tft.height() / 5 - 4, 4);
  }
  else {
    tft.drawCentreString("Temperature", tft.width() / 2, tft.height() / 5 - 4, 4);
  }
  tft.drawCentreString("%", tft.width() / 2 + 14, temperatureY, 4);
  tft.setTextColor(COLOR_WARNING_TEXT);
  drawStop();
  state_blink = 1;
  while (!digitalRead(pulse));
  analogWrite(FAN1, fanSpeed * 2.55);
  analogWrite(FAN2, fanSpeed * 2.55);
  analogWrite(FAN3, fanSpeed * 2.55);
  if (temperatureAtStart > temperature[cornerNTC]) {
    temperatureAtStart = temperature[cornerNTC];
  }
  startPID();
  while (1) {
    updateData();
    if (temperature[cornerNTC] < desiredIn3Temp) {
      heatUp();
    }
    else {
      analogWrite(HEATER, 0);
    }
    if (digitalRead(pulse)) {
      last_pulsed = millis();
      delay(100);
    }
    else {
      delay(50);
      while (!digitalRead(pulse)) {
        updateData();
        if (digitalRead(pulse) == 0) {
          back_mode();
        }
      }
      delay(50);
    }
    if (millis() - blinking > 1000) {
      blinking = millis();
      state_blink = !state_blink;
      if (state_blink) {
        tft.setTextColor(ILI9341_ORANGE);
      }
      else {
        tft.setTextColor(COLOR_MENU);
        blinking += 400;
      }
      drawStop();
    }
  }
}

void heatUp() {
  if (temperature[heaterNTC] < heaterLimitTemp) {
    analogWrite(HEATER, 200);
  }
  else {
    analogWrite(HEATER, 0);
  }
}

