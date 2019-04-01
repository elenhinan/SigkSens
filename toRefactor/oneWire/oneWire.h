#ifndef _oneWire_H_
#define _oneWire_H_

#include "../../../sigksens.h"
#include "../sensorStorage.h"


class OneWireSensorInfo : public SensorInfo {
  public:
    OneWireSensorInfo(String addr);
    OneWireSensorInfo(String addr, String path);

    static OneWireSensorInfo *fromJson(JsonObject &jsonSens);
    void toJson(JsonObject &jsonSens);
};

void setup1Wire(bool&);

#endif
