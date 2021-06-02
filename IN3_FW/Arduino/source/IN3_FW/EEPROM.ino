
//EEPROM variables
#define EEPROM_firstTurnOn 0
#define EEPROM_autoLock 1
#define EEPROM_language 2
#define EEPROM_maxHeaterPower 3
#define EEPROM_UARTDebug 5
#define EEPROM_diffHumidity 10
#define EEPROM_diffTemperature 20
#define EEPROM_usedGenericMosfet 40
#define EEPROM_checkStatus 100

bool firstTurnOn;

void initEEPROM() {
  EEPROM.PageBase0 = 0x801F000;
  EEPROM.PageBase1 = 0x801F800;
  EEPROM.PageSize  = 0x400;
  EEPROM.init();
  if (EEPROM.read(EEPROM_checkStatus)) {
    EEPROM.write(EEPROM_checkStatus, 0);
    delay(30);
    if (EEPROM.read(EEPROM_checkStatus) != 0) {
      EEPROM.format();
    }
  }
  else {
    EEPROM.write(EEPROM_checkStatus, 1);
    delay(30);
    if (EEPROM.read(EEPROM_checkStatus) != 1) {
      EEPROM.format();
    }
  }
  firstTurnOn=EEPROM.read(EEPROM_firstTurnOn);
  if (firstTurnOn) { //firstTimePowerOn
    EEPROM.format();
    for (int i = 0; i <= 253; i++) {
      EEPROM.write(i, 0);
    }
    loadStandardValues();
    logln("[FLASH] -> First turn on, loading standard values");
  }
  else {
    logln("[FLASH] -> Loading variables stored in flash");
    recapVariables();
  }
}

void loadStandardValues() {
  autoLock = standardAutoLock;
  EEPROM.write(EEPROM_autoLock, autoLock);
  maxHeaterPower = standardmaxHeaterPower;
  EEPROM.write(EEPROM_maxHeaterPower, maxHeaterPower);
  UARTDebug = standardUARTDebug;
  EEPROM.write(EEPROM_UARTDebug, UARTDebug);
}

void recapVariables() {
  autoLock = EEPROM.read(EEPROM_autoLock);
  language = EEPROM.read(EEPROM_language);
  diffTemperature[babyNTC] = EEPROM.read(EEPROM_diffTemperature);
  if (diffTemperature[babyNTC] > 1000) {
    diffTemperature[babyNTC] -= 65535;
  }
  diffTemperature[babyNTC] /= 10;
  diffHumidity = EEPROM.read(EEPROM_diffHumidity);
  if (diffHumidity > 1000) {
    diffHumidity -= 65535;
  }
  maxHeaterPower = EEPROM.read(EEPROM_maxHeaterPower);
  UARTDebug = EEPROM.read(EEPROM_UARTDebug);
}

long EEPROMReadLong(int p_address)
{
  int lowByte = EEPROM.read(p_address);
  int highByte = EEPROM.read(p_address + 1);
  return ((lowByte << 0) & 0xFFFF) + ((highByte << 16) & 0xFFFF0000);
}
void EEPROMWriteLong(int p_address, long p_value)
{
  int lowByte = ((p_value >> 0) & 0xFFFFFFFF);
  int highByte = ((p_value >> 16) & 0xFFFFFFFF);

  EEPROM.write(p_address, lowByte);
  EEPROM.write(p_address + 1, highByte);
}
