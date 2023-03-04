/*
 * The header file defines a simple Ethernet header structure 
 * with the destination address, source address, and type fields. 
 * It also includes function declarations for initializing the Ethernet driver ethernet_driver_init, s
 * ending Ethernet packets ethernet_driver_send, 
 * and receiving Ethernet packets ethernet_driver_receive. 
 * The functions take pointers to data buffers and their lengths as arguments.
 */

#ifndef ETHERNET_DRV_H
#define ETHERNET_DRV_H

#include <stdint.h>

/* Ethernet header */
typedef struct {
    uint8_t dest[6];
    uint8_t src[6];
    uint16_t type;
} ethernet_header_t;

/* Ethernet driver initialization */
int ethernet_driver_init(void);

/* Ethernet driver send function */
int ethernet_driver_send(const uint8_t* data, uint32_t len);

/* Ethernet driver receive function */
int ethernet_driver_receive(uint8_t* data, uint32_t len);

#endif /* ETHERNET_DRV_H */
