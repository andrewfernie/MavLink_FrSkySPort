/*
  FrSky FUEL sensor class for Teensy 3.x and 328P based boards (e.g. Pro Mini, Nano, Uno)
  (c) Clooney 20150814
  Not for commercial use
*/

#include "FrSkySportSensorFuel.h" 

FrSkySportSensorFuel::FrSkySportSensorFuel(SensorId id) : FrSkySportSensor(id) { }

void FrSkySportSensorFuel::setData(float fuel)
{
  FrSkySportSensorFuel::fuel = (uint32_t)(fuel);
}

void FrSkySportSensorFuel::send(FrSkySportSingleWireSerial& serial, uint8_t id, uint32_t now)
{
  if(sensorId == id)
  {
    if(now > fuelTime)
    {
      fuelTime = now + FUEL_DATA_PERIOD;
      serial.sendData(FUEL_DATA_ID, fuel);
    }
    else
    {
      serial.sendEmpty(FUEL_DATA_ID);
    }
  }
}
