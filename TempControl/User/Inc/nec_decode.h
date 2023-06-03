#ifndef NEC_DECODE_H_
#define NEC_DECODE_H_

#include "stm32f1xx_hal.h"

void NEC_Init(void);
void NEC_RX_Executer (void);
void necDecodeCallback(void);
uint8_t* IrGetMessage(void);

#endif
