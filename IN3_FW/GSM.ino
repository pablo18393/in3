#define serverDirectory "POST /In3/public/api/v1/session HTTP/1.1"
#define serverAdress "Host pub.scar.io"
#define serverAutorization "Authorization: Bearer"
#define serverToken "eyJ0eXAiOiJKV1QiLCJhbGciOiJIUzI1NiJ9.eyJpc3MiOiJodHRwOlwvXC9sb2NhbGhvc3RcL0luM1wvcHVibGljXC9hcGlcL3YxXC9hdXRoXC9sb2dpbiIsImlhdCI6MTU2MTExOTI3NCwiZXhwIjoxNTYxMTIyODc0LCJuYmYiOjE1NjExMTkyNzQsImp0aSI6IlFXcHVrZllQc05UTHNBZEwiLCJzdWIiOjEsInBydiI6ImY2YjcxNTQ5ZGI4YzJjNDJiNzU4MjdhYTQ0ZjAyYjdlZTUyOWQyNGQifQ.M-qSoNpIIR71OAOdGOLm17VwrCp_1nCkXJ5Bd5JjpQg"
#define dataContentLength "content-length: "
int GPRSPartialMessageLength;

//GPRS variables
#define environmentalWords[] = "TimeOn","Temperature","Humidity","DesiredTemp","HeaterPower","PIDOutput","HeaterTempLimit";
#define pulseSensorVariablesWords[] = "BPM","IBI";
#define pulseSensorRawWords[] = "PulseMeasurements";
#define aliveRefreshWords[] = "timeOn","LEDsON";

bool initGPRS() {
  Serial1.begin(115200);
  bool initOK = 0;
  if (GSM_command("AT", "OK")) {
    if (GSM_command("AT+CFUN=1", "OK")) {
      if (GSM_command("AT+CPIN?", "+CPIN:READY")) {
        if (GSM_setAPN()) {
          if (GSM_command("AT+CIICR", "OK")) {
            initOK = 1;
            getServerLength();
          }
        }
      }
    }
  }
  return (initOK);
}

void getServerLength() {
  GPRSPartialMessageLength += strlen(serverDirectory) + 1;
  GPRSPartialMessageLength += strlen(serverAdress) + 1;
  GPRSPartialMessageLength += strlen(serverAutorization) + 1;
  GPRSPartialMessageLength += strlen(serverToken) + 1;
  GPRSPartialMessageLength += strlen(dataContentLength) + 1;
}

bool GSM_setAPN() {
  clearSerialBuffer();
  Serial1.print("AT+CSTT=\"");
  Serial1.print("Terminal");
  Serial1.println("\",\"\",\"\"");
  return (1);
}

bool postGPRS(int variable) {
  clearSerialBuffer();
  int GPRSTotalMessageLenght = GPRSPartialMessageLength;
  Serial.println("AT+CIPSEND=");
  delay(50);
  clearSerialBuffer();
  Serial.println(serverDirectory);
  Serial.println(serverAdress);
  Serial.println(serverAutorization);
  Serial.println(serverToken);
  Serial.println(dataContentLength);
  switch (variable) {
    case environmental:
      break;
    case pulseSensorVariables:
      break;
    case pulseSensorRaw:
      break;
    case turnedOn:
      break;
  }
  if (variable == environmental) {

  }
  GSM_command("AT+CIPSTART=\"TCP\",\"pub.scar.io\",80", "OK");
  Serial1.println((char)26);// ASCII code of CTRL+Z
  GSM_command("AT+CIPCLOSE", "CLOSE OK");
}

bool GSM_command(char command[64], char expectedResponse[64]) {
  clearSerialBuffer();
  Serial1.println(command);
  delay(200);
  char response[64];
  byte i = 0;
  if (Serial1.available()) {
    for (int j = 0; j < strlen(command) + 2; j++) {
      Serial1.read();
    }
    while (Serial1.available()) {
      response[i] = Serial1.read();
      Serial.write(response);
      i++;
    }
    //response[i - 2] = '\0';
    Serial.println(strlen(response));
    Serial.println(strlen(expectedResponse));
  }
  if (!strcmp(expectedResponse, response)) {
    Serial.println("success");
  }
  else {
    Serial.println("fail");
  }
}

void clearSerialBuffer() {
  while (Serial1.available()) {
    Serial.read();
  }
}