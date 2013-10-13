#include <MFRC522.h>
#include <SPI.h>

/* 16^0 ~ 16^7 */
unsigned long table[8] = {1,16,256,4096,65536,1048576,16777216,268435456};

MFRC522 mfrc522;

unsigned char uid[5];

void setup()
{
  Serial.begin(9600);
  SPI.begin();
  mfrc522.Init();
}

void loop()
{
  unsigned char status;
  unsigned char str[MAX_LEN];
  
  status = mfrc522.Request(PICC_REQIDL,str); 
  if (status != MI_OK){
    return;
  }
  
  status = mfrc522.Anticoll(str);
  if (status == MI_OK){
    memcpy(uid,str,5);
    mfrc522.SelectTag(uid);	//防止同張卡片持續感應與讀取
    
    Serial.print("Original UID : ");
    unsigned long sum = 0;	//將UID每位byte加總（4byte uid & 1byte check）
    for(int i = 3;i >= 0;--i){
      unsigned long temp_h = (uid[i] >> 4) & 0x0000000F;
      Serial.print(temp_h,HEX);
      sum += (temp_h * table[(i*2)+1]);	//加上UID每byte的高4bit
      unsigned long temp_l = (uid[i] & 0x0000000F);
      Serial.print(temp_l,HEX);
      sum += (temp_l * table[i*2]);		//加上UID每byte的低4bit
    }
    Serial.println("");
    
    Serial.print("8H10D UID : ");
    String _8H10D = String(sum);
    if(_8H10D.length() < 10){
      for(int i = 0;i < (10 - _8H10D.length());++i){
        Serial.print("0");	//8H10D格式未滿十碼開頭補零
      }
    }
    for(int i = 0;i < _8H10D.length();++i){
      Serial.print(_8H10D.charAt(i));
    }  
  }
  Serial.println("");
  
  mfrc522.Halt();
  delay(100);
}