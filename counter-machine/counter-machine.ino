/**
 * Project: Counter Machine
 * Developers: Reza, Felicia, Radit
 * Description: This is a counte machine to count the productivity of a machine in production line. There are several modes in this function to be selected as 
 *              is needed. The machine will connect to Server to monitor the productivity. Design and other documentation will be provided
 * Copyright: PT Pura Barutama - Hologram Security Label. 2020. All rights reserved
 * 
 */
#include "WiFi.h"
#include <WiFiClient.h>
#include <WebServer.h>
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include <Keypad_I2C.h>
#include <Keypad.h>

// Initialized keypad layout
const byte ROWS = 4;
const byte COLS = 4;
char keys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};

//Initialized keypad interface to I2C PCF8574
byte rowPins[ROWS] = {4, 5, 6, 7}; 
byte colPins[COLS] = {0, 1, 2, 3};
Keypad_I2C kpd = Keypad_I2C(makeKeymap(keys), rowPins, colPins, ROWS, COLS, 0x20, PCF8574);

//Initialize lcd interface to I2C
LiquidCrystal_I2C lcd(0x27, 16, 4);

//Initialize access control to server
const char* ssid = "HSL-01";
const char* password = "hslbisa01";
const char* host = "192.168.37.177";

/*
 * This is the Pinout setting on ESP32. The details are:
 *      G17 -> Interrupt button
 *      G5  -> Status checking from PLC
 *      G2  -> Conunter from PLC
 */
int pinInterrupt = 17; 
int pinStat = 5; 
int pinCounter = 2; 
void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  Serial.print("MAC: ");
  Serial.println(WiFi.macAddress());

  //Start lcd and keypad setup
  Wire.begin();
  lcd.begin();
  kpd.begin();

  //Set pin mode of the pin input
  pinMode(pinInterrupt, INPUT_PULLUP);
  pinMode(pinStat, INPUT_PULLUP);
  pinMode(pinCounter, INPUT_PULLUP);
  
  //Initialize server access
  WiFi.begin(ssid, password);
  Serial.println();

  Serial.print("Connecting");
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Connecting");
  int dotCount = 0;
  while(WiFi.status() != WL_CONNECTED){
    Serial.print(".");
    lcd.setCursor((10+dotCount),0);
    lcd.print(".");
    dotCount += 1;
    delay(500);
    Serial.println(dotCount);
    if(dotCount > 3){
      lcd.setCursor(10,0);
      lcd.print("    ");
      dotCount = 0;
      delay(500);
    }
  }

  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Connected to: ");
  lcd.setCursor(0,1);
  lcd.print(ssid);
  Serial.print("IP address: ");

  delay(3000);
  lcd.clear();
}

bool flagAkt = LOW;

/*
 * These functions are used to give interrupt access to interrupt button and counter pin.
 *    void fungsiInterrupt()
 *    void fungsiCounter()
 * Adjust the setting to give different output of the interrupt actions.
 */
void fungsiInterrupt(){
 static unsigned long last_interrupt_time = 0;
 unsigned long interrupt_time = millis();
 
 if (interrupt_time - last_interrupt_time > 500){
  flagAkt = HIGH;
  Serial.println();
 }
 last_interrupt_time = interrupt_time;
}

int counter = 0;
void fungsiCounter(){
 static unsigned long last_interrupt_time = 0;
 unsigned long interrupt_time = millis();
 
 if (interrupt_time - last_interrupt_time > 180){
  counter++;
 }
 last_interrupt_time = interrupt_time;
}

int switchState = 1;
bool isActivityAccessed;
String num;
String printedNIK;
String count;

void insertNIK(){
  count = "";
  counter = 0;
  lcd.setCursor(0,0);
  Serial.println("Enter NIK: ");
  lcd.print("Enter NIK:");
  char customKey = kpd.getKey();
  if (customKey == '1' || customKey == '2' || customKey == '3' || customKey == '4' || customKey == '5' || customKey == '6' || customKey == '7' || customKey == '8' || customKey == '9' || customKey == '0'){
    num = num + customKey;
    lcd.setCursor(0,0);
    Serial.println("Enter NIK: ");
    lcd.print("Enter NIK: ");
    Serial.println(num);
    lcd.setCursor(0,1);
    lcd.print(num);
    delay(100);
  }
  else if(customKey == '*'){
    num = "";
    Serial.println("Enter NIK: ");
    Serial.println(num);
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Enter NIK: ");
  }
  else if(customKey == '#'){
    printedNIK = num;
    num = "";
    switchState = 2;
  }
}


void checkNIK(){
  if(printedNIK.length() != 5 || printedNIK == "00000"){
    Serial.println("NIK salah!");
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("NIK Salah!");
    switchState = 1;
    delay(3000);
  }
  else{
    lcd.clear();
    isActivityAccessed = 1;
    Serial.println("NIK OK");
    switchState = 3;
  }
}

String actCode;
String printedActCode;
bool isActivityIdle;
bool accessOnce = LOW;
unsigned long currentTime;
unsigned long previousTime;
bool isPinInterruptEnabled;
bool isProduksiAccessed;

void insertActivityCode(){
  detachInterrupt(digitalPinToInterrupt(pinInterrupt));
  detachInterrupt(digitalPinToInterrupt(pinCounter));
  if (isActivityAccessed){
    previousTime = millis();
    isActivityAccessed = false;
  }
  lcd.setCursor(0,0);
  lcd.print("NIK Anda: " + printedNIK);
  lcd.setCursor(0,1);
  lcd.print("Enter activity:");
  lcd.setCursor(0,2);
  
  char customKey = kpd.getKey();
  if (customKey == '1' || customKey == '2' || customKey == '3' || customKey == '4' || customKey == '5' || customKey == '6' || customKey == '7' || customKey == '8' || customKey == '9' || customKey == '0'){
    actCode = actCode + customKey;
    lcd.setCursor(0,0);
    lcd.print("NIK Anda: " + printedNIK);
    lcd.setCursor(0,1);
    lcd.print("Enter activity:");
    lcd.setCursor(0,2);
    lcd.print(actCode);
    delay(100);
  }
  else if(customKey == '*'){
    actCode = "";
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("NIK Anda: " + printedNIK);
    lcd.setCursor(0,1);
    lcd.print("Enter activity:");
    lcd.setCursor(0,2);
  }
  else if(customKey == '#'){
    printedActCode = actCode;
    actCode = "";
    switchState = 4;
  }
  currentTime = millis();
  
  if(currentTime - previousTime >= 10000){
    isActivityIdle = true;
  }
  if(isActivityIdle){
    if(accessOnce){
      actCode = "";
      switchState = 5;
      isActivityIdle = false;
      lcd.clear();
    }
    else{
      switchState = 1;
      isActivityIdle = false;
      lcd.clear();
    }
  }
}

String activity;

void checkActivityCode(){
  switch(printedActCode.toInt()){
    case 0:
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("TERIMA KASIH");
      lcd.setCursor(0,1);
      lcd.print("ATAS PEKERJAAN");
      lcd.setCursor(-4,2);
      lcd.print("ANDA!");
      delay(2500);
      lcd.clear();
      switchState = 1;
      accessOnce = false;
      break;
    case 1:
      lcd.clear();
      activity = "5R";
      switchState = 5;
      accessOnce = true;
      delay(100);
      break;
    case 2:
      lcd.clear();
      activity = "PERSIAPAN BAHAN";
      switchState = 5;
      accessOnce = true;
      delay(100);
      break;
    case 3:
      lcd.clear();
      activity = "SETTING MESIN";
      switchState = 5;
      accessOnce = true;
      delay(100);
      break;
    case 4:
      lcd.clear();
      activity = "ACC";
      switchState = 5;
      accessOnce = true;
      delay(100);
      break;   
    case 5:
      lcd.clear();
      activity = "PRODUKSI";
      switchState = 5;
      accessOnce = true;
      delay(100);
      break;
    case 6:
      lcd.clear();
      activity = "ISHOMA";
      switchState = 5;
      accessOnce = true;
      delay(100);
      break; 
    case 7:
      lcd.clear();
      activity = "TUNGGU";
      switchState = 5;
      accessOnce = true;
      delay(100);
      break;
    case 8:
      lcd.clear();
      activity = "PERBAIKAN";
      switchState = 5;
      accessOnce = true;
      delay(100);
      break;  
    case 9:
      lcd.clear();
      activity = "LAIN-LAIN";
      switchState = 5;
      accessOnce = true;
      delay(100);
      break; 
    default:
      lcd.clear();
      previousTime = millis();
      lcd.setCursor(0,0);
      lcd.print("INPUT KODE");
      lcd.setCursor(0,1);
      lcd.print("AKTIVITAS SALAH!");
      delay(2000);
      lcd.clear();
      switchState = 3;
      break;
  }
}

void dbDataSend(){
  attachInterrupt(digitalPinToInterrupt(pinInterrupt), fungsiInterrupt, FALLING);
  attachInterrupt(digitalPinToInterrupt(pinCounter), fungsiCounter, FALLING);
  
  delay(4000);
  lcd.clear();
  String activ = printedActCode;
  int stat = digitalRead(pinStat);
  count = (String)counter;

  if(printedActCode.toInt() != 5){
    lcd.setCursor(0,0);
    lcd.print("Connecting to");
    lcd.setCursor(0,1);
    lcd.print(host);
    lcd.setCursor(-4,2);
    lcd.print("Aktivitas:");
    lcd.setCursor(-4,3);
    lcd.print(activity);
  }
  else{
    lcd.setCursor(0,0);
    lcd.print("Aktivitas:");
    lcd.setCursor(0,1);
    lcd.print(activity);
    lcd.setCursor(-4,2);
    lcd.print("Counter: " + count);
    lcd.setCursor(-4,3);
    lcd.print(host);
  }
  
  // Mengirimkan ke alamat host dengan port 80 -----------------------------------
  WiFiClient client;
  if (!client.connect(host, 8888)) {
    lcd.clear();
    Serial.println("connection failed");
    lcd.setCursor(0, 0);
    lcd.print("Connection");
    lcd.setCursor(0,1);
    lcd.print("Failed!");
    delay(2000);
    switchState = 1;
    lcd.clear();
  } 
    
  String formatUrl = "/write-data.php?nik=";
  String url = formatUrl + printedNIK + "&aktivasi=" + activ + "&status=" + stat + "&counter=" + count;
  
  Serial.print("Requesting URL: ");
  Serial.println(url);
 
  // Mengirimkan Request ke Server -----------------------------------------------
  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
      "Host: " + host + "\r\n" +
      "Connection: close\r\n\r\n");
  
      
  unsigned long timeout = millis();
  while (client.available() == 0) {
    if (millis() - timeout > 4000) {
      lcd.clear();
      Serial.println(">>> Client Timeout !");
      lcd.setCursor(0,0);
      lcd.print("Client Timeout!");
      client.stop();
      if(flagAkt){
        switchState = 3;
        flagAkt = LOW;
        isActivityAccessed = true;
        lcd.clear();
        delay(100);
      }
    }
  }
  
  while (client.available()) {
    String line = client.readStringUntil('\r');
    Serial.print(line);
  }
  
  Serial.println();
  Serial.println("closing connection");
  Serial.print("flagAkt: ");
  Serial.println(flagAkt);
  Serial.print("pinInterrupt: ");
  Serial.println(stat);
  Serial.print("Counter: ");
  Serial.println(counter);

  if(flagAkt){
    switchState = 3;
    flagAkt = LOW;
    isActivityAccessed = true;
    lcd.clear();
    delay(100);
  }
}

/*
 * Below are functions to control activity. Function is choosen according to activity code inserted by user.
 */

 

void loop() {
  switch(switchState){
    case 1:
      insertNIK();
      break;

    case 2:
      checkNIK();
      break;
    case 3:
      insertActivityCode();
      break;
    case 4:
      checkActivityCode();
      break;
    case 5:
      dbDataSend();
      break;
  }

}
