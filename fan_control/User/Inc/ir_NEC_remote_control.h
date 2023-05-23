#ifndef ir_NEC_remote_control
#define ir_NEC_remote_control

#include "stm32f1xx_hal.h"
#include "tim.h"

void IrRemoteControInit(void);
void IrRemoteControlCallback(void);

#endif /* ir_NEC_remote_control */
