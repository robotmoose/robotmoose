size_t numSensors = 5; // Five microphone array in pentagon shape.
uint16_t micData[5][4];
uint32_t timeRead[5];
char buff;

// Connect 
void setup() {
    Serial.begin(921600, SERIAL_8O2); // odd parity, 2 stop bits
}

void loop() {
   // while(true) { // read analog values as fast as possible
            // Send CRLF to synchronize with server
            Serial.write(10); // CR
            Serial.write(13); // LF
            for(int i=0; i<numSensors; ++i) {
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
                timeRead[i] = micros(); // Note: micros has 4 microsecond resolution.
            }
            // Send over microphone data over serial with time data
            for(int i=0; i<numSensors; ++i) {
                for(int j=3; j>=0; --j) { // sizeof(uint32_t) - 1 = 3
                    // write is lower latency than print, just requires more backend parsing.
                    Serial.write( (uint8_t) (timeRead[i] >> 8*j) );
                    //Serial.print(timeRead[i]);
                    //Serial.print(",");
                }
                for(int j=1; j>=0; --j) { // sizeof(uint16_t) - 1 = 1
                    Serial.write( (uint8_t) (micData[i][0] >> 8*j) );
                    //Serial.print(micData[i][0]);
                    //Serial.print(",");
                }
            }
            //Serial.println("");
    //}
}
