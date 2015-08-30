/*
  FrSky GPS sensor class for Teensy 3.x and 328P based boards (e.g. Pro Mini, Nano, Uno)
  (c) Pawelsky 20150725
  Not for commercial use
*/

#include "FrSkySportSensorGps.h" 

FrSkySportSensorGps::FrSkySportSensorGps(SensorId id) : FrSkySportSensor(id) { }

void FrSkySportSensorGps::setData(float lat, float lon, float alt, float speed, float cog, uint16_t hdop)
{
  FrSkySportSensorGps::lat = setLatLon(lat, true);
  FrSkySportSensorGps::lon = setLatLon(lon, false);
  FrSkySportSensorGps::cog = cog * 100;
  FrSkySportSensorGps::speed = speed;
  FrSkySportSensorGps::alt = alt;
  FrSkySportSensorGps::hdop = hdop;
}

uint32_t FrSkySportSensorGps::setLatLon(float latLon, bool isLat)
{
  uint32_t data = (uint32_t)((latLon < 0 ? -latLon : latLon) * 60 * 10000) & 0x3FFFFFFF;
  if(isLat == false) data |= 0x80000000;
  if(latLon < 0) data |= 0x40000000;

  return data;
}

void FrSkySportSensorGps::send(FrSkySportSingleWireSerial& serial, uint8_t id, uint32_t now)
{
  if(sensorId == id)
  {
    switch(sensorDataIdx)
    {
      case 0:
        if(now > latTime)
        {
          latTime = now + GPS_LAT_LON_DATA_PERIOD;
          serial.sendData(GPS_LAT_LON_DATA_ID, lat);
        }
        else
        {
          serial.sendEmpty(GPS_LAT_LON_DATA_ID);
        }
        break;
      case 1:
        if(now > lonTime)
        {
          lonTime = now + GPS_LAT_LON_DATA_PERIOD;
          serial.sendData(GPS_LAT_LON_DATA_ID, lon);
        }
        else
        {
          serial.sendEmpty(GPS_LAT_LON_DATA_ID);
        }
        break;
      case 2:
        if(now > altTime)
        {
          altTime = now + GPS_ALT_DATA_PERIOD;
          serial.sendData(GPS_ALT_DATA_ID, alt);
        }
        else
        {
          serial.sendEmpty(GPS_ALT_DATA_ID);
        }
        break;
      case 3:
        if(now > speedTime)
        {
          speedTime = now + GPS_SPEED_DATA_PERIOD;
          serial.sendData(GPS_SPEED_DATA_ID, speed);
        }
        else
        {
          serial.sendEmpty(GPS_SPEED_DATA_ID);
        }
        break;
      case 4:
        if(now > cogTime)
        {
          cogTime = now + GPS_COG_DATA_PERIOD;
          serial.sendData(GPS_COG_DATA_ID, cog);
        }
        else
        {
          serial.sendEmpty(GPS_COG_DATA_ID);
        }
        break;
      case 5:
	if(now > hdopTime)
        {
          hdopTime = now + GPS_HDOP_DATA_PERIOD;
          serial.sendData(GPS_HDOP_DATA_ID, hdop);
        }
        else
        {
          serial.sendEmpty(GPS_HDOP_DATA_ID);
        }
        break;
    }
    sensorDataIdx++;
    if(sensorDataIdx >= GPS_DATA_COUNT) sensorDataIdx = 0;
  }
}
