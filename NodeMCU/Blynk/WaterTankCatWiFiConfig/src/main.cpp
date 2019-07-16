/**
 * @author Akshay Kalyan
 * @email akshaykalyan2307@gmail.com
 * @create date 2019-07-16 12:22:18
 * @modify date 2019-07-16 12:22:18
 * @desc Blynk Water tank with auto hydroelectric motor cuttoff monitoring electric IoT NodeMCU 1.0 Intelligent WiFi hotspot configurator
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
char ssid[] = "LSM";
char pass[] = "lesmartomation";

//wifi portal timeout
int portalTimeout = 30;

const int trigPin = D6; // Trigger Pin of Ultrasonic Sensor
const int echoPin = D7; // Echo Pin of Ultrasonic Sensor
const int buttonPin = D1;
const int relayPin = D5;

bool buttonState;
bool relayState;
int clearanceOfTank = 30;
int heightOfTank = 112;

long duration, distance;
int prev;
int prevprev;
double formula;
bool autocut;
// Timer for blynking
BlynkTimer timer;
static bool value = true;

WiFiManager wifiManager;

//flag for saving data
bool shouldSaveConfig = false;

//callback notifying us of the need to save config
void saveConfigCallback()
{
  Serial.println("Should save config");
  shouldSaveConfig = true;
}
//Declaration
void turnoffmotor();
bool getState(bool pullUp);
void writeToRelay(int relayPin, bool pinValue);
void buttonPressedOnline();
void buttonPressedOffline();
void toggleRelayState(int relayPin, bool relayState);
void readConfig();
void writeConfig();

void blynkAnotherDevice()
{
  if (buttonState != getState(digitalRead(buttonPin)))
  {
    toggleRelayState(relayPin, relayState);
    buttonState = getState(digitalRead(buttonPin));
  }
  Serial.println(distance);

  if (distance < clearanceOfTank && prev < clearanceOfTank && prevprev < clearanceOfTank && distance != 0 && prev != 0 && prevprev != 0 && autocut == true)
  {
    turnoffmotor();
  }
  prevprev = prev;
  prev = distance;
  float A, B;
  float C;
  A = heightOfTank - distance;
  B = heightOfTank - clearanceOfTank;
  C = A / B;

  formula = C * 100;
  Serial.println(formula);
  if (distance != 0)
  {
    Blynk.virtualWrite(V7, formula);
  }
}
void turnoffmotor()
{
  toggleRelayState(relayPin, 1);
  buttonState = getState(digitalRead(buttonPin));
}
BLYNK_WRITE(V1)
{
  int pinValue = param.asInt();
  writeToRelay(relayPin, pinValue);
}
BLYNK_WRITE(V8)
{
  int pinValue = param.asInt();
  autocut = pinValue;
}
void setup()
{
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  Serial.begin(9600); // Starting Serial Terminal
  Blynk.begin(auth, ssid, pass);
  pinMode(buttonPin, INPUT_PULLUP);
  pinMode(relayPin, OUTPUT);
  buttonState = getState(digitalRead(buttonPin));
  timer.setInterval(1000L, blynkAnotherDevice);
}

void loop()
{
  buttonPressedOffline();

  digitalWrite(trigPin, LOW);
  delayMicroseconds(5);

  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  duration = pulseIn(echoPin, HIGH);
  // Calculate the distance
  distance = duration * 0.034 / 2;
  //Print the distance on the Serial Monitor (Ctrl+Shift+M)

  Blynk.run();

  timer.run();
}

void buttonPressedOffline()
{
  if (Blynk.connected() == false)
  {
    if (buttonState != getState(digitalRead(buttonPin)))
    {
      writeToRelay(relayPin, !relayState);
    }
    delay(100);
  }
}
void toggleRelayState(int relayPin, bool relayState)
{
  writeToRelay(relayPin, !relayState);
  Blynk.virtualWrite(V1, !relayState);
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