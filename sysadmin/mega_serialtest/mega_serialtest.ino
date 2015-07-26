void setup() {
	Serial.begin(57600);
	Serial1.begin(57600);
	Serial2.begin(57600);
	Serial3.begin(57600);
	Serial.print("Starting serial test...");
}

void recv(int num,HardwareSerial &port) {
	while (port.available()) {
		int c=port.read();
		if (c<'0'||c>'z') continue;
		Serial.print("  rx");
		Serial.print(num);
		Serial.print(" <- tx");
		Serial.print((char)c);
	}
}

void loop() {
	Serial1.write('1');
	Serial2.write('2');
	Serial3.write('3');
	recv(1,Serial1);
	recv(2,Serial2);
	recv(3,Serial3);
        Serial.println(".");
	delay(50);
}

