// PhysicalTwinsIdentification using WireComm ##########################
// Masterthesis V 1.1.0 ################################################
// by Joel Lehmann #####################################################
// 24.09.2021 ##########################################################

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
#define defHonoTenant "HSMA"
#define defHonoNamespace "HSMA"
#define defHonoDevice "smartDTtest"
#define defHonoDevicePassword "sehrgeheim"
#define defServerIP "http://twinserver.kve.hs-mannheim.de"
#define defDevRegPort "28443"
#define defDittoPort "38443"
#define defProvDelay 1000
#define defDevOpsUser "devops"
#define defDevOpsPwd "foo"
#define defDittoUser "ditto"
#define defDittoPwd "ditto"
#define defCaseColor "white"
#define defDisplayColor "green"
#define Version "1.0.0"
#define Date "24.09.2021"

//######################################################################
// Constants, Variables and Instances ##################################
//######################################################################

const int16_t I2C_MASTER = 0x42;
int16_t I2C_SLAVE = 0x01;
int i, j, k = 0;
char c;
char metaCount;
char mesCount;
int mesGesCount = -1;
char buffer[200];
char buffertmp[200];
bool bEOS;
String sMeta;
StaticJsonDocument<200> doc;
byte error;
const size_t lenJsonString = 10000;
char jsonString[lenJsonString];
int iAttDevs[2];
int i2cCount;
bool colreq = false;
const String honoTenant = defHonoTenant;
const String honoNamespace = defHonoNamespace;
const String honoDevice = defHonoDevice;
const char* chonoTenant = defHonoTenant;
const char* chonoNamespace = defHonoNamespace;
const char* chonoDevice = defHonoDevice;
const char* SSID = WLANSSID;
const char* PSK = WLANPSK;
const char* MQTT_BROKER = "141.19.44.65";
const String tmpMqttUser = honoDevice + "@" + honoTenant;
const char* mqttUser = tmpMqttUser.c_str();
const char* mqttPassword = defHonoDevicePassword;
const char* clientId = defHonoDevice;
char wiFiHostname[ ] = defHonoDevice;
char antwort[9];
bool noWifi = false;
int cntWifi;
int httpResponse;
int counter = 4990;

typedef struct
{
  String key;
  String value;
}  kvpmeta;
  
typedef struct
{
  String key;
  String value;
  String devid;
}  kvpmes;

kvpmeta META[23];
kvpmes MES[23];
DigitalTwin DigitalTwinInstance;
NodeRed NodeRedInstance;
WiFiClient espClient;
PubSubClient client(espClient);

//######################################################################
// Build publish String ################################################
//######################################################################

void buildPubString (String prop, String val)
{
  memset(jsonString, NULL, sizeof jsonString);
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

//######################################################################
// MQTT CALLBACK #######################################################
//######################################################################

void callback(char* topic, byte* payload, unsigned int length) 
{
  if (String(topic) == "command///req//espRestart") 
  {
    Serial.println("-----------------------");
    Serial.println("COMMAND FROM DIGITAL TWIN: ESP RESTART");
    Serial.println("-----------------------");
    delay(3000);
    ESP.restart();
  }
  Serial.println("-----------------------");
}

//######################################################################
// SETUP WiFi ##########################################################
//######################################################################

void setup_wifi() 
{
  Serial.println();
  Serial.print("Connecting to: ");
  Serial.println(SSID);
  wifi_station_set_hostname(wiFiHostname);
  WiFi.begin(SSID, PSK);
  while (WiFi.status() != WL_CONNECTED && cntWifi < 40) 
  {
    delay(500);
    Serial.print(".");
    cntWifi++;
    noWifi = false;
  }
  if (cntWifi > 35)
  {
    noWifi = true;
    Serial.println("NO WIFI");
    ESP.restart();
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

//######################################################################
// SETUP ###############################################################
//######################################################################

void setup() 
{
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
      "attachedDevices": {)=====");

  //######################################################################
  //Building up ATTRIBUTES JSON ##########################################
  //######################################################################

  i2cCount = 0;
  Serial.println("");
  while(I2C_SLAVE < 0x09)
  {
    Wire.beginTransmission(I2C_SLAVE);
    error = Wire.endTransmission();
    if (error == 0)
    {
      Serial.print("Device found at address: ");
      iAttDevs[i2cCount]=I2C_SLAVE;
      Serial.println(iAttDevs[i2cCount]);
      i2cCount++;
    }
    I2C_SLAVE++;
  }

  //######################################################################
  //Request Number of Metadata ###########################################
  //######################################################################

  k = 0;
  if (i2cCount > 1)
  {
    colreq = true;
  }

  if (i2cCount > 0)
  {
    while (k < i2cCount)
    {
      Serial.println("");
      Serial.print("### I2C Device ");
      Serial.print(k+1);
      Serial.print(" of ");
      Serial.print(i2cCount);
      Serial.print(" at address ");
      Serial.print(iAttDevs[k]);
      Serial.println(" #####################################");
      Wire.beginTransmission(iAttDevs[k]);
      Wire.write("reqMetaCount\n");
      Wire.endTransmission();
      delay(100);
      Wire.requestFrom(iAttDevs[k], 1);
      while (Wire.available()) 
      { 
        c = Wire.read();
        metaCount = c;      
      }
      while (j < (int)metaCount-48)
      {
        Wire.beginTransmission(iAttDevs[k]);
        Wire.write((String(j)+"\n").c_str());
        Wire.endTransmission();  
        delay(100);
        Wire.requestFrom(iAttDevs[k], 23);
        while (Wire.available()) 
        { 
          c = Wire.read();
          if (c != 255 && !bEOS) 
          {   
            sMeta += c;     
          } else {
            bEOS = true;
          }
        }
        Serial.println("");
        Serial.print("KVP ");
        Serial.print(j);
        Serial.print(" -> ");
        if (sMeta != "")
        {
          META[j].key = sMeta.substring(0,sMeta.indexOf(":"));
          META[j].value = sMeta.substring(sMeta.indexOf(":")+1,sMeta.length());
          Serial.print(META[j].key);
          Serial.print(":");
          Serial.print(META[j].value);
          Serial.println("");
        } else
        {
          Serial.println("Error, retry getting KVP!");
          j--;
        }
        sMeta = "";
        bEOS = false;
        j++;
      }
      JsonObject devid = doc.createNestedObject(META[0].value);
      j = 1;  
      while (j < (int)metaCount-48) 
      {
        devid[META[j].key] = META[j].value;
        j++;
      }
      serializeJson(doc, buffertmp);
      strncat(buffer, buffertmp+1,strlen(buffertmp)-2);
      j = 0;
      doc.clear();
      strcat(jsonString,buffer); 
      memset(buffer, 0, sizeof buffer);
      memset(buffertmp, 0, sizeof buffer);
      if (colreq && k < i2cCount - 1)
      {
        strcat(jsonString,",");
      }
      k++;
    }
  }

  strcat(jsonString,R"=====(}},
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

  strcpy(jsonString, R"=====(
  {
    "telemetry": {
      "properties":{ )=====");

  //Building up FEATURES JSON ############################################

  k = 0;
  if (i2cCount > 1)
  {
    colreq = true;
  }
  if (i2cCount > 0)
  {
    while (k < i2cCount)
    {
      Serial.println("");
      Serial.print("### I2C Device: ");
      Serial.print(k+1);
      Serial.print(" of ");
      Serial.print(i2cCount);
      Serial.println(" #####################################");
      Wire.beginTransmission(iAttDevs[k]);  
      Wire.write("reqMesCount\n");
      Wire.endTransmission();
      delay(100);
      Wire.requestFrom(iAttDevs[k], 1);
      while (Wire.available()) 
      { 
        c = Wire.read();
        mesCount = c;          
      }
      while (j < (int)mesCount-48)
      {
        Wire.beginTransmission(iAttDevs[k]);
        Wire.write((String(j)+"\n").c_str());
        Wire.endTransmission();  
        delay(100);
        Wire.requestFrom(iAttDevs[k], 23);
        while (Wire.available()) 
        { 
          c = Wire.read();
          if (c != 255 && !bEOS) 
          {   
            sMeta += c;     
          } else 
          {
              bEOS = true;
          }
        }
        Serial.print("KVP ");
        Serial.print(j);
        Serial.print(" -> ");
        if (sMeta != "")
        {
          mesGesCount++;
          MES[mesGesCount].key = sMeta.substring(0,sMeta.indexOf(":"));
          MES[mesGesCount].value = sMeta.substring(sMeta.indexOf(":")+1,sMeta.length());
          MES[mesGesCount].devid = (String)iAttDevs[k];
          Serial.print(MES[mesGesCount].key);
          Serial.print(":");
          Serial.print(MES[mesGesCount].value);
          Serial.print(" @ ");
          Serial.print(MES[mesGesCount].devid);
          Serial.println("");    
          JsonObject mesid = doc.createNestedObject(MES[mesGesCount].key);
          mesid["value"] = "init";
          mesid["unit"] = MES[mesGesCount].value; 
        } else
        {
          j--;
        }
        sMeta = "";
        bEOS = false;
        j++;
      }      
      j = 0;
      serializeJson(doc, buffertmp);
      strncat(buffer, buffertmp+1,strlen(buffertmp)-2);
      doc.clear();
      strcat(jsonString,buffer); 
      memset(buffer, 0, sizeof buffer);
      memset(buffertmp, 0, sizeof buffer);
      if (colreq && k < i2cCount - 1)
      {
        strcat(jsonString,",");
      }
      k++;
    }
  }
  strcat(jsonString,"}}}");
  //Serial.println(jsonString);
  httpResponse = DigitalTwinInstance.createDittoFeatures(jsonString);
  memset(jsonString, NULL, sizeof jsonString);
}

//######################################################################
// LOOP ################################################################
//######################################################################

void loop() 
{ 
  while (!client.connected()) 
  {
    Serial.println("Connecting to MQTT...");
    if (client.connect(clientId, mqttUser, mqttPassword)) 
    {
      Serial.println("connected");
      client.subscribe("command/+/+/req/#");  
    } else 
    {
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
    while (j <= mesGesCount)
    {
      Wire.beginTransmission(atoi((MES[j].devid).c_str()));
      Wire.write(("req"+String(MES[j].key)+"\n").c_str());
      Wire.endTransmission();  
      delay(100);
      Wire.requestFrom(atoi((MES[j].devid).c_str()), 23);
      while (Wire.available()) 
      { 
        c = Wire.read();
        if (c != 255 && !bEOS) 
        {   
          sMeta += c;     
        } else {
          bEOS = true;
        }
      }
      buildPubString(MES[j].key, sMeta);
      client.publish("telemetry", jsonString,false);
      Serial.print(MES[j].key);
      Serial.print(" -> ");
      Serial.print(sMeta);
      Serial.print(" from devid: ");
      Serial.println(MES[j].devid);
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


