
#include <FS.h> //this needs to be first, or it all crashes and burns...

#ifdef ESP32
#include "SPIFFS.h"
#endif

#include <ArduinoJson.h>     //https://github.com/bblanchon/ArduinoJson
#include <string>

#include "config.h"

#include "FSConfig.h"

#include "src/sensors/sensorStorage.h"
#include "src/net/webSocket.h"

#ifdef ENABLE_DIGITALIN
  #include "src/sensors/digitalIn/digitalIn.h"
#endif
#ifdef ENABLE_ANALOGIN
  #include "src/sensors/analogIn/analogIn.h"
#endif
#ifdef ENABLE_SYSTEMHZ
  #include "src/sensors/systemHz/systemHz.h"
#endif
#include "sigksens.h"

/*----------------------------------------------------------------------------
------------------------------------------------------------------------------
Config Save/Load/Reset
------------------------------------------------------------------------------
----------------------------------------------------------------------------*/


void setupFS() {
  bool mountOk = false;
  #ifdef ESP8266
  mountOk = SPIFFS.begin();
  #elif defined ESP32
  mountOk = SPIFFS.begin(true); //true = format spiffs if it's not already done :)
  #endif
  if (mountOk) {
    Serial.println(F("mounted file system"));
    #ifdef ESP8266
    Serial.println(F("FS Contents:"));
    String str = "";
    Dir dir = SPIFFS.openDir("/");
    while (dir.next()) {
      str += dir.fileName();
      str += " / ";
      str += dir.fileSize();
      str += "\r\n";
    }
    Serial.print(str); 
    #endif
  } else {
    Serial.println(F("failed to mount filesystem, It is possible that the SPIFF setting in your uploader is set to 'No SPIFFS'."));
    delay(64000);

    #ifdef ESP8266
      ESP.reset();
    #elif defined(ESP32)
      ESP.restart();
    #endif
        
  }

}

void saveConfig() {
  Serial.println(F("saving config"));
  DynamicJsonBuffer jsonBuffer;

  JsonObject& json = jsonBuffer.createObject();
  
  json["signalKHost"] = signalKClientInfo.configuredHost;
  json["signalKPort"] = signalKClientInfo.configuredPort;
  json["signalKPath"] = signalKClientInfo.path;
  json["signalKToken"] = signalKClientInfo.authToken;


  //sensors
  JsonArray& jsonSensors = json.createNestedArray("sensors");
  //for (uint8_t i=0; i < sensorStorage.size(); i++) 
  sensorStorageForEach([&](SensorInfo* si){
    Serial.print(F("Saving sensor "));
    Serial.println(si->address);
    si->toJson(jsonSensors.createNestedObject());
  });


  
  File configFile = SPIFFS.open("/config.json", "w");
  if (!configFile) {
    Serial.println(F("failed to open config file for writing"));
  }

  json.prettyPrintTo(Serial);
  json.printTo(configFile);
  configFile.close();
}

void loadConfig() {
  SensorInfo *newSensor;
  char tempStr[255];

  if (SPIFFS.exists("/config.json")) {
    //file exists, reading and loading
    Serial.println(F("reading config file"));
    File configFile = SPIFFS.open("/config.json", "r");
    if (configFile) {
      Serial.println(F("opened config file"));
      size_t size = configFile.size();
      // Allocate a buffer to store contents of the file.
      std::unique_ptr<char[]> buf(new char[size]);

      configFile.readBytes(buf.get(), size);
      DynamicJsonBuffer jsonBuffer;
      JsonObject& json = jsonBuffer.parseObject(buf.get());
      Serial.println(F("Current Configuration:"));
      json.prettyPrintTo(Serial);
      if (json.success()) {
        Serial.println(F(""));
        //signalk
        strcpy(tempStr, json["signalKHost"]); signalKClientInfo.configuredHost = tempStr;
        strcpy(tempStr, json["signalKPath"]); signalKClientInfo.path = tempStr;
        strcpy(tempStr, json["signalKToken"]); signalKClientInfo.authToken = tempStr;
        signalKClientInfo.configuredPort = json["signalKPort"];

        // load known sensors
        for (uint8_t i=0; i < json["sensors"].size(); i++) {
          int type = json["sensors"][i]["type"];
          fromJsonFunc func = fromJson[type];
          if ((int)func != 0) {
            newSensor = fromJson[type](json["sensors"][i]);
            sensorStorage[(int)newSensor->type].add(newSensor);
          }
        }

      } else {
        Serial.println(F("failed to load json config"));
      }
    }
  }
}
