#include <SPI.h>
/////////////////////////////////////////////////////////////////////
//set the pin
/////////////////////////////////////////////////////////////////////
// PIN MAP
// Arduino |  THM3060
// GND     |   GND
// 3.3V    |   DVDD
// 13      |   SCLK
// 12      |   MISO
// 11      |   MOSI
// 10      |   RSTN
// 9       |   STANDBY
// 8       |   SS_N
////////////////////////////////////////////////////////////////////
const int RST = 10;
const int STANDBY = 9;
const int chipSelectPin = 8;

/////////////////////////////////////////////////////////////////////
// THM  
/////////////////////////////////////////////////////////////////////
#define DATA        0x00

#define PSEL        0x01

#define FCONB       0x02

#define EGT         0x03

#define CRCSEL      0x04

#define RSTAT       0x05

#define SCNTL       0x06

#define INTCON      0x07

#define RSCH        0x08

#define RSCL        0x09    

#define CRCH        0x0a

#define CRCL        0x0b

#define TMRH        0x0c

#define TMRL        0x0d

#define BPOS        0x0e

#define SMOD        0x10

#define PWTH        0x11



//#define RSTAT bits

#define FEND        0x01

#define CRCERR      0x02

#define TMROVER     0x04

#define DATOVER     0x08

#define FERR        0x10

#define PERR        0x20

#define CERR        0x40



#define TYPE_A      0x10

#define TYPE_B      0x00

#define ISO15693    0x20

#define ETK         0x30

#define MIFARE      0x50

#define SND_BAUD_106K   0x00

#define SND_BAUD_212K   0x04

#define SND_BAUD_424K   0x08

#define SND_BAUD_848K   0x0c



#define RCV_BAUD_106K   0x00

#define RCV_BAUD_212K   0x01

#define RCV_BAUD_424K   0x02

#define RCV_BAUD_848K   0x03



//THe mistake code that return when communicate with MF522
#define MI_OK 0
#define MI_NOTAGERR 1
#define MI_ERR 2
unsigned char revbuff[100];
unsigned short len;
unsigned char temp;
void setup() 
{ 
  Serial.begin(57600); 
  //init SPI
  SPI.begin();
  SPI.setDataMode (SPI_MODE0);
  SPI.setClockDivider(SPI_CLOCK_DIV32);
  SPI.setBitOrder(MSBFIRST);
  //set Chip select
  pinMode(chipSelectPin,OUTPUT); 
  digitalWrite(chipSelectPin, HIGH); 
  //Reset Chip
  pinMode(RST,OUTPUT);
  digitalWrite(RST, LOW);
  digitalWrite(RST, HIGH);
  //STANDBY must be set
  pinMode(STANDBY,OUTPUT);
  digitalWrite(STANDBY, LOW);
  //open RF
  delay(1000); 
   THM_OpenRF();
   delay(1000);
   temp = THM_ReadReg(SCNTL);
   Serial.println("RF is open now....");
   delay(1000);
   
   //THM_ChangeProtBaud(TYPE_A,SND_BAUD_106K,RCV_BAUD_106K);
   THM_ChangeProtBaud(ISO15693,SND_BAUD_106K,RCV_BAUD_106K);
}


void loop()
{

   //THM_ChangeProtBaud(TYPE_A,SND_BAUD_106K,RCV_BAUD_106K);
   //delay(1000);
   unsigned char jiaotongreqb[] = {0x26};
   unsigned char nailreqb[] = {0x26, 0x01, 0x00};
   unsigned char revbuff[100], tempbuff[200];
   unsigned short len;
   unsigned char temp;
   //THM_SendFrame(jiaotongreqb,1);
   THM_SendFrame(nailreqb,3);
   temp =THM_WaitReadFrame(&len,revbuff);
   if (temp == FEND ) {
   Serial.println("=====================");
   for (int i =0 ; i < len;i++) {
      Serial.print(String(revbuff[i], HEX));
   
   }
   Serial.println();
   Serial.println("====================="); 
   delay(3000); 
   }
   
   
}


void THM_WriteReg(unsigned char address,unsigned char content)

{  
  unsigned char temp_buff[2];

  temp_buff[0] = address | 0x80;
  temp_buff[1] = content;
  SPI_FRAME_START();
  SPI_SendBuff(temp_buff, 2);
  SPI_FRAME_END();
}

unsigned char THM_ReadReg(unsigned char address)

{

  unsigned char value;

  SPI_FRAME_START();	
  address = address & 0x7f;
  SPI.transfer(address);
  value = SPI.transfer(0x00);
  SPI_FRAME_END();
  return(value);

}

void SPI_FRAME_START() {
  digitalWrite(chipSelectPin, LOW);
}

void SPI_FRAME_END() {
  digitalWrite(chipSelectPin, HIGH);
}


unsigned char THM_WaitReadFrame(unsigned short *len, unsigned char *buffer)

{

  unsigned char temp,temp1;	

  *len = 0;

  while (1)	

  {  	    
    temp = THM_ReadReg(RSTAT);   
    if (temp & 0x80)
      break;
  }	           
  
  
  //Serial.println(temp);

  if (temp & CERR )   { 

    temp = CERR; 
     //Serial.println("get card state: CERR");   
  }
  //BitPos  

  else if (temp & PERR)    {    

    temp = PERR;
    //Serial.println("get card state: PERR");   

  }
  //Frame Error

  else if (temp & FERR) {       

    temp = FERR;
     //Serial.println("get card state: FERR");   
  }

  else if (temp & DATOVER)    {    

    temp = DATOVER;  
    //Serial.println("get card state: DATOVER");      
  }
  //Data Overflow

  else if (temp & TMROVER) {

    temp = TMROVER;   
    //Serial.println("get card state: TMROVER");   
  }
  //Timeout

  else if (temp & CRCERR)    {

    temp = CRCERR;             
  }


  else  {

    temp = FEND;

  }
 

  *len =((unsigned short)(THM_ReadReg(RSCH)) <<8 ) + THM_ReadReg(RSCL);  		    

  if (*len != 0x00 )

  {

    SPI_FRAME_START();



    temp1 = DATA;

    SPI_SendBuff( &temp1,1);

    SPI_RecvBuff( buffer,*len);



    SPI_FRAME_END();

  }      


  THM_WriteReg(RSTAT,0x00);        

  return (temp);

}


void SPI_SendBuff(unsigned char *buf,unsigned int num)

{

  if ((buf== NULL)||(num ==0)) return;

  while( num--)

  {

    SPI.transfer(*buf++);

  }  

}	   


void SPI_RecvBuff(unsigned char *buf,unsigned int num)

{

  if ((buf== NULL)||(num ==0)) return;

  while(num--)

  {

    *buf++ = SPI.transfer(0x00);	

  }

}

void THM_SendFrame(unsigned char *buffer,unsigned short num)

{

  unsigned char temp;	

  THM_WriteReg(SCNTL, 0x5);	                              
  THM_WriteReg(SCNTL, 0x01);                                 
  temp = DATA | 0x80;	
  SPI_FRAME_START();
  SPI_SendBuff(&temp,1);

  SPI_SendBuff(buffer,num);	                               

  SPI_FRAME_END();

  THM_WriteReg(SCNTL, 0x03);                                 
}


void THM_OpenRF()

{

  THM_WriteReg(SCNTL,0x01);

  return;

}
void THM_CloseRF()

{

  THM_WriteReg(SCNTL,0x00);

  return;

}

void THM_ChangeProtBaud(unsigned char prot, unsigned char sndbaud, unsigned char rcvbaud)

{

  THM_WriteReg( PSEL, prot | sndbaud | rcvbaud );

  return;

} 


unsigned int str_hex(unsigned char *str,unsigned char *hex)

{

  unsigned char ctmp, ctmp1,half;

  unsigned int num=0;

  do{

    do{

      half = 0;

      ctmp = *str;

      if(!ctmp) break;

      str++;

    }
    while((ctmp == 0x20)||(ctmp == 0x2c)||(ctmp == '\t'));

    if(!ctmp) break;

    if(ctmp>='a') ctmp = ctmp -'a' + 10;

    else if(ctmp>='A') ctmp = ctmp -'A'+ 10;

    else ctmp=ctmp-'0';

    ctmp=ctmp<<4;

    half = 1;

    ctmp1 = *str;

    if(!ctmp1) break;

    str++;

    if((ctmp1 == 0x20)||(ctmp1 == 0x2c)||(ctmp1 == '\t'))

    {

      ctmp = ctmp>>4;

      ctmp1 = 0;

    }

    else if(ctmp1>='a') ctmp1 = ctmp1 - 'a' + 10;

    else if(ctmp1>='A') ctmp1 = ctmp1 - 'A' + 10;

    else ctmp1 = ctmp1 - '0';

    ctmp += ctmp1;

    *hex = ctmp;

    hex++;

    num++;

  }
  while(1);

  if(half)

  {

    ctmp = ctmp>>4;

    *hex = ctmp;

    num++;

  }

  return(num);



}



//hex to string

void hex_str(unsigned char *inchar, unsigned int len, unsigned char *outtxt)

{

  unsigned char hbit,lbit;

  unsigned int i;

  for(i=0;i<len;i++)

  {

    hbit = (*(inchar+i)&0xf0)>>4;

    lbit = *(inchar+i)&0x0f;

    if (hbit>9) outtxt[2*i]='A'+hbit-10;

    else	outtxt[2*i]='0'+hbit;

    if (lbit>9) outtxt[2*i+1]='A'+lbit-10;

    else    outtxt[2*i+1]='0'+lbit;

  }

  outtxt[2*i] = 0;

}



