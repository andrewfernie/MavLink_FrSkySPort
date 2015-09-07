#include "kinetis.h"
#include "core_pins.h"
#include "HardwareSerial.h"
#include "FrSkySPort.h"

//#define ENABLE_FLVSS

#define _FrSkySPort_Serial            Serial1
#define _FrSkySPort_C1                UART0_C1
#define _FrSkySPort_C2                UART0_C2
#define _FrSkySPort_C3                UART0_C3
#define _FrSkySPort_C4                UART0_C4
#define _FrSkySPort_S2                UART0_S2
#define _FrSkySPort_BAUD              57600

#define UART_C3_TXINV                 0x10
#define UART_C3_TXDIR                 0x20
#define UART_S2_RXINV                 0x10

#define IRQ_PRIORITY                  64  // 0 = highest priority, 255 = lowest

#define DIM(array) (sizeof(array) / sizeof(array[0]))

short crc;                         // used for crc calc of frsky-packet
boolean waitingForSensorId = false;
uint8_t cell_count = 0;
uint8_t latlong_flag = 0;
uint32_t latlong = 0;

uint8_t nextFLVSS = 0;
uint8_t nextFAS = 0;
uint8_t nextGPS = 0;

uint8_t sportdata[32];
uint8_t sportlen = 0;
uint8_t sportindex = 0;

// Scale factor for roll/pitch:
// We need to scale down 360 deg to fit when max value is 256, and 256 equals 362 deg
float scalefactor = 360.0/((362.0/360.0)*256.0);

void _FrSkySPort_SerialBegin(uint32_t divisor)
{
  SIM_SCGC4 |= SIM_SCGC4_UART0;
  CORE_PIN0_CONFIG = PORT_PCR_PE | PORT_PCR_PS | PORT_PCR_PFE | PORT_PCR_MUX(3);
  CORE_PIN1_CONFIG = PORT_PCR_DSE | PORT_PCR_SRE | PORT_PCR_MUX(3);
  UART0_BDH = (divisor >> 13) & 0x1F;
  UART0_BDL = (divisor >> 5) & 0xFF;
  _FrSkySPort_C4 = divisor & 0x1F;
  _FrSkySPort_C1 = UART_C1_ILT;
  _FrSkySPort_C2 = UART_C2_TE | UART_C2_RE | UART_C2_RIE | UART_C2_ILIE;
  _FrSkySPort_C3 = UART_C3_TXINV;                // Tx invert
  _FrSkySPort_C1 = UART_C1_LOOPS | UART_C1_RSRC; // Single wire mode
  _FrSkySPort_S2 = UART_S2_RXINV;                // Rx Invert
  attachInterruptVector(IRQ_UART0_STATUS, sport_uart0_status_isr);
  NVIC_ENABLE_IRQ(IRQ_UART0_STATUS);
}

// ***********************************************************************
void FrSkySPort_Init(void)
{
  _FrSkySPort_SerialBegin(BAUD2DIV(_FrSkySPort_BAUD));
  
}

uint8_t FrSkyDataIdTable[] = {
  SENSOR_ID_VARIO,
  SENSOR_ID_ALTITUDE,
  SENSOR_ID_FLVSS,
  SENSOR_ID_FAS,
  SENSOR_ID_GPS,
  SENSOR_ID_RPM,
  SENSOR_ID_HDOP,
  SENSOR_ID_ACCX,
  SENSOR_ID_ACCY,
  SENSOR_ID_ACCZ,
  SENSOR_ID_GPS_STATUS,
  SENSOR_ID_ROLL_ANGLE,
  SENSOR_ID_PITCH_ANGLE,
  SENSOR_ID_FLIGHT_MODE,
  SENSOR_ID_ARM_MODE,
};

uint8_t FrSkyDataIdIndex = 0;
void FrSkySPort_Process()
{
  FrSkySPort_ProcessSensorRequest(FrSkyDataIdIndex);
  FrSkyDataIdIndex += 1;
  if (FrSkyDataIdIndex >= DIM(FrSkyDataIdTable))
    FrSkyDataIdIndex = 0;
}

// ***********************************************************************
uint16_t sendValueFlvssVoltage = 0;
uint16_t sendValueFASCurrent = 0;
uint16_t sendValueFASVoltage = 0;
void FrSkySPort_ProcessSensorRequest(uint8_t sensorId) 
{
  switch(sensorId)
  {
#ifdef ENABLE_FLVSS
  case SENSOR_ID_FLVSS:
  {
      uint32_t temp=0;
      uint8_t offset;

      printDebugPackageSend("FLVSS", nextFLVSS+1, 3);
      // We need cells to continue
      if(ap_cell_count < 1)
        break;
      // Make sure all the cells gets updated from the same voltage average
      if(nextFLVSS == 0)
      {
        sendValueFlvssVoltage = readAndResetMinimumVoltage();  
      }
      // Only respond to request if we have a value
      if(sendValueFlvssVoltage < 1)
        break; 

      switch(nextFLVSS)
      {
      case 0:
        if(ap_cell_count > 0) 
        {
          // First 2 cells
          offset = 0x00 | ((ap_cell_count & 0xF)<<4);
          temp=((sendValueFlvssVoltage/(ap_cell_count * 2)) & 0xFFF);
          FrSkySPort_SendPackage(FR_ID_CELLS,(temp << 20) | (temp << 8) | offset);  // Battery cell 0,1
        }
        break;
      case 1:    
        // Optional 3 and 4 Cells
        if(ap_cell_count > 2) {
          offset = 0x02 | ((ap_cell_count & 0xF)<<4);
          temp=((sendValueFlvssVoltage/(ap_cell_count * 2)) & 0xFFF);
          FrSkySPort_SendPackage(FR_ID_CELLS,(temp << 20) | (temp << 8) | offset);  // Battery cell 2,3
        }
        break;
      case 2:    // Optional 5 and 6 Cells
        if(ap_cell_count > 4) {
          offset = 0x04 | ((ap_cell_count & 0xF)<<4);
          temp=((sendValueFlvssVoltage/(ap_cell_count * 2)) & 0xFFF);
          FrSkySPort_SendPackage(FR_ID_CELLS,(temp << 20) | (temp << 8) | offset);  // Battery cell 2,3
        }
        break;     
      }
      nextFLVSS++;
      if(nextFLVSS>2)
        nextFLVSS=0;
    }
    break;
#endif
  case SENSOR_ID_VARIO:
    FrSkySPort_SendPackage(FR_ID_VARIO, ap_climb_rate );       // 100 = 1m/s        
    break;
    
  case SENSOR_ID_ALTITUDE: 
    FrSkySPort_SendPackage(FR_ID_ALTITUDE,ap_bar_altitude);   // from barometer, 100 = 1m
    break;

  case SENSOR_ID_FAS:
    {
      printDebugPackageSend("FAS", nextFAS+1, 2);
      // Use average of atleast 2 samples
      if(currentCount < 2)
        return;
      if(nextFAS == 0)
      {
        sendValueFASVoltage = readAndResetAverageVoltage();
        sendValueFASCurrent = readAndResetAverageCurrent();  
      }
      if(sendValueFASVoltage < 1)
        break;
      
      switch(nextFAS)
      {
      case 0:
        FrSkySPort_SendPackage(FR_ID_VFAS,sendValueFASVoltage/10); // Sends voltage as a VFAS value
        break;
      case 1:
        FrSkySPort_SendPackage(FR_ID_CURRENT, sendValueFASCurrent / 10);
        break;
      }
      if(++nextFAS > 1)
        nextFAS = 0;
    }
    break;

  case SENSOR_ID_GPS:
    {
      printDebugPackageSend("GPS", nextGPS+1, 5);
      switch(nextGPS)
      {
      case 0:        // Sends the ap_longitude value, setting bit 31 high
        if(ap_fixtype==3) {
          if(ap_longitude < 0)
            latlong=((abs(ap_longitude)/100)*6)  | 0xC0000000;
          else
            latlong=((abs(ap_longitude)/100)*6)  | 0x80000000;
          FrSkySPort_SendPackage(FR_ID_LATLONG,latlong);
        }
        break;
      case 1:        // Sends the ap_latitude value, setting bit 31 low  
        if(ap_fixtype==3) {
          if(ap_latitude < 0 )
            latlong=((abs(ap_latitude)/100)*6) | 0x40000000;
          else
            latlong=((abs(ap_latitude)/100)*6);
          FrSkySPort_SendPackage(FR_ID_LATLONG,latlong);
        }
        break;  
      case 2:
        if(ap_fixtype==3) {
          FrSkySPort_SendPackage(FR_ID_GPS_ALT,ap_gps_altitude / 10);   // from GPS,  100=1m
        }
        break;
      case 3:
      // Note: This is sending GPS Speed now
        if(ap_fixtype==3) {
          //            FrSkySPort_SendPackage(FR_ID_SPEED,ap_groundspeed *20 );  // from GPS converted to km/h
          FrSkySPort_SendPackage(FR_ID_SPEED,ap_gps_speed *20 );  // from GPS converted to km/h
        }
        break;
      case 4:
         // Note: This is sending Course Over Ground from GPS as Heading
         // before we were sending this: FrSkySPort_SendPackage(FR_ID_HEADING,ap_cog * 100); 

        FrSkySPort_SendPackage(FR_ID_GPS_COURSE, ap_heading * 100);   // 10000 = 100 deg
        break;
      }
      if(++nextGPS > 4)
        nextGPS = 0;
    }
    break;    

  case SENSOR_ID_RPM:
    FrSkySPort_SendPackage(FR_ID_RPM, ap_battery_remaining);   
    break;

  case SENSOR_ID_HDOP:
    FrSkySPort_SendPackage(FR_ID_ADC2, ap_gps_hdop);
    break;
    
  case SENSOR_ID_ACCX:
    FrSkySPort_SendPackage(FR_ID_ACCX, fetchAccX());    
    break;
    
  case SENSOR_ID_ACCY:
    FrSkySPort_SendPackage(FR_ID_ACCY, fetchAccY());     
    break;

  case SENSOR_ID_ACCZ:
    FrSkySPort_SendPackage(FR_ID_ACCZ, fetchAccZ());    
    break;
    
  case SENSOR_ID_GPS_STATUS:
    FrSkySPort_SendPackage(FR_ID_T1, gps_status);    
    break;
    
  case SENSOR_ID_ROLL_ANGLE:
    FrSkySPort_SendPackage(FR_ID_A3_FIRST, handle_A2_A3_value((ap_roll_angle+180)/scalefactor));
    break;
    
  case SENSOR_ID_PITCH_ANGLE:
    FrSkySPort_SendPackage(FR_ID_A4_FIRST, handle_A2_A3_value((ap_pitch_angle+180)/scalefactor));
    break;
    
  case SENSOR_ID_FLIGHT_MODE:
    if (ap_custom_mode >= 0) {
      FrSkySPort_SendPackage(FR_ID_FUEL, ap_custom_mode); 
    }
    break;   
    
  case SENSOR_ID_ARM_MODE:
      {
        // 16 bit value: 
        // bit 1: armed
        // bit 2-5: severity +1 (0 means no message)
        // bit 6-15: number representing a specific text
        uint32_t ap_status_value = ap_base_mode&0x01;
        // If we have a message-text to report (we send it multiple times to make sure it arrives even on telemetry glitches)
        if(ap_status_send_count > 0 && ap_status_text_id > 0)
        {
          // Add bits 2-15
          ap_status_value |= (((ap_status_severity+1)&0x0F)<<1) |((ap_status_text_id&0x3FF)<<5);
          ap_status_send_count--;
          if(ap_status_send_count == 0)
          {
             // Reset severity and text-message after we have sent the message
             ap_status_severity = 0; 
             ap_status_text_id = 0;
          }          
        }
        FrSkySPort_SendPackage(FR_ID_T2, ap_status_value); 
      }
      break;
    
  }
}

uint32_t handle_A2_A3_value(uint32_t value)
{
  return (value *330-165)/0xFF;
}

// ***********************************************************************
void printDebugPackageSend(const char * pkg_name, uint8_t pkg_nr, uint8_t pkg_max)
{
#ifdef DEBUG_FRSKY_SENSOR_REQUEST
  debugSerial.print(millis());
  debugSerial.print("\tCreating frsky package for ");
  debugSerial.print(pkg_name);
  debugSerial.print(" (");
  debugSerial.print(pkg_nr);
  debugSerial.print("/");
  debugSerial.print(pkg_max);
  debugSerial.print(")");
  debugSerial.println();
#endif
}

// ***********************************************************************
void FrSkySPort_SendByte(uint8_t byte) {

  if (byte == 0x7E) {
    sportdata[sportlen++] = 0x7D;
    sportdata[sportlen++] = 0x5E;
  }
  else if (byte == 0x7D) {
    sportdata[sportlen++] = 0x7D;
    sportdata[sportlen++] = 0x5D;
  }
  else {
    sportdata[sportlen++] = byte;
  }
  FrSkySPort_UpdateCRC(byte);
}

void FrSkySPort_UpdateCRC(uint8_t byte)
{
  // CRC update
  crc += byte;         //0-1FF
  crc += crc >> 8;   //0-100
  crc &= 0x00ff;
}

// ***********************************************************************
void FrSkySPort_SendCrc()
{
  sportdata[sportlen++] = 0xFF-crc;
  crc = 0;          // CRC reset
}

// ***********************************************************************
void FrSkySPort_SendPackage(uint16_t id, uint32_t value)
{
  if(MavLink_Connected) {
    digitalWrite(led,HIGH);
  }
  
  sportlen = 0;
  sportindex = 0;

  FrSkySPort_SendByte(DATA_FRAME);
  uint8_t *bytes = (uint8_t*)&id;
  FrSkySPort_SendByte(bytes[0]);
  FrSkySPort_SendByte(bytes[1]);
  bytes = (uint8_t*)&value;
  FrSkySPort_SendByte(bytes[0]);
  FrSkySPort_SendByte(bytes[1]);
  FrSkySPort_SendByte(bytes[2]);
  FrSkySPort_SendByte(bytes[3]);
  FrSkySPort_SendCrc();
  
  digitalWrite(led,LOW);
}

extern "C" void sport_uart0_status_isr(void)
{
  if (UART0_S1 & UART_S1_RDRF) {
    uint8_t data = UART0_D;
    if( data == START_STOP) {
      waitingForSensorId = true;
    }
    else {
      if ((waitingForSensorId == true) && (data == 0x22)) {  
        FrSkySPort_Process();
        _FrSkySPort_C3 |= UART_C3_TXDIR;
        _FrSkySPort_C2 &= ~UART_C2_RE;
      }
      waitingForSensorId = false;
    }
  }
  
  if (UART0_S1 & UART_S1_TDRE) {
    if (sportindex < sportlen) {
      UART0_D = sportdata[sportindex++];
    }
    else {
      _FrSkySPort_C2 |= UART_C2_TCIE;
    }
  }
  
  if (UART0_S1 & UART_S1_TC) {
    _FrSkySPort_C3 &= ~UART_C3_TXDIR;
    _FrSkySPort_C2 |= UART_C2_RE;
  }
}
