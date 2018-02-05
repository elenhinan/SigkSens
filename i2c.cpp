extern "C" {
#include "user_interface.h"
}

#include <Wire.h>

#include "sigksens.h"
#include "sht30.h"
#include "mpu9250.h"
#include "mpu.h"
#include "ads1115.h"
#include "i2c.h"

bool sensorSHT30Present = false;
bool sensorMPU925XPresent = false;
bool sensorADS1115Present = false;

bool getSensorSHT30Present() { return sensorSHT30Present; }
bool getSensorMPU925XPresent() { return sensorMPU925XPresent; }
bool getSensorADS1115Present() { return sensorADS1115Present; }


bool scanI2CAddress(uint8_t address) {
  uint8_t errorCode;
  
  Wire.beginTransmission(address);
  errorCode = Wire.endTransmission();
  if (errorCode == 0) {
    return true;
  }
  return false;
}


void scanI2C(bool &need_save) {
  SensorInfo *tmpSensorInfo;

  Serial.println("Scanning for i2c Sensors...");

  //SHT30
  if (scanI2CAddress(0x45)) { //Sht on D1 sheild
    sensorSHT30Present = true;
    bool known = false;
    for (int x=0;x<sensorList.size() ; x++) {
      tmpSensorInfo = sensorList.get(x);
      if (strcmp(tmpSensorInfo->address, "0x45") == 0) {
        known = true;                
      }
    }
    if (!known) {
      Serial.print("New SHT Sensor found at: 0x45 ");
      SensorInfo *newSensor = new SHT30SensorInfo("0x45");
      sensorList.add(newSensor);
      need_save = true;
    }    
  }

  //MPU925X
  if (scanI2CAddress(0x68)) {
    sensorMPU925XPresent = true;
    bool known = false;
    for (int x=0;x<sensorList.size() ; x++) {
      tmpSensorInfo = sensorList.get(x);
      if (strcmp(tmpSensorInfo->address, "0x68") == 0) {
        known = true;                
      }
    }    
    if (!known) {
      Serial.print("New MPU925X found at: 0x68 ");
      SensorInfo *newSensor = new MPU9250SensorInfo("0x68");
      sensorList.add(newSensor);
      need_save = true;
    }    
  }

  //ADS1115
  if (scanI2CAddress(0x48)) {
    sensorADS1115Present = true;
    Serial.println("Found ADS1115 chip at 0x48");
    bool known = false;
    for (int x=0;x<sensorList.size() ; x++) {
      tmpSensorInfo = sensorList.get(x);
      if (strcmp(tmpSensorInfo->address, "0x48") == 0) {
        known = true;                
      }
    }    
    if (!known) {
      SensorInfo *newSensor = new ADSSensorInfo("0x48");
      sensorList.add(newSensor);
      need_save = true;
    }    
  }
}


void setupI2C(bool &need_save) {
  //Wire.setClock(0000L);

  Wire.begin();
  Wire.setClock(400000); // choose 400 kHz I2C rate
  
  scanI2C(need_save);

  if (sensorSHT30Present) {
    setupSHT30();
  }

  if (sensorMPU925XPresent) {
    setupMPU9250();
  } 

  if (sensorADS1115Present) {
    setupADS1115();
  }
}


void handleI2C() {
  if (sensorMPU925XPresent) {
    handleMPU9250();
  }
}


void handleI2C_slow() {
  if (sensorSHT30Present) {
    handleSHT30();  
  }
  if (sensorADS1115Present) {
    handleADS1115();
  }
}
