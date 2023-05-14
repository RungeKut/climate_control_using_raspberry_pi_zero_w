#include "nec_decode.h"
#include "tim.h"
#include "stm32f1xx_hal_tim.h"
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
#define timeStampBufferSize 300 //Размер буффера времени
#define tolerance 150 //Разброс времени +-
uint8_t isStartMessage = 0; //Начало новой посылки
uint8_t pt = 0; //Член Массива времен
volatile uint8_t timElapsedCount = 1; //Количество кругов таймера
uint32_t timeStampBuffer[timeStampBufferSize] = {0}; //Массив с измеренными временами. Четное - период, Нечетное - длина импульса
uint8_t timeStampBuffer_IsFull = 0; //Буфер времени заполнен

void NEC_Init(void)
{
  HAL_TIM_Base_Start_IT(&htim2); // запуск таймера
  NVIC_EnableIRQ(TIM2_IRQn); // разрешаем прерывания
  HAL_TIM_IC_Start_IT(&htim2, TIM_CHANNEL_1); // запускаем канал в режиме захвата
  HAL_TIM_IC_Start_IT(&htim2, TIM_CHANNEL_2); // запускаем канал в режиме захвата
}
/*

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
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
  if(htim->Instance == TIM2)
  {
    if(htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1) // RISING с LOW на HIGH
    {
      if (!timeStampBuffer_IsFull)
        timeStampBuffer[2*pt] = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_1) * timElapsedCount;
    }
    else if(htim->Channel == HAL_TIM_ACTIVE_CHANNEL_2) // FALLING с HIGH на LOW
    {
      if (isStartMessage)
      {
        TIM2->CNT = 0x0000; // обнуление счётчика
        timElapsedCount = 1;
        isStartMessage = 0;
      }
      else
      {
        if (!timeStampBuffer_IsFull)
          timeStampBuffer[2*pt + 1] = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_2) * timElapsedCount;
        if (pt > timeStampBufferSize/2-1) timeStampBuffer_IsFull = 1; else pt++;
        TIM2->CNT = 0x0000; // обнуление счётчика
        timElapsedCount = 1;
      }
    }
  }
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  if(htim->Instance == TIM2)
  {
    //считать только если прошло меньше 1сек
    if (!isStartMessage)
    {
      if (timElapsedCount <= 15) timElapsedCount++; //15 переполнений это примерно 1 секунда
      else
      {
        isStartMessage = 1; //Больше секунды - значит новая посылка
        pt = 0;
      }
    }
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
