// buffer de 768 para el serial

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
long GPRSTimeOut = 50000; //in millisecs
struct GPRSstruct {
  bool firstPost;
  bool error;
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
  bool postRPS;
  bool postHeaterPower;
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
  bool readLatitud;
  bool readLongitud;
  bool readLocalDayTime;
  bool readLocalHourTime;
  String token;
  String line;
  String lastLine;
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
  GPRS.sendPeriod = 180; //in secs
  GPRS.postBabyTemp = 1;
  GPRS.postHumidity = 1;
  GPRS.powerUp = 1;
}

void GPRSHandler() {
  if (GPRS.powerUp) {
    GPRSPowerUp();
  }
  if (GPRS.connect) {
    GPRSStablishConnection();
  }
  if (GPRS.location) {
    GPRSLocation();
  }
  if (GPRS.post) {
    GPRSPost();
  }
  readGPRSData();
  checkFirstPost();
  GPRSStatusHandler();
}

void GPRSStatusHandler() {
  if (GPRS.timeOut) {
    GPRS.timeOut = 0;
    GPRS.process = 0;
    GPRS.post = 0;
    GPRS.connect = 0;
    GPRS.powerUp = 1;
  }
  if (GPRS.powerUp || GPRS.connect || GPRS.post || GPRS.location) {
    if (millis() - GPRS.processTime > GPRSTimeOut) {
      GPRS.timeOut = 1;
      logln("timeOut" + String(GPRS.powerUp) + String(GPRS.connect) + String(GPRS.post) + String(GPRS.location) + String(GPRS.process));
      GPRS.processTime = millis();
    }
  }

  if (!GPRS.powerUp && !GPRS.connect && !GPRS.post) {
    if (!GPRS.connectionStatus) {
      GPRS.powerUp = 1;
    }
    if (millis() - GPRS.lastSent > GPRS.sendPeriod * 1000) {
      GPRS.post = 1;
    }
  }
}

void GPRSLocation() {
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

void checkFirstPost() {
  if (!GPRS.firstPost && GPRS.connectionStatus) {
    GPRS.location = 1;
  }
}

void checkGPRSConnection() {
  switch (GPRS.process) {
    case 0:
      GPRS.processSuccess = 1;
      Serial1.print("AT\n");
      GPRS.process++;
      break;
    case 1:
      checkSerial("OK", "ERROR");
      break;
    case 2:
      if (!GPRS.processSuccess) {
        GPRS.powerUp = 1;
      }
      GPRS.process = 0;
      break;
  }
}

void GPRSPowerUp() {
  switch (GPRS.process) {
    case 0:
      GPRS.processTime = millis();
      digitalWrite(GPRS_PWRKEY, LOW);
      GPRS.processSuccess = 1;
      GPRS.process++;
      GPRS.packetSentenceTime = millis();
      logln("powering up GPRS");
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
      GPRS.location = 0;
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
      GPRS.process++;
      break;
    case 1:
      checkSerial("OK", "ERROR");
      break;
    case 2:
      Serial1.write("AT+CPIN?\n");
      GPRS.packetSentenceTime = millis();
      GPRS.process++;
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
      if (GPRS.processSuccess) {
        GPRS.connectionStatus = 1;
      }
      else {
        GPRS.powerUp = 1;
      }
      GPRS.connect = 0;
      GPRS.process = 0;
      break;
  }
}

void GPRSPost() {
  switch (GPRS.process) {
    case 0:
      GPRS.processTime = millis();
      GPRS.processSuccess = 1;
      GPRS.lastSent = millis();
      Serial1.print("AT+CIPSTART=\"TCP\",\"" + server + "\",80\n");
      GPRS.process++;
      postGPRSVariables();
      GPRSLocalLog(); //log GPRS data in SD module
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
      checkSerial("CLOSED", "\n\n", "ey");
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
      logln("GPRS POST SUCCESS");
      GPRS.post = 0;
      GPRS.process = 0;
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
  return (checkSerial(success, error, ""));
}

String checkSerial(String success, String error, String includeOnly) {
  if (!GPRS.initVars) {
    GPRS.initVars = 1;
    GPRS.enable = 1;
    GPRS.line = "";
    GPRS.lastLine = "";
  }

  if (GPRS.charToRead) {
    char c = GPRS.buffer[GPRS.bufferPos];
    GPRS.charToRead--;
    GPRS.bufferPos++;
    if (GPRS.bufferPos > 1023) {
      GPRS.bufferPos = 0;
    }
    if (c == '\r') {
      //log(GPRS.line + '\r');
      GPRS.lastLine = GPRS.line;
      GPRS.line = "";
      if (GPRS.lastLine.equals(success) || GPRS.lastLine.equals(error)) {
        if (GPRS.lastLine == error) {
          GPRS.processSuccess = 0;
        }
        GPRS.initVars = 0;
        GPRS.process++;
        clearGPRSBuffer();
      }
    }
    else {
      if (c != '\n') {
        GPRS.line = GPRS.line + String(c);
      }
    }
  }
  return GPRS.line;
}

void clearGPRSBuffer() {
  while (Serial1.available()) {
    logln("Not reading char: " + Serial1.read());
  }
  GPRS.charToRead = 0;
  GPRS.bufferPos = 0;
  GPRS.bufferWritePos = 0;
}

void postGPRSVariables() {
  req2[7] = "{";
  req2[7] += "\"origin\":\"" + serialNumber + "\"";
  if (GPRS.postBabyTemp) {
    req2[7] += ",\"temperature\":\"" + String(temperature[babyNTC], 2) + "\"";
  }
  if (GPRS.postHumidity) {
    req2[7] += ",\"humidity\":\"" + String(humidity) + "\"";
  }
  req2[7] += "}\n";
}