#include "nec_decode.h"
#include "gpio.h"
#include "stm32f1xx_hal_gpio_ex.h"
/*
NEC_FRAME frame;
NEC_FRAME lastFrame;
int8_t pos = -10;	// change
uint32_t rawdata = 0;
uint8_t status = 0; // failed=3, received=1 , undetermined=0
uint8_t AGC_OK = 0;
uint32_t thigh = 0;
uint32_t tlow = 0;
uint8_t p = 0;
*/
void NEC_Init(void)
{
   NVIC_EnableIRQ(EXTI4_IRQn);
  /*
	GPIO_InitTypeDef GPIO_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	EXTI_InitTypeDef EXTI_InitStructure;

	RCC_APB2PeriphClockCmd(IR_GPIO_PORT_CLK, ENABLE);
	//RCC_APB2PeriphClockCmd(RCC_APB1Periph_SYSCFG, ENABLE);

	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_InitStructure.GPIO_Pin = IR_GPIO_PIN;
	GPIO_Init(IR_GPIO_PORT, &GPIO_InitStructure);

//	SYSCFG_EXTILineConfig(IR_EXTI_PORT_SOURCE, IR_EXTI_PIN_SOURCE);
	GPIO_EXTILineConfig(IR_EXTI_PORT_SOURCE, IR_EXTI_PIN_SOURCE);
	EXTI_InitStructure.EXTI_Line = IR_EXTI_LINE;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising_Falling;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);
	EXTI_ClearITPendingBit(IR_EXTI_LINE);

	NVIC_InitStructure.NVIC_IRQChannel = IR_NVIC_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
*/
}
/*
uint32_t NEC_GetRawData() {
	return rawdata;
}

void NEC_Reset() {
	AGC_OK = 0;
	pos = -10;	// change
	p=0;
	thigh=0;
	tlow=0;
	status = 0;
	rawdata = 0;
	frame.Address = 0;
	frame.Command = 0;
}

void NEC_TimerRanOut() {
	NEC_StopTimer();
	if (AGC_OK == 1) {
		frame = lastFrame;
		NEC_ReceiveInterrupt(frame);
		GPIO_SetBits(GPIOC, GPIO_Pin_9);
		NEC_Reset();
	} else {
		GPIO_ResetBits(GPIOC, GPIO_Pin_9);
		NEC_Reset();
	}
}

void NEC_PushBit(uint8_t bit) {
	bit &= 1;
	if (pos == 31) { // This is the last bit
		rawdata |= bit << pos;

		if (EXTENDED == 0) { // if it is not extended check consistency of address and command
			// Check the received data for consistency
			uint8_t a = 0;
			uint8_t na = 0;

			a = (uint8_t)(rawdata & 0xFF);
			na = (uint8_t)((rawdata & 0xFF00) >> 8);

			if (~a == na) { // address is correct

				uint8_t c = 0;
				uint8_t nc = 0;

				c = (uint8_t)((rawdata & 0xFF0000) >> 16);
				nc = (uint8_t)((rawdata & 0xFF000000) >> 24);

				if (~c == nc) { // command is correct
					frame.Address = a;
					frame.Command = c;
					NEC_ReceiveInterrupt(frame);
					lastFrame = frame;

					NEC_Reset();
					return;
				} else {
					status = 3;
					NEC_Reset();
					return;
				}

			} else {
				status = 3;
				NEC_Reset();
				return;
			}
		} else { // if it is extended check only command
			uint8_t ah = 0;
			uint8_t al = 0;
			uint16_t a = 0;
			uint8_t c = 0;
			uint8_t nc = 0;

			ah = (uint8_t)((rawdata & 0xFF00) >> 8);
			al = (uint8_t)(rawdata & 0xFF);
			a = (ah << 8) | al;
			c = (uint8_t)((rawdata & 0xFF0000) >> 16);
			nc = (uint8_t)((rawdata & 0xFF000000) >> 24);
			uint8_t r = ~c;

			if (r == nc) { // command is correct
				frame.Address = a;
				frame.Command = c;
				NEC_ReceiveInterrupt(frame);
				lastFrame = frame;
				NEC_Reset();
				return;
			} else {
				status = 3;
				NEC_Reset();
				return;
			}
		}

	} else if (pos >= 0) { // This is not the last bit
		rawdata |= bit << pos;
	}
	++pos;
}

void NEC_TimingDecode(uint32_t th, uint32_t tl) {

	if (AGC_OK == 1) { // AGC Pulse has been received
		if ((th <= T_PULSE * (1.0 + T_TOLERANCE)) && (th >= T_PULSE * (1.0
				- T_TOLERANCE))) { // HIGH pulse is OK
			if ((tl <= T_ZERO_SPACE * (1.0 + T_TOLERANCE)) && (tl
					>= T_ZERO_SPACE * (1.0 - T_TOLERANCE))) { // LOW identified as ZERO
				NEC_PushBit(0);
			} else if ((tl <= T_ONE_SPACE * (1.0 + T_TOLERANCE)) && (tl
					>= T_ONE_SPACE * (1.0 - T_TOLERANCE))) { // LOW identified as ONE
				NEC_PushBit(1);
			} else {
				status = 3;
				NEC_Reset();
			}
		} else {
			status = 3;
			NEC_Reset();
		}
	} else { //AGC Pulse has not been received yet
		if ((th <= T_AGC_PULSE * (1.0 + T_TOLERANCE)) && (th >= T_AGC_PULSE
				* (1.0 - T_TOLERANCE))) { // HIGH AGC Pulse is OK
			if ((tl <= T_AGC_SPACE * (1.0 + T_TOLERANCE)) && (tl >= T_AGC_SPACE
					* (1.0 - T_TOLERANCE))) { // LOW AGC Pulse OK
				AGC_OK = 1;
			}
		}
	}
}
*/
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
  if(GPIO_Pin == GPIO_PIN_4)
  {
    HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
  }
}
/*
void NEC_HandleEXTI(void)
{
	GPIO_SetBits(GPIOC, GPIO_Pin_8);
	if (GPIO_ReadInputDataBit(IR_GPIO_PORT, IR_GPIO_PIN) == 0) { // pin is now HW:LOW,NEC:HIGH
		if (p == 1) {
			tlow = NEC_GetTime();
			NEC_StopTimer();
			NEC_TimingDecode(thigh, tlow);
		} else if (p == 0) {
			++p;
		}
		NEC_StartTimer();
	} else { // pin is now HW:HIGH,NEC:LOW
		thigh = NEC_GetTime();
		NEC_StopTimer();
		NEC_StartTimer();
	}
	GPIO_ResetBits(GPIOC, GPIO_Pin_8);
}
/*
void NEC_StartTimer() { // Timer for overflow detection
	NVIC_InitTypeDef NVIC_InitStructure;
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	// Enable the TIM2 gloabal Interrupt
	NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 4;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	// TIM2 clock enable
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
	// Time base configuration
	TIM_TimeBaseStructure.TIM_Period = 12000; // 12ms oveflow
//	TIM_TimeBaseStructure.TIM_Prescaler = 84 - 1; // 84 MHz Clock down to 1 MHz (adjust per your clock to count microseconds)
	TIM_TimeBaseStructure.TIM_Prescaler = 24 - 1; // 84 MHz Clock down to 1 MHz (adjust per your clock to count microseconds)
	TIM_TimeBaseStructure.TIM_ClockDivision = 0;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);

	TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
	// TIM IT enable
	TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);
	// TIM5 enable counter
	TIM_Cmd(TIM2, ENABLE);

}

uint16_t NEC_GetTime() {
	return (uint16_t)(TIM2->CNT);
}

void NEC_StopTimer() {
	NVIC_InitTypeDef NVIC_InitStructure;
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	// Enable the TIM2 gloabal Interrupt
	NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 4;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = DISABLE;
	NVIC_Init(&NVIC_InitStructure);

	// TIM2 clock disable
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, DISABLE);
	// Time base configuration
	TIM_TimeBaseStructure.TIM_Period = 12000; // 12ms oveflow
	//TIM_TimeBaseStructure.TIM_Prescaler = 42 - 1; // 42 MHz Clock down to 1 MHz (adjust per your clock to count microseconds)
	TIM_TimeBaseStructure.TIM_Prescaler = 24 - 1; // 42 MHz Clock down to 1 MHz (adjust per your clock to count microseconds)
	TIM_TimeBaseStructure.TIM_ClockDivision = 0;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);
	// TIM IT enable
	TIM_ITConfig(TIM2, TIM_IT_Update, DISABLE);
	// TIM5 enable counter
	TIM_Cmd(TIM2, DISABLE);
}


void Int2Str(char *str, u32 intnum) {
	u32 Div = 1000000000;
	int i, j = 0, Status = 0;
	for (i = 0; i < 10; i++) {
		str[j++] = (intnum / Div) + 48;
		intnum = intnum % Div;
		Div /= 10;
		if ((str[j - 1] == '0') & (Status == 0)) {
			str[j] = '\0';
			j = 0;
		} else {
			str[j] = '\0';
			Status++;
		}
	}
}

void TIM2_IRQHandler() {
	if (TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET) {
		TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
		NEC_TimerRanOut();
	}
}

void EXTI0_IRQHandler() {
	if (EXTI_GetITStatus(IR_EXTI_LINE) != RESET) {
		NEC_HandleEXTI();
	}
	EXTI_ClearITPendingBit(IR_EXTI_LINE);
}

void NEC_ReceiveInterrupt(NEC_FRAME f) {
	char buf[12];
	uint8_t i;
	LCD_YX(1,0);
	for (i=0; i<17; i++){
		LCD_Str(" ");
	}

	LCD_YX(1, 1);
	Int2Str(buf, f.Address);
	LCD_Str(buf);
	LCD_YX(1, 8);
	Int2Str(buf, f.Command);
	LCD_Str(buf);
}*/
