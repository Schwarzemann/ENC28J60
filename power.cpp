/*
This AVR microcontroller code reads analog values from a specified ADC channel, 
calculates the average and divergence, and sends the results through UDP while toggling an 
LED to indicate processing stages in handling power-related measurements.
*/

#include <avr/io.h>
#include <util/delay.h>
#include <iostream>
#include <string.h>
#include <avr/interrupt.h>
#include <stdlib.h>
#include "network.h"

#include "power.h"

extern MAC_address myMAC;
extern MergedPacket MashE;
extern IP4_address myIP;

// ----------------------------------------------------------------------------
uint16_t ReadADC(uint8_t channel)
{
    uint16_t result;

    ADMUX = (AVCC_REF | channel);
    ADCSRA |= (1 << ADSC); // Start conversion

    while (!(ADCSRA & (1 << ADIF)));  // wait for it

    ADCSRA |= (1 << ADIF);  // clear interrupt

    result = ADCL; // 8 lowest bits (MUST be read first)
    result |= ((uint16_t)ADCH) << 8;

    return result;
}

// --------------------------------------------------------------------------------
void handlePower(uint8_t myADC)
{ // Handle a received Power message request to read a specific ADC
    // ADC ranges from 0 to 2.  0 is current; 1 is voltage; 2 is current/2

    uint16_t i, tmp;
    uint32_t total;
    int16_t average;
    uint8_t SafeADC;

    IP4_address his_IP4;

    copyIP4(&his_IP4, &MashE.IP4.source);

    SafeADC = (myADC % 6); // 0-5 allowable physically, avoids conflict with other bits

    LED_ON;

    for (i = 0; i < MAX_TIME_SAMPLES; i++)
    {
        tmp = ReadADC(SafeADC); // to slow down
        MashE.Power.waveform[i] = tmp;
        tmp = ReadADC(SafeADC); // to slow down
        tmp = ReadADC(SafeADC); // to slow down
    }

    LED_OFF;

    total = 0;
    for (i = 0; i < CYCLE; i++) // Across the 50Hz cycle, get average
        total += MashE.Power.waveform[i];
    average = (int16_t)(0.5 + (float)total / (float)CYCLE);

    total = 0;
    for (i = 0; i < CYCLE; i++) // Divergence from average proportional to current
        total += abs((int16_t)(MashE.Power.waveform[i]) - average); // Safe: spectrum is 10 bit

    tmp = (uint16_t)(0.5 + (float)total / (float)CYCLE); // Average divergence

    MashE.Power.current = BYTESWAP16(tmp);

#ifdef LITTLEENDIAN // Although implicit in BYTESWAP, this is faster
    for (i = 0; i < MAX_TIME_SAMPLES; i++) // Test MashE.Power.waveform[i]=BYTESWAP16(i*60);
        MashE.Power.waveform[i] = BYTESWAP16(MashE.Power.waveform[i]);
#endif

    //launchUDP(&MashE, &his_IP4, POWER_MY_PORT, MashE.UDP.sourcePort, sizeof(MashE.Power), NULL, 0);  // Prob OK
    launchUDP(&MashE, &his_IP4, POWER_MY_PORT, MashE.UDP.sourcePort, 2 * (1 + MAX_TIME_SAMPLES), NULL, 0);

    delay_ms(10);
    LED_OFF;

    return;
}