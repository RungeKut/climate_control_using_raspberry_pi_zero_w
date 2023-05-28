#include "nec_decode.h"
#include "tim.h"
#include "stm32f1xx_hal_tim.h"

#define timeStampBufferSize 300 //Размер буффера времени
#define tolerance 20 //Разброс времени в % от ширины импульса (не периода, т.к. период следования переменный) +-
#define byteLength 32 //Длинна посылки в байтах

#define high_Max 4*(100+tolerance) // 4 - это части периода. 1 часть импульс и 3 паузы такой же длинны (кодируется 1)
#define high_Min 4*(100-tolerance)
#define low_Max 2*(100+tolerance) // 2 - это части периода. 1 часть импульс и 1 пауза такой же длинны (кодируется 0)
#define low_Min 2*(100-tolerance)

const uint8_t MDV_on_17_low_auto[6] = {0x4D,0xB2,0xF8,0x07,0x10,0xEF};
const uint8_t MDV_on_17_low_fan[6] = {0x4D,0xB2,0xF9,0x06,0x27,0xD8};
const uint8_t MDV_on_17_low_heat[6] = {0x4D,0xB2,0xF9,0x06,0x30,0xCF};
const uint8_t MDV_on_17_low_dry[6] = {0x4D,0xB2,0xF8,0x07,0x20,0xDF};
const uint8_t MDV_on_17_auto_cool[6] = {0x4D,0xB2,0xFD,0x02,0x00,0xFF};
const uint8_t MDV_on_17_high_cool[6] = {0x4D,0xB2,0xFC,0x03,0x00,0xFF};
const uint8_t MDV_on_17_med_cool[6] = {0x4D,0xB2,0xFA,0x05,0x00,0xFF};
const uint8_t MDV_on_17_low_cool[6] = {0x4D,0xB2,0xF9,0x06,0x00,0xFF};
const uint8_t MDV_on_18_low_cool[6] = {0x4D,0xB2,0xF9,0x06,0x08,0xF7};
const uint8_t MDV_on_19_low_cool[6] = {0x4D,0xB2,0xF9,0x06,0x0C,0xF3};
const uint8_t MDV_on_20_low_cool[6] = {0x4D,0xB2,0xF9,0x06,0x04,0xFB};
const uint8_t MDV_on_21_low_cool[6] = {0x4D,0xB2,0xF9,0x06,0x06,0xF9};
const uint8_t MDV_on_22_low_cool[6] = {0x4D,0xB2,0xF9,0x06,0x0E,0xF1};
const uint8_t MDV_on_23_low_cool[6] = {0x4D,0xB2,0xF9,0x06,0x0A,0xF5};
const uint8_t MDV_on_24_low_cool[6] = {0x4D,0xB2,0xF9,0x06,0x02,0xFD};
const uint8_t MDV_on_25_low_cool[6] = {0x4D,0xB2,0xF9,0x06,0x03,0xFC};
const uint8_t MDV_on_26_low_cool[6] = {0x4D,0xB2,0xF9,0x06,0x0B,0xF4};
const uint8_t MDV_on_27_low_cool[6] = {0x4D,0xB2,0xF9,0x06,0x09,0xF6};
const uint8_t MDV_on_28_low_cool[6] = {0x4D,0xB2,0xF9,0x06,0x01,0xFE};
const uint8_t MDV_on_29_low_cool[6] = {0x4D,0xB2,0xF9,0x06,0x05,0xFA};
const uint8_t MDV_on_30_low_cool[6] = {0x4D,0xB2,0xF9,0x06,0x0D,0xF2};
const uint8_t MDV_off[6] = {0x4D,0xB2,0xDE,0x21,0x07,0xF8};
const uint8_t MDV_sleepOn[6] = {0x4D,0xB2,0x07,0xF8,0xC0,0x3F};
const uint8_t MDV_sleepOff[6] = {0x4D,0xB2,0xFD,0x02,0x0B,0xF4};
const uint8_t MDV_swing[6] = {0x4D,0xB2,0xD6,0x29,0x07,0xF8};
const uint8_t MDV_direct[6] = {0x4D,0xB2,0xF0,0x0F,0x07,0xF8};
const uint8_t MDV_shortCut[6] = {0x4D,0xB2,0xF8,0x07,0x1B,0xE4};
const uint8_t MDV_clean[6] = {0xAD,0x52,0xAF,0x50,0x55,0xAA};
const uint8_t MDV_ledDisplay[6] = {0xAD,0x52,0xAF,0x50,0xA5,0x5A};
const uint8_t MDV_turbo[6] = {0xAD,0x52,0xAF,0x50,0x45,0xBA};

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
