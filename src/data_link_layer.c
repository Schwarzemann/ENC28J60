#include "data_link_layer.h"

int send_frame(const EthernetFrame* frame, uint32_t len) {
    /* Send the Ethernet frame over the network */
    int ret = ethernet_driver_send((const uint8_t*)frame, len);
    if (ret < 0) {
        handle_eth_send_error((const uint8_t*)frame, len);
        return -1;
    }
    return 0;
}

int receive_frame(EthernetFrame* frame, uint32_t len) {
    /* Receive an Ethernet frame from the network */
    int ret = ethernet_driver_receive((uint8_t*)frame, len);
    if (ret < 0) {
        handle_eth_receive_error();
        return -1;
    }
    return ret;
}
