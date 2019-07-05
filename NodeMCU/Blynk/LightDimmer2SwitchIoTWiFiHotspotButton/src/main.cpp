/**
 * @author Akshay Kalyan
 * @email akshaykalyan2307@gmail.com
 * @create date 2019-07-05 14:54:09
 * @modify date 2019-07-05 16:04:16
 * @desc Blynk Light Dimmer 2 Switch IoT NodeMCU 1.0 Button WiFi hotspot configurator
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
#include <RBDdimmer.h>

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

const int buttonPin1 = D3;
const int relayPin1 = D5;
const int buttonPin2 = D2;
const int relayPin2 = D7;

const int outputPin = 12;
const int zerocross = 5;
int dimS;
dimmerLamp dimmer(outputPin, zerocross);
int outVal = 0;

void readConfig();
void writeConfig();
void checkHotpotButton();
void makeHotspot();

bool buttonState1;
bool relayState1;

bool buttonState2;
bool relayState2;

bool getState(bool pullUp);
void buttonPressedOnline();
void buttonPressedOffline();
void writeToRelay1(int relayPin, bool pinValue);
bool toggleRelayState1(int relayPin, bool relayState);
void writeToRelay2(int relayPin, int pinValue);
bool toggleRelayState2(int relayPin, bool relayState);

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
void setup()
{
  Serial.begin(9600);
  Blynk.config(auth);
  readConfig();
  Blynk.config(auth);
  Blynk.connect();

  pinMode(buttonPin1, INPUT_PULLUP);
  pinMode(relayPin1, OUTPUT);
  pinMode(buttonPin2, INPUT_PULLUP);
  pinMode(relayPin2, OUTPUT);

  writeToRelay1(relayPin1, true);
  writeToRelay2(relayPin2, true);

  buttonState1 = getState(digitalRead(buttonPin1));
  buttonState2 = getState(digitalRead(buttonPin2));

  dimmer.begin(NORMAL_MODE, OFF); //dimmer initialisation: name.begin(MODE, STATE)
  dimmer.setPower(-1);
  Blynk.syncAll();

  timer.setInterval(100L, buttonPressedOnline);
}

void loop()
{
  checkHotpotButton();
  dimmer.setPower(dimS);
  buttonPressedOffline();
  Blynk.run();
  ESP.wdtFeed();
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
void writeToRelay2(int relayPin, int pinValue)
{
  if (pinValue >= 0)
  {
    dimS = pinValue;
    relayState2 = true;
    dimmer.setState(ON);
  }
  else
  {
    relayState2 = false;
    dimmer.setState(OFF);
  }
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
  if (relayState)
  {
    writeToRelay2(relayPin, -1);
    Blynk.virtualWrite(V2, -1);
  }
  else
  {
    writeToRelay2(relayPin, 100);
    Blynk.virtualWrite(V2, 100);
  }
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
    else
    {
      makeHotspot();
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
    makeHotspot();
  }
}
void makeHotspot()
{
  wifiManager.setCustomHeadElement("<style> body{background-color:#212121;color:white;} button{background-color:#feb018;} </style>");
  WiFiManagerParameter custom_blynk_token("Blynk", "blynk token", auth, 33);
  wifiManager.setSaveConfigCallback(saveConfigCallback);
  wifiManager.addParameter(&custom_blynk_token);
  wifiManager.startConfigPortal(ssid);
  Serial.println("connected:)");
  strcpy(auth, custom_blynk_token.getValue());
  writeConfig();
}