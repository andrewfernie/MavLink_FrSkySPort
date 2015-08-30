/*
  FrSky GPS sensor class for Teensy 3.x and 328P based boards (e.g. Pro Mini, Nano, Uno)
  (c) Pawelsky 20150725
  Not for commercial use
*/

#ifndef _FRSKY_SPORT_SENSOR_GPS_H_
#define _FRSKY_SPORT_SENSOR_GPS_H_

#include "FrSkySportSensor.h"

#define GPS_DEFAULT_ID ID4
#define GPS_DATA_COUNT 6
#define GPS_LAT_LON_DATA_ID   0x0800
#define GPS_ALT_DATA_ID       0x0820
#define GPS_SPEED_DATA_ID     0x0830
#define GPS_COG_DATA_ID       0x0840
#define GPS_HDOP_DATA_ID      0xF103	// ADC2_ID

#define GPS_LAT_LON_DATA_PERIOD   1000
#define GPS_ALT_DATA_PERIOD       500
#define GPS_SPEED_DATA_PERIOD     500
#define GPS_COG_DATA_PERIOD       500
#define GPS_HDOP_DATA_PERIOD      500


class FrSkySportSensorGps : public FrSkySportSensor
{
  public:
    FrSkySportSensorGps(SensorId id = GPS_DEFAULT_ID);
    void setData(float lat, float lon, float alt, float speed, float cog, uint16_t hdop);
    virtual void send(FrSkySportSingleWireSerial& serial, uint8_t id, uint32_t now);

  private:
    static uint32_t setLatLon(float latLon, bool isLat);
    uint32_t lat;
    uint32_t lon;
    int32_t alt;
    uint32_t speed;
    uint32_t cog;
    uint16_t hdop;
    uint32_t latTime;
    uint32_t lonTime;
    uint32_t altTime;
    uint32_t speedTime;
    uint32_t cogTime;
    uint32_t hdopTime;
};

#endif // _FRSKY_SPORT_SENSOR_GPS_H_
