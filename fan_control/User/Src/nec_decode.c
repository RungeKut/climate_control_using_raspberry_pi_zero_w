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
#define tolerance 15 //Разброс времени в % от ширины импульса (не периода, т.к. период следования переменный) +-

uint8_t isStartMessage = 0; //Начало новой посылки
uint8_t pt = 0; //Член Массива времен
volatile uint8_t timElapsedCount = 1; //Количество кругов таймера
uint32_t timeStampBuffer[timeStampBufferSize] = {0}; //Массив с измеренными временами. Четное - период, Нечетное - длина импульса
uint8_t timeStampBuffer_IsFull = 0; //Буфер времени заполнен
uint8_t timeStampBuffer_IsLock = 0; //Буфер времени заблокирован для декодирования

void NEC_Init(void)
{
  HAL_TIM_Base_Start_IT(&htim2); // запуск таймера
  NVIC_EnableIRQ(TIM2_IRQn); // разрешаем прерывания
  HAL_TIM_IC_Start_IT(&htim2, TIM_CHANNEL_1); // запускаем канал в режиме захвата
  HAL_TIM_IC_Start_IT(&htim2, TIM_CHANNEL_2); // запускаем канал в режиме захвата
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

uint8_t NEC_TimingDecode(uint32_t duration, uint32_t period)
{
	uint32_t data = period * 100 / duration;
	if ((data > (4 * (100 + tolerance))) && (data < (4 * (100 - tolerance))) return 1;
	else if ((data > (2 * (100 + tolerance))) && (data < (2 * (100 - tolerance))) return 0;
	else return 2;
}

void NEC_Task (void)
{
	if (timeStampBuffer_IsLock)
	{
		uint8_t data = 0;
		for (uint8_t cell = 0; cell < timeStampBufferSize/2-1; cell++;)
		{
			data = NEC_TimingDecode(timeStampBuffer[2*cell], timeStampBuffer[2*cell+1]);
			if (data > 1) return;
		}
	}
}

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
	timeStampBuffer_IsLock = 1; //Блокируем чтобы провести декодирование
        pt = 0;
      }
    }
  }
}
/*
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
}*/
