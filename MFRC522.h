#ifndef __MFRC522__
#define __MFRC522__


#if ARDUINO >= 100
#include <Arduino.h>
#else
#include <WProgram.h>
#endif

#include <SPI.h>

//data array maxium length
const unsigned int MAX_LEN = 16;

//MFRC522 command bits
const unsigned char PCD_IDLE = 0x00; //NO action; cancel current commands
const unsigned char PCD_AUTHENT = 0x0E; //verify password key
const unsigned char PCD_RECEIVE = 0x08; //receive data
const unsigned char PCD_TRANSMIT = 0x04; //send data
const unsigned char PCD_TRANSCEIVE = 0x0C; //send and receive data
const unsigned char PCD_RESETPHASE = 0x0F; //reset
const unsigned char PCD_CALCCRC = 0x03; //CRC check and caculation
 
//Mifare_One card command bits
const unsigned char PICC_REQIDL = 0x26; //Search the cards that not into sleep mode in the antenna area 
const unsigned char PICC_REQALL = 0x52; //Search all the cards in the antenna area
const unsigned char PICC_ANTICOLL = 0x93; //prevent conflict
const unsigned char PICC_SElECTTAG = 0x93; //select card
const unsigned char PICC_AUTHENT1A = 0x60; //verify A password key
const unsigned char PICC_AUTHENT1B = 0x61; //verify B password key
const unsigned char PICC_READ = 0x30; //read 
const unsigned char PICC_WRITE = 0xA0; //write
const unsigned char PICC_DECREMENT = 0xC0; //deduct value
const unsigned char PICC_INCREMENT = 0xC1; //charge up value
const unsigned char PICC_RESTORE = 0xC2; //Restore data into buffer
const unsigned char PICC_TRANSFER = 0xB0; //Save data into buffer
const unsigned char PICC_HALT = 0x50; //sleep mode
 
 
//THe mistake code that return when communicate with MFRC522
const unsigned char MI_OK = 0;
const unsigned char MI_NOTAGERR = 1;
const unsigned char MI_ERR = 2;
 
 
//------------------MFRC522 register ---------------
//Page 0:Command and Status
const unsigned char Reserved00 = 0x00; 
const unsigned char CommandReg = 0x01;
const unsigned char CommIEnReg = 0x02;
const unsigned char DivlEnReg = 0x03; 
const unsigned char CommIrqReg = 0x04; 
const unsigned char DivIrqReg = 0x05;
const unsigned char ErrorReg = 0x06; 
const unsigned char Status1Reg = 0x07; 
const unsigned char Status2Reg = 0x08; 
const unsigned char FIFODataReg = 0x09;
const unsigned char FIFOLevelReg = 0x0A;
const unsigned char WaterLevelReg = 0x0B;
const unsigned char ControlReg = 0x0C;
const unsigned char BitFramingReg = 0x0D;
const unsigned char CollReg = 0x0E;
const unsigned char Reserved01 = 0x0F;
//Page 1:Command 
const unsigned char Reserved10 = 0x10;
const unsigned char ModeReg = 0x11;
const unsigned char TxModeReg = 0x12;
const unsigned char RxModeReg = 0x13;
const unsigned char TxControlReg = 0x14;
const unsigned char TxAutoReg = 0x15;
const unsigned char TxSelReg = 0x16;
const unsigned char RxSelReg = 0x17;
const unsigned char RxThresholdReg = 0x18;
const unsigned char DemodReg = 0x19;
const unsigned char Reserved11 = 0x1A;
const unsigned char Reserved12 = 0x1B;
const unsigned char MifareReg = 0x1C;
const unsigned char Reserved13 = 0x1D;
const unsigned char Reserved14 = 0x1E;
const unsigned char SerialSpeedReg = 0x1F;
//Page 2:CFG 
const unsigned char Reserved20 = 0x20; 
const unsigned char CRCResultRegM = 0x21;
const unsigned char CRCResultRegL = 0x22;
const unsigned char Reserved21 = 0x23;
const unsigned char ModWidthReg = 0x24;
const unsigned char Reserved22 = 0x25;
const unsigned char RFCfgReg = 0x26;
const unsigned char GsNReg = 0x27;
const unsigned char CWGsPReg = 0x28;
const unsigned char ModGsPReg = 0x29;
const unsigned char TModeReg = 0x2A;
const unsigned char TPrescalerReg = 0x2B;
const unsigned char TReloadRegH = 0x2C;
const unsigned char TReloadRegL = 0x2D;
const unsigned char TCounterValueRegH = 0x2E;
const unsigned char TCounterValueRegL = 0x2F;
//Page 3:TestRegister 
const unsigned char Reserved30 = 0x30;
const unsigned char TestSel1Reg = 0x31;
const unsigned char TestSel2Reg = 0x32;
const unsigned char TestPinEnReg = 0x33;
const unsigned char TestPinValueReg = 0x34;
const unsigned char TestBusReg = 0x35;
const unsigned char AutoTestReg = 0x36;
const unsigned char VersionReg = 0x37;
const unsigned char AnalogTestReg = 0x38;
const unsigned char TestDAC1Reg = 0x39;
const unsigned char TestDAC2Reg = 0x3A; 
const unsigned char TestADCReg = 0x3B; 
const unsigned char Reserved31 = 0x3C; 
const unsigned char Reserved32 = 0x3D; 
const unsigned char Reserved33 = 0x3E;
const unsigned char Reserved34 = 0x3F;
//-----------------------------------------------

class MFRC522
{
public:
	MFRC522();

	void Write(const unsigned char,const unsigned char);
	unsigned char Read(const unsigned char);
	void SetBitMask(const unsigned char,const unsigned char);
	void ClearBitMask(const unsigned char,const unsigned char);
	void AntennaOn();
	void AntennaOff();
	void Reset();
	void Init();

	unsigned char MFRC522ToCard(const unsigned char,
		const unsigned char*,
		const unsigned char,
		unsigned char*,
		unsigned int*);
	unsigned char WriteBlock(unsigned char,unsigned char*);
	unsigned char Request(unsigned char,unsigned char*);
	unsigned char Anticoll(unsigned char*);
	void CalulateCRC(unsigned char*,unsigned char,unsigned char*);
	unsigned char SelectTag(unsigned char*);
	void Halt();
private:
	const static int chip_select_pin = 10;
	const static int NRSTPD = 5;
};

#endif