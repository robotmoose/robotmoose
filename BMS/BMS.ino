// Written By Clayton Auld
// clayauld@gmail.com

// Some Variable names and values borrowed from 
// Ricky Gu's Open BMS Project
// https://github.com/rickygu/openBMS

// Nov 20 2014


//Registers

//Defines



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

#define SS_PIN        53  // Designate Chip select pin (can be any pin on mega or uno, because arduino is in master mode)
#define ADDRESS      0x80 // Designate Chip address: 10000000
byte CFGR0 = 0xE1;
byte CFGR1 = 0x00;
byte CFGR2 = 0x00;
byte CFGR3 = 0xFF;
byte CFGR4 = 0x00;
byte CFGR5 = 0x00;
byte conf[6];

#include <SPI.h> 

// PEC Variables
static byte crc8_table[256];    // 8-bit table for PEC calc
static int made_table = 0;     // table made flag
byte packet[18]={0};           // used for PEC calc
byte PECbyte;                  // PEC of a byte
byte PECpacket;                // PEC of packet
byte PECpacketREAD;            // value that PECpacket should be as read from 6803
unsigned int Response[20];
float cellVoltage[18];

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

void SetConfig()
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

void getCellVolts()
{
  digitalWrite(SS_PIN, LOW); // Chip Select
  SPI.transfer(RDCV); // Send command to read cells 1-4
  SPI.transfer(0xDC); // Send PEC byte for command
  Serial.println("Reading Cell Voltage Registers");

  for(int i=0; i<6; i++)
  {
    Response[i] = SPI.transfer(0x00);   // send command to read voltage registers
    //Serial.println(Response[i], HEX);
  }
  byte PECresponse;
  PECresponse=SPI.transfer(0x00);
  digitalWrite(SS_PIN, HIGH); // Chip Deselect

}

void ADCconvert()
{
  Serial.println("ADC conversion started");
  digitalWrite(SS_PIN, LOW); // Chip Select
  SPI.transfer(STCVAD); // Send command to start ADC conversion
  SPI.transfer(0xB0); // Send PEC byte for command
  while (byte i=LOW)
  {
    i = SPI.transfer(0x000);
  }
  digitalWrite(SS_PIN, HIGH); // Chip Deselect
}

unsigned int BitShiftCombine(unsigned char x_high, unsigned char x_low)
{
  unsigned int combined;
  unsigned int Cell0;
  combined = x_high;                //send x_high to rightmost 8 bits
  combined = combined << 8;         //shift x_high over to leftmost 8 bits
  combined = combined | x_low;              //logical OR keeps x_high intact in combined and fills in rightmost 8 bits
  return combined;
}

int CellConvert(unsigned int combined1, unsigned int combined2, unsigned int combined3, unsigned int combined4)
{
  int Cell[4];
  Cell[0] = combined1 & 4095;
  Cell[1] = combined2 >> 4;
  Cell[2] = combined3 & 4095;
  Cell[3] = combined4 >> 4;
  float cellVoltage[4];
  for(int i=0; i<4; i++)
  {
    cellVoltage[i] = 1.5/1000*(Cell[i] - 512);
    Serial.println(cellVoltage[i]);
  }
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


void loop()
{
 // Serial.println("Old Config");
  GetConfig();
  //Serial.println("New Config");
  SetConfig();
  //GetConfig();
  ADCconvert();
  getCellVolts();
  //Serial.println("Combined bytes:");
  int combined1=BitShiftCombine(Response[1], Response[0]);
  int combined2=BitShiftCombine(Response[2], Response[1]);
  int combined3=BitShiftCombine(Response[4], Response[3]);
  int combined4=BitShiftCombine(Response[5], Response[4]);
  CellConvert(combined1, combined2, combined3, combined4);
  //findHighCell;
  
  Serial.println("-------------------------------------");
  Serial.println(" ");
  delay(900);
}
