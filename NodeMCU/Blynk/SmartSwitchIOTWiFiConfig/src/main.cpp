#include <FS.h>  
#include <Arduino.h>
#define BLYNK_PRINT Serial
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <EEPROM.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>

char auth[33];
char ssid[] = "king";
char pass[] = "parveen1325";

WiFiManager wifiManager;

const int buttonPin = D1;
const int relayPin = D5;

bool buttonState;
bool relayState;

BlynkTimer timer;

bool getState(bool pullUp);
void writeToRelay(int relayPin, bool pinValue);
void buttonPressedOnline();
void buttonPressedOffline();
bool toggleRelayState(int relayPin, bool relayState);

BLYNK_CONNECTED()
{
  Blynk.syncAll();
}

BLYNK_WRITE(V1)
{
  int pinValue = param.asInt();
  writeToRelay(relayPin, pinValue);
}

void setup()
{
  Serial.begin(9600);
  // WiFi.begin(ssid, pass);
  wifiManager.setCustomHeadElement("<style> body{background-color:#212121;color:white;} button{background-color:#feb018;} </style>");
  WiFiManagerParameter custom_blynk_token("Blynk", "blynk token", auth, 33);
  wifiManager.addParameter(&custom_blynk_token);
  wifiManager.autoConnect(ssid, pass);
  // wifiManager.setConfigPortalTimeout(180);
  strcpy(auth, custom_blynk_token.getValue());
  Serial.println(auth);
  Blynk.config(auth);

  pinMode(buttonPin, INPUT_PULLUP);
  pinMode(relayPin, OUTPUT);

  writeToRelay(relayPin, false);

  buttonState = getState(digitalRead(buttonPin));
  timer.setInterval(100L, buttonPressedOnline);
}

void loop()
{
  buttonPressedOffline();
  Blynk.run();
  timer.run();
}

bool getState(bool pullUp)
{
  return !pullUp;
}

void writeToRelay(int relayPin, bool pinValue)
{
  digitalWrite(relayPin, pinValue);
  relayState = pinValue;
}

void buttonPressedOnline()
{
  if (buttonState != getState(digitalRead(buttonPin)))
  {
    toggleRelayState(relayPin, relayState);
    buttonState = getState(digitalRead(buttonPin));
  }
}
void buttonPressedOffline()
{
  if (Blynk.connected() == false)
  {
    if (buttonState != getState(digitalRead(buttonPin)))
    {
      writeToRelay(relayPin, !relayState);
    }
  }
  delay(100);
}

bool toggleRelayState(int relayPin, bool relayState)
{
  writeToRelay(relayPin, !relayState);
  Blynk.virtualWrite(V1, !relayState);
}