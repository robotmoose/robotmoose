#include <SoftwareSerial.h>
#include "tabula_config.h"

void setup()
{
  tabula_setup();
}

void loop()
{
  action_loop();
}

class heartbeat : public action {
public:
   void loop() { Serial.println("Still alive!"); }
};

REGISTER_TABULA_DEVICE(heartbeat, 
  actions_1s.add(new heartbeat());
)

