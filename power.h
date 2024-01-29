// Necessary definitons for power management.

#ifndef POWER_H
#define POWER_H

// ADC is on PORTC. Connected as follows:
const uint8_t ADC_I = 1 << 0;  // The current waveform
const uint8_t ADC_V = 1 << 1;  // The voltage waveform
const uint8_t ADC_IOVER2 = 1 << 2;  // Current waveform divided by 2

const uint8_t INTERNAL_REF = (1 << REFS1) | (1 << REFS2);  // Internal reference
const uint8_t AVCC_REF = 1 << REFS0;  // AVCC reference
const uint8_t AREF = 0;  // Analog reference

void InitialiseADC();
uint16_t ReadADC(uint8_t channel);
void InitPower();
void handlePower(uint8_t myADC);

#endif  // POWER_H