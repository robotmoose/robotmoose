// Written By Clayton Auld
// clayauld@gmail.com
// Nov 20 2014

#include <SPI.h>
#include <Wire.h>
#include <stdlib.h>
#include "battery.h"

// LTC6803 parameters

#define WRCFG        0x01 // Write configuration register group
#define RDCFG        0x02 // Read configuration register group
#define RDCV         0x04 // Command to read all cell voltages
#define RDCVA        0x06 // Command to read cell voltages 1-4
#define RDCVB        0x08 // Read cell voltages 5-8
#define RDCVC        0x0A // Read cell voltage 9-12
#define RDFLG        0x0C // Read flag register group
#define RDTMP        0x0E // Read temperature register group
#define STCVAD       0x10 // Start cell voltage ADC conversions and poll status
#define STCST1       0x1E // ADC conversion self test 1
#define STCST2       0x1F // ADC conversion self test 2
#define STOWAD       0x20 // Start open-wire ADC conversions and poll status
#define STTMPAD      0x30 // Start temperature ADC conversions and poll status
#define PLADC        0x40 // Poll ADC Converter status
#define PLINT        0x50 // Poll interrupt status
#define DAGN         0x52 // Start diagnose and poll status
#define RDDGNR       0x54 // Read Diagnostic register
#define STCVDC       0x60 // Start cell voltage ADC conversions and poll status, with discharge permitted
#define STOWDC       0x70 // Start open-wire ADC conversions and poll status, with discharge permitted
#define CLEAR        0x1D // Clear the cell voltage registers and temperature registers

// Charge function parameters

#define ABS_max           4.200  // Max Cell voltage: DO NOT CHARGE HIGHER
#define max_working       4.100  // Discharge cells higher than this value
#define low_cell_working  3.500  // Warning Voltage
#define ABS_min           3.200  //E-Stop or risk damage

// Configuration Registers for measure mode

#define CFGR0       0xE1
byte CFGR1=0x00;
#define CFGR2       0x00
#define CFGR3       0xFF
#define CFGR4       0x00
#define CFGR5       0x00

// Arduino Pins

#define SS_PIN        10   // Designate Chip select pin ***Change this to pin 53 when uploading to Mega***
#define CHARGE_INPUT  5    // Will be pulled high when AC power is available
#define CHARGE_RELAY  9    // Set to high to turn on charging relay
#define POWER         6    // Power pin for BMS shield
#define ADDRESS       0x80 // Designate Chip address: 10000000

//---------------------------------------------------------------------------------------------------------------------
// Calculation Variables
//byte conf[6];                // Only needed when GetConfig() function is in use.
unsigned int RawData[6];       // Raw data from voltage registers
float cellVoltage[3];          // Calculated voltages for each cell
float AvgCellVolts;
float cellVoltTotal;
byte chargeflag;

// PEC Variables
static byte crc8_table[256];   // 8-bit table for PEC calc
static int made_table = 0;     // table made flag
byte packet[18]={0};           // used for PEC calc
// NOT NEEDED PEC Variables
//byte PECbyte;                // PEC of a byte
//byte PECpacket;              // PEC of packet
//byte PECpacketREAD;          // value that PECpacket should be as read from 6803
//---------------------------------------------------------------------------------------------------------------------

int x=0;

void setup()
{
  pinMode(SS_PIN, OUTPUT);
  pinMode(CHARGE_INPUT, INPUT);
  pinMode(POWER, OUTPUT);
  pinMode(CHARGE_RELAY, OUTPUT);
  digitalWrite(SS_PIN, HIGH); // Chip Deselect

  // SPI Configuration: MSB First, CPOL = 1, CPHA = 1, DIV16 = 1Mhz
  SPI.setBitOrder(MSBFIRST);
  SPI.setDataMode(SPI_MODE3);
  SPI.setClockDivider(SPI_CLOCK_DIV16);

  SPI.begin();          // Start SPI
  Serial.begin(57600);   // Open serial port
//---------------------------------------------------------------------------------------------------------------------
// I2C configs
 Wire.begin();                // join i2c bus with address #2
 //Wire.onRequest(requestEvent); // register event
 // Wire.onReceive(receiveEvent);
  
  //---------------------------------------------------------------------------------------------------------------------

}

void SetConfig()  // Send configuration registers to put LTC6803 into "Measure mode."
{
  //Serial.println("Writing Configuration Registers");
  digitalWrite(SS_PIN, LOW); // Chip Select
  SPI.transfer(WRCFG);
  SPI.transfer(0xC7);
  SPI.transfer(CFGR0);
  SPI.transfer(CFGR1);
  SPI.transfer(CFGR2);
  SPI.transfer(CFGR3);
  SPI.transfer(CFGR4);
  SPI.transfer(CFGR5);
  packet[0]=CFGR0;
  packet[1]=CFGR1;
  packet[2]=CFGR2;
  packet[3]=CFGR3;
  packet[4]=CFGR4;
  packet[5]=CFGR5;
  SPI.transfer(calcPECpacket(6));
  digitalWrite(SS_PIN, HIGH); // Chip Deselect
}

// This code is only needed to verify that the configuration registers are getting written to correctly
void GetConfig()
{
  digitalWrite(SS_PIN, LOW);
  SPI.transfer(RDCFG);
  SPI.transfer(0xCE);
  int conf[6];
  for(int i=0; i<6; i++)
  {
    conf[i] = SPI.transfer(0x00);
    Serial.println(conf[i], HEX);
  }
  digitalWrite(SS_PIN, HIGH);
}


unsigned int getCellVolts()
{
  digitalWrite(SS_PIN, LOW); // Chip Select
  SPI.transfer(RDCV); // Send command to read cells 1-4
  SPI.transfer(0xDC); // Send PEC byte for command
  //Serial.println("Reading Cell Voltage Registers");

  for(int i=0; i<6; i++)
  {
    RawData[i] = SPI.transfer(0x00);   // send command to read voltage registers
    //Serial.println(RawData[i], HEX);
  }
  byte PECresponse;
  PECresponse=SPI.transfer(0x00);
  digitalWrite(SS_PIN, HIGH); // Chip Deselect
  return RawData[6];
}

void ADCconvert()  // Send command to run ADC conversion
{
  //Serial.println("ADC conversion started");
  digitalWrite(SS_PIN, LOW); // Chip Select
  SPI.transfer(STCVDC); // Send command to start ADC conversion
  SPI.transfer(0xE7); // Send PEC byte for command
  digitalWrite(SS_PIN, HIGH); // Chip Deselect
}

unsigned int BitShiftCombine(unsigned char x_high, unsigned char x_low)  // Combine 2 registers into 16-bit number
{
  unsigned int combined;
  unsigned int Cell0;
  combined = x_high;                        //send x_high to rightmost 8 bits
  combined = combined << 8;                 //shift x_high over to leftmost 8 bits
  combined = combined | x_low;              //logical OR keeps x_high intact in combined and fills in rightmost 8 bits
  return combined;
}

int CellConvert(unsigned int combined1, unsigned int combined2, unsigned int combined3)  // Convert 16-bit number to correct 12-bit number for voltage calculation
{
  int Cell[3];
  Cell[0] = combined1 & 4095;
  Cell[1] = combined2 >> 4;
  Cell[2] = combined3 & 4095;
  for(int i=0; i<3; i++)
  {
    cellVoltage[i] = 1.5/1000*(Cell[i] - 512);
  }
  return cellVoltage[3];
}

float AvgerageCell()
{
  AvgCellVolts=(cellVoltage[0]+cellVoltage[1]+cellVoltage[2])/3;
  return AvgCellVolts;
}

float totalCell()
{
  cellVoltTotal = cellVoltage[0] + cellVoltage[1] + cellVoltage[2];
  return cellVoltTotal;
}

void setCFGR1( float cellVoltage[], byte & CFGR1 )
{
  CFGR1 = 0; //Clears out GFGR1
  if (cellVoltage[0] >= max_working)
  {
    CFGR1 += 0x01; //Sets DCC1 = 1
  }
  if (cellVoltage[1] >= max_working)
  {
    CFGR1 += 0x02; //Sets DCC2 = 1
  }
  if (cellVoltage[2] >= max_working)
  {
    CFGR1 += 0x04; //Sets DCC3 = 1
  }
}

void Charge()    // Function to turn on charging and cell balancing
{
//  // Artificially set to test if statements:
//  cellVoltage[0] = 4.15;
//  cellVoltage[1] = 3.9;
//  cellVoltage[2]=4.15;
//  digitalWrite(CHARGE_INPUT, HIGH);


  //Sets CFGR1 to manage cell discharging
  setCFGR1(cellVoltage, CFGR1);
  SetConfig();
  if (digitalRead(CHARGE_INPUT) == HIGH)
  {
    //If a single cell's voltage is greater than the absolute max, turns off charging
    if ((cellVoltage[0] >= ABS_max) || (cellVoltage[1] >= ABS_max) || (cellVoltage[2] >= ABS_max))
    {
      digitalWrite(CHARGE_RELAY, LOW);
      chargeflag=0;
    }
    //If not charging is OK
    else
    {
      chargeflag=1;
      digitalWrite(CHARGE_RELAY, HIGH);
    }
  }
  else if (digitalRead(CHARGE_INPUT) == LOW)
  {
    digitalWrite(CHARGE_RELAY, LOW);
    chargeflag=0;
  }
}

/*
int findHighCell(float cellVoltage[])
{
int cell[4];
int highCell = cell[0];
for(int i = 1; i <= totalCells; i++)
{
if(cellVoltage[i] > cellVoltage[i-1])
highCell = cell[i];
}
return highCell;
}
*/

//---------------------------------------------------------------------------------------------------------------------
// Following PEC code by Dale (KiloOne) on Endless Sphere
// http://endless-sphere.com/forums/viewtopic.php?f=14&t=40962

static void init_crc8() // Generate PEC lookup table
{
  int z,j;
  byte cr;
  if (!made_table) {
    for (z = 0; z < 256; z ++) {
      cr = z;
      for (j = 0; j < 8; j ++) {
        cr = (cr << 1) ^ ((cr & 0x80) ? 0x07 : 0);
      }
      crc8_table[z] = cr & 0xFF;
    }
    made_table = 1;
  }
}

byte calcPECbyte(byte m) // Calculate PEC from single byte
{
  byte PECbyte = 0x41;  // initialize PECbyte
  if (!made_table) {  // Check and make sure lookup table is generated
    init_crc8();  // Generate Table
  }
  PECbyte = crc8_table[(PECbyte) ^ m];
  return PECbyte;
}

byte calcPECpacket(byte np) // Calculate PEC for an array of bytes. np is number of bytes currently in packet[]
{
  int z;
  byte PECpacket = 0x41;  // initialize PECpacket
  if (!made_table) {  // Check and make sure lookup table is generated
    init_crc8();  // Generate Table
  }
  for (z = 0; z < np; z ++) {
    PECpacket = crc8_table[(PECpacket) ^ packet[z]];
  }
  return PECpacket;
}

//---------------------------------------------------------------------------------------------------------------------
// I2C communication code

void requestEvent()
{
  byte data [2] = {(byte)make_battery(cellVoltage).percentage, setChargeByte()};
  Wire.write(data, 2);
  //Serial.print("I2C Request Performed: ");
  //Serial.println(make_battery(cellVoltage).percentage);
}

void receiveEvent(int numbergot)
{
  int input = Wire.read();
  Serial.print("got i2c data from master!!!!! it was =");
  Serial.println(input);
  if(input==2)
  {   
    byte data [2] = {(byte)make_battery(cellVoltage).percentage, setChargeByte()};
    Wire.write(data, 2);
    //Serial.print("I2C Request Performed: ");
    //Serial.println(make_battery(cellVoltage).percentage);
  }
  else
  {
    byte data [2] = {0xFF, 0xFF};
    Wire.write(data, 2);
    //Serial.print("I2C Request Performed: ");
    //Serial.println(make_battery(cellVoltage).percentage);
  }
}


byte setChargeByte()
{
  return ((chargeflag << 3) | CFGR1);
}
  

//---------------------------------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------------------------
void loop()
{
  digitalWrite(POWER, HIGH);
  //GetConfig();                    // Only needed for debugging purposes.
  SetConfig();
  //GetConfig();                    // Only needed for debugging purposes.
  ADCconvert();
  getCellVolts();
  cellVoltage[3]=CellConvert(BitShiftCombine(RawData[1], RawData[0]), BitShiftCombine(RawData[2], RawData[1]), BitShiftCombine(RawData[4], RawData[3]));
  AvgerageCell();
  totalCell();
  Charge();

  for(int i=0; i<3; i++)
  {
    Serial.print("Cell ");
    Serial.print(i);
    Serial.print(": ");
    Serial.print(cellVoltage[i], 4);
    Serial.println(" V");
  }
  
  /*Serial.print("Battery Percentage: ");
  Serial.print(make_battery(cellVoltage).percentage);
  Serial.println("%");*/

  Serial.print("Average Cell Voltage: ");
  Serial.print(AvgCellVolts,4);
  Serial.println(" V");

  Serial.print("Total Cell Voltages: ");
  Serial.print(cellVoltTotal,4);
  Serial.println(" V");

  Serial.print("Charge flag: ");
  Serial.println(chargeflag);
  
  Serial.println("-------------------------------------");
  
  Wire.beginTransmission(2);
  byte data [2] = {(byte)make_battery(cellVoltage).percentage, setChargeByte()};
  Wire.write(data, 2);
  Wire.endTransmission();
  
  
  delay(1200);
}
//---------------------------------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------------------------


