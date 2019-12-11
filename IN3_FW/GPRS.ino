//post content variables
#define defaultPost 0
#define jaundiceLEDON 1
#define jaundiceLEDOFF 2
#define actuatorsModeON 3
#define actuatorsModeOFF 4
#define pulseFound 5
#define pulseLost 6
#define standByMode 7
#define addLocation 7
#define removeLocation 8

String user = "admin@admin.com";
String password = "admin";
String server = "pub.scar.io";
char GPRSRXBuffer[1024];

String req[] = {
  "POST /In3/public/api/v1/auth/login?c=1 HTTP/1.1\n",
  "Host: " + server + "\n",
  "Content-Type: application/x-www-form-urlencoded\n",
  "Content-Length: 46\n",
  "Connection: close\n",
  "\n",
  "{\"email\":\"" + user + "\",\"password\":\"" + password + "\"}\n"
};

String req2[] = {
  "POST /In3/public/api/v1/session?c=1 HTTP/1.1\n",
  "Host: " + server + "\n",
  "Content-Type: application/x-www-form-urlencoded\n",
  "Authorization: Bearer \n",
  "Content-Length: 46\n",
  "Connection: close\n\n",
  "\n",
  ""
};

int len = 0;
long GPRSTimeOut = 60000; //in millisecs

struct GPRSstruct {
  bool firstPost;
  bool postSN;
  bool postBabyTemp;
  bool postHeaterTemp;
  bool postBoardTemp1;
  bool postBoardTemp2;
  bool postBoardTemp3;
  bool postHumidity;
  bool postLongitud;
  bool postLatitud;
  bool postJaundicePower;
  bool postBPM;
  bool postIBI;
  bool postRPD;
  bool postRSSI;
  bool postCurrentConsumption;
  bool postHeaterPower;
  bool postComment;
  int sendPeriod;
  long lastSent;
  char buffer[1024];
  int charToRead;
  int bufferPos;
  int bufferWritePos;
  String latitud;
  String longitud;
  String localDayTime;
  String localHourTime;
  String comment;
  String RSSI;
  bool readLatitud;
  bool readLongitud;
  bool readLocalDayTime;
  bool readLocalHourTime;
  String token;
  String reply;
  long lastEvent;
  bool processSuccess;
  bool readToken;
  bool enable;
  bool initVars;
  bool powerUp;
  bool connect;
  bool connectionStatus;
  bool timeOut;
  byte process;
  long processTime;
  long packetSentenceTime;
  bool post;
  bool location;
  byte postProcess;
};

int GPRSsequence = 0;

struct GPRSstruct GPRS;

void initGPRS()
{
  Serial.begin(115200);
  Serial1.begin(115200);
  setPostVariables(defaultPost, "");
  GPRS.sendPeriod = 10800; //in secs
  GPRS.postBabyTemp = 1;
  GPRS.postHumidity = 1;
  GPRS.powerUp = 1;
}

void GPRSHandler() {
  if (GPRS.powerUp) {
    GPRSPowerUp();
  }
  if (GPRS.location) {
    getLocation();
  }
  if (GPRS.connect) {
    GPRSStablishConnection();
  }
  if (GPRS.post) {
    GPRSPost();
  }
  readGPRSData();
  GPRSStatusHandler();
}

void GPRSStatusHandler() {
  if (GPRS.process) {
    if (GPRS.powerUp || GPRS.connect || GPRS.post || GPRS.location) {
      if (millis() - GPRS.processTime > GPRSTimeOut) {
        logln("timeOut" + String(GPRS.powerUp) + String(GPRS.connect) + String(GPRS.post) + String(GPRS.location) + String(GPRS.process));
        GPRS.timeOut = 0;
        GPRS.process = 0;
        GPRS.post = 0;
        GPRS.connect = 0;
        GPRS.location = 0;
        GPRS.powerUp = 1;
        logln("powering module down...");
        Serial1.print("AT+CPOWD=1\n");
        GPRS.packetSentenceTime = millis();
        GPRS.processTime = millis();
      }
    }
  }
  if (!GPRS.powerUp && !GPRS.connect && !GPRS.post) {
    if (!GPRS.connectionStatus) {
      GPRS.powerUp = 1;
    }
    if (millis() - GPRS.lastSent > GPRS.sendPeriod * 1000) {
      GPRS.connect = 1;
    }
  }
  if (!GPRS.firstPost && GPRS.connectionStatus) {
    GPRS.location = 1;
    setPostVariables(addLocation, "first post");
  }
}

void getLocation() {
  switch (GPRS.process) {
    case 0:
      GPRS.processTime = millis();
      GPRS.latitud = "";
      GPRS.longitud = "";
      GPRS.localDayTime = "";
      GPRS.localHourTime = "";
      GPRS.processSuccess = 1;
      Serial1.print("AT+CGATT=1\n");
      GPRS.process++;
      break;
    case 1:
      checkSerial("OK", "ERROR");
      break;
    case 2:
      Serial1.print("AT+SAPBR=3,1,\"CONTYPE\",\"GPRS\"\n");
      GPRS.process++;
      break;
    case 3:
      checkSerial("OK", "ERROR");
      break;
    case 4:
      Serial1.print("AT+SAPBR=3,1,\"APN\",\"TM\"\n");
      GPRS.process++;
      break;
    case 5:
      checkSerial("OK", "ERROR");
      break;
    case 6:
      Serial1.print("AT+SAPBR=1,1\n");
      GPRS.process++;
      break;
    case 7:
      checkSerial("OK", "ERROR");
      break;
    case 8:
      Serial1.print("AT+SAPBR=2,1\n");
      GPRS.process++;
      break;
    case 9:
      checkSerial("OK", "ERROR");
      break;
    case 10:
      Serial1.print("AT+CIPGSMLOC=1,1\n");
      GPRS.process++;
      break;
    case 11:
      checkSerial("OK", "ERROR");
      break;
    case 12:
      if (!GPRS.processSuccess) {
        GPRS.powerUp = 1;
      }
      else {
        GPRS.firstPost = 1;
        GPRS.post = 1;
      }
      GPRS.process = 0;
      GPRS.location = 0;
      GPRS.readLongitud = 0;
      GPRS.readLatitud = 0;
      break;
  }
}

void GPRSPowerUp() {
  switch (GPRS.process) {
    case 0:
      if (millis() - GPRS.packetSentenceTime > 2000) {
        GPRS.processTime = millis();
        digitalWrite(GPRS_PWRKEY, LOW);
        GPRS.processSuccess = 1;
        GPRS.process++;
        GPRS.packetSentenceTime = millis();
        logln("powering up GPRS");
      }
      break;
    case 1:
      if (millis() - GPRS.packetSentenceTime > 2000) {
        digitalWrite(GPRS_PWRKEY, HIGH);
        GPRS.packetSentenceTime = millis();
        GPRS.process++;
        logln("GPRS powered");
      }
      break;
    case 2:
      if (millis() - GPRS.packetSentenceTime > 8000) {
        Serial1.print("AT\n");
        GPRS.process++;
        GPRS.packetSentenceTime = millis();
        logln("Sending AT command");
      }
      break;
    case 3:
      if (millis() - GPRS.packetSentenceTime > 1000) {
        Serial1.print("AT\n");
        GPRS.packetSentenceTime = millis();
      }
      checkSerial("OK", "ERROR");
      break;
    case 4:
      if (GPRS.processSuccess) {
        GPRS.powerUp = 0;
        GPRS.connect = 1;
        logln("Power up success");
      }
      GPRS.process = 0;
      break;
  }
}

void GPRSStablishConnection() {
  switch (GPRS.process) {
    case 0:
      GPRS.processTime = millis();
      GPRS.processSuccess = 1;
      Serial1.write("AT+CFUN=1\n");
      GPRS.packetSentenceTime = millis();
      GPRS.process++;
      break;
    case 1:
      checkSerial("OK", "ERROR");
      break;
    case 2:
      if (millis() - GPRS.packetSentenceTime > 4000) {
        Serial1.write("AT+CPIN?\n");
        GPRS.packetSentenceTime = millis();
        GPRS.process++;
      }
      break;
    case 3:
      checkSerial("OK", "ERROR");
      break;
    case 4:
      if (millis() - GPRS.packetSentenceTime > 4000) {
        Serial1.write("AT+CSTT=\"TM\",\"\",\"\"\n\n");
        GPRS.process++;
      }
      break;
    case 5:
      checkSerial("OK", "ERROR");
      break;
    case 6:
      Serial1.write("AT+CIICR\n");
      GPRS.process++;
      break;
    case 7:
      checkSerial("OK", "ERROR");
      break;
    case 8:
      Serial1.write("AT+CIFSR\n");
      GPRS.process++;
      break;
    case 9:
      Serial1.write("AT\n");
      GPRS.process++;
      break;
    case 10:
      checkSerial("OK", "ERROR");
      break;
    case 11:
      Serial1.write("AT+CSQ\n");
      GPRS.process++;
      break;
    case 12:
      GPRS.RSSI = checkSerial(",", "ERROR");
      if (GPRS.RSSI.length()) {
        if (GPRS.RSSI.length() == 10) {
          GPRS.RSSI = String(GPRS.RSSI.charAt(7)) + String(GPRS.RSSI.charAt(8));
        }
        else {
          GPRS.RSSI = String(GPRS.RSSI.charAt(7));
        }
      }
      break;
    case 13:
      if (GPRS.processSuccess) {
        GPRS.connectionStatus = 1;
      }
      else {
        GPRS.powerUp = 1;
      }
      GPRS.connect = 0;
      GPRS.process = 0;
      if (GPRS.firstPost) {
        GPRS.post = 1;
      }
      break;
  }
}

void GPRSPost() {
  switch (GPRS.process) {
    case 0:
      GPRS.processSuccess = 1;
      GPRS.lastSent = millis();
      GPRS.processTime = millis();
      if (GPRSLoadVariables()) {
        logln("Starting POST process");
        Serial1.print("AT+CIPSTART=\"TCP\",\"" + server + "\",80\n");
        GPRS.process++;
        GPRSLocalLog(); //log GPRS data in SD module
      }
      else {
        logln("Wrong post value");
      }
      break;
    case 1:
      checkSerial("CONNECT OK", "ERROR");
      break;
    case 2:
      len = 0;
      for (int i = 0; i < 7; i++) {
        len += req[i].length();
      }
      Serial1.print("AT+CIPSEND=" + String(len - 1) + "\n");
      GPRS.process++;
      GPRS.packetSentenceTime = millis();
      break;
    case 3:
      if (millis() - GPRS.packetSentenceTime > 200) {
        Serial1.print(req[GPRSsequence]);
        GPRSsequence++;
        if (GPRSsequence == 7) {
          GPRS.process++;
          GPRSsequence = 0;
        }
        GPRS.packetSentenceTime = millis();
      }
      break;
    case 4:
      checkSerial("SEND OK", "\n\n");
      break;
    case 5:
      checkSerial("CLOSED", "\n\n");
      break;
    case 6:
      if (GPRSsequence == 0) {
        GPRS.token.trim();
        req2[3] = "Authorization: Bearer " + GPRS.token + "\n";
        req2[4] = "Content-Length: " + String(req2[7].length()) + "\n";
        len = 0;
      }
      len += req2[GPRSsequence].length();
      GPRSsequence++;
      if (GPRSsequence == 8) {
        Serial1.print("AT+CIPSTART=\"TCP\",\"" + server + "\",80\n");
        GPRS.process++;
        GPRSsequence = 0;
      }
      GPRS.packetSentenceTime = millis();
      break;
    case 7:
      checkSerial("CONNECT OK", "ERROR");
      break;
    case 8:
      Serial1.print("AT+CIPSEND=" + String(len - 1) + "\n");
      GPRS.packetSentenceTime = millis();
      GPRS.process++;
      break;
    case 9:
      if (millis() - GPRS.packetSentenceTime > 200) {
        if (GPRSsequence == 3) {
          for (int i = 0; i < req2[3].length(); i++) {
            Serial1.print(req2[3][i]);
          }
        }
        else {
          Serial1.print(req2[GPRSsequence]);
        }
        GPRSsequence++;
        if (GPRSsequence == 8) {
          GPRS.process++;
          GPRSsequence = 0;
        }
        GPRS.packetSentenceTime = millis();
      }
      break;
    case 10:
      checkSerial("CLOSED", "\n\n");
      break;
    case 11:
      Serial1.print("AT+CIPSHUT\n");
      GPRS.process++;
      break;
    case 12:
      checkSerial("SHUT OK", "ERROR");
      break;
    case 13:
      logln("GPRS POST SUCCESS");
      GPRS.post = 0;
      GPRS.process = 0;
      setPostVariables(removeLocation, "");
      break;
  }
}

void readGPRSData() {
  if (GPRS.enable) {
    while (Serial1.available()) {
      if (GPRS.bufferWritePos > 1023) {
        GPRS.bufferWritePos -= 1024;
        logln("Buffer overflow");
      }
      GPRS.buffer[GPRS.bufferWritePos] = Serial1.read();
      if (GPRS.post) {
        if (!GPRS.readToken) {
          if (GPRS.buffer[GPRS.bufferWritePos] == char('y')) {
            if (GPRS.buffer[GPRS.bufferWritePos - 1] == char('e')) {
              GPRS.readToken = 1;
              GPRS.token = "\n";
              GPRS.token += "ey";
            }
          }
        }
        else {
          if (GPRS.buffer[GPRS.bufferWritePos] == '\n') {
            GPRS.readToken = 0;
          }
          else {
            GPRS.token += GPRS.buffer[GPRS.bufferWritePos];
          }
        }
      }
      else if (GPRS.location) {
        if (GPRS.process == 11) {
          if (GPRS.buffer[GPRS.bufferWritePos] == char(',') ) {
            if (GPRS.buffer[GPRS.bufferWritePos - 2] != char('=')) {
              if (GPRS.longitud == "") {
                GPRS.readLongitud = 1;
              }
              else if (GPRS.latitud == "") {
                GPRS.readLongitud = 0;
                GPRS.readLatitud = 1;
              }
              else if (GPRS.localDayTime == "") {
                GPRS.readLocalDayTime = 1;
                GPRS.readLatitud = 0;
              }
              else {
                GPRS.readLocalDayTime = 0;
                GPRS.readLocalHourTime = 1;
              }
            }
          }
          else {
            if (GPRS.readLatitud) {
              GPRS.latitud += GPRS.buffer[GPRS.bufferWritePos];
            }
            if (GPRS.readLongitud) {
              GPRS.longitud += GPRS.buffer[GPRS.bufferWritePos];
            }
            if (GPRS.readLocalDayTime) {
              GPRS.localDayTime += GPRS.buffer[GPRS.bufferWritePos];
            }
            if (GPRS.readLocalHourTime) {
              if (GPRS.buffer[GPRS.bufferWritePos] == '\n') {
                GPRS.readLocalHourTime = 0;
                setTime((int(GPRS.localHourTime.charAt(0)) - 48) * 10 + (int(GPRS.localHourTime.charAt(1)) - 48), (int(GPRS.localHourTime.charAt(3)) - 48) * 10 + (int(GPRS.localHourTime.charAt(4)) - 48), (int(GPRS.localHourTime.charAt(6)) - 48) * 10 + (int(GPRS.localHourTime.charAt(7)) - 48), (int(GPRS.localDayTime.charAt(8)) - 48) * 10 + (int(GPRS.localDayTime.charAt(9)) - 48), (int(GPRS.localDayTime.charAt(5)) - 48) * 10 + (int(GPRS.localDayTime.charAt(6)) - 48), 2000 + (int(GPRS.localDayTime.charAt(2)) - 48) * 10 + (int(GPRS.localDayTime.charAt(3)) - 48)); // setTime(hr,min,sec,day,month,yr); // Another way to set
                logln("Local time: " + GPRS.localDayTime + "," + GPRS.localHourTime);
              }
              else {
                GPRS.localHourTime += GPRS.buffer[GPRS.bufferWritePos];
              }
            }
          }
        }
      }
      GPRS.bufferWritePos++;
      GPRS.charToRead++;
    }
  }
}

String checkSerial(String success, String error) {
  if (!GPRS.initVars) {
    GPRS.initVars = 1;
    GPRS.enable = 1;
    GPRS.reply = "";
  }

  if (GPRS.charToRead) {
    GPRS.reply += String(GPRS.buffer[GPRS.bufferPos]);
    if (GPRS.bufferPos > 1023) {
      GPRS.bufferPos = 0;
    }
    if (GPRS.buffer[GPRS.bufferPos] == '\r') {
      log(GPRS.reply);
      GPRS.reply = "";
    }
    if (GPRS.buffer[GPRS.bufferPos] == success.charAt(success.length() - 1)) {
      bool foundSuccessString = 1;
      for (int i = 0; i < success.length(); i++) {
        if (GPRS.buffer[GPRS.bufferPos - success.length() + i + 1] != success.charAt(i)) {
          foundSuccessString = 0;
        }
      }
      if (foundSuccessString) {
        log(GPRS.reply);
        GPRS.initVars = 0;
        GPRS.process++;
        clearGPRSBuffer();
        return (GPRS.reply);
      }
    }
    if (GPRS.buffer[GPRS.bufferPos] == success.charAt(error.length() - 1)) {
      bool foundErrorString = 1;
      for (int i = 0; i < error.length(); i++) {
        if (GPRS.buffer[GPRS.bufferPos - error.length() + i + 1] != error.charAt(i)) {
          foundErrorString = 0;
        }
      }
      if (foundErrorString) {
        log(GPRS.reply);
        GPRS.processSuccess = 0;
        GPRS.initVars = 0;
        GPRS.process++;
        clearGPRSBuffer();
      }
    }
    GPRS.charToRead--;
    GPRS.bufferPos++;
  }
  return "";
}

void clearGPRSBuffer() {
  while (Serial1.available()) {
    logln("Not reading char: " + Serial1.read());
  }
  GPRS.charToRead = 0;
  GPRS.bufferPos = 0;
  GPRS.bufferWritePos = 0;
}

void setPostVariables(byte postContent, String postComment) {
  if (postComment.length()) {
    GPRS.postComment = 1;
    GPRS.comment += postComment;
  }
  else {
    GPRS.comment = "";
  }
  switch (postContent) {
    case defaultPost:
      GPRS.postBabyTemp = 1;
      GPRS.postHeaterTemp = 1;
      GPRS.postBoardTemp1 = 1;
      GPRS.postBoardTemp2 = 1;
      GPRS.postBoardTemp3 = 1;
      GPRS.postHumidity = 1;
      GPRS.RSSI = 1;
      GPRS.postCurrentConsumption = 1;
      break;
    case addLocation:
      GPRS.postLatitud = 1;
      GPRS.postLongitud = 1;
      break;
    case removeLocation:
      GPRS.postLatitud = 0;
      GPRS.postLongitud = 0;
      break;
    case jaundiceLEDON:
      GPRS.postJaundicePower = 1;
      break;
    case jaundiceLEDOFF:
      GPRS.postJaundicePower = 0;
      break;
    case actuatorsModeON:
      GPRS.postJaundicePower = 1;
      GPRS.postHeaterPower = 1;
      break;
    case actuatorsModeOFF:
      GPRS.postJaundicePower = 0;
      GPRS.postHeaterPower = 0;
      break;
    case pulseFound:
      GPRS.postBPM = 1;
      GPRS.postIBI = 1;
      GPRS.postRPD = 1;
      break;
    case pulseLost:
      GPRS.postBPM = 0;
      GPRS.postIBI = 0;
      GPRS.postRPD = 0;
      break;
  }
}

bool GPRSLoadVariables() {
  req2[7] = "{";
  req2[7] += "\"sn\":\"" + serialNumber + "\"";
  if (GPRS.postBabyTemp) {
    req2[7] += ",\"baby_temp\":\"" + String(temperature[babyNTC], 2) + "\"";
  }
  if (GPRS.postHeaterTemp) {
    req2[7] += ",\"heater_temp\":\"" + String(temperature[heaterNTC], 2) + "\"";
  }
  if (GPRS.postBoardTemp1) {
    req2[7] += ",\"board_temp1\":\"" + String(temperature[inBoardLeftNTC], 2) + "\"";
  }
  if (GPRS.postBoardTemp2) {
    req2[7] += ",\"board_temp2\":\"" + String(temperature[inBoardRightNTC], 2) + "\"";
  }
  if (GPRS.postBoardTemp3) {
    req2[7] += ",\"board_temp3\":\"" + String(temperature[digitalTempSensor], 2) + "\"";
  }
  if (GPRS.postHumidity) {
    req2[7] += ",\"humidity\":\"" + String(humidity) + "\"";
  }
  if (GPRS.postLongitud) {
    req2[7] += ",\"long\":\"" + GPRS.longitud + "\"";
  }
  if (GPRS.postLatitud) {
    req2[7] += ",\"lat\":\"" + GPRS.latitud + "\"";
  }
  if (GPRS.RSSI) {
    req2[7] += ",\"rssi\":\"" + GPRS.RSSI + "\"";
  }
  if (GPRS.postJaundicePower) {
    req2[7] += ",\"jaundice_power\":\"" + String(jaundiceLEDIntensity) + "\"";
  }
  if (GPRS.postHeaterPower) {
    req2[7] += ",\"heater_power\":\"" + String(heaterPower) + "\"";
  }
  if (GPRS.postBPM) {
    req2[7] += ",\"BPM\":\"" + String(BPM) + "\"";
  }
  if (GPRS.postIBI) {
    req2[7] += ",\"IBI\":\"" + String(IBI) + "\"";
  }
  if (GPRS.postRPD) {
    //add RPD logic, ensure maximum data packet size
  }
  if (GPRS.postCurrentConsumption) {
    //add current consumption logic
  }
  if (GPRS.postComment) {
    req2[7] += ",\"comments\":\"" + GPRS.comment + "\"";
  }
  req2[7] += "}\n";
  logln("Posting: " + req2[7]);
  if (req2[7].indexOf("nan") >= 0) {
    logln("Detected zero");
    return false;
  }
  if (req2[7].length() > 1200) {
    logln("posting packet size oversized");
    return false;
  }
  return true;
}