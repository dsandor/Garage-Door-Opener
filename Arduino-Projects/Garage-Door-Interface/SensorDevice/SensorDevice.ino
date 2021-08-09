/**
   This is a modified BasicHTTPClient.ino from the esp8266 examples.


   SENSOR DEVICE


   Please modify the defines below to set up your device.

   STASSID    - the ssid of your WiFi.
   STAPSK     - the password for your WiFi.
   API_KEY    - a password you set here to be allowed to talk to the API. You make this value up and enter it in
                this file, the sensor Arduino project, the mobile app, and the AWS SAM Template.  The values should be the same 
                in all three.
   API_URL    - enter the AWS APIGW URL for your iot stack that you created. If you do not have one please read the blog article
                for more information on how to get this URL.
   DEVICE_ID  - This is a unique ID used to identify this sensor device. The value will also be set in the mobile app.
   
*/

#ifndef STASSID
#define STASSID "ENTER_YOUR_WIFI_SSID_HERE"
#define STAPSK  "ENTER_YOUR_WIFI_PASSWORD_HERE"
#endif
#ifndef DEVICE_ID
#define DEVICE_ID "ENTER_A_UNIQUE_DEVICE_ID_HERE_FOR_YOUR_SENSOR_DEVICE"
#endif
#ifndef API_KEY
#define API_KEY "ENTER_AN_API_KEY_HERE_SEE_NOTE_ABOVE"
#endif
#ifndef API_URL
#define API_URL "ENTER_YOUR_AWS_APIGW_URL_HERE_SEE_NOTE_ABOVE"
#endif

#include <Arduino.h>
#include <ArduinoOTA.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecureBearSSL.h>
#include <WiFiClient.h>


ESP8266WiFiMulti WiFiMulti;
const char* ssid     = STASSID;
const char* password = STAPSK;
String device_id = DEVICE_ID;
String apiUrlBase = API_URL;

unsigned long last_heartbeat = 0;
unsigned long heartbeat_every = 10000; 


HTTPClient http;
std::unique_ptr<BearSSL::WiFiClientSecure>client(new BearSSL::WiFiClientSecure);
void setup() {

  Serial.begin(115200);

  pinMode(A0, INPUT);    
  
  Serial.println();
  Serial.println();

  for (uint8_t t = 4; t > 0; t--) {
    Serial.printf("[SETUP] WAIT %d...\n", t);
    Serial.flush();
    delay(500);
  }
  
  WiFi.mode(WIFI_STA);
  WiFiMulti.addAP(ssid, password);

  Serial.println();
  Serial.println("Device ID: " + device_id);
  Serial.print("Wait for WiFi to connect... ");

  while (WiFiMulti.run() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  // sets up ArdunioOTA for OTA updates of firmware.
  configureOTA();
  
  // configure ssl client
  client->setInsecure();

  isOpen();
  // have device heartbeat when it starts up.
  heartbeat();
}

void loop() {
  ArduinoOTA.handle();
  if ((millis() - last_heartbeat) > heartbeat_every) {
    heartbeat();
  }
}


void heartbeat() {
  Serial.print("heartbeat begin...\n");
  last_heartbeat = millis();
  if (http.begin(*client, apiUrlBase + device_id)) {
    Serial.print("[HTTP] Sending heartbeat...\n");
    // start connection and send HTTP header
    http.addHeader("authorization", API_KEY);
    
    int httpCode = http.PUT("{\"status\":\"online\",\"ip\":\"" + WiFi.localIP().toString() + "\",\"isOpen\":" + isOpen() + "}");

    http.end();
  }
}

String isOpen() {
  // get a few readings..
  int stat = 0;
  
  for(int i = 0; i < 10; i++) {
    stat = analogRead(A0);
    Serial.print("Status: ");
    Serial.println(stat);
    delay(10);  
  }

  if (stat >= 100) {
    return "true";
  }

  return "false";
}

void configureOTA() {
  // Hostname defaults to esp8266-[ChipID]
  ArduinoOTA.setHostname("garage-door-sensor");

  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH) {
      type = "sketch";
    } else { // U_FS
      type = "filesystem";
    }

    // NOTE: if updating FS this would be the place to unmount FS using FS.end()
    Serial.println("Start updating " + type);
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) {
      Serial.println("Auth Failed");
    } else if (error == OTA_BEGIN_ERROR) {
      Serial.println("Begin Failed");
    } else if (error == OTA_CONNECT_ERROR) {
      Serial.println("Connect Failed");
    } else if (error == OTA_RECEIVE_ERROR) {
      Serial.println("Receive Failed");
    } else if (error == OTA_END_ERROR) {
      Serial.println("End Failed");
    }
  });
  ArduinoOTA.begin();
}
