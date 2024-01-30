/*
    ENC28J60 Ethernet module driver.
*/

#ifdef USE_ENC28J60

#include <avr/io.h>
#include <util/delay.h>
#include <avr/eeprom.h>
#include <stdio.h>
#include <string.h>

#include "network.h"
#include "transport.h"
#include "enc28j60.h"

#ifdef USE_DHCP
uint8_t magic_cookie[4]={99,130,83,99};
#define MAGIC_CSUM (BYTESWAP16(0xB6E5))
uint8_t dhcp_option_overload;  

#endif

#define SPIN_SPI(X)    SPDR=(X);WAIT_SPI()   
#define WAIT_SPI()     while(!(SPSR&(1<<SPIF)))
#define ETH_ACTIVATE    (ETH_SPI_SEL_PORT&=(~(1<<ETH_SPI_SEL_CS)))
#define ETH_DEACTIVATE  (ETH_SPI_SEL_PORT|=  (1<<ETH_SPI_SEL_CS))

extern IP4_address myIP;
extern MAC_address myMAC;
 
static uint16_t ptrNextPacket;  
static uint16_t ptrThisPacket;
static uint8_t inProgress=FALSE;  
static uint8_t currentBank=99;    
static uint16_t IPoptlen;         

union {  
  uint16_t word;
  struct { 
    uint8_t byte_1;   
    uint8_t byte_2;   
  };
} join;

uint8_t  linkPacketsAvailable(void);


static uint8_t readEthRegister(uint8_t reg) 
{ 
ETH_ACTIVATE;
SPIN_SPI(reg);
SPIN_SPI(0);
ETH_DEACTIVATE ;
return (SPDR);
}

static void writeEthRegister(uint8_t reg,uint8_t data) 
{ 
ETH_ACTIVATE ;
SPIN_SPI(0x40 | reg);
SPIN_SPI(data);
ETH_DEACTIVATE ;
return;
}

static uint8_t readMACRegister(uint8_t reg) 
{
uint8_t sure;  
do {
  ETH_ACTIVATE ;
  SPIN_SPI(reg);
  SPIN_SPI(0);  
  SPIN_SPI(0); 
  ETH_DEACTIVATE ;
  ETH_ACTIVATE ;
  SPIN_SPI(reg);
  SPIN_SPI(0);  
  SPIN_SPI(0); 
  sure=SPDR;
  ETH_DEACTIVATE ;
  ETH_ACTIVATE ;
  SPIN_SPI(reg);
  SPIN_SPI(0);  
  SPIN_SPI(0); 
  ETH_DEACTIVATE ;
} while (sure!=SPDR);
return (sure);
}
#define readMIIRegister(X) (readMACRegister((X))) 
static void writeMACRegister(uint8_t reg,uint8_t data) 
{
do    { writeEthRegister(reg,data); } 
while (readMACRegister(reg)!=data); 
}
#define writeMIIRegister(X,Y) (writeEthRegister((X),(Y))) 
static void setBank(uint8_t bank) 
{
if (currentBank!=bank) {
  uint8_t dummy=readEthRegister(ETH_ECON1) & 0xFC;   
  currentBank=(bank & 0x03);
  writeEthRegister(ETH_ECON1,(dummy | currentBank));
}
}
void linkReadBufferMemoryArray(uint16_t len,uint8_t * dataBuffer) 
{ 
ETH_ACTIVATE ;
SPIN_SPI(0x3A);
while (len--) { 
  SPIN_SPI(0);
  *(dataBuffer++)=SPDR;
}
ETH_DEACTIVATE ;
}
static void readBufferByte(uint8_t * data) { linkReadBufferMemoryArray(1,data); }
uint8_t linkNextByte(void) 
          { uint8_t data; linkReadBufferMemoryArray(1,&data); return data; }
static void writeBufferMemoryArray(uint16_t len,uint8_t * dataBuffer) 
{ 
ETH_ACTIVATE ;
SPIN_SPI(0x7A);
while (len--) {  SPIN_SPI(*(dataBuffer++)); }
ETH_DEACTIVATE ;
}
static void writeBufferMemoryZeros(uint16_t len) 
{ 
ETH_ACTIVATE ;
SPIN_SPI(0x7A);
while (len--) {  SPIN_SPI(0); }
ETH_DEACTIVATE ;
}

static void writeBufferByte(uint8_t data) {writeBufferMemoryArray(1,&data); }

static void ethBitFieldSet(uint8_t reg,uint8_t mask)
{ 
ETH_ACTIVATE ;
SPIN_SPI(0x80 | reg);
SPIN_SPI(mask);
ETH_DEACTIVATE ;
}

static void ethBitFieldClr(uint8_t reg,uint8_t mask)
{ 
ETH_ACTIVATE ;
SPIN_SPI(0xA0 | reg);
SPIN_SPI(mask);
ETH_DEACTIVATE ;
}

static uint16_t readPhyRegister(uint8_t reg)
{  

setBank(2);
writeEthRegister(0x14,reg);  
ethBitFieldSet(0x12,1<<0);   
delay_ms(1.0);  
setBank(3);
uint8_t state=TRUE;
while (state) state=readEthRegister(0x0A)&0x01;  
setBank(2);
ethBitFieldClr(0x12,1<<0);   
uint16_t result=readEthRegister(0x18);         
result|=((uint16_t)readEthRegister(0x19))<<8;  

return result;
}

static void writePhyRegister(uint8_t reg,uint16_t data)
{  

setBank(2);
writeEthRegister(0x14,reg);        
writeEthRegister(0x16,data&0xFF);  
writeEthRegister(0x17,data>>8);    
delay_ms(1.0);  
setBank(3);
uint8_t state=TRUE;
while (state) state=readEthRegister(0x0A)&0x01;  

return;
}

static void linkSoftReset(void)
{
ETH_ACTIVATE ;
SPIN_SPI(0xFF);
ETH_DEACTIVATE ;
delay_ms(100);
currentBank=99;  
}

void setClockoutScale(uint8_t data) 
{ 
setBank(3);
writeEthRegister(0x15,data&0x07);  
return;
}
void setClockout25MHz(void)     { setClockoutScale(1); }
void setClockout12pt5MHz(void)  { setClockoutScale(2); }
void setClockout8pt33MHz(void)  { setClockoutScale(3); }
void setClockout6pt25MHz(void)  { setClockoutScale(4); }
void setClockout3pt125MHz(void) { setClockoutScale(5); }
static inline void wrapReadIndex(uint16_t * index) 
{
if (*index>ERXND) *index-=(ERXND+1);
return;
}
static inline void setReadPointer(uint16_t ptr,uint8_t isReadBuffer) 
{ 
if (isReadBuffer) wrapReadIndex(&ptr);
writeEthRegister(0x00,ptr&0xFF);           
writeEthRegister(0x01,ptr>>8);  
}
static uint16_t resolveCsum(uint32_t csum)
{
uint32_t carry=(csum&0xFFFF0000)>>16;
csum =(csum&0x0000FFFF)+carry; 
csum+=(csum>>16);   
return (((uint16_t)csum)^0xFFFF);  
}
static uint32_t pktCsumRaw(uint16_t ptrStart,uint16_t bytes,uint8_t isReadBuffer) 
{
setBank(0);
setReadPointer(ptrStart,isReadBuffer); 
uint32_t result=0;

uint16_t i;
for (i=0;i<(bytes/2);i++) {
  linkReadBufferMemoryArray(2,&join.byte_1);
  result+=join.word;
}

if (bytes%2) { 
  readBufferByte(&join.byte_1);
  join.byte_2=0;
  result+=join.word;
}
return (result);
}
static uint16_t pktCsum(uint16_t ptrStart,uint16_t bytes,uint8_t isReadBuffer) 
{ 
return (resolveCsum(pktCsumRaw(ptrStart,bytes,isReadBuffer)));
}
static uint16_t ICMPchecksum(MergedPacket * Mash) { 
uint16_t length;
length=(BYTESWAP16(Mash->IP4.totalLength)) - Mash->IP4.headerLength*4;  
if (length % 2) 
  if (length<(MAX_STORED_SIZE-ETH_HEADER_SIZE+Mash->IP4.headerLength*4)) 
    Mash->bytes[ETH_HEADER_SIZE+Mash->IP4.headerLength*4+length++]=0;
return (checksum(&(Mash->words[(ETH_HEADER_SIZE+Mash->IP4.headerLength*4)/2]),(length/2))); 
}                                                        
static uint16_t TransportCsum(uint16_t ptrStart,uint8_t * dataBuffer,
                uint16_t inBuffer,uint16_t stopAt,uint32_t csum,
                uint16_t protocol,uint8_t isReadBuffer) 
{ 

MergedPacket * Mash=(MergedPacket *)dataBuffer;
uint16_t optionOffset=(Mash->IP4.headerLength-5)*4;  
uint16_t size=ETH_HEADER_SIZE+BYTESWAP16(Mash->IP4.totalLength)-optionOffset;
if (stopAt==0) stopAt=size;
if (protocol==TCPinIP4) {
  csum+=BYTESWAP16(TCPinIP4); 
  checksumBare(&csum,(uint16_t *)&Mash->TCP,8); 
  uint16_t payloadInBuffer=inBuffer-(ETH_HEADER_SIZE+Mash->IP4.headerLength*4+TCP_HEADER_SIZE);
  uint16_t payloadToConsider=stopAt-(ETH_HEADER_SIZE+Mash->IP4.headerLength*4+TCP_HEADER_SIZE);
  if (payloadInBuffer>payloadToConsider) payloadInBuffer=payloadToConsider;
  checksumBare(&csum,&Mash->TCP.urgent,1+payloadInBuffer/2); 
  if (stopAt>inBuffer) {
    uint16_t ptrToLink=ptrStart+inBuffer+optionOffset;  
    csum+=pktCsumRaw(ptrToLink,(stopAt-inBuffer),isReadBuffer);
  } else { 
    if (payloadInBuffer%2) { 
      join.word=Mash->TCP_payload.words[payloadInBuffer/2];
      join.byte_2=0;  
      csum+=join.word;
    }
  }
  csum+=BYTESWAP16((size-(ETH_HEADER_SIZE+IP_HEADER_SIZE))); 
} else if (protocol==UDPinIP4) {
  csum+=BYTESWAP16(UDPinIP4); 
  checksumBare(&csum,(uint16_t *)&Mash->UDP,3); 
  uint16_t payloadInBuffer=inBuffer-(ETH_HEADER_SIZE+Mash->IP4.headerLength*4+UDP_HEADER_SIZE);
  uint16_t payloadToConsider=stopAt-(ETH_HEADER_SIZE+Mash->IP4.headerLength*4+UDP_HEADER_SIZE);
  if (payloadInBuffer>payloadToConsider) payloadInBuffer=payloadToConsider;
  checksumBare(&csum,Mash->UDP_payload.words,payloadInBuffer/2); 

  if (stopAt>inBuffer) {
    uint16_t ptrToLink=ptrStart+inBuffer+optionOffset;  
    csum+=pktCsumRaw(ptrToLink,(stopAt-inBuffer),isReadBuffer);
  } else { 
    if (payloadInBuffer%2) { 
      join.word=Mash->UDP_payload.words[payloadInBuffer/2];
      join.byte_2=0;  
      csum+=join.word;
    }
  }
  csum+=Mash->UDP.messageLength; 
}
checksumBare(&csum,(uint16_t *)&dataBuffer[ETH_HEADER_SIZE+12],4);
return (resolveCsum(csum));
}
void linkInitialise(MAC_address myMAC)
{ 
  
ETH_SPI_SEL_DDR |= (1<<ETH_SPI_SEL_CS);
ETH_DEACTIVATE ;
SPI_CONTROL_DDR |= ((1<<SPI_CONTROL_MOSI) | (1<<SPI_CONTROL_SCK));
SPI_CONTROL_DDR &= (~(1<<SPI_CONTROL_MISO));
SPI_CONTROL_PORT&=(~((1<<SPI_CONTROL_MOSI)|(1<<SPI_CONTROL_SCK)));
SPSR |= (1 << SPI2X); 
SPCR &=  ~((1 << SPR1) | (1 << SPR0));
SPCR |= (1<<SPE)|(1<<MSTR);
linkSoftReset();
setBank(0);    
ptrNextPacket=0;  

writeEthRegister(0x08,ERXST&0xFF);  
writeEthRegister(0x09,ERXST>>8);    
writeEthRegister(0x0A,ERXND&0xFF);  
writeEthRegister(0x0B,ERXND>>8);
writeEthRegister(0x0C,ERXST&0xFF);  
writeEthRegister(0x0D,ERXST>>8);

setBank(1); 

#if defined USE_mDNS | defined USE_LLMNR

writeEthRegister(0x14, 0x00);
writeEthRegister(0x15, 0x00); 
writeEthRegister(0x08, 0x1f);  
writeEthRegister(0x09, 0x00);
writeEthRegister(0x0A, 0x00);
writeEthRegister(0x0B, 0xC0);  
writeEthRegister(0x0C, 0x01);  
writeEthRegister(0x0D, 0x00);
writeEthRegister(0x0E, 0x00);  

uint8_t pat=0;

uint32_t csum=0x0100 + 0x5E00 + 0x00E0;

  mp->IP4.checksum=0;
  join.word=IP4checksum(mp);

  writeEthRegister(0x02,(ETXST+IP_CHECKSUM_AT)&0xFF);  
  writeEthRegister(0x03,(ETXST+IP_CHECKSUM_AT)>>8);    

  writeBufferMemoryArray(2,&join.byte_1);  
}

if (checksums & CS_UDP) {

  join.word=TransportCsum(CTRL_HEADER_SIZE+ETXST,dataBuffer,
     (length<MAX_STORED_SIZE)?length:MAX_STORED_SIZE,forCsum,precompute,UDPinIP4,FALSE);  
  writeEthRegister(0x02,(ETXST+UDP_CHECKSUM_AT)&0xFF);  
  writeEthRegister(0x03,(ETXST+UDP_CHECKSUM_AT)>>8);    
  
  writeBufferMemoryArray(2,&join.byte_1);  
}
if (checksums & CS_TCP) {

  join.word=TransportCsum(CTRL_HEADER_SIZE+ETXST,dataBuffer,
             (length<MAX_STORED_SIZE)?length:MAX_STORED_SIZE,forCsum,precompute,TCPinIP4,FALSE);  
  writeEthRegister(0x02,(ETXST+TCP_CHECKSUM_AT)&0xFF);  
  writeEthRegister(0x03,(ETXST+TCP_CHECKSUM_AT)>>8);    
  writeBufferMemoryArray(2,&join.byte_1);  
}

ethBitFieldSet(ETH_ECON1,ECON1_TXRTS); 

return;
}
#endif




