#include "bt_user_control.h"
#include "bt_usart.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "main.h"
#include "ir_NEC_remote_control.h"
#include "nec_decode.h"
#include "aht21.h"
#include "scd30.h"

#define TIMEOUT_CMD 50  // 50ms timeout
#define maxByteLength 32 //Максимальная длина посылки в байтах

ring_command_buffer cmd_buffer = { { 0 }, 0, 0};

ring_command_buffer *_cmd_buffer;

uint8_t message[maxByteLength] = {0};

void CommandBuf_init(void)
{
  _cmd_buffer = &cmd_buffer;
}

void CmdBufferUpdate(void)
{
  if (!IsDataAvailable()) { return; }
  //HAL_GPIO_WritePin(GREEN_LED_GPIO_Port, GREEN_LED_Pin, GPIO_PIN_RESET);
  unsigned int i = (unsigned int)(_cmd_buffer->head + 1) % COMMAND_BUFFER_SIZE;
	if (i == _cmd_buffer->tail) return;
  
  char string[STRING_SIZE] = "";
  int data = 0;
  repeatData:
  while ( data != -1 )
  {
    data = Uart_read();
    if ((strlen(string) + 2 <= STRING_SIZE) && (data > 0))
    {
      char temp = (char)data;
      strncat(string, &temp, 1);
    }
  }
  HAL_Delay (TIMEOUT_CMD);
  if (IsDataAvailable())
  {
    data = 0;
    goto repeatData;
  }
  else
  {
    for (int p=0; p<STRING_SIZE; p++)
    {
      _cmd_buffer->buffer[_cmd_buffer->head][p] = string[p];
    }
  }
  _cmd_buffer->head = i;
}

/* Пользовательские функции */

void bt_IrSendMessage(void) //Отправка данных через инфракрасный диод
{
  char *pEnd;
  pEnd = &_cmd_buffer->buffer[_cmd_buffer->tail][14];
  int length = strtol(pEnd, NULL, 10);
  if ((length > maxByteLength) || (length < 1))
  {
    Uart_sendstring("Message length 1 - 32 byte!\r\n");
  }
  if ((length > 0) || (length < 10)) //Двигаем указатель в зависимости от разрядности числа
    pEnd = &_cmd_buffer->buffer[_cmd_buffer->tail][16];
  else if ((length >= 10) || (length <= maxByteLength))
    pEnd = &_cmd_buffer->buffer[_cmd_buffer->tail][17];

  for(uint8_t i = 0; i < length; i++) //Читаем само сообщение
  {
   message[i] = strtol(pEnd, &pEnd, 16);
  }
 pEnd++;
 int repeat = strtol(pEnd, NULL, 10);
 IrSendMessage(length, message, repeat);
}

void bt_IrGetMessage(void) //Вывод принятых через инфракрасный приемник данных
{
  uint8_t *posDat = IrGetMessage();
  uint8_t len = 0;
  for (int i = 1 ; i < maxByteLength ; i++)
  {
    uint8_t dat = *posDat;
    if ( dat != 0 ) len = i; //Ищем длину сообщения в байтах
    posDat++; //Берем следующий байт
  }
  if ( len == 0 ) return;
  char finishBuf[256] = {0}, *finishBufPos = finishBuf;
  finishBufPos += sprintf(finishBufPos, "IrMessage length=");
  finishBufPos += sprintf(finishBufPos, "%d", len);
  finishBufPos += sprintf(finishBufPos, " message: ");
  posDat = IrGetMessage();
  for (int i = 0 ; i < len ; i++)
  {
    uint8_t dat = *posDat;
    if (i)
    {
      finishBufPos += sprintf(finishBufPos, " "); //Добавляем пробел между байтами
    }
    finishBufPos += sprintf(finishBufPos, "%02x", dat); //Преобразуем число в строку в 16-тиричной форме
    posDat++; //Берем следующий байт
  }
  if ( len >= maxByteLength - 1 )
    finishBufPos += sprintf(finishBufPos, " Buffer overflow!");
  finishBufPos += sprintf(finishBufPos, "\r\n");
  Uart_sendstring(finishBuf);
}

void bt_GetClimate(void)
{
  //float *ClimData = AHT_GetData();
	float *ClimData = SCD30_GetData();
  char buf[30] = {0}, *bufPos = buf;
  bufPos += sprintf(bufPos, "Climate:");
  bufPos += sprintf(bufPos, "%.1f ", ClimData[0]);
  bufPos += sprintf(bufPos, "%.1f ", ClimData[1]);
	bufPos += sprintf(bufPos, "%.1f", ClimData[2]);
	bufPos += sprintf(bufPos, "\r\n");
  Uart_sendstring(buf);
}

void bt_IrGetMessageAfterRecive(void)
{
  bt_IrGetMessage();
}

void bt_SendHelp(void) //Вывод информации для пользователя
{
  Uart_sendstring
    (
      "\r\n"
      "*** Welcome to Temp control in kids room! ***\r\n"
      "\r\n"
      "Climate? - Get Temperature °C & Humidity % & CO2 ppm\r\n"
      "Ex.send: Climate:24.5 36.8 658.9\r\n"
      "\r\n"
      "IrSendMessage <lenhth> <message> <repeat>\r\n"
      "Ex.send: IrSendMessage 6 4D B2 F8 07 10 EF 2\r\n"
      "\r\n"
      "IrGetMessage?\r\n"
      "Ex.get: length=6 message:4D B2 F8 07 10 EF\r\n"
      "\r\n"
    );
}


/***** Исполнитель *****/

void BT_UI_Executer(void)
{
  CmdBufferUpdate();
  
  if(_cmd_buffer->head == _cmd_buffer->tail) return;
  
  if (!strncmp(_cmd_buffer->buffer[_cmd_buffer->tail], "IrSendMessage", 13)) bt_IrSendMessage();
  
  else if (!strncmp(_cmd_buffer->buffer[_cmd_buffer->tail], "IrGetMessage?", 13)) bt_IrGetMessage();
  
  else if (!strncmp(_cmd_buffer->buffer[_cmd_buffer->tail], "Climate?", 8)) bt_GetClimate();
  
  else if (!strncmp(_cmd_buffer->buffer[_cmd_buffer->tail], "help", 4)) bt_SendHelp();
  
  else { Uart_sendstring("Please enter \"help\"\r\n"); }
  
  _cmd_buffer->tail = (unsigned int)(_cmd_buffer->tail + 1) % COMMAND_BUFFER_SIZE;
  HAL_Delay (TIMEOUT_CMD);
  //HAL_GPIO_WritePin(GREEN_LED_GPIO_Port, GREEN_LED_Pin, GPIO_PIN_SET);
}
