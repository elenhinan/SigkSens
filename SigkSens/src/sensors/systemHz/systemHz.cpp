#ifdef ESP8266
extern "C" {
#include "user_interface.h"
}
#endif

#include "../../../config.h"
#include "src/services/configStore.h"
#include "systemHz.h"

SystemHzSensorInfo::SystemHzSensorInfo(String addr) {
  strcpy(address, addr.c_str());
  signalKPath[0] = String("sensors.") + configStore.getString("myHostname") + String(".systemHz");
  signalKPath[1] = String("sensors.") + configStore.getString("myHostname") + String(".freeMem");
  signalKPath[2] = String("sensors.") + configStore.getString("myHostname") + String(".uptime");
  attrName[0] = "systemHz";
  attrName[1] = "freeMem";
  attrName[2] = "uptime";
  type = SensorType::local;
  valueJson[0] = "null";
  valueJson[1] = "null";
  valueJson[2] = "null";

  isUpdated = false;
}

SystemHzSensorInfo::SystemHzSensorInfo(String addr, 
                                       String path1, 
                                       String path2,
                                       String path3) {
  strcpy(address, addr.c_str());
  signalKPath[0] = path1;
  signalKPath[1] = path2;
  signalKPath[2] = path3;
  attrName[0] = "systemHz";
  attrName[1] = "freeMem";
  attrName[2] = "uptime";
  type = SensorType::local;
  valueJson[0] = "null";
  valueJson[1] = "null";
  valueJson[2] = "null";

  isUpdated = false;
}

SystemHzSensorInfo *SystemHzSensorInfo::fromJson(JsonObject &jsonSens) {
  return new SystemHzSensorInfo(
    jsonSens["address"],
    jsonSens["attrs"][0]["signalKPath"],
    jsonSens["attrs"][1]["signalKPath"],
    jsonSens["attrs"][2]["signalKPath"]
  );
}

void SystemHzSensorInfo::toJson(JsonObject &jsonSens) {
  jsonSens["address"] = address;
  jsonSens["type"] = (int)SensorType::local;
  JsonArray& jsonAttrs = jsonSens.createNestedArray("attrs");
  for (int x=0 ; x < MAX_SENSOR_ATTRIBUTES ; x++) {
    if (strcmp(attrName[x].c_str(), "") == 0 ) {
      break; //no more attributes
    }
    JsonObject& attr = jsonAttrs.createNestedObject();
    attr["name"] = attrName[x];
    attr["signalKPath"] = signalKPath[x];
    attr["value"] = valueJson[x];
  }
}


uint32_t systemHzCount = 0, systemHzMs = 0;

float systemHz = 0;


// forward declarations
void countSystemHz();
void updateSystemHz();


void setupSystemHz(bool &need_save) {
  systemHzMs = millis();

  // Setup "sensor" if not already existing
  bool known = sensorStorage[(int)SensorType::local].find(
    "Local") != nullptr;

  if (!known) {
    Serial.print(F("Setting up System info "));
    SensorInfo *newSensor = new SystemHzSensorInfo("Local");
    sensorStorage[(int)newSensor->type].add(newSensor);
    need_save = true;
  }
  app.onTick(countSystemHz);
  app.onRepeat(1000, updateSystemHz);
}


void countSystemHz() {
  systemHzCount++;
}


void updateSystemHz() {
  uint32_t elapsed = millis() - systemHzMs;
  
  if (elapsed == 0) { return; } // getting sporadic divide by 0 exceptions, no harm in skipping a loop.
  
  systemHz = (systemHzCount*1000) / elapsed;
 // Serial.print ("System Hz :");
 // Serial.println (systemHz);

  sensorStorage[(int)SensorType::local].forEach([&](SensorInfo* si) {
    if (strcmp(si->signalKPath[0].c_str(),  "") != 0) {
      si->valueJson[0] = systemHz;
      si->isUpdated = true;
    }
    if (strcmp(si->signalKPath[1].c_str(),  "") != 0) {
      si->valueJson[1] = ESP.getFreeHeap();
      si->isUpdated = true;
    }
    if (strcmp(si->signalKPath[2].c_str(),  "") != 0) {
      si->valueJson[2] = (float)millis()/1000.0;
      si->isUpdated = true;
    }
  });

  systemHzCount = 0;
  systemHzMs = millis();
}
