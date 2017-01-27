byte readLine(int timeout, Stream *stream);
void onESPvESPrino();
void sendNow();

void processUserInput() {
  if (Serial.available()) {
    if (readLine(30000, &Serial) > 0) {
      handleCommand(FROM_SERIAL);
      //Serial <<"rtrttt" << endl;
    }
  }
  //Serial <<"next" << endl;
  int i=0, k=0;
  if (espIsOn && esp.available()) {
      //Serial <<"aloooo" << endl;

    //String rcv;
    espLastActivity = millis();
    while (espIsOn && esp.available() && readLine(1000, &esp) > 0) {
      //i++;
     // k+= strlen(line);
      //rcv += line;
      //rcv += "|";
      handleCommand(FROM_ESP);
      //Serial <<" hmmm" << endl;
    }
   // Serial << "Received:" << i << "lines, " << k << " bytes" << endl;
    //Serial << rcv << endl;
  }
}

byte readLine(int timeout, Stream *stream) {
  unsigned long deadline = millis() + timeout;
  byte i = 0;
  while (millis() < deadline) {
    if (stream->available()) {
      line[i++] = (char) stream->read();
      //Serial << (int)line[i-1];
      if      (line[i-1] == '\r')  i--;   
      else if (i == LINE_LEN - 1)  break; 
      else if (line[i-1] == '\n')  {i--; break;}
    }
  }
  line[i] = 0;
  return i;
}


void handleCommand(CommandSource cs) {
  String x = String(line);
  if (cs == FROM_SERIAL) {
    Serial << line << endl;
    //if      (x.startsWith(F("wifi"))) doConnect();
    if (x.startsWith(F( "lt" ))) menuEnterColorRanges(trim(&line[2]));
    else if (x.startsWith(F("debug"))) switchDebugInfoPrint();
    else if (x.startsWith(F("factory"))) menuMainFactoryReset();
    else if (x.startsWith(F("sendNow"))) sendNow();
    
   // else if (x.startsWith(F("tskey"))) saveLineToEE(trim(&line[5]), EE_40B_TSKEY);
   // else if (x.startsWith(F("ubik")))  saveLineToEE(trim(&line[4]), EE_40B_UBIKEY);
   // else if (x.startsWith(F("ubiv")))  saveLineToEE(trim(&line[4]), EE_40B_UBIVAR);
  //  else if (x.startsWith(F("test")))  sendToThingSpeak(567);
  #ifdef BRG
    else if (x.startsWith(F("brg"  ))) menuEnterLedBrightness(trim(&line[3]));
  #endif
    else if (x.startsWith(F("brf"  ))) EEPROM.put(EE_1B_BRG_FACTOR, (byte)(atoi(&line[4])));
  //  else if (x.startsWith(F("gray" ))) switchGrayBox();
    else if (x.startsWith(F("sim"  ))) simulateCO2();
    //else if (x.startsWith(F("ota"  ))) espOTA();
  //  else if (x.startsWith(F("ping" ))) espPing();
    else if (x.startsWith(F("proxy"))) startSerialProxy();
    else if (x.startsWith(F("ppm"  ))) setPPM(trim(&line[3]));
  #ifdef TGS4161
    else if (x.startsWith(F("ppx"  ))) setPPX(trim(&line[3]));
  #endif
    else if (x.startsWith(F("rco"  ))) resetCO2();
    else if (x.startsWith(F("wsi"  ))) setWifiSendInterval(trim(&line[3]));
  //  else if (x.startsWith(F("esp"  ))) onlyESP();
//    else if (x.startsWith(F("sap "))) EEPROM.put(EE_1B_HASSAPCFG, (byte)(line[4]-'0'));
    //else if (x.startsWith(F("ccc"))) Serial << rawReadCM1106_CO2() << endl;
    else if (x.startsWith(F("esp"))) esp << &x[4] << endl;
    Serial << F(">") << endl;
  } else {
   // Serial << F("e:") << line[0] << endl;  
    if (x.startsWith(F("vESPrino"))) onESPvESPrino(); 
    if (x.startsWith(F("wifi.ssid="))) espWifiConfigured = true; 
    if (x.indexOf(F("GOT IP"))) espWifiConnected = true; 
    Serial << F("e:") << line << endl;  
  }
}

void onESPvESPrino() {
  hasESP = true;
  //Serial << "detected ESP" << endl;
  espSend(String(F("prop_setr \"logger.slow\",\"1\"\r\n")).c_str());
  espSend(String(F("prop_list\r\n")).c_str());
}


//
//void makeBeep() {
////  Serial << F("BEEP\n");
//  
//  for (int i=0; i<1; i++) {
//    tone(A4, 200, 50);
//    delay(200);
//    tone(A4, 150, 50);
//    delay(200);
//    tone(A4, 100, 50);
//    delay(200);
//    tone(A4, 70, 50);
//    delay(200);
//  }
//
//}
//

void saveLineToEE(const char *str, int address) {
  while (*str)  EEPROM.put(address++, (byte)*(str++));
  EEPROM.put(address, 0);
}

char* trim(const char *str) {
  while (*str == ' ') str++;
  return (char*)str;
}
//
void setPPM(char *val) {
  sPPM = String(val).toInt();
  processColors();
  oledCO2Level();
}
void setWifiSendInterval(char *val) {
  EEPROM.put(EE_2B_WIFI_SND_INT_S, atoi(val));
}
#ifdef TGS4161
void setPPX(char *val) {
  currentCO2MaxMv = getTGSEstMaxMv(atoi(val), raCO2mv.getAverage());
  storeCurrentCO2MaxMv();  
}

#endif

void resetCO2() {
#ifdef TGS4161
  EEPROM.put(EE_FLT_CURRENT_PERIOD_CO2_HIGHESTMV, (double)0);
  EEPROM.put(EE_FLT_PREV_PERIOD_CO2_HIGHESTMV,    (double)0);
  prevCO2MaxMv = currentCO2MaxMv = 0;
#else
  Serial << F("Put the device at fresh air. Once it worked for 7 minutes after it startted it will start the calibration procedure, which is 2 minutes. ") << endl;
  EEPROM.put(EE_1B_RESET_CO2, (byte)1);
  softwareReset();
#endif
}

//void onlyESP() {
//  pinMode(ESP_RX, INPUT);
//  pinMode(ESP_TX, INPUT);
//  espToggle();
//}

int menuEnterLedBrightness(const char *str) {
  int res = atoi(str);
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
  Serial << F("Done\n");
}

int menuMainFactoryReset() {
  clearEEPROM();
  Serial << F("\nConfiguration reset...\n");
  softwareReset();
  return 0;
}

#ifdef GRAY
int switchGrayBox() {
  byte val = EEPROM.read(EE_1B_ISGRAY);
  if (val == 255) val = 1;
  else val = 255;
  EEPROM.update(EE_1B_ISGRAY, val);
  Serial << F("\nBox:");
  if (val == 1) Serial << F("G\n");
  else Serial << F("W\n");
  //Serial << endl;
  return 0;
}
#endif

int switchDebugInfoPrint() {
  DEBUG=true;
  dumpDebuggingInfo = !dumpDebuggingInfo;
  Serial << endl  << F("Debug Info is: ") << dumpDebuggingInfo << endl;
  if (dumpDebuggingInfo) displayDebugInfo();
  return 0;
}
//
int simulateCO2() {
  for (int i = 400; i <=3000; i+=100) {
    sPPM = i;
    processNeopixels();
    oledCO2Level();
    delay(250);
  }
}

//int doConnect() {
//  Serial << endl << F("Connecting to Wifi...\n");
//  espToggle();
//  if (!serialFind("ready", ESP_DEBUG, 10000)) {
//    Serial << F("Wifi module not working\n");
//  } else {
//    Serial.flush();
//    delay(1000);
//    int res = setESPWifi(trim(&line[4]));
//    if (res < 0) {
//      Serial << F("Could not connect to Wifi: ") << res << endl;
//    } else {
//      setWifiStat("WiFi OK");
//    }
//  }
//}

      
