/*********************************************
 
Header for Network layer protocols IPV4, ARP

********************************************/

#ifndef NETWORK_H
#define NETWORK_H

#include <cstdint>

#ifndef TRUE
#define TRUE (1 == 1)
#define FALSE (0 == 1)
#endif

#ifdef LITTLEENDIAN
#define BYTESWAP16(A) (((A) >> 8) | ((A & 0xFF) << 8))
#define BYTESWAP32(A) ((((A) >> 24) & 0xFF) | (((A) >> 8) & (0x0000FF00)) | (((A) << 8) & (0x00FF0000)) | (((A) << 24) & (0xFF000000)))
#define OCTET1(A) ((uint8_t)((A)&0xFF))
#define OCTET2(A) ((uint8_t)(((A) >> 8) & 0xFF))
#define OCTET3(A) ((uint8_t)(((A) >> 16) & 0xFF))
#define OCTET4(A) ((uint8_t)(((A) >> 24) & 0xFF))
#else // BIG ENDIAN
#define BYTESWAP16(A) (A)
#define BYTESWAP32(A) (A)
#define OCTET1(A) ((uint8_t)(((A) >> 24) & 0xFF))
#define OCTET2(A) ((uint8_t)(((A) >> 16) & 0xFF))
#define OCTET3(A) ((uint8_t)(((A) >> 8) & 0xFF))
#define OCTET4(A) ((uint8_t)((A)&0xFF))
#endif

#define ARP_REQUEST (0x0001)
#define ARP_REPLY (0x0002)

#define ARPforIP (0x0800)
#define DLLisETHERNET (0x0001)

#define ARPinETHERNET (0x0806)
#define RARPinETHERNET (0x8035)
#define IP4inETHERNET (0x0800)
#define IP6inETHERNET (0x86DD)

#define UDPinIP4 (0x11)
#define TCPinIP4 (0x06)
#define ICMPinIP4 (0x01)

#define DONT_FRAGMENT (0x40)
#define MORE_FRAGMENTS (0x20)

#define DEFAULT_TTL (0x40)

#define OUR_ETHERNET_UNICAST (1)
#define ETHERNET_BROADCAST (2)
#define LLMNR_MULTICAST (3)
#define mDNS_MULTICAST (4)

#define OUR_IP_UNICAST (3)
#define LIMITED_IP_BROADCAST (4)
#define SUBNET_IP_BROADCAST (5)
#define mDNS_IP_MULTICAST (6)
#define LLMNR_IP_MULTICAST (7)

#define ICMP_ECHO_REQ (8)
#define ICMP_ECHO_REPLY (0)
#define PING (ICMP_ECHO_REQ)
#define PONG (ICMP_ECHO_REPLY)

#define MAX_STORED_SIZE (46 + 189 * 2)

#define MAX_PACKET_SIZE (590)

#define ETH_HEADER_SIZE (14)
#define ARP_HEADER_SIZE (28)
#define MAX_HEADER_SIZE (ETH_HEADER_SIZE + ARP_HEADER_SIZE)
#define IP_HEADER_SIZE (20)

#define TICKS_TO_HOLD_MAC (120)

#define MAX_ARP_HELD (3)

typedef uint32_t IP4_address;

typedef struct {
    uint8_t IP6[16];
} IP6_address;

typedef struct {
    uint8_t MAC[6];
} MAC_address;

typedef struct {
    unsigned headerLength : 4;
    unsigned version : 4;
    unsigned unused : 2;
    unsigned reliability : 1;
    unsigned throughput : 1;
    unsigned delay : 1;
    unsigned precedence : 3;
    uint16_t totalLength;
    uint16_t id;
    unsigned fragmentOffset : 13;
    unsigned moreFragments : 1;
    unsigned dontFragment : 1;
    unsigned reserved : 1;
    uint8_t TTL;
    uint8_t protocol;
    uint16_t checksum;
    IP4_address source;
    IP4_address destination;
} IP4_header;

typedef struct {
    unsigned version : 4;
    unsigned traffic_class : 8;
    unsigned flowLabelHigh : 4;
    unsigned flowLabelLow : 16;
    unsigned payloadLength : 16;
    unsigned nextHeader : 8;
    unsigned hopLimit : 8;
    IP6_address source;
    IP6_address destination;
} IP6_header;

typedef struct {
    uint8_t messagetype;
    uint8_t code;
    uint16_t checksum;
    union {
        uint32_t quench;
        struct {
            uint16_t id;
            uint16_t seq;
        };
    };
    uint8_t data[32];
} ICMP_message;

typedef struct {
    uint16_t hardware;
    uint16_t protocol;
    uint8_t hardware_size;
    uint8_t protocol_size;
    uint16_t type;
    MAC_address sourceMAC;
    IP4_address sourceIP;
    MAC_address destinationMAC;
    IP4_address destinationIP;
} ARP_message;

typedef struct {
    MAC_address destinationMAC;
    MAC_address sourceMAC;
    uint16_t type;
} Ethernet_header;

typedef struct {
    unsigned ARP : 2;
    unsigned IP : 4;
    unsigned TIME : 2;
} Status;

#define TCP_IDLE (0)

#define DHCP_IDLE (0)
#define DHCP_WAIT_OFFER (1)
#define DHCP_SEND_REQ (2)
#define DHCP_WAIT_ACK (3)
#define APIPA_TRY (4)
#define APIPA_FAIL (5)
#define IP_SET (6)

#define TIME_UNSET (0)
#define TIME_WAIT_DNS (1)
#define TIME_REQUESTED (2)
#define TIME_SET (3)

typedef struct {
    union {
        struct {
            Ethernet_header Ethernet;
            ARP_message ARP;
        };
        uint8_t bytes[MAX_HEADER_SIZE];
        uint16_t words[MAX_HEADER_SIZE / 2];
    };
} MergedARP;

#define EEPROM_MAGIC1 (100)
#define EEPROM_MAGIC2 (101)
#define EEPROM_IP_SEQ (102)
#define EEPROM_TCP_PORT (104)
#define EEPROM_UDP_PORT (108)
#define EEPROM_MY_IP (110)
#define EEPROM_MY_MAC (EEPROM_MY_IP + 4)

#include "transport.h"

void Error();
void copyIP4(IP4_address* IPTo, const IP4_address* IPFrom);
void copyMAC(MAC_address* MACTo, const MAC_address* MACFrom);
uint8_t IP4_match(const IP4_address* IP1, const IP4_address* IP2);
void refreshMACList();
void delay_ms(uint16_t ms);
uint16_t checksum(uint16_t* words, int16_t length);
uint32_t checksumSupport(uint16_t* words, int16_t length);
void checksumBare(uint32_t* scratch, uint16_t* words, int16_t length);
uint16_t IP4checksum(MergedPacket* Mash);
int8_t mDNS(IP4_address* IP);
int8_t LLMNR(IP4_address* IP);
int8_t MACForUs(MAC_address* MAC);
int8_t IP4ForUs(IP4_address* IP);

uint8_t handlePacket();
uint8_t launchIP4(MergedPacket*, uint8_t csums, void (*callback)(uint16_t start, uint16_t length, uint8_t* result), uint16_t offset);
void launchARP(MergedARP* Mish);
void prepareIP4(MergedPacket* Mash, uint16_t payload_length, IP4_address* ToIP, uint8_t protocol);

void IP4toBuffer(IP4_address IP);

#ifdef LITTLEENDIAN
#define MAKEIP4(MSB, B2, B3, LSB) ((((uint32_t)LSB) << 24) | (((uint32_t)B3) << 16) | (((uint32_t)B2) << 8) | ((uint32_t)MSB))
#else // BIG ENDIAN
#define MAKEIP4(MSB, B2, B3, LSB) ((((uint32_t)MSB) << 24) | (((uint32_t)B2) << 16) | (((uint32_t)B3) << 8) | ((uint32_t)LSB))
#endif

#endif
