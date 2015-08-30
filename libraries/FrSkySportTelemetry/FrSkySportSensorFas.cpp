/*
  FrSky FAS-40A/FAS-150A current sensor class for Teensy 3.x and 328P based boards (e.g. Pro Mini, Nano, Uno)
  (c) Pawelsky 20150725
  Not for commercial use
*/

#include "FrSkySportSensorFas.h" 

FrSkySportSensorFas::FrSkySportSensorFas(SensorId id) : FrSkySportSensor(id) { }

void FrSkySportSensorFas::setData(float current, float voltage)
{
  FrSkySportSensorFas::current = (uint32_t)current;
  FrSkySportSensorFas::voltage = (uint32_t)voltage;
}

void FrSkySportSensorFas::send(FrSkySportSingleWireSerial& serial, uint8_t id, uint32_t now)
{
  if(sensorId == id)
  {
    switch(sensorDataIdx)
    {
      case 0:
        if(now > currentTime)
        {
          currentTime = now + FAS_CURR_DATA_PERIOD;
          serial.sendData(FAS_CURR_DATA_ID, current);
        }
        else
        {
          serial.sendEmpty(FAS_CURR_DATA_ID);
        }
        break;
      case 1:
        if(now > voltageTime)
        {
          voltageTime = now + FAS_VOLT_DATA_PERIOD;
          serial.sendData(FAS_VOLT_DATA_ID, voltage);
        }
        else
        {
          serial.sendEmpty(FAS_VOLT_DATA_ID);
        }
        break;
    }
    sensorDataIdx++;
    if(sensorDataIdx >= FAS_DATA_COUNT) sensorDataIdx = 0;
  }
}
