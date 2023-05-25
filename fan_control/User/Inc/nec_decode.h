#ifndef NEC_DECODE_H_
#define NEC_DECODE_H_

#include "stm32f1xx_hal.h"

void NEC_Init(void);
void NEC_Task (void);
void necDecodeCallback(void);

#endif
