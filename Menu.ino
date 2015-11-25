#define LINE_LEN 60
char line[LINE_LEN];

void processUserInput() {
  if (!Serial.available()) return;
  if (readLine(30000) >= 0) handleCommand();
}

byte readLine(int timeout) {
  unsigned long deadline = millis() + timeout;
  byte i = 0;
  while (millis() < deadline) {
    if (Serial.available()) {
      line[i++] = (char) Serial.read();
      if      (line[i-1] == '\r')  i--;   
      else if (i == LINE_LEN - 1)  break; 
      else if (line[i-1] == '\n')  {i--; break;}
    }
  }
  line[i] = 0;
  return i;
}


void handleCommand() {
  //if (String(line) == F("dfd"));
  String x = String(line);
  if      (x.startsWith(F("wifi"))) setESPWifi(trim(&line[4]));
  else if (x.startsWith(F( "lt" ))) menuEnterColorRanges(trim(&line[2]));
  else if (x.startsWith(F("debug"))) switchDebugInfoPrint();
  else if (x.startsWith(F("reset"))) menuMainFactoryReset();
  else if (x.startsWith(F("tskey"))) doSetTSKey(trim(&line[5]));
  else if (x.startsWith(F("brg"  ))) menuEnterLedBrightness(trim(&line[3]));
  else if (x.startsWith(F("gray" ))) switchGrayBox();
  else if (x.startsWith(F("sim"  ))) simulateCO2();
  else if (x.startsWith(F("ota"  ))) espOTA();
  else if (x.startsWith(F("ping" ))) espPing();
  else if (x.startsWith(F("proxy"))) startSerialProxy();
  else if (x.startsWith(F("ppm"  ))) setPPM(trim(&line[3]));
  else if (x.startsWith(F("esp"  ))) onlyESP();
}

char* trim(const char *str) {
  while (*str == ' ') str++;
  return (char*)str;
}

void setPPM(char *val) {
  sPPM = String(val).toInt();
  processColors();
  oledCO2Level();
}

void onlyESP() {
  pinMode(ESP_RX, INPUT);
  pinMode(ESP_TX, INPUT);
  espToggle();
}

int menuEnterLedBrightness(const char *str) {
  //Serial  << line << ",," << String(line).toInt() << endl;
  int res = String(str).toInt();
  if (res < 255) res = constrain(res, 0, 150);
  overrideBrightness = res;
  Serial << F("New Brightness value: ") << overrideBrightness << endl;
  EEPROM.write(EE_1B_BRG, overrideBrightness);
  //menuHandler = menuPrintMain;
  return 0;
}

void menuEnterColorRanges(char *s) {
  storeColorRanges(s);
  processNeopixels();
  Serial << F("Done") << endl;
}

int menuMainFactoryReset() {
  clearEEPROM();
  Serial << endl << F("Configuration reset...") << endl;
  softwareReset();
  return 0;
}

int switchGrayBox() {
  byte val = EEPROM.read(EE_1B_ISGRAY);
  if (val == 255) val = 1;
  else val = 255;
  EEPROM.update(EE_1B_ISGRAY, val);
  Serial << endl << F("Box color set to: ");
  if (val == 1) Serial <<("GRAY");
  else Serial << F("WHITE");
  Serial << endl;
  return 0;
}

int switchDebugInfoPrint() {
  dumpDebuggingInfo = !dumpDebuggingInfo;
  Serial << endl  << F("Debug Info is: ") << dumpDebuggingInfo << endl;
  if (dumpDebuggingInfo) displayDebugInfo();
  return 0;
}

int simulateCO2() {
  for (int i = 400; i <=3000; i+=100) {
    sPPM = i;
    processNeopixels();
    oledCO2Level();
    delay(250);
  }
}

void doSetTSKey(const char *key) {
  char kk3[30];
  strcpy(kk3, key);
  EEPROM.put(EE_30B_TSKEY, kk3);
  Serial << F("Testing connection by sending 456 ppm value") << endl;
  sendToThingSpeak(456);
  Serial << F("Done. Please check if 456 was received.") << endl;  
}
//int onWifiEnterTSKey() {
//  doSetTSKey(line);
//  return 0  ;
//}

      
