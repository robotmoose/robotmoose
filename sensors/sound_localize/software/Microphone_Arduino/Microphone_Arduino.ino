uint16_t micData[5];
uint32_t timeRead[5];
char buff;

void setup() {
    Serial.begin(921600, SERIAL_8O2); // odd parity, 2 stop bits
}

void loop() {
    while(true) {
            for(int i=0; i<5; ++i) {
                micData[i] = analogRead(i);
                timeRead[i] = micros();
            }
            // Send over serial with time data
            for(int i=0; i<5; ++i) {
                for(int j=3; j>=0; --j) {
                    Serial.write( (uint8_t) (timeRead[i] >> 8*j) );
                }
                for(int j=1; j>=0; --j) {
                    Serial.write( (uint8_t) (micData[i] >> 8*j) );
                }
            }
            Serial.write(10); // CR
            Serial.write(13); // LF
    }
}
