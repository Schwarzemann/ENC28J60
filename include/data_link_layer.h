#ifndef DATA_LINK_LAYER_H
#define DATA_LINK_LAYER_H

#include <stdint.h>
#include "error_handling.h"

#define MAC_ADDRESS_LEN 6

/* Define the Ethernet frame structure */
typedef struct {
    uint8_t dest_address[MAC_ADDRESS_LEN];
    uint8_t src_address[MAC_ADDRESS_LEN];
    uint16_t type;
    uint8_t payload[];
} EthernetFrame;

/**
 * Sends an Ethernet frame over the network.
 *
 * @param frame The Ethernet frame to be sent.
 * @param len The length of the Ethernet frame.
 *
 * @return 0 on success, -1 on failure.
 */
int send_frame(const EthernetFrame* frame, uint32_t len);

/**
 * Receives an Ethernet frame from the network.
 *
 * @param frame A buffer to store the received Ethernet frame.
 * @param len The maximum length of the buffer.
 *
 * @return The length of the received Ethernet frame, or -1 on failure.
 */
int receive_frame(EthernetFrame* frame, uint32_t len);

#endif /* DATA_LINK_LAYER_H */
