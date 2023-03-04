#ifndef ERROR_HANDLING_H
#define ERROR_HANDLING_H

#include <stdint.h>

/*
 * Prints an error message for a failed ethernet packet send operation
*/

void print_eth_send_error(const uint8_t* data, uint32_t len);

/*
 * Prints an error message for a failed ethernet packet receive operation
*/

void print_eth_receive_error(void);

/*
 * Handles an error that occurs during an ethernet packet send operation.
 * Prints an error message and the corresponding system error message.
*/

void handle_eth_send_error(const uint8_t* data, uint32_t len);

/*
 * Handles an error that occurs during an ethernet packet receive operation.
 * Prints an error message and the corresponding system error message.
*/

void handle_eth_receive_error(void);

#endif /* ERROR_HANDLING_H */
