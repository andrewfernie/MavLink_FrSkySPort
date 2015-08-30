/*
  FrSky FAS-40A/FAS-150A current sensor class for Teensy 3.x and 328P based boards (e.g. Pro Mini, Nano, Uno)
  (c) Pawelsky 20150725
  Not for commercial use
*/

#ifndef _FRSKY_SPORT_SENSOR_FAS_H_
#define _FRSKY_SPORT_SENSOR_FAS_H_

#include "FrSkySportSensor.h"

#define FAS_DEFAULT_ID ID3
#define FAS_DATA_COUNT 2
#define FAS_CURR_DATA_ID 0x0200
#define FAS_VOLT_DATA_ID 0x0210

#define FAS_CURR_DATA_PERIOD 500
#define FAS_VOLT_DATA_PERIOD 500

class FrSkySportSensorFas : public FrSkySportSensor
{
  public:
    FrSkySportSensorFas(SensorId id = FAS_DEFAULT_ID);
    void setData(float current, float voltage);
    virtual void send(FrSkySportSingleWireSerial& serial, uint8_t id, uint32_t now);

  private:
    uint32_t current;
    uint32_t voltage;
    uint32_t currentTime;
    uint32_t voltageTime;
};

#endif // _FRSKY_SPORT_SENSOR_FAS_H_
