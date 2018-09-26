#ifndef _sht30_H_
#define _sht30_H_

#include "../../../sigksens.h"
#include "../sensorStorage.h"



class SHT30SensorInfo : public SensorInfo {
  public:
    SHT30SensorInfo(String addr);
    SHT30SensorInfo(String addr, String path1, String path2);

    static SHT30SensorInfo *fromJson(JsonObject &jsonSens);
    void toJson(JsonObject &jsonSens);
};


void setupSHT30();
void handleSHT30(bool&);


#endif
