#include <FS.h>
#include <Arduino.h>
#define BLYNK_PRINT Serial
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <EEPROM.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>
#include <ArduinoJson.h>

char auth[33];

//default hotspot password
char ssid[] = "LSM";
char pass[] = "lesmartomation";

//wifi portal timeout
int portalTimeout = 30;

const int buttonPin = D1;
const int relayPin = D5;
//flag for saving data
bool buttonState;
bool relayState;

BlynkTimer timer;

WiFiManager wifiManager;

bool shouldSaveConfig = false;

//callback notifying us of the need to save config
void saveConfigCallback()
{
  Serial.println("Should save config");
  shouldSaveConfig = true;
}

bool getState(bool pullUp);
void writeToRelay(int relayPin, bool pinValue);
void buttonPressedOnline();
void buttonPressedOffline();
bool toggleRelayState(int relayPin, bool relayState);
void readConfig();
void writeConfig();
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
  readConfig();
  wifiManager.setCustomHeadElement("<style> body{background-color:#212121;color:white;} button{background-color:#feb018;} </style>");
  WiFiManagerParameter custom_blynk_token("Blynk", "blynk token", auth, 33);
  wifiManager.setSaveConfigCallback(saveConfigCallback);
  wifiManager.addParameter(&custom_blynk_token);
  // wifiManager.autoConnect(ssid, pass);
  wifiManager.setConfigPortalTimeout(portalTimeout);
  if (!wifiManager.autoConnect(ssid, pass))
  {
    Serial.println("failed to connect and hit timeout");
    delay(3000);
    //reset and try again, or maybe put it to deep sleep
    ESP.reset();
    delay(5000);
  }
  Serial.println("connected:)");
  strcpy(auth, custom_blynk_token.getValue());
  writeConfig();
  Blynk.config(auth);
  bool result = Blynk.connect();

  if (result != true)
  {
    Serial.println("BLYNK Connection Fail");
    Serial.println(auth);
    wifiManager.resetSettings();
    ESP.reset();
    delay(5000);
  }
  else
  {
    Serial.println("BLYNK Connected");
  }

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
  //end read
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
    //end save
  }
}