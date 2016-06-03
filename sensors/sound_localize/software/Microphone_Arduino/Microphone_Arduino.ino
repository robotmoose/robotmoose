// Arduino Firmware for interfacing with microphone array.
// NOTE! This code is written specifically for the Arduino Due.

// Ryker Dial
// UAF ITEST

size_t numSensors = 5; // Five microphone array in pentagon shape.
uint16_t micData[5][4];
uint32_t timeRead[5];
char buff;

// Connect 
void setup() {
    //Serial.begin(921600, SERIAL_8O2); // odd parity, 2 stop bits
    //REG_ADC_MR = (REG_ADC_MR & 0xFFF0FFFF) | 0x00020000;
    ADC->ADC_MR |= 0x80;  //set free running mode on ADC
    //SerialUSB.begin(921600, SERIAL_8O2);
    SerialUSB.begin(9600);
}

void loop() {
            // Send CRLF to synchronize with server
            SerialUSB.write(10); // CR
            SerialUSB.write(13); // LF
            for(int i=0; i<numSensors; ++i) {
               // micData[i][0] = analogRead(i);
                // Get 4 samples. Gaussian noise decreases by 1/N^2, where N is the
                //     number of samples
                for(int j=0; j<4; ++j) {
                    micData[i][j] = analogRead(i);
                }
                // Average samples
                for(int j=1; j<4; ++j) {
                    micData[i][0] += micData[i][j];
                }
                micData[i][0] = micData[i][0]/2;
               // timeRead[i] = micros(); // Note: micros has 4 microsecond resolution.
            }
            timeRead[0] = micros();
            // Send over microphone data over serial with time data
                for(int j=3; j>=0; --j) { // sizeof(uint32_t) - 1 = 3
                    // write is lower latency than print, just requires more backend parsing.
                    SerialUSB.write( (uint8_t) (timeRead[0] >> 8*j) );

                }
            for(int i=0; i<numSensors; ++i) {
                /*for(int j=3; j>=0; --j) { // sizeof(uint32_t) - 1 = 3
                    // write is lower latency than print, just requires more backend parsing.
                    SerialUSB.write( (uint8_t) (timeRead[i] >> 8*j) );
                }*/
                for(int j=1; j>=0; --j) { // sizeof(uint16_t) - 1 = 1
                    SerialUSB.write( (uint8_t) (micData[i][0] >> 8*j) );
                }
            }
}
