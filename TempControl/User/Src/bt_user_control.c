#include "bt_user_control.h"
#include "bt_usart.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "main.h"
#include "ir_NEC_remote_control.h"
#include "nec_decode.h"

#define TIMEOUT_CMD 50  // 50ms timeout
#define maxByteLength 32 //Максимальная длина посылки в байтах

ring_command_buffer cmd_buffer = { { 0 }, 0, 0};

ring_command_buffer *_cmd_buffer;

uint16_t speedToReg[101] = {0,655,1311,1966,2621,3277,3932,4587,5243,5898,6554,7209,7864,8520,9175,9830,10486,11141,11796,12452,13107,13762,14418,15073,15728,16384,17039,17694,18350,19005,19661,20316,20971,21627,22282,22937,23593,24248,24903,25559,26214,26869,27525,28180,28835,29491,30146,30801,31457,32112,32768,33423,34078,34734,35389,36044,36700,37355,38010,38666,39321,39976,40632,41287,41942,42598,43253,43908,44564,45219,45874,46530,47185,47841,48496,49151,49807,50462,51117,51773,52428,53083,53739,54394,55049,55705,56360,57015,57671,58326,58981,59637,60292,60948,61603,62258,62914,63569,64224,64880,65535};
uint8_t ventSpeed = 50;
uint8_t message[maxByteLength] = {0};
  
void CommandBuf_init(void)
{
  _cmd_buffer = &cmd_buffer;
}

void CmdBufferUpdate(void)
{
  if (!IsDataAvailable()) { return; }
  HAL_GPIO_WritePin(GREEN_LED_GPIO_Port, GREEN_LED_Pin, GPIO_PIN_RESET);
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

void bt_user_control(void)
{
  CmdBufferUpdate();
  
  if(_cmd_buffer->head == _cmd_buffer->tail) return;
  if (!strcmp(_cmd_buffer->buffer[_cmd_buffer->tail], "VentSpeed?"))
  {
    char str [3];
    snprintf(str, 3, "%d", ventSpeed);
    Uart_sendstring(str);
  }
  else if (!strncmp(_cmd_buffer->buffer[_cmd_buffer->tail], "VentSpeed=", 10))
  {
    char *pEnd;
    pEnd = &_cmd_buffer->buffer[_cmd_buffer->tail][10];
    int data = strtol(pEnd, NULL, 10);
    
    if (data > 100)
    {
      Uart_sendstring("VentSpeed= 0 - 100 !\r\n");
    }
    else
    {
      ventSpeed = data;
      TIM4->CCR1=speedToReg[ventSpeed];
    }
  }
  else if (!strncmp(_cmd_buffer->buffer[_cmd_buffer->tail], "IrSendMessage", 13))
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
  else if (!strncmp(_cmd_buffer->buffer[_cmd_buffer->tail], "IrGetMessage?", 13))
  {
    char buf[128] = {0}, *pos = buf;
    uint8_t *posDat = IrGetMessage();
    uint8_t len = 0;
    for (int i = 0 ; i < maxByteLength ; i++)
    {
      uint8_t dat = *posDat;
      if ( dat != 0 ) len = i; //Ищем длину сообщения в байтах
      posDat++; //Берем следующий байт
    }
    char finishBuf[256] = {0}, *finishBufPos = finishBuf;
    finishBufPos += sprintf(finishBufPos, "length=");
    finishBufPos += sprintf(finishBufPos, "%d", len);
    finishBufPos += sprintf(finishBufPos, " message:");
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
      finishBufPos += sprintf(pos, " Buffer overflow!");
    finishBufPos += sprintf(pos, "\r\n");
    Uart_sendstring(finishBuf);
  }
  else if (!strcmp(_cmd_buffer->buffer[_cmd_buffer->tail], "help"))
  {
    Uart_sendstring
    (
      "\r\n"
      "*** Welcome to Vent control! ***\r\n"
      "\r\n"
      "VentSpeed? - motor speed, %\r\n"
      "VentSpeed=58 - set motor speed, %\r\n"
      "\r\n"
      "IrSendMessage <lenhth> <message> <repeat>\r\n"
      "Ex.send: IrSendMessage 6 4D B2 F8 07 10 EF 2\r\n"
      "\r\n"
      "IrGetMessage?\r\n"
      "Ex.get: length=6 message:4D B2 F8 07 10 EF\r\n"
      "\r\n"
    );
  }
  else
  {
    Uart_sendstring("Please enter \"help\"\r\n");
  }
  _cmd_buffer->tail = (unsigned int)(_cmd_buffer->tail + 1) % COMMAND_BUFFER_SIZE;
  HAL_Delay (TIMEOUT_CMD);
  HAL_GPIO_WritePin(GREEN_LED_GPIO_Port, GREEN_LED_Pin, GPIO_PIN_SET);
}
