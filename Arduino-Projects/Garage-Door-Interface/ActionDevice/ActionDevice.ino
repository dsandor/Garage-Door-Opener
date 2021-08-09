/**
   This is a modified BasicHTTPClient.ino from the esp8266 examples.


   ACTION DEVICE
   

   Please modify the defines below to set up your device.

   STASSID    - the ssid of your WiFi.
   STAPSK     - the password for your WiFi.
   API_KEY    - a password you set here to be allowed to talk to the API. You make this value up and enter it in
                this file, the sensor Arduino project, the mobile app, and the AWS SAM Template.  The values should be the same 
                in all three.
   API_URL    - enter the AWS APIGW URL for your iot stack that you created. If you do not have one please read the blog article
                for more information on how to get this URL.
   DEVICE_ID  - This is a unique ID used to identify this relay device (action device). The value will also be set in the mobile app.
*/

#ifndef STASSID
#define STASSID "ENTER_YOUR_WIFI_SSID_HERE"
#define STAPSK  "ENTER_YOUR_WIFI_PASSWORD_HERE"
#endif
#ifndef DEVICE_ID
#define DEVICE_ID "ENTER_A_UNIQUE_DEVICE_ID_HERE_FOR_YOUR_ACTION_DEVICE"
#endif
#ifndef API_KEY
#define API_KEY "ENTER_AN_API_KEY_HERE_SEE_NOTE_ABOVE"
#endif
#ifndef API_URL
#define API_URL "ENTER_YOUR_AWS_APIGW_URL_HERE_SEE_NOTE_ABOVE"
#endif


#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ArduinoOTA.h>
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
unsigned long duty_cycle_for_switch_ms = 2000;

HTTPClient http;
std::unique_ptr<BearSSL::WiFiClientSecure>client(new BearSSL::WiFiClientSecure);
void setup() {

  Serial.begin(115200);

  pinMode(D1, OUTPUT);    
  digitalWrite(D1, LOW);
  
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

  // have device heartbeat when it starts up.
  heartbeat();
}

void loop() {
  ArduinoOTA.handle();
  if (abs(int(millis() - last_heartbeat)) > heartbeat_every) {
    heartbeat();
  }

  delay(10000);
}

void configureOTA() {
  // Hostname defaults to esp8266-[ChipID]
  ArduinoOTA.setHostname("garage-door");

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

void heartbeat() {
  Serial.print("heartbeat begin...\n");
  last_heartbeat = millis();
  if (http.begin(*client, apiUrlBase + device_id)) {
    Serial.print("[HTTP] Sending heartbeat...\n");
    // start connection and send HTTP header
    http.addHeader("authorization", API_KEY);
    
    int httpCode = http.PUT("{\"status\":\"online\",\"ip\":\"" + WiFi.localIP().toString() + "\"}");

    // httpCode will be negative on error
    if (httpCode > 0) {
      // HTTP header has been send and Server response header has been handled
      Serial.printf("[HTTP] PUT... code: %d\n", httpCode);

      // file found at server
      if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
        String payload = http.getString();
        Serial.println(payload);
        if (payload == "{\"shouldOperate\":true}") {
          operateSwitch();
        }
      }
    } else {
      Serial.printf("[HTTP] PUT... failed, error: %s\n", http.errorToString(httpCode).c_str());
    }

    http.end();
  }
}

void acknowledgeOperation() {
  Serial.print("acknowledgeOperation begin...\n");
  
  if (http.begin(*client, apiUrlBase + "ack-operate/" + device_id)) {
    Serial.print("[HTTP] Sending ack-operate...\n");
    // start connection and send HTTP header
    http.addHeader("authorization", API_KEY);
    int httpCode = http.GET();

    // httpCode will be negative on error
    if (httpCode > 0) {
      // HTTP header has been send and Server response header has been handled
      Serial.printf("[HTTP] GET... code: %d\n", httpCode);
    } else {
      Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
    }

    http.end();
  }
}

void operateSwitch() {
  Serial.println("Operating switch.");
  
  digitalWrite(D1, HIGH);
  digitalWrite(LED_BUILTIN, HIGH);
  
  delay(duty_cycle_for_switch_ms);
  
  digitalWrite(D1, LOW);
  digitalWrite(LED_BUILTIN, LOW);
  acknowledgeOperation();
  delay(200);
}
