#include "nec_decode.h"
#include "tim.h"
#include "stm32f1xx_hal_tim.h"
#include "bt_user_control.h"

#define timeStampBufferSize 300 //Размер буффера времени
#define tolerance 20 //Разброс времени в % от ширины импульса (не периода, т.к. период следования переменный) +-
#define byteLength 32 //Длинна посылки в байтах

#define high_Max 4*(100+tolerance) // 4 - это части периода. 1 часть импульс и 3 паузы такой же длинны (кодируется 1)
#define high_Min 4*(100-tolerance)
#define low_Max 2*(100+tolerance) // 2 - это части периода. 1 часть импульс и 1 пауза такой же длинны (кодируется 0)
#define low_Min 2*(100-tolerance)

volatile uint8_t isStartMessage = 0; //Начало новой посылки
volatile uint8_t pt = 0; //Член Массива времен
volatile uint8_t timElapsedCount = 1; //Количество кругов таймера
volatile uint32_t timeStampBuffer[timeStampBufferSize] = {0}; //Массив с измеренными временами. Четное - период, Нечетное - длина импульса
volatile uint8_t timeStampBuffer_IsFull = 0; //Буфер времени заполнен
volatile uint8_t timeStampBuffer_IsLock = 0; //Буфер времени заблокирован для декодирования
uint8_t cmdData[byteLength] = {0};

uint8_t* IrGetMessage(void)
{
  return cmdData;
}

void NEC_Init(void)
{
  HAL_TIM_Base_Start_IT(&htim2); // запуск таймера
  NVIC_EnableIRQ(TIM2_IRQn); // разрешаем прерывания
  HAL_TIM_IC_Start_IT(&htim2, TIM_CHANNEL_1); // запускаем канал в режиме захвата
  HAL_TIM_IC_Start_IT(&htim2, TIM_CHANNEL_2); // запускаем канал в режиме захвата
}

uint8_t NEC_TimingDecode(uint32_t duration, uint32_t period)
{
	uint32_t data = (uint32_t)(period * 100 / duration);
	if ((data < high_Max) && (data > high_Min))
    return 1;
	else if ((data < low_Max) && (data > low_Min))
    return 0;
	else
    return 2; // Если нет удовлетворительного  результата - возвращаем код ошибки.
}

void cmdData_Clear (void)
{
  for (uint8_t i = 0; i < byteLength; i++)
	{
    cmdData[i] = 0;
  }
}

void NEC_Task (void)
{
  uint8_t bitCount = 0;
  uint8_t byteCount = 0;
	if (timeStampBuffer_IsLock)
	{
    cmdData_Clear();
		uint8_t data = 0;
		for (uint8_t cell = 1; cell < timeStampBufferSize/2-1; cell++)
		{
			data = NEC_TimingDecode(timeStampBuffer[2*cell], timeStampBuffer[2*cell+1]);
			if (data > 1)
      {
        timeStampBuffer_IsLock = 0;
        timeStampBuffer_IsFull = 0;
        bt_IrGetMessageAfterRecive();
        return; // Если ошибка, то выходим без продолжения кодирования
      }
      repeat:
			if (bitCount < 8)
      {
        cmdData[byteCount] |= (data << bitCount);
        bitCount++;
      }
      else
      {
        bitCount = 0;
        byteCount++;
        if (byteCount >= byteLength) return;
        goto repeat;
      }
		}
    timeStampBuffer_IsLock = 0;
    timeStampBuffer_IsFull = 0;
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

void necDecodeCallback(void)
{
  //считать только если прошло меньше 1сек
  if (!isStartMessage)
  {
    if (timElapsedCount <= 2) timElapsedCount++; //15 переполнений это примерно 1 секунда
    else
    {
      isStartMessage = 1; //Больше секунды - значит новая посылка
      timeStampBuffer_IsLock = 1; //Блокируем чтобы провести декодирование
      pt = 0;
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
