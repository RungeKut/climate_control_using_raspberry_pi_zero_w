#include "ir_NEC_remote_control.h"

#define OUT_ON TIM3->CCR1=0x0297
#define OUT_OFF TIM3->CCR1=0x0000

void IrRemoteControInit(void)
{
  HAL_TIM_Base_Start_IT(&htim3); // запуск таймера генератора
  NVIC_EnableIRQ(TIM3_IRQn); // разрешаем его прерывания
  HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1); // включаем первый канал таймера
  OUT_OFF; // зададим начальную скважность ШиМ 35%
}

#define PREAMBLE 167 //Длина преамбулы для настройки приемника кондиционера
#define HALFBIT 21 //Длина импульса бита посылки
#define TRUEBIT 4 //Длина единицы, 1 импульс + 3 паузы
#define FALSEBIT 2 //Длина нуля, 1 импульс + 1 пауза
#define byteLength 6 //Длинна посылки в байтах

uint8_t ptr = 0; //Указатель на член массива передаваемых байт
uint8_t bitMask = 1; //Бегущая маска
uint16_t pulseCounter = 0; //Счетчик импульсов
uint8_t isSendComplete = 0; //Флаг завершения отправки
uint8_t isSendPreamble = 0; //Флаг завершения отправки преамбулы
uint8_t cmdPush[byteLength] = {0x4D,0xB2,0xF9,0x06,0x0B,0xF4};
uint8_t dataLength = 0;

void IrRemoteControlCallback(void)
{
  if ( isSendComplete )
  {
    ptr = 0;
    bitMask = 1;
    pulseCounter = 0;
    isSendComplete = 0;
    isSendPreamble = 0;
  }
  else
  {
    if ( !isSendPreamble ) //Отправляем преамбулу
    {
      if ( pulseCounter == 0 ) OUT_ON;
      if ( pulseCounter == PREAMBLE ) OUT_OFF;
      if ( pulseCounter >= PREAMBLE*2 )
      {
        isSendPreamble = 1;
        pulseCounter = 0;
        return;
      }
    }
    else //Отправляем данные
    {
      if ( cmdPush[ptr] &&  bitMask == 0)
      {
        dataLength = FALSEBIT;
      }
      else
      {
        dataLength = TRUEBIT;
      }
      if ( pulseCounter == 0 )
        OUT_ON;
      if ( pulseCounter == HALFBIT )
        OUT_OFF;
      if ( pulseCounter >= HALFBIT*dataLength )
      {
        if ( ptr < byteLength )
        {
          if ( bitMask != 0x80 )
            bitMask = bitMask << 1;
          else
          {
            bitMask = 1;
            ptr++;
          }
        }
        else
        {
          isSendComplete = 1;
        }
      }
    }
    pulseCounter++;
  }
}
