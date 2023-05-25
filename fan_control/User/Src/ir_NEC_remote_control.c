#include "ir_NEC_remote_control.h"

#define OUT_ON TIM3->CCR1=0x0297 //Зададим скважность ШиМ 35%
#define OUT_OFF TIM3->CCR1=0x0000 //А тут скважность 0%

void IrRemoteControInit(void)
{
  HAL_TIM_Base_Start_IT(&htim3); // запуск таймера генератора
  NVIC_EnableIRQ(TIM3_IRQn); // разрешаем его прерывания
  HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1); // включаем первый канал таймера
  OUT_OFF; // Выключим выход на всякий случай
}

#define PREAMBLE 167 //Длина преамбулы для настройки приемника кондиционера
#define HALFBIT 21 //Длина импульса бита посылки
#define TRUEBIT 4 //Длина единицы, 1 импульс + 3 паузы
#define FALSEBIT 2 //Длина нуля, 1 импульс + 1 пауза
#define byteLength 6 //Длина посылки в байтах

uint8_t ptr = 0; //Указатель на член массива передаваемых байт
uint8_t bitMask = 1; //Бегущая маска
uint16_t pulseCounter = 0; //Счетчик импульсов
uint8_t isSendComplete = 0; //Флаг завершения отправки
uint8_t isSendPreamble = 0; //Флаг завершения отправки преамбулы
uint8_t mesPush[byteLength] = {0x4D,0xB2,0xF9,0x06,0x0B,0xF4};
uint8_t dataLength = 0; //Длина бита, изменяющаяся при передаче
uint8_t isSendSleep = 1; //Разрешение передачи

void IrRemoteControlCallback(void)
{
  if ( isSendComplete ) //Если передача завершена, возвращаем начальные условия
  {
    ptr = 0;
    bitMask = 1;
    pulseCounter = 0;
    isSendComplete = 0;
    isSendPreamble = 0;
  }
  else if ( isSendSleep ) //Выходим если передача запрещена
    return;
  else
  {
    if ( !isSendPreamble ) //Отправляем преамбулу
    {
      if ( pulseCounter == 0 ) //Включаем генерацию
      {
        OUT_ON;
      }
      else if ( pulseCounter == PREAMBLE ) //На половине длинны отключаем
      {
        OUT_OFF;
      }
      else if ( pulseCounter >= PREAMBLE*2 ) //После передачи полной длинны преамбулы завершаем ее передачу
      {
        isSendPreamble = 1; //Преамбула передана
        pulseCounter = 0; //Обнуляем счетчик импульсов
        return;
      }
    }
    else //Отправляем данные
    {
      if ( pulseCounter == 0 ) //Включаем генерацию
      {
        uint8_t data = mesPush[ptr] && bitMask; //Смотрим значение текущего бита
        if ( data == 0 ) //Выбираем длину
        {
          dataLength = FALSEBIT;
        }
        else
        {
          dataLength = TRUEBIT;
        }
        OUT_ON; //Включаем генерацию
      }
      else if ( pulseCounter == HALFBIT ) //На половине длинны отключаем
      {
        OUT_OFF;
      }
      else if ( pulseCounter >= HALFBIT*dataLength ) //После передачи полной выбранной длинны бита завершаем его передачу
      {
        if ( ptr < byteLength ) //Проверяем не закончилось ли сообщение
        {
          if ( bitMask != 0x80 ) //Проверяем не передан ли весь байт целиком
          {
            bitMask = bitMask << 1; //Если еще нет, то двигаемся к следующему биту
          }
          else
          {
            bitMask = 1; //Если байт передан, ставим маску в начало
            ptr++; //И выбираем следующий байт сообщения
          }
        }
        else
        {
          isSendComplete = 1; //Если сообщение закончилось, ставим флаг завершения передачи
          isSendSleep = 1; //Запрещаем дальнейшую отправку
        }
      }
    }
    pulseCounter++; //Считаем импульсы (Количество перезагрузок таймера) (38кГц)
  }
}

void IrSendMessage(uint8_t message[byteLength])
{
  
  isSendSleep = 0;
}
