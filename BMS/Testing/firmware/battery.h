#ifndef BATTERY_H
#define BATTERY_H

#include <SPI.h>
#include <Wire.h>

struct battery_t
{
	float voltage;
	float cell[3];
	uint8_t percentage;
};

#define BATTERY_T_SIZE (4*4+1)

inline void send_battery(const battery_t& bat)
{
  Wire.write((uint8_t*)&bat,BATTERY_T_SIZE);
}

inline battery_t receive_battery(const uint8_t* data)
{
  battery_t bat;

  for(int ii=0;ii<BATTERY_T_SIZE;++ii)
    ((uint8_t*)&bat)[ii]=data[ii];

  return bat;
}

inline battery_t make_battery(float cells[3])
{
  battery_t bat;
  bat.voltage=cells[0]+cells[1]+cells[2];
  bat.percentage=(uint8_t)(((bat.voltage-9.6)/(12.3-9.6))*100.0);  //0% is set at 9.6V; 100% is set at 12.3V!
  bat.cell[0]=cells[0];
  bat.cell[1]=cells[1];
  bat.cell[2]=cells[2];
  return bat;
}


#endif
