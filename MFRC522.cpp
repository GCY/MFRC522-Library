#include <MFRC522.h>

#if ARDUINO >= 100
#include <Arduino.h>
#else
#include <WProgram.h>
#endif

MFRC522::MFRC522()
{
	pinMode(chip_select_pin,OUTPUT);
	digitalWrite(chip_select_pin,LOW);

    pinMode(NRSTPD,OUTPUT);
	digitalWrite(NRSTPD,HIGH);
}

void MFRC522::Write(const unsigned char address,const unsigned char value)
{
	digitalWrite(chip_select_pin,LOW);
	SPI.transfer((address << 1) & 0x7E); 
    SPI.transfer(value);
	digitalWrite(chip_select_pin,HIGH);
}

unsigned char MFRC522::Read(const unsigned char address)
{
	unsigned char value;

	digitalWrite(chip_select_pin,LOW);
    SPI.transfer(((address << 1) & 0x7E) | 0x80); 
    value = SPI.transfer(0x00);
	digitalWrite(chip_select_pin,HIGH);

	return value;
}

void MFRC522::SetBitMask(const unsigned char reg,const unsigned char mask)
{
    unsigned char temp;
    temp = Read(reg);
    Write(reg,temp | mask);
}

void MFRC522::ClearBitMask(const unsigned char reg,const unsigned char mask)
{
    unsigned char temp;
    temp = Read(reg);
    Write(reg,temp & (~mask)); // clear bit mask
}

void MFRC522::AntennaOn()
{
    unsigned char temp;
 
    temp = Read(TxControlReg);
    if(!(temp & 0x03)){
		SetBitMask(TxControlReg,0x03);
	}
}

void MFRC522::AntennaOff()
{
	ClearBitMask(TxControlReg,0x03);
}

void MFRC522::Reset()
{
	Write(CommandReg,PCD_RESETPHASE);
}

void MFRC522::Init()
{
	Reset();

    Write(TModeReg,0x8D);
    Write(TPrescalerReg,0x3E);
    Write(TReloadRegL,30); 
    Write(TReloadRegH,0);
     
    Write(TxAutoReg,0x40);
    Write(ModeReg,0x3D);

	AntennaOn();
}

unsigned char MFRC522::MFRC522ToCard(const unsigned char command,const unsigned char *send_data,const unsigned char send_len,unsigned char *back_data,unsigned int *back_len)
{
    unsigned char status = MI_ERR;
    unsigned char irqEn = 0x00;
    unsigned char waitIRq = 0x00;
    unsigned char lastBits;
    unsigned char n;
    unsigned int i;
 
    switch(command)
    {
        case PCD_AUTHENT: //verify card password
        {
            irqEn = 0x12;
            waitIRq = 0x10;
            break;
        }
        case PCD_TRANSCEIVE: //send data in the FIFO
        {
            irqEn = 0x77;
            waitIRq = 0x30;
            break;
        }
        default:
            break;
    }
    
    Write(CommIEnReg,irqEn | 0x80); //Allow interruption
    ClearBitMask(CommIrqReg,0x80); //Clear all the interrupt bits
    SetBitMask(FIFOLevelReg,0x80); //FlushBuffer=1, FIFO initilizate
     
    Write(CommandReg,PCD_IDLE); //NO action;cancel current command ???
 
    //write data into FIFO
    for (i = 0; i < send_len;++i){ 
        Write(FIFODataReg,send_data[i]); 
    }
 
    //procceed it
    Write(CommandReg,command);
    if (command == PCD_TRANSCEIVE){ 
		SetBitMask(BitFramingReg,0x80); //StartSend=1,transmission of data starts 
    } 
     
    //waite receive data is finished
    i = 2000; //i should adjust according the clock, the maxium the waiting time should be 25 ms???
    do {
        //CommIrqReg[7..0]
        //Set1 TxIRq RxIRq IdleIRq HiAlerIRq LoAlertIRq ErrIRq TimerIRq
        n = Read(CommIrqReg);
        --i;
    }

    while ((i != 0) && !(n & 0x01) && !(n & waitIRq));
 
    ClearBitMask(BitFramingReg,0x80); //StartSend=0
     
    if (i != 0){ 
		if(!(Read(ErrorReg) & 0x1B)){ //BufferOvfl Collerr CRCErr ProtecolErr
            status = MI_OK;
            if(n & irqEn & 0x01){ 
                status = MI_NOTAGERR; //?? 
            }
             
            if(command == PCD_TRANSCEIVE){
                n = Read(FIFOLevelReg);
                lastBits = Read(ControlReg) & 0x07;
                if (lastBits){ 
                    *back_len = (n - 1) * 8 + lastBits; 
                }
                else{ 
                    *back_len = n*8; 
                }
                 
                if(n == 0){ 
                    n = 1; 
                }
                if (n > MAX_LEN){ 
                    n = MAX_LEN; 
                }
                 
                //read the data from FIFO
                for (i=0; i < n;++i){ 
                    back_data[i] = Read(FIFODataReg); 
                }
            }
        }
        else{ 
            status = MI_ERR; 
        }
         
    }
     
    //SetBitMask(ControlReg,0x80); //timer stops
    //Write(CommandReg, PCD_IDLE); 
 
    return status;
}

unsigned char MFRC522::WriteBlock(unsigned char block_address,unsigned char  *write_data)
{
    unsigned char status;
    unsigned int recvBits;
    unsigned char i;
    unsigned char buff[18]; 
     
    buff[0] = PICC_WRITE;
    buff[1] = block_address;
    CalulateCRC(buff,2,&buff[2]);
    status = MFRC522ToCard(PCD_TRANSCEIVE,buff,4,buff,&recvBits);
 
    if ((status != MI_OK) || (recvBits != 4) || ((buff[0] & 0x0F) != 0x0A)){ 
        status = MI_ERR; 
    }
         
    if (status == MI_OK){
		for (i=0;i < 16;++i){ //Write 16 bytes data into FIFO
            buff[i] = *(write_data+i); 
        }
        CalulateCRC(buff,16,&buff[16]);
        status = MFRC522ToCard(PCD_TRANSCEIVE,buff,18,buff,&recvBits);
         
        if ((status != MI_OK) || (recvBits != 4) || ((buff[0] & 0x0F) != 0x0A)){ 
            status = MI_ERR; 
        }
    }
     
    return status;
}

unsigned char MFRC522::Request(unsigned char mode,unsigned char *tag_type)
{
    unsigned char status; 
    unsigned int backBits; //the data bits that received
 
    Write(BitFramingReg,0x07); //TxLastBists = BitFramingReg[2..0] ???
     
    tag_type[0] = mode;
    status = MFRC522ToCard(PCD_TRANSCEIVE,tag_type,1,tag_type,&backBits);
 
    if ((status != MI_OK) || (backBits != 0x10)){ 
        status = MI_ERR;
    }
    
    return status;
}

unsigned char MFRC522::Anticoll(unsigned char *uid)
{
    unsigned char status;
    unsigned char i;
    unsigned char serNumCheck=0;
    unsigned int unLen;
     
    //ClearBitMask(Status2Reg, 0x08); //strSensclear
    //ClearBitMask(CollReg,0x80); //ValuesAfterColl
    Write(BitFramingReg,0x00); //TxLastBists = BitFramingReg[2..0]
  
    uid[0] = PICC_ANTICOLL;
    uid[1] = 0x20;
    status = MFRC522ToCard(PCD_TRANSCEIVE,uid,2,uid,&unLen);
 
    if (status == MI_OK){
        //Verify card UID
        for (i=0;i < 4;++i){ 
            serNumCheck ^= uid[i];
        }
        if (serNumCheck != uid[i]){ 
            status = MI_ERR; 
        }
    }
 
    //SetBitMask(CollReg, 0x80); //ValuesAfterColl=1
 
    return status;
}

void MFRC522::CalulateCRC(unsigned char *input,unsigned char len,unsigned char *output)
{
    unsigned char i,n;
 
    ClearBitMask(DivIrqReg,0x04); //CRCIrq = 0
    SetBitMask(FIFOLevelReg,0x80); //Clear FIFO pointer
    //Write(CommandReg, PCD_IDLE);
 
    //Write data into FIFO 
    for (i=0;i < len;i++){ 
        Write(FIFODataReg,*(input + i)); 
    }
    Write(CommandReg,PCD_CALCCRC);
 
    //waite CRC caculation to finish
    i = 0xFF;
    do {
        n = Read(DivIrqReg);
        --i;
    }
    while ((i != 0) && !(n & 0x04)); //CRCIrq = 1
 
    //read CRC caculation result
    output[0] = Read(CRCResultRegL);
    output[1] = Read(CRCResultRegM);
}

unsigned char MFRC522::SelectTag(unsigned char *uid)
{
	unsigned char i;
	unsigned char status;
	unsigned char size;
	unsigned int recvBits;
	unsigned char buffer[9];

	//ClearBitMask(Status2Reg, 0x08);                        //MFCrypto1On=0

	buffer[0] = PICC_SElECTTAG;
	buffer[1] = 0x70;

	for (i=0; i<5; i++){
    buffer[i + 2] = *(uid + i);
	}

  CalulateCRC(buffer,7,&buffer[7]);
  
  status = MFRC522ToCard(PCD_TRANSCEIVE,buffer,9,buffer,&recvBits);
  if ((status == MI_OK) && (recvBits == 0x18)){
    size = buffer[0];
  }
  else{
    size = 0;
  }

  return size;
}

void MFRC522::Halt()
{
    unsigned char status;
    unsigned int unLen;
    unsigned char buff[4]; 
 
    buff[0] = PICC_HALT;
    buff[1] = 0;
    CalulateCRC(buff,2,&buff[2]);
  
    status = MFRC522ToCard(PCD_TRANSCEIVE,buff,4,buff,&unLen);
}