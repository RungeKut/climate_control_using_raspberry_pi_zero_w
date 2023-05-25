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
#define maxByteLength 32 //Максимальная длина посылки в байтах
uint8_t byteLength = 6; //Длина посылки в байтах
uint8_t ptr = 0; //Указатель на член массива передаваемых байт
uint8_t bitMask = 1; //Бегущая маска
uint16_t pulseCounter = 0; //Счетчик импульсов
uint8_t isSendComplete = 0; //Флаг завершения отправки
uint8_t isSendPreamble = 0; //Флаг завершения отправки преамбулы
uint8_t mesPush[maxByteLength] = {0}; //Массив с передаваемым сообщением
uint8_t dataLength = 0; //Длина бита, изменяющаяся при передаче
uint8_t isSendSleep = 1; //Разрешение передачи
uint8_t repeatMessage = 0; //Количество повторений
uint8_t delayBetRepeat = 180; //Задержка между повторениями (Количество перезагрузок таймера) (38кГц)
uint8_t isDelayBetRepeat = 0; //Флаг активности задержки между повторами

void IrRemoteControlCallback(void)
{
  if ( isSendComplete ) //Если передача завершена, возвращаем начальные условия
  {
    ptr = 0; //Ставим указатель в начало сообщения
    bitMask = 1; //Ставим маску в начало
    pulseCounter = 0; //Обнуляем счетчик импульсов
    isSendComplete = 0; //Сбрасываем флаг завершения отправки
    isSendPreamble = 0; //Сбрасываем флаг завершения отправки преамбулы
  }
  else if ( isSendSleep ) //Выходим если передача запрещена
    return;
  else
  {
    if ( isDelayBetRepeat ) //Генерируем задержку
    {
      if ( pulseCounter == 0 ) //На всякий еще раз откл.чаем генерацию
      {
        OUT_OFF;
      }
      else if ( pulseCounter >= delayBetRepeat ) //Если задержкапрошла
      {
        isDelayBetRepeat = 0; //Задержка прошла
        isSendPreamble = 0; //После задержеи нужна преамбула
        pulseCounter = 0; //Обнуляем счетчик импульсов
        return;
      }
    }
    else if ( !isSendPreamble ) //Отправляем преамбулу
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
    else if ( isSendPreamble || !isDelayBetRepeat) //Отправляем данные
    {
      if ( pulseCounter == 0 ) //Включаем генерацию
      {
        uint8_t data = mesPush[ptr] & bitMask; //Смотрим значение текущего бита
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
          pulseCounter = 0; //Обнуляем счетчик импульсов
          return;
        }
        else
        {
          if ( repeatMessage == 0 ) //Если не нужно повторять
          {
            isSendComplete = 1; //Если сообщение закончилось, ставим флаг завершения передачи
            isSendSleep = 1; //Запрещаем дальнейшую отправку
          }
          else
          {
            isDelayBetRepeat = 1; //Разрешаем генерацию задержки между сообщениями
            ptr = 0; //Ставим указатель в начало сообщения
            bitMask = 1; //Ставим маску в начало
            pulseCounter = 0; //Обнуляем счетчик импульсов
            repeatMessage--; //Уменьшаем счетчик повторений
            return;
          }
        }
      }
    }
    pulseCounter++; //Считаем импульсы (Количество перезагрузок таймера) (38кГц)
  }
}

//Отправка сообщения
/*
  *   length - длина сообщения в байтах
  *   message[length] - само сообщение
  *   repeat - количество повторений
*/
uint8_t IrSendMessage(uint8_t length, uint8_t *message, uint8_t repeat)
{
  if ( length < maxByteLength )
    byteLength = length;
  else
    return 2; //Длина посылки превышает максимальную
  if ( isSendSleep != 0 )
  {
    repeatMessage = repeat;
    for(uint8_t i = 0; i < byteLength; i++) //Копируем сообщение
    {
      mesPush[i] = *message; //Копируем значение по указателю
      message++; //Перемещаем указатель на следующий элемент
    }
    isSendSleep = 0;
    return 0; //Успешное начало передачи
  }
  else
  {
    return 1; //Интерфейс занят передачей
  }
}

void WaitForSendMessage(void) //Дождаться отправки сообщения
{
  while ( isSendSleep == 0 ) ;
}
