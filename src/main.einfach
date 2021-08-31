// PhysicalTwinsIdentification using WireComm ##########################
// Masterthesis V 0.1.0 ################################################
// by Joel Lehmann #####################################################
// 31.08.2021 ##########################################################

#include <Arduino.h>
#include <Wire.h>
#include <SoftwareSerial.h> 
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include "dtprovision.h"
#include "noderedprovision.h"
#include "..\..\Credentials\credentials.h"
#include <ArduinoJson.h>

#define SDA_PIN 4
#define SCL_PIN 5
#define RX_PIN D7                                          
#define TX_PIN D6 
#define defHonoTenant "HSMA"
#define defHonoNamespace "HSMA"
#define defHonoDevice "smartDTtest"
#define defHonoDevicePassword "sehrgeheim"
#define defServerIP "http://twinserver.kve.hs-mannheim.de"
#define defTelemetryPort 18443
#define defDevRegPort "28443"
#define defDittoPort "38443"
#define defProvDelay 1000
#define defDevOpsUser "devops"
#define defDevOpsPwd "foo"
#define defDittoUser "ditto"
#define defDittoPwd "ditto"
#define defCaseColor "white"
#define defDisplayColor "green"
#define Version "0.1.0"
#define Date "31.08.2021"

const int16_t I2C_MASTER = 0x42;
const int16_t I2C_SLAVE = 0x08;
int count = 0;
int j = 0;
char c;
char metaCount;
char mesCount;
char buffer[200];
char buffertmp[200];
bool bEOS;
String sMeta;
StaticJsonDocument<200> doc;

typedef struct
  {
    String key;
    String value;
  }  kvp;

kvp META[23];
kvp MES[23];

String serverName;
int httpResponseCode;
unsigned long lastTime = 0;
unsigned long timerDelay = 5000;
int httpResponse;
int httpResonse;
const String honoTenant = defHonoTenant;
const String honoNamespace = defHonoNamespace;
const String honoDevice = defHonoDevice;
int counter = 4990;


const size_t lenJsonString = 10000;
char jsonString[lenJsonString];
const char* chonoTenant = defHonoTenant;
const char* chonoNamespace = defHonoNamespace;
const char* chonoDevice = defHonoDevice;

DigitalTwin DigitalTwinInstance;
NodeRed NodeRedInstance;

const char* SSID = WLANSSID;
const char* PSK = WLANPSK;
const char* MQTT_BROKER = "141.19.44.65";
String tmpMqttUser = honoDevice + "@" + honoTenant;
const char* mqttUser = tmpMqttUser.c_str();
const char* mqttPassword = defHonoDevicePassword;
const char* clientId = defHonoDevice;
char wiFiHostname[ ] = defHonoDevice;
int i = 0;
char antwort[9];
bool noWifi = false;
int cntWifi;

WiFiClient espClient;
PubSubClient client(espClient);

//######################################################################
// Build publish String ################################################
//######################################################################

void buildPubString (String prop, String val )
{
  //char * property[] = prop.c_str();
  //char * value[] = val.c_str();
  memset(jsonString, NULL, sizeof jsonString);
  char tmp[32];

  strcpy(jsonString,"{  \"topic\": \"");
  strcat(jsonString,chonoNamespace);
  strcat(jsonString,"/");
  strcat(jsonString,chonoDevice);
  strcat(jsonString,"/things/twin/commands/modify\",  \"headers\": {},  \"path\": \"/features/telemetry/properties/");
  strcat(jsonString,prop.c_str());
  strcat(jsonString,"/value\",  \"value\": ");
  strcat(jsonString,val.c_str());
  strcat(jsonString,"}");
}

void buildPubStringString (char * property, char * value )
{
  memset(jsonString, NULL, sizeof jsonString);
  char tmp[32];

  strcpy(jsonString,"{  \"topic\": \"");
  strcat(jsonString,chonoNamespace);
  strcat(jsonString,"/");
  strcat(jsonString,chonoDevice);
  strcat(jsonString,"/things/twin/commands/modify\",  \"headers\": {},  \"path\": \"/features/telemetry/properties/");
  strcat(jsonString,property);
  strcat(jsonString,"/value\",  \"value\": ");
  strcat(jsonString,value);
  strcat(jsonString,"}");
}

//######################################################################
// MQTT CALLBACK #######################################################
//######################################################################

void callback(char* topic, byte* payload, unsigned int length) {
 
  //Serial.print("Message arrived in topic: ");
  //Serial.println(topic);
 
  //Serial.print("Message:");
  //for (int i = 0; i < length; i++) {
  //  Serial.print((char)payload[i]);
  //}
  // hier strcmp verwenden!!!

  if (String(topic) == "command///req//espRestart") {
    Serial.println("-----------------------");
    Serial.println("COMMAND FROM DIGITAL TWIN: Restarting Device in 3 sec.");
    Serial.println("-----------------------");
    delay(3000);
    ESP.restart();
  }

  Serial.println("-----------------------");
 
}

//######################################################################
// SETUP WiFi ##########################################################
//######################################################################

void setup_wifi() {
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(SSID);
    wifi_station_set_hostname(wiFiHostname);
    WiFi.begin(SSID, PSK);
    while (WiFi.status() != WL_CONNECTED && cntWifi < 40) {
        delay(500);
        Serial.print(".");
        cntWifi++;
        noWifi = false;
    }
    
    if (cntWifi > 35){
      noWifi = true;
      Serial.println("NO WIFI");
    }
    
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
}

//######################################################################
// SETUP ###############################################################
//######################################################################

void setup() {
  
  Serial.begin(9600);
  Wire.begin(SDA_PIN, SCL_PIN, I2C_MASTER);
  Wire.setClock(30000);

  setup_wifi();
  client.setServer(MQTT_BROKER, 8883);
  client.setCallback(callback);
  client.subscribe("command/+/+/req/#");

  //######################################################################
  // DTI INIT ############################################################
  //######################################################################

  DigitalTwinInstance.init(espClient, defServerIP, defDevRegPort, defDittoPort, defProvDelay);

  //######################################################################
  // HTTP POST CREATE TENANT #############################################
  //######################################################################

  httpResponse = DigitalTwinInstance.createHonoTenant(chonoTenant);

  //######################################################################
  // HTTP POST CREATE DEVICE #############################################
  //######################################################################

  httpResponse = DigitalTwinInstance.createHonoDevice(defHonoNamespace, defHonoDevice);

  //######################################################################
  // HTTP PUT CREDENTIALS ################################################
  //######################################################################

  httpResponse = DigitalTwinInstance.createHonoCredentials(defHonoDevicePassword);

  //######################################################################
  // HTTP POST DITTO PIGGYBACK ###########################################
  //######################################################################
  
  strcpy(jsonString,"{\"targetActorSelection\":\"/system/sharding/connection\",\"headers\":{\"aggregate\":false},\"piggybackCommand\":{\"type\":\"connectivity.commands:createConnection\",\"connection\":{\"id\":\"hono-connection-for-");
  strcat(jsonString,chonoTenant);
  strcat(jsonString,"\",\"connectionType\":\"amqp-10\",\"connectionStatus\":\"open\",\"uri\":\"amqp://consumer%40HONO:verysecret@c2e-dispatch-router-ext:15672\",\"failoverEnabled\":true,\"sources\":[{\"addresses\":[\"telemetry/");
  strcat(jsonString,chonoTenant);
  strcat(jsonString,"\",\"event/");
  strcat(jsonString,chonoTenant);
  strcat(jsonString,"\"],\"authorizationContext\":[\"pre-authenticated:hono-connection\"],\"enforcement\":{\"input\":\"{{header:device_id}}\",\"filters\":[\"{{entity:id}}\"]},\"headerMapping\":{\"hono-device-id\":\"{{header:device_id}}\",\"content-type\":\"{{header:content-type}}\"},\"replyTarget\":{\"enabled\":true,\"address\":\"{{header:reply-to}}\",\"headerMapping\":{\"to\":\"command/");
  strcat(jsonString,chonoTenant);
  strcat(jsonString,"/{{header:hono-device-id}}\",\"subject\":\"{{header:subject|fn:default(topic:action-subject)|fn:default(topic:criterion)}}-response\",\"correlation-id\":\"{{header:correlation-id}}\",\"content-type\":\"{{header:content-type|fn:default(\'application/vnd.eclipse.ditto+json\')}}\"},\"expectedResponseTypes\":[\"response\",\"error\"]},\"acknowledgementRequests\":{\"includes\":[],\"filter\":\"fn:filter(header:qos,\'ne\',\'0\')\"}},{\"addresses\":[\"command_response/");
  strcat(jsonString,chonoTenant);
  strcat(jsonString,"/replies\"],\"authorizationContext\":[\"pre-authenticated:hono-connection\"],\"headerMapping\":{\"content-type\":\"{{header:content-type}}\",\"correlation-id\":\"{{header:correlation-id}}\",\"status\":\"{{header:status}}\"},\"replyTarget\":{\"enabled\":false,\"expectedResponseTypes\":[\"response\",\"error\"]}}],\"targets\":[{\"address\":\"command/");
  strcat(jsonString,chonoTenant);
  strcat(jsonString,"\",\"authorizationContext\":[\"pre-authenticated:hono-connection\"],\"topics\":[\"_/_/things/live/commands\",\"_/_/things/live/messages\"],\"headerMapping\":{\"to\":\"command/");
  strcat(jsonString,chonoTenant);
  strcat(jsonString, "/{{thing:id}}\",\"subject\":\"{{header:subject|fn:default(topic:action-subject)}}\",\"content-type\":\"{{header:content-type|fn:default(\'application/vnd.eclipse.ditto+json\')}}\",\"correlation-id\":\"{{header:correlation-id}}\",\"reply-to\":\"{{fn:default(\'command_response/");
  strcat(jsonString,chonoTenant);
  strcat(jsonString,"/replies\')|fn:filter(header:response-required,\'ne\',\'false\')}}\"}},{\"address\":\"command/");
  strcat(jsonString,chonoTenant);
  strcat(jsonString,"\",\"authorizationContext\":[\"pre-authenticated:hono-connection\"],\"topics\":[\"_/_/things/twin/events\",\"_/_/things/live/events\"],\"headerMapping\":{\"to\":\"command/");
  strcat(jsonString,chonoTenant);
  strcat(jsonString,"/{{thing:id}}\",\"subject\":\"{{header:subject|fn:default(topic:action-subject)}}\",\"content-type\":\"{{header:content-type|fn:default(\'application/vnd.eclipse.ditto+json\')}}\",\"correlation-id\":\"{{header:correlation-id}}\"}}]}}}");
  
  httpResponse = DigitalTwinInstance.createDittoPiggyback(defDevOpsUser, defDevOpsPwd, jsonString);
  
  memset(jsonString, NULL, sizeof jsonString);

  //######################################################################
  // HTTP CREATE DITTO POLICY ############################################
  //######################################################################

  strcpy(jsonString, R"=====(
    {
      "entries": {
        "DEFAULT": {
          "subjects": {
            "{{ request:subjectId }}": {
              "type": "Ditto user authenticated via nginx"
            }
          },
          "resources": {
            "thing:/": {
              "grant": ["READ", "WRITE"],
              "revoke": []
            },
            "policy:/": {
              "grant": ["READ", "WRITE"],
              "revoke": []
            },
            "message:/": {
              "grant": ["READ", "WRITE"],
              "revoke": []
            }
          }
        },
        "HONO": {
          "subjects": {
            "pre-authenticated:hono-connection": {
              "type": "Connection to Eclipse Hono"
            }
          },
          "resources": {
            "thing:/": {
              "grant": ["READ", "WRITE"],
              "revoke": []
            },
            "message:/": {
              "grant": ["READ", "WRITE"],
              "revoke": []
            }
          }
        }
      }
    }
  )=====");

  httpResponse = DigitalTwinInstance.createDittoPolicy(defDittoUser, defDittoPwd, jsonString);
  
  memset(jsonString, NULL, sizeof jsonString);
  
  //######################################################################
  //Request Number of Metadata ###########################################
  //######################################################################

  Wire.beginTransmission(I2C_SLAVE);
  Wire.write("reqMetaCount\n");
  Wire.endTransmission();
  delay(100);
  Wire.requestFrom(I2C_SLAVE, 1);
  while (Wire.available()) { 
    c = Wire.read();
    metaCount = c;      
  }

  //######################################################################
  //Request Metadata #####################################################
  //######################################################################

  while (j < (int)metaCount-48)
    {
      Wire.beginTransmission(I2C_SLAVE);
      Wire.write((String(j)+"\n").c_str());
      Wire.endTransmission();  
      delay(100);
      Wire.requestFrom(I2C_SLAVE, 23);
      while (Wire.available()) { 
      c = Wire.read();
      if (c != 255 && !bEOS) {   
        sMeta += c;     
      } else {
        bEOS = true;
      }
    }
    Serial.println("");
    Serial.print("KVP1 ");
    Serial.print(j);
    Serial.print(" -> ");
    META[j].key = sMeta.substring(0,sMeta.indexOf(":"));
    META[j].value = sMeta.substring(sMeta.indexOf(":")+1,sMeta.length());
    sMeta = "";
    bEOS = false;

    Serial.print(META[j].key);
    Serial.print(":");
    Serial.print(META[j].value);
    Serial.println("");
    j++;
    }
  j = 0;

  //######################################################################
  // HTTP CREATE DITTO THING #############################################
  //######################################################################

  strcpy(jsonString, R"=====(
    {
      "policyId": ")====="); 
  strcat(jsonString,chonoNamespace);
  strcat(jsonString,":");
  strcat(jsonString,chonoDevice);
  strcat(jsonString,R"=====(",
      "attributes": {
        "devicetype": "Indoor Air Quality Measurement",
        "deviceColor": ")=====");
  strcat(jsonString,defCaseColor); 
  strcat(jsonString,R"=====(",
        "location": "Mannheim",
        "institute": "KVE",
        "building": "P",
        "room": "007" ,
      "attachedDevices": )=====");

  //Building up ATTRIBUTES JSON ##########################################

  JsonObject devid = doc.createNestedObject(META[0].value);
  j = 1;
  
  while (j < (int)metaCount-48) 
  {
    devid[META[j].key] = META[j].value;
    j++;
  }

  serializeJson(doc, buffer);
  j = 0;
  doc.clear();

  strcat(jsonString,buffer); 
  memset(buffer, 0, sizeof buffer);
  strcat(jsonString,R"=====(},
      "features": {
      }
    }
  )=====");

  //Serial.println(jsonString);
  httpResponse = DigitalTwinInstance.createDittoThing(jsonString);

  memset(jsonString, NULL, sizeof jsonString);
 
  //######################################################################
  // HTTP CREATE DITTO THING FEATURES ####################################
  //######################################################################

  //Building up FEATURES JSON ############################################

  Wire.beginTransmission(I2C_SLAVE);  
  Wire.write("reqMesCount\n");
  Wire.endTransmission();
  delay(100);
  Wire.requestFrom(I2C_SLAVE, 1);
  while (Wire.available()) { 
    c = Wire.read();
    mesCount = c;      
  }

  while (j < (int)mesCount-48)
    {
      Wire.beginTransmission(I2C_SLAVE);
      Wire.write((String(j)+"\n").c_str());
      Wire.endTransmission();  
      delay(100);
      Wire.requestFrom(I2C_SLAVE, 23);
      while (Wire.available()) { 
        c = Wire.read();
        if (c != 255 && !bEOS) {   
          sMeta += c;     
        } else {
          bEOS = true;
        }
      }
      Serial.print("KVP2 ");
      Serial.print(j);
      Serial.print(" -> ");
      MES[j].key = sMeta.substring(0,sMeta.indexOf(":"));
      MES[j].value = sMeta.substring(sMeta.indexOf(":")+1,sMeta.length());
      sMeta = "";
      bEOS = false;

      Serial.print(MES[j].key);
      Serial.print(":");
      Serial.print(MES[j].value);
      Serial.println("");
      j++;
    }
  j = 0;

  while (j < (int)mesCount-48) 
    {
      JsonObject mesid = doc.createNestedObject(MES[j].key);
      mesid["value"] = "init";
      mesid["unit"] = MES[j].value;
      j++;
    }

    serializeJson(doc, buffer);
    Serial.println(buffer);
    j = 0;
    
    doc.clear();

  strcpy(jsonString, R"=====(
    {
      "telemetry": {
        "properties": )=====");

  strcat(jsonString,buffer);
  memset(buffer, 0, sizeof buffer);

  strcat(jsonString,R"=====(   }
    }
  )=====");

  Serial.println(jsonString);

  httpResponse = DigitalTwinInstance.createDittoFeatures(jsonString);

  memset(jsonString, NULL, sizeof jsonString);

  //######################################################################
  // HTTP CREATE NODERED DASHBOARD #######################################
  //######################################################################
/*
  lcd.clear();
  lcd.setCursor(0,1);
  lcd.print("NR Dashboard PROV");
  lcd.setCursor(0,3);
  //lcd.print("HTTP RESPONSE:");
  //lcd.setCursor(16,3);
  //lcd.print(httpResponse);

  NodeRedInstance.init(espClient, "http://twinserver.kve.hs-mannheim.de:18443", honoNamespace + ":" + honoDevice, "10");

  String dittoAddress = "http://ditto:ditto@twinserver.kve.hs-mannheim.de:38443/api/2/things/"+honoNamespace+":"+honoDevice+"/features/telemetry/properties/";

  NodeRedInstance.addText(dittoAddress + "DisplayBacklight/value", "Display Backlight");

  NodeRedInstance.addGauge(dittoAddress + "Temperature/value", "TEMP", "°C", 50, 5, 1);
  NodeRedInstance.addGauge(dittoAddress + "Humidity/value", "HUM", "%", 100, 0, 1);
  NodeRedInstance.addGauge(dittoAddress + "Pressure/value", "PRESS", "hPa", 1050, 950, 1);
  NodeRedInstance.addGauge(dittoAddress + "CO2/value", "CO2", "ppm", 2000, 400, 15);

  //NodeRedInstance.addChart(dittoAddress + "Temp/value", "TEMP", 50, 5, 1, 10);
  //NodeRedInstance.addChart(dittoAddress + "Hum/value", "HUM", 100, 0, 1, 10);
  //NodeRedInstance.addChart(dittoAddress + "Press/value", "PRESS", 1050, 950, 1, 10);
  NodeRedInstance.addChart(dittoAddress + "CO2/value", "CO2", 2000, 400, 15, 10);

  String dittoCommandAddress = "http://ditto:ditto@twinserver.kve.hs-mannheim.de:38443/api/2/things/"+honoNamespace+":"+honoDevice+"/inbox/messages/";

  NodeRedInstance.addSwitch(dittoCommandAddress + "backlightOff?timeout=0", dittoCommandAddress + "backlightOn?timeout=0", "Display Backlight");

  NodeRedInstance.addButton(dittoCommandAddress + "espRestart?timeout=0", "Reboot Device");
  
  NodeRedInstance.createNodeRedDashboard();
*/
}

//######################################################################
// LOOP ################################################################
//######################################################################
void loop() 
{ 

  while (!client.connected()) {
    Serial.println("Connecting to MQTT...");
     if (client.connect(clientId, mqttUser, mqttPassword)) {
      Serial.println("connected");
      client.subscribe("command/+/+/req/#");  
    } else {
      Serial.print("failed with state ");
      Serial.print(client.state());
      delay(2000);
    }
  }

  client.loop();

  //######################################################################
  // REFRESH SENSORDATA AND PUBLISH TO HONO ##############################
  //###################################################################### 

if (counter > 5000)
{
  while (j < (int)mesCount-48)
    {
      Wire.beginTransmission(I2C_SLAVE);
      Wire.write(("req"+String(MES[j].key)+"\n").c_str());
      Wire.endTransmission();  
      delay(100);
      Wire.requestFrom(I2C_SLAVE, 23);
      while (Wire.available()) { 
      c = Wire.read();
      if (c != 255 && !bEOS) {   
        sMeta += c;     
      } else {
        bEOS = true;
      }
    }
    buildPubString(MES[j].key, sMeta);
    client.publish("telemetry", jsonString,false);
    Serial.print(MES[j].key);
    Serial.print(" -> ");
    Serial.println(sMeta);
    sMeta = "";
    bEOS = false;
    j++;
    }
    Serial.println("");
    j = 0;

    counter = 0;
} else 
{
  counter++;
}
  delay(1);  
}

