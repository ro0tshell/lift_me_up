/*
 * Rui Santos 
 * Complete Project Details https://randomnerdtutorials.com
*/

#include <Arduino.h>
#include <desk_hight_sensor.h>

////////////////////////////////////////////////////////////////////////////////////////////////////////
// developed by foxytocin
// website https://anfuchs.de
// based on code from iMicknl

// #include <SoftwareSerial.h>
#include "fauxmoESP.h"

#define WIFI_SSID "REPLACE_WITH_YOUR_SSID"
#define WIFI_PASS "REPLACE_WITH_YOUR_PASSWORD"

#define TISCH "Tisch"
#define displayPin20 23 // D2 GPIO4
#define rxPin 16 // D5 GPIO12
#define txPin 17 // D6 GPIO14 

fauxmoESP fauxmo;
bool hoch false;
bool runter false;

// Wi-Fi Connection
void wifiSetup() {
  // Set WIFI module to STA mode
  WiFi.mode(WIFI_STA);

  // Connect
  Serial.printf("[WIFI] Connecting to %s ", WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PASS);

  // Wait
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(100);
  }
  Serial.println();

  // Connected!
  Serial.printf("[WIFI] STATION Mode, SSID: %s, IP address: %s\n", WiFi.SSID().c_str(), WiFi.localIP().toString().c_str());
}

// SoftwareSerial sSerial(rxPin, txPin); // RX, TX
byte history[2];

// Supported Commands
const byte wakeup[] = { 0x9b, 0x06, 0x02, 0x00, 0x00, 0x6c, 0xa1, 0x9d };
const byte command_up[] = { 0x9b, 0x06, 0x02, 0x01, 0x00, 0xfc, 0xa0, 0x9d };
const byte command_down[] = { 0x9b, 0x06, 0x02, 0x02, 0x00, 0x0c, 0xa0, 0x9d };
const byte command_m[] =  {0x9b, 0x06, 0x02, 0x20, 0x00, 0xac, 0xb8, 0x9d };
const byte command_preset_1[] = { 0x9b, 0x06, 0x02, 0x04, 0x00, 0xac, 0xa3, 0x9d };
const byte command_preset_2[] = { 0x9b, 0x06, 0x02, 0x08, 0x00, 0xac, 0xa6, 0x9d };
const byte command_preset_3[] = { 0x9b, 0x06, 0x02, 0x10, 0x00, 0xac, 0xac, 0x9d };
const byte command_preset_4[] = { 0x9b, 0x06, 0x02, 0x00, 0x01, 0xac, 0x60, 0x9d };


void setup() {
  wifiSetup();  
  Serial.begin(115200);   // Debug serial
  // sSerial.begin(9600);    // Flexispot E8
  // Note the format for setting a serial port is as follows: Serial2.begin(baud-rate, protocol, RX pin, TX pin);
  Serial2.begin(9600, SERIAL_8N1, RXD2, TXD2);

  fauxmo.createServer(true); // not needed, this is the default value
  fauxmo.setPort(80); // This is required for gen3 devices

  // You have to call enable(true) once you have a WiFi connection
  // You can enable or disable the library at any moment
  // Disabling it will prevent the devices from being discovered and switched
  fauxmo.enable(true);
  // You can use different ways to invoke alexa to modify the devices state:
  // "Alexa, turn lamp two on"

  // Add virtual devices
  fauxmo.addDevice(Tisch);
  //fauxmo.addDevice(LAMP_2);  

  fauxmo.onSetState([](unsigned char device_id, const char * device_name, bool state, unsigned char value) {
        Serial.printf("[MAIN] Device #%d (%s) f\x84hrt auf Position value: %d\n", device_id, device_name, state ? "hoch" : "runter", value);
  });
  
  pinMode(displayPin20, OUTPUT);
  digitalWrite(displayPin20, LOW);

  // Executes a demo
  if ( (strcmp(device_name, Tisch) == 0) ) {
      demo();
  }      
}


void demo() {

  if (value == 1) {
	  // Calls sit-preset and waits 20 seconds
	  sit();
	  delay(20000);
	  runter = false;
	  wake();
  }

  if (value == 2) {
	  // Calls stand-preset and waits 20 seconds
	  stand();
	  delay(20000);
	  hoch = false;
	  wake();
  }

  // Wakeup the table to retrieve the current height
  // At the moment this is only represented as HEX value
  // wake();
}


void turnon() {
  // Turn desk in operating mode by setting controller pin20 to HIGH
  Serial.println("sending turn on command");
  digitalWrite(displayPin20, HIGH);
  delay(1000);
  digitalWrite(displayPin20, LOW);
}


void wake() {
  turnon();

  // This will allow us to receive the current height
  Serial.println("sending wakeup command");
  Serial2.flush();
  Serial2.enableTx(true);
  Serial2.write(wakeup, sizeof(wakeup));
  Serial2.enableTx(false);
}


void sit() {
  turnon();

  // This send the preset_4 command to trigger the sit position
  Serial.println("sending sit preset (preset_4)");
  Serial2.flush();
  Serial2.enableTx(true);
  Serial2.write(command_preset_4, sizeof(command_preset_4));
  Serial2.enableTx(false);
}


void stand() {
  turnon();

  // This send the preset_3 command to trigger the stand position
  Serial.println("sending stand preset (preset_3)");
  Serial2.flush();
  Serial2.enableTx(true);
  Serial2.write(command_preset_3, sizeof(command_preset_3));
  Serial2.enableTx(false);
}


void loop() {
  fauxmo.handle();
    
  while (Serial2.available())
  {
    unsigned long in = Serial2.read();

    // Start of packet
    if (in == 0x9b) {
      Serial.println();
    }

    // Second byte defines the message length
    if (history[0] == 0x9b) {
      int msg_len = (int)in;
      Serial.print("(LENGTH:");
      Serial.print(in);
      Serial.print(")");
    }

    // Get package length (second byte)
    history[1] = history[0];
    history[0] = in;

    // Print hex for debug
    Serial.print(in, HEX);
    Serial.print(" ");
  }
}
