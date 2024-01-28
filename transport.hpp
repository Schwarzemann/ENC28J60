/*********************************************

Header for Transport layer protocols TCP, UDP

 *********************************************/

#ifndef TRANSPORT_H
#define TRANSPORT_H

#define UDP_PORT 1
#define TCP_PORT 2

//#define START_DYNAMIC_PORTS  0xC000
#define START_DYNAMIC_PORTS  0xC080

#define TCP_CLOSED 1
#define TCP_LISTEN 2
#define TCP_SYN_RCVD 3
#define TCP_SYN_SENT 4
#define TCP_ESTABLISHED 5
#define TCP_CLOSE_WAIT 6
#define TCP_FIN_WAIT1 7
#define TCP_FIN_WAIT2 8
#define TCP_CLOSING 9
#define TCP_TIME_WAIT 10
#define TCP_LAST_ACK 11

#define MAX_TCP_ROLES 2
#define TCP_CLIENT 0
#define TCP_SERVER 1
#define TCP_FTP_CLIENT 0
#define TCP_FTP_PASSIVE 1

#define TCP_REJECT (MAX_TCP_ROLES + 1)

#define ACK_BIT 1
#define DATA_BIT 2
#define CONSEC_BIT 4

struct UDP_header {
  uint16_t sourcePort;
  uint16_t destinationPort;
  uint16_t messageLength;
  uint16_t UDP_checksum;
};

#define UDP_HEADER_SIZE 8

struct TCP_header {
  uint16_t sourcePort;
  uint16_t destinationPort;
  int32_t sequence;
  int32_t ack;
  uint8_t flags;
  uint16_t windowSize;
  uint16_t TCP_checksum;
  uint16_t urgent;
};

#define TCP_HEADER_SIZE 20

struct Retransmit {
  uint8_t role;
  bool active : 2;
  uint8_t retries : 6;
  uint8_t timeout;
  uint16_t payloadLength;
  uint16_t headerLength;
  int32_t lastByte;
  uint8_t* data;
  uint16_t (*callback)(uint16_t start, uint16_t length, uint8_t* result);
  int32_t sequence;
  uint16_t start;
};

struct TCP_TCB {
  uint8_t status : 4;
  uint8_t age : 4;
  uint16_t localPort;
  uint16_t remotePort;
  IP4_address remoteIP;
  int32_t lastByteSent;
  int32_t lastAckSent;
  int32_t lastByteReceived;
  int32_t lastAckReceived;
  uint16_t windowSize;
};

#define TCP_MAX_AGE 5
#define TCP_TIMEOUT 1
#define TCP_RETRIES 3

struct MergedACK {
  Ethernet_header Ethernet;
  IP4_header IP4;
  union {
    TCP_header TCP;
    struct {
      UDP_header UDP;
      uint8_t dummy[12];
    };
  };
};

#define MAX_PACKET_PAYLOAD (MAX_PACKET_SIZE - IP_HEADER_SIZE - TCP_HEADER_SIZE - ETH_HEADER_SIZE)

// Global functions
void initialiseTCP();
void countdownTCP();
void retxTCP();
void cleanupOldTCP();
void initiateTCPConnection(MergedPacket* Mash, uint16_t source_port, uint16_t destination_port, IP4_address ToIP, uint8_t role);
void TCP_SimpleDataOut(const char* send, const uint8_t role, bool reTx);
void TCP_SimpleDataOutProgmem(const char* send, const uint8_t role, bool reTx);
void TCP_DataIn(MergedPacket* Mash, uint16_t length, const uint8_t role);
void TCP_ComplexDataOut(MergedPacket* Mash, const uint8_t role, uint16_t payloadLength, uint16_t (*callback)(uint16_t start, uint16_t length, uint8_t* result), uint16_t offset, bool reTx);

void handleTCP(MergedPacket* Mash);
void handleUDP(MergedPacket* Mash, uint8_t flags);
void launchUDP(MergedPacket* Mash, IP4_address* ToIP, uint16_t sourcePort, uint16_t destinationPort, uint16_t data_length, void (*callback)(uint16_t start, uint16_t length, uint8_t* result), uint16_t offset);
uint16_t newPort(uint8_t protocol);
void MemOverflow();

#endif