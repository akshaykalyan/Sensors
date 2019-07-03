/**
 * @author Akshay Kalyan
 * @email akshaykalyan2307@gmail.com
 * @create date 2019-07-03 08:07:14
 * @modify date 2019-07-03 08:25:23
 * @desc Blynk 4 Switch IoT NodeMCU 1.0 Button WiFi hotspot configurator
 */

#include <FS.h>
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <EEPROM.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>
#include <ArduinoJson.h>

char auth[33];
//default hotspot password
char ssid[] = "Smart_Switch_LSM";
//Hotspot Button "0" for inbuilt Flash key
const int hotspotPin = 0;
BlynkTimer timer;
WiFiManager wifiManager;
//flag for saving data
bool shouldSaveConfig = false;
//callback notifying us of the need to save config
void saveConfigCallback()
{
  Serial.println("Should save config");
  shouldSaveConfig = true;
}

void readConfig();
void writeConfig();
void checkHotpotButton();

const int buttonPin1 = D1;
const int buttonPin2 = D2;
const int buttonPin3 = D6;
const int buttonPin4 = D7;
const int relayPin1 = D5;
const int relayPin2 = D6;
const int relayPin3 = D3;
const int relayPin4 = D4;

bool buttonState1;
bool relayState1;

bool buttonState2;
bool relayState2;

bool buttonState3;
bool relayState3;

bool buttonState4;
bool relayState4;

bool getState(bool pullUp);
void buttonPressedOnline();
void buttonPressedOffline();
void writeToRelay1(int relayPin, bool pinValue);
bool toggleRelayState1(int relayPin, bool relayState);
void writeToRelay2(int relayPin, bool pinValue);
bool toggleRelayState2(int relayPin, bool relayState);
void writeToRelay3(int relayPin, bool pinValue);
bool toggleRelayState3(int relayPin, bool relayState);
void writeToRelay4(int relayPin, bool pinValue);
bool toggleRelayState4(int relayPin, bool relayState);

BLYNK_CONNECTED()
{
  Blynk.syncAll();
}

BLYNK_WRITE(V1)
{
  int pinValue = param.asInt();
  writeToRelay1(relayPin1, pinValue);
}
BLYNK_WRITE(V2)
{
  int pinValue = param.asInt();
  writeToRelay2(relayPin2, pinValue);
}
BLYNK_WRITE(V3)
{
  int pinValue = param.asInt();
  writeToRelay3(relayPin3, pinValue);
}
BLYNK_WRITE(V4)
{
  int pinValue = param.asInt();
  writeToRelay4(relayPin4, pinValue);
}
void setup()
{
  Serial.begin(9600);
  readConfig();
  Blynk.config(auth);
  Blynk.connect();

  pinMode(hotspotPin, INPUT_PULLUP);

  pinMode(buttonPin1, INPUT_PULLUP);
  pinMode(relayPin1, OUTPUT);
  pinMode(buttonPin2, INPUT_PULLUP);
  pinMode(relayPin2, OUTPUT);
  pinMode(buttonPin3, INPUT_PULLUP);
  pinMode(relayPin3, OUTPUT);
  pinMode(buttonPin4, INPUT_PULLUP);
  pinMode(relayPin4, OUTPUT);

  writeToRelay1(relayPin1, true);
  writeToRelay2(relayPin2, true);
  writeToRelay3(relayPin3, true);
  writeToRelay4(relayPin4, true);

  buttonState1 = getState(digitalRead(buttonPin1));
  buttonState2 = getState(digitalRead(buttonPin2));
  buttonState3 = getState(digitalRead(buttonPin3));
  buttonState4 = getState(digitalRead(buttonPin4));

  timer.setInterval(100L, buttonPressedOnline);
}

void loop()
{
  checkHotpotButton();
  buttonPressedOffline();
  Blynk.run();
  timer.run();
}

bool getState(bool pullUp)
{
  return !pullUp;
}

void writeToRelay1(int relayPin, bool pinValue)
{
  digitalWrite(relayPin, pinValue);
  relayState1 = pinValue;
}
void writeToRelay2(int relayPin, bool pinValue)
{
  digitalWrite(relayPin, pinValue);
  relayState2 = pinValue;
}
void writeToRelay3(int relayPin, bool pinValue)
{
  digitalWrite(relayPin, pinValue);
  relayState3 = pinValue;
}
void writeToRelay4(int relayPin, bool pinValue)
{
  digitalWrite(relayPin, pinValue);
  relayState4 = pinValue;
}

void buttonPressedOnline()
{
  if (buttonState1 != getState(digitalRead(buttonPin1)))
  {
    toggleRelayState1(relayPin1, relayState1);
    buttonState1 = getState(digitalRead(buttonPin1));
  }
  if (buttonState2 != getState(digitalRead(buttonPin2)))
  {
    toggleRelayState2(relayPin2, relayState2);
    buttonState2 = getState(digitalRead(buttonPin2));
  }
  if (buttonState3 != getState(digitalRead(buttonPin3)))
  {
    toggleRelayState3(relayPin3, relayState3);
    buttonState3 = getState(digitalRead(buttonPin3));
  }
  if (buttonState4 != getState(digitalRead(buttonPin4)))
  {
    toggleRelayState4(relayPin4, relayState4);
    buttonState4 = getState(digitalRead(buttonPin4));
  }
}

void buttonPressedOffline()
{
  if (Blynk.connected() == false)
  {
    if (buttonState1 != getState(digitalRead(buttonPin1)))
    {
      writeToRelay1(relayPin1, !relayState1);
    }
    if (buttonState2 != getState(digitalRead(buttonPin2)))
    {
      writeToRelay2(relayPin2, !relayState2);
    }
    if (buttonState3 != getState(digitalRead(buttonPin3)))
    {
      writeToRelay3(relayPin3, !relayState3);
    }
    if (buttonState4 != getState(digitalRead(buttonPin4)))
    {
      writeToRelay4(relayPin4, !relayState4);
    }
  }
  delay(100);
}

bool toggleRelayState1(int relayPin, bool relayState)
{
  writeToRelay1(relayPin, !relayState);
  Blynk.virtualWrite(V1, !relayState);
}
bool toggleRelayState2(int relayPin, bool relayState)
{
  writeToRelay2(relayPin, !relayState);
  Blynk.virtualWrite(V2, !relayState);
}
bool toggleRelayState3(int relayPin, bool relayState)
{
  writeToRelay3(relayPin, !relayState);
  Blynk.virtualWrite(V3, !relayState);
}
bool toggleRelayState4(int relayPin, bool relayState)
{
  writeToRelay4(relayPin, !relayState);
  Blynk.virtualWrite(V4, !relayState);
}

void readConfig()
{

  //read configuration from FS json
  Serial.println("mounting FS...");

  if (SPIFFS.begin())
  {
    Serial.println("mounted file system");
    if (SPIFFS.exists("/config.json"))
    {
      //file exists, reading and loading
      Serial.println("reading config file");
      File configFile = SPIFFS.open("/config.json", "r");
      if (configFile)
      {
        Serial.println("opened config file");
        size_t size = configFile.size();
        // Allocate a buffer to store contents of the file.
        std::unique_ptr<char[]> buf(new char[size]);

        configFile.readBytes(buf.get(), size);
        DynamicJsonBuffer jsonBuffer;
        JsonObject &json = jsonBuffer.parseObject(buf.get());
        json.printTo(Serial);
        if (json.success())
        {
          Serial.println("\nparsed json");

          strcpy(auth, json["blynk_token"]);
        }
        else
        {
          Serial.println("failed to load json config");
        }
      }
    }
  }
  else
  {
    Serial.println("failed to mount FS");
  }
}
void writeConfig()
{
  //save the custom parameters to FS
  if (shouldSaveConfig)
  {
    Serial.println("saving config");
    DynamicJsonBuffer jsonBuffer;
    JsonObject &json = jsonBuffer.createObject();

    json["blynk_token"] = auth;

    File configFile = SPIFFS.open("/config.json", "w");
    if (!configFile)
    {
      Serial.println("failed to open config file for writing");
    }

    json.printTo(Serial);
    json.printTo(configFile);
    configFile.close();
  }
}
void checkHotpotButton()
{
  if (!digitalRead(hotspotPin))
  {
    wifiManager.setCustomHeadElement("<style> body{background-color:#212121;color:white;} button{background-color:#febhotspotPin18;} </style>");
    WiFiManagerParameter custom_blynk_token("Blynk", "blynk token", auth, 33);
    wifiManager.setSaveConfigCallback(saveConfigCallback);
    wifiManager.addParameter(&custom_blynk_token);
    wifiManager.startConfigPortal(ssid);
    Serial.println("connected:)");
    strcpy(auth, custom_blynk_token.getValue());
    writeConfig();
  }
}