# Makefile for Arduino firmware build and upload

#ubuntu
#sudo apt-get install arduino

#mac
#brew tap larsimmisch/avr
#brew install avrdude avr-binutils avr-gcc avr-libc

#STUFF YOU GOTTA CHANGE...
ifeq ($(shell uname),Darwin)
	#MAC
	PROGRAM_PORT=$(shell echo /dev/tty.usb*)
else
	#LINUX
	PROGRAM_PORT=/dev/ttyACM0
endif

#Builders
COMPILER=avr-g++
LINKER=avr-objcopy
PROGRAMMER=avrdude

#Search Directories
ARDUINO_IDE=/usr/share/arduino
ARDUINO_DIR=$(ARDUINO_IDE)/hardware/arduino
ARD_LIB_DIR=$(ARDUINO_IDE)/libraries
INCLUDE_DIR=-I$(ARDUINO_DIR)/cores -I$(ARDUINO_DIR)/cores/arduino
ATMEGA328_INCLUDE_DIR=-I$(ARDUINO_DIR)/variants/standard
ATMEGA2560_INCLUDE_DIR=-I$(ARDUINO_DIR)/variants/mega
LIB_DIR=-I$(ARD_LIB_DIR)/Servo -I$(ARD_LIB_DIR)/SoftwareSerial -I$(ARD_LIB_DIR)/Wire -Ipatched
LIB_SRC=$(ARD_LIB_DIR)/Servo/Servo.cpp patched/SoftwareSerial.cpp $(ARD_LIB_DIR)/Wire/Wire.cpp patched/twi.cpp
DIRS=$(INCLUDE_DIR) -I. -I$(SHARED_DIR) -I$(SRC_DIR)  $(LIB_DIR)

#Compiler Flags
OPTS=-Os  # optimize for size, to fit on an Uno
CFLAGS=-Wno-sign-compare -Wno-unused-variable -Wall -lm $(OPTS) -Wl,--gc-sections -ffunction-sections \
	-fdata-sections -DUSB_VID=null -DUSB_PID=null -DARDUINO=103 -Wno-strict-aliasing
# -std=c++11 

#Device Flags
CLOCK=16000000UL
ATMEGA328_BAUD=115200
ATMEGA2560_BAUD=115200
ATMEGA328_PROTOCOL=arduino
ATMEGA2560_PROTOCOL=wiring
ATMEGA328_COMPILE_CHIP=atmega328p
ATMEGA2560_COMPILE_CHIP=atmega2560
ATMEGA328_PROGRAM_CHIP=m328p
ATMEGA2560_PROGRAM_CHIP=m2560
PFLAGS=-D

COMPILER328=$(COMPILER) $(ATMEGA328_INCLUDE_DIR) -mmcu=$(ATMEGA328_COMPILE_CHIP) -DF_CPU=$(CLOCK)
COMPILER2560=$(COMPILER) $(ATMEGA2560_INCLUDE_DIR) -mmcu=$(ATMEGA2560_COMPILE_CHIP) -DF_CPU=$(CLOCK)
PROG328=$(PROGRAMMER) -p$(ATMEGA328_PROGRAM_CHIP) -b$(ATMEGA328_BAUD)  -c$(ATMEGA328_PROTOCOL)
PROG2560=$(PROGRAMMER) -p$(ATMEGA2560_PROGRAM_CHIP) -b$(ATMEGA2560_BAUD) -c$(ATMEGA2560_PROTOCOL)


#Sources
#Arduino
ARDUINO_SRC=$(ARDUINO_DIR)/cores/arduino/*.cpp $(ARDUINO_DIR)/cores/arduino/*.c
MAIN_SRC=*.cpp $(LIB_SRC)
BIN_DIR=.
BIN_FILE=$(BIN_DIR)/main

#Builds
all: main.cpp main upload

main.cpp: arduino.ino
	cp $< $@

main: main.cpp $(MAIN_SRC) *.h
	$(COMPILER2560) $(DIRS) $(CFLAGS) $(INCLUDE) $(MAIN_SRC) $(ARDUINO_SRC) -o $(BIN_DIR)/$@.elf
	$(LINKER) -R .eeprom -O ihex $(BIN_FILE).elf $(BIN_FILE).hex

upload: main
	$(PROG2560) -P$(PROGRAM_PORT) -Uflash:w:$(BIN_FILE).hex $(PFLAGS)

clean:
	@rm -rf $(BIN_DIR)/main.hex

