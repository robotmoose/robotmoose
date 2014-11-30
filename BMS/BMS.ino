// Written By Clayton Auld
// clayauld@gmail.com

// Some Variable names and values borrowed from 
// Ricky Gu's Open BMS Project
// https://github.com/rickygu/openBMS

// Nov 20 2014

#include <SPI.h> 

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
#define STOWAD       0x20 // Start open-wire ADC conversions and poll status
#define STTMPAD      0x30 // Start temperature ADC conversions and poll status
#define PLADC        0x40 // Poll ADC Converter status
#define PLINT        0x50 // Poll interrupt status
#define DAGN         0x52 // Start diagnose and poll status
#define RDDGNR       0x54 // Read Diagnostic register
#define STCVDC       0x60 // Start cell voltage ADC conversions and poll status, with discharge permitted
#define STOWDC       0x70 // Start open-wire ADC conversions and poll status, with discharge permitted

// Configuration Registers

#define CFGR0       0xE1
#define CFGR1       0x00
#define CFGR2       0x00
#define CFGR3       0xFF
#define CFGR4       0x00
#define CFGR5       0x00

#define SS_PIN        10  // Designate Chip select pin 
#define ADDRESS      0x80 // Designate Chip address: 10000000

//---------------------------------------------------------------------------------------------------------------------
// Calculation Variables
//byte conf[6];                // Only needed when GetConfig() function is in use.
unsigned int RawData[6];       // Raw data from voltage registers
float cellVoltage[3];          // Calculated voltages for each cell

// PEC Variables
static byte crc8_table[256];   // 8-bit table for PEC calc
static int made_table = 0;     // table made flag
byte packet[18]={0};           // used for PEC calc
// NOT NEEDED PEC Variables
//byte PECbyte;                // PEC of a byte
//byte PECpacket;              // PEC of packet
//byte PECpacketREAD;          // value that PECpacket should be as read from 6803
//---------------------------------------------------------------------------------------------------------------------

void setup()
{
  pinMode(SS_PIN, OUTPUT);
  digitalWrite(SS_PIN, HIGH); // Chip Deselect

  //SPI Configuration: MSB First, CPOL = 1, CPHA = 1, DIV16 = 1Mhz
  SPI.setBitOrder(MSBFIRST);
  SPI.setDataMode(SPI_MODE3);
  SPI.setClockDivider(SPI_CLOCK_DIV16);

  SPI.begin();          // Start SPI 
  Serial.begin(9600);   // Open serial port

}

void SetConfig()  // Send configuration registers to put LTC6803 into "Measure mode."
{
  Serial.println("Writing Configuration Registers");
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

/* This code is only needed to verify that the configuration registers are getting written to correctly
void GetConfig()
{
  Serial.println("Reading Configuration Registers");
  digitalWrite(SS_PIN, LOW);
  SPI.transfer(RDCFG);
  SPI.transfer(0xCE);
  for(int i=0; i<6; i++)
  {
    conf[i] = SPI.transfer(0x00);
  }
  digitalWrite(SS_PIN, HIGH);
}
*/

unsigned int getCellVolts()
{
  //unsigned int Respon
  digitalWrite(SS_PIN, LOW); // Chip Select
  SPI.transfer(RDCV); // Send command to read cells 1-4
  SPI.transfer(0xDC); // Send PEC byte for command
  Serial.println("Reading Cell Voltage Registers");

  for(int i=0; i<6; i++)
  {
    RawData[i] = SPI.transfer(0x00);   // send command to read voltage registers
    //Serial.println(Response[i], HEX);
  }
  byte PECresponse;
  PECresponse=SPI.transfer(0x00);
  digitalWrite(SS_PIN, HIGH); // Chip Deselect
  return RawData[6];
}

void ADCconvert()  // Send command to run ADC conversion
{
  Serial.println("ADC conversion started");
  digitalWrite(SS_PIN, LOW); // Chip Select
  SPI.transfer(STCVAD); // Send command to start ADC conversion
  SPI.transfer(0xB0); // Send PEC byte for command
  while (byte i=LOW)
  {
    i = SPI.transfer(0x00);
  }
  digitalWrite(SS_PIN, HIGH); // Chip Deselect
}

unsigned int BitShiftCombine(unsigned char x_high, unsigned char x_low)  // Combine 2 registers into 16-bit number
{
  unsigned int combined;
  unsigned int Cell0;
  combined = x_high;                //send x_high to rightmost 8 bits
  combined = combined << 8;         //shift x_high over to leftmost 8 bits
  combined = combined | x_low;              //logical OR keeps x_high intact in combined and fills in rightmost 8 bits
  return combined;
}

int CellConvert(unsigned int combined1, unsigned int combined2, unsigned int combined3)  // Conver 16-bit number to correct 12-bit number for voltage calculation
{
  int Cell[3];
  Cell[0] = combined1 & 4095;
  Cell[1] = combined2 >> 4;
  Cell[2] = combined3 & 4095;
  //float cellVoltage[3];
  for(int i=0; i<3; i++)
  {
    cellVoltage[i] = 1.5/1000*(Cell[i] - 512);
  }
  return cellVoltage[4];
}

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


void loop()
{
  //GetConfig();                    // Only needed for debugging purpose.
  SetConfig();
  //GetConfig();                    // Only needed for debugging purpose.
  ADCconvert();
  getCellVolts();
  cellVoltage[3]=CellConvert(BitShiftCombine(RawData[1], RawData[0]), BitShiftCombine(RawData[2], RawData[1]), BitShiftCombine(RawData[4], RawData[3]));
  for(int i=0; i<3; i++)
  {
    Serial.print("Cell ");
    Serial.print(i);
    Serial.print(": ");
    Serial.print(cellVoltage[i]);
    Serial.println(" V");
  }
  Serial.println("-------------------------------------");
  Serial.println(" ");
  delay(1200);
}

