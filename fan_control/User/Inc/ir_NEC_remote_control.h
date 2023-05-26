#ifndef ir_NEC_remote_control
#define ir_NEC_remote_control

#include "stm32f1xx_hal.h"
#include "tim.h"

void IrRemoteControInit(void);
void IrRemoteControlCallback(void);
uint8_t IrSendMessage(uint8_t length, uint8_t *message, uint8_t repeat);
void WaitForSendMessage(void);

#endif /* ir_NEC_remote_control */
