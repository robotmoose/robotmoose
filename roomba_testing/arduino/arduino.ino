#include "roomba.h"

roomba_serial_t roomba_serial(Serial3);
roomba_t roomba(roomba_serial);

void setup()
{
  Serial.begin(57600);
  Serial3.begin(115200);
  roomba.reset();
  roomba.start();
  roomba.set_mode(roomba_t::FULL);
  roomba.drive(100,0);
}

void loop()
{
}
