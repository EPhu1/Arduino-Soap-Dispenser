#include <Servo.h>
#include <Wire.h>
#include "rgb_lcd.h"

#include <HX711_ADC.h>
#include <EEPROM.h>

#include "WiFiEsp.h"
// Emulate Serial1 on pins 6/7 if not present
#ifndef HAVE_HWSERIAL1
#include "SoftwareSerial.h"
SoftwareSerial Serial1(10, 11); // RX, TX
#endif

rgb_lcd lcd;
Servo servo;

const int HX711_dout = 4; //mcu > HX711 dout pin
const int HX711_sck = 5; //mcu > HX711 sck pin
const int infraredpin = 9;
const int servopin = 8;

//HX711 constructor:
HX711_ADC LoadCell(HX711_dout, HX711_sck);

const int fullweight = 770; //full weight of the bottle with everything + water
const int empty = 125; //weight of bottle without water
const int calVal_eepromAdress = 0;
long t;
const int colorR = 0;
const int colorG = 255;
const int colorB = 0;
unsigned long prevMillis = 0;
int count = 0;
int pressedCount = 0;
boolean pressed = false;
boolean isconnected = false;
boolean serverprint = false; 

char ssid[] = "FBI Black Van 2.4"; // your network SSID (name)
char pass[] = "slavetotherave"; // your network password
int status = WL_IDLE_STATUS; // the Wifi radio's status
char server[] = "13.58.148.61";
char var[15];
char get_request[200];
// Initialize the Ethernet client object
WiFiEspClient client;

void printWifiStatus()
{
// print the SSID of the network you're attached to
Serial.print("SSID: ");
Serial.println(WiFi.SSID());
// print your WiFi shield's IP address
IPAddress ip = WiFi.localIP();
Serial.print("IP Address: ");
Serial.println(ip);
// print the received signal strength
long rssi = WiFi.RSSI();
Serial.print("Signal strength (RSSI):");
Serial.print(rssi);
Serial.println(" dBm");
}

void setup() {
  servo.attach(servopin);
//  servo.write(0);
  pinMode(infraredpin, INPUT);
  Serial.begin(115200);
  Serial1.begin(115200);
  WiFi.init(&Serial1);
  if (WiFi.status() == WL_NO_SHIELD) {
  Serial.println("WiFi shield not present");
  // don't continue
  while (true);
  }
  // attempt to connect to WiFi network
  while ( status != WL_CONNECTED) {
  Serial.print("Attempting to connect to WPA SSID: ");
  Serial.println(ssid);
  // Connect to WPA/WPA2 network
  status = WiFi.begin(ssid, pass);
  }
  Serial.println("You're connected to the network");
  printWifiStatus();
  
  lcd.begin(16, 2);
  lcd.setRGB(colorR, colorG, colorB);
  lcd.print("Timer:");
  Serial.println("Starting Loadcell...");
  LoadCell.begin();
  float calibrationValue; // calibration value (see example file "Calibration.ino")
  calibrationValue = 113.27; // uncomment this if you want to set the calibration value in the sketch
  long stabilizingtime = 2000; // preciscion right after power-up can be improved by adding a few seconds of stabilizing time
  boolean _tare = true; //set this to false if you don't want tare to be performed in the next step
  LoadCell.start(stabilizingtime, _tare);
  if (LoadCell.getTareTimeoutFlag()) {
    Serial.println("Timeout, check MCU>HX711 wiring and pin designations");
    while (1);
  }
  else {
    LoadCell.setCalFactor(calibrationValue); // set calibration value (float)
    Serial.println("Startup is complete");
  }
}

void loop() {
//  lcd.print(millis()/1000);
  lcd.setCursor(6, 0);
  if(digitalRead(infraredpin) == LOW){
      serverprint = true;
      servo.write(0);
      prevMillis = millis();
      count = 20;
      if(pressed == false){
        pressed = true;
        pressedCount++;
        Serial.println("Pressed " + String(pressedCount) + " times.");
      }
  }
  else{
      pressed = false;
      servo.write(90);
  }
  if(count <= 2){
    lcd.print(0);
    String x = "Pressed:" + String(pressedCount);
    strcpy(var, x.c_str());
    if(serverprint){
       if (!client.connected()){
      Serial.println("Starting connection to server...");
      client.connect(server, 5000);
      Serial.println("Connected to server");
      }
      // Make a HTTP request
      sprintf(get_request,"GET /?var=%s HTTP/1.1\r\nHost: 18.221.147.67\r\nConnection: close\r\n\r\n", var);
      client.print(get_request);
      delay(500);
      while (client.available()) {
      char c = client.read();
      Serial.write(c);
      }
      serverprint = false;
      delay(5000);
    }
  }
  else{
    if(prevMillis + 20000 - millis() <= 1000){
      count = 0;
    }
    lcd.print((prevMillis + 20000 - millis())/1000);
    lcd.print(" ");
//    Serial.println(prevMillis + 20000 - millis());

  static boolean newDataReady = 0;
  const int serialPrintInterval = 0; //increase value to slow down serial print activity
  if (LoadCell.update()) newDataReady = true;

  // get smoothed value from the dataset:
  if (newDataReady) {
    if (millis() > t + serialPrintInterval) {
      float i = LoadCell.getData();
//      Serial.print("Load_cell output val: ");
//      Serial.println(i);
//      Serial.println(((i-empty)/fullweight)*100);
      lcd.setCursor(0, 1);
      float percentage = ((i-empty)/fullweight)*100;
      lcd.print("Full%: ");
      lcd.print(percentage);
 
      newDataReady = 0;
      t = millis();
    }
  }

  // receive command from serial terminal, send 't' to initiate tare operation:
  if (Serial.available() > 0) {
    float i;
    char inByte = Serial.read();
    if (inByte == 't') LoadCell.tareNoDelay();
  }

  // check if last tare operation is complete:
  if (LoadCell.getTareStatus() == true) {
    Serial.println("Tare complete");
  }
 }
  }
