/*
  FrSky FUEL sensor class for Teensy 3.x and 328P based boards (e.g. Pro Mini, Nano, Uno)
  (c) Clooney 20150814
  Not for commercial use
*/

#ifndef _FRSKY_SPORT_SENSOR_FUEL_H_
#define _FRSKY_SPORT_SENSOR_FUEL_H_

#include "FrSkySportSensor.h"

#define FUEL_DEFAULT_ID ID8
#define FUEL_DATA_COUNT 1
#define FUEL_DATA_ID 0x0600

#define FUEL_DATA_PERIOD 500

class FrSkySportSensorFuel : public FrSkySportSensor
{
  public:
    FrSkySportSensorFuel(SensorId id = FUEL_DEFAULT_ID);
    void setData(float fuel);
    virtual void send(FrSkySportSingleWireSerial& serial, uint8_t id, uint32_t now);

  private:
    uint32_t fuel;
    uint32_t fuelTime;
};

#endif // _FRSKY_SPORT_SENSOR_FUEL_H_
