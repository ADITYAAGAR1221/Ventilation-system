#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include "Wire.h"
#include <OneWire.h>  
#include <DallasTemperature.h>


#define ONE_WIRE_BUS 12
#define TEMPERATURE_PRECISION 9

int sensor=10;
// Setup a oneWire instance to communicate with any OneWire devices (Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature.
DallasTemperature sensors(&oneWire);

DeviceAddress insideThermometer, outsideThermometer;

float tempC=0;

#define motor 14    
#define buzzer 16  

ESP8266WiFiMulti WiFiMulti;
String s="";
int pwm=0;

void setup() {

  Serial.begin(9600);

pinMode(motor,OUTPUT);
pinMode(buzzer,OUTPUT);
  
  for (uint8_t t = 4; t > 0; t--) {
    Serial.printf("[SETUP] WAIT %d...\n", t);
    Serial.flush();
    delay(1000);
  }

  WiFi.mode(WIFI_STA);
  WiFiMulti.addAP("Priyash", "12345678");

  sensors.begin();

   // set the resolution to 9 bit per device
  sensors.setResolution(insideThermometer, TEMPERATURE_PRECISION);

for(int i=0;i<10;i++)
{
  digitalWrite(buzzer,1);
  delay(200);
  digitalWrite(buzzer,0);
  delay(200);
}

}

long times = 0;
byte Available = 0;

void loop() {

delay(200);
  sensors.requestTemperatures();
  tempC = sensors.getTempC(insideThermometer);

      if(tempC>36)
      {
      pwm = map(tempC, 36, 90, 100, 255);
      analogWrite(motor,pwm);
      }
      else
      {
        pwm=0;
      }

  int gas = analogRead(A0);

 if(gas>800)
 {
   digitalWrite(buzzer,1);
   delay(100);
    digitalWrite(buzzer,0);
 }
 else
 {
  digitalWrite(buzzer,0);
 }

      delay(10);
      s = "http://api.thingspeak.com/update?api_key=GVS3RPNPDHBXXROA&field1=";
      s += gas;
      s += "&field2=";
      s += tempC;
      s += "&field3=";
      if(gas>800)
      {
       s += 1; 
      }
      else
      {
         s += 0; 
      }
      s += "&field4=";
      s += pwm; 
     
      Serial.println(s);
      Available = 1;
  

  if(Available == 1 & millis()>times+16000)
  {
  if ((WiFiMulti.run() == WL_CONNECTED)) {

    WiFiClient client;
    HTTPClient http;

    int len = s.length();
    char c[len+5];
    s.toCharArray(c, len+1);
 
    if (http.begin(client, c)) {  // HTTP

      Serial.print("[HTTP] GET...\n");
      // start connection and send HTTP header
      int httpCode = http.GET();

      // httpCode will be negative on error
      if (httpCode > 0) {
        // HTTP header has been send and Server response header has been handled
        Serial.printf("[HTTP] GET... code: %d\n", httpCode);

        // file found at server
        if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
          String payload = http.getString();
          Serial.println(payload);      
          Available = 0;
          times = millis();
        }
      } else {
        Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
      }
      http.end();
    } else {
      Serial.printf("[HTTP} Unable to connect\n");
    }
  }
  }

}
