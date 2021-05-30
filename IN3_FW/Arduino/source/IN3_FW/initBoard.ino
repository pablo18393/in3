void initBoard() {
  watchdogInit();
  initDebug();
  initEEPROM();
  initHardware();
  //initSD();
  initInterrupts();
  initTimers();
}

void initDebug() {
  Serial4.begin(115200);
  logln("in3ator debug uart, version " + String (FWversion) + ", SN: " + String (serialNumber));
}

void initInterrupts() {
  attachInterrupt(PWR_ALERT, powerAlertISR, RISING);
  attachInterrupt(ENC_SWITCH, encSwitchISR, FALLING);
  attachInterrupt(ENC_A, encoderISR, CHANGE);
  attachInterrupt(ENC_B, encoderISR, CHANGE);
}

void initTimers() {
  encoderTimer.pause();
  encoderTimer.setPeriod(peripheralsISRPeriod); // in microseconds
  encoderTimer.setChannel1Mode(TIMER_OUTPUT_COMPARE);
  encoderTimer.setCompare(TIMER_CH1, 1);  // Interrupt 1 count after each update
  encoderTimer.attachCompare1Interrupt(peripheralsISR);
  nvic_irq_set_priority(NVIC_TIMER8_CC, 15);
  nvic_irq_set_priority(NVIC_USART1 , 13);
  nvic_irq_set_priority(NVIC_UART4 , 14);
  encoderTimer.refresh();
  encoderTimer.resume();

  humidifierMaxPWM = humidifierTimer.getOverflow();
  screenBackLightMaxPWM = humidifierTimer.getOverflow();
}
