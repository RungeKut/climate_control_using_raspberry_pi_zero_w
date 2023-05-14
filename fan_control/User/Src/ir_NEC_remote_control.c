#include "ir_NEC_remote_control.h"
#include "usart.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define TIMEOUT_DEF 500  // 500ms timeout
uint16_t IrDA_timeout;

IrDA_ring_buffer IrDA_rx_buffer = { { 0 }, 0, 0};
IrDA_ring_buffer IrDA_tx_buffer = { { 0 }, 0, 0};

IrDA_ring_buffer *_irda_rx_buffer;
IrDA_ring_buffer *_irda_tx_buffer;

void IrDA_Ringbuf_init(void)
{
  _irda_rx_buffer = &IrDA_rx_buffer;
  _irda_tx_buffer = &IrDA_tx_buffer;

  //Разрешаем прерывание при ошибке UART: (ошибка кадра, ошибка шума, ошибка переполнения)
  __HAL_UART_ENABLE_IT(irda, UART_IT_ERR);

  //Разрешаем прерывание, о появлении новых данных в регистре UART
  __HAL_UART_ENABLE_IT(irda, UART_IT_RXNE);
}

void IrDA_store_char(unsigned char c, IrDA_ring_buffer *buffer)
{
  int i = (unsigned int)(buffer->head + 1) % IrDA_BUFFER_SIZE;

  //Если нужно сохранять полученный символ в местоположении непосредственно перед хвостом
  //(это означает, что голова переместится в текущее местоположение хвоста),
  //чтобы не переполнить буфер, мы не записываем символ и не продвигаем заголовок.
  if(i != buffer->tail) {
    buffer->buffer[buffer->head] = c;
    buffer->head = i;
  }
}

// Проверяет есть ли указанная строка в указанном буфере
static int IrDA_check_for (char *str, char *buffertolookinto)
{
	int stringlength = strlen (str);
	int bufferlength = strlen (buffertolookinto);
	int so_far = 0;
	int indx = 0;
repeat:
	while (str[so_far] != buffertolookinto[indx])
		{
			indx++;
			if (indx>stringlength) return 0;
		}
	if (str[so_far] == buffertolookinto[indx])
	{
		while (str[so_far] == buffertolookinto[indx])
		{
			so_far++;
			indx++;
		}
	}

	if (so_far == stringlength);
	else
	{
		so_far =0;
		if (indx >= bufferlength) return -1;
		goto repeat;
	}

	if (so_far == stringlength) return 1;
	else return -1;
}

int IrDA_read(void)
{
  // Если голова не находится впереди хвоста - то буфер пуст
  if(_irda_rx_buffer->head == _irda_rx_buffer->tail)
  {
    return -1;
  }
  else
  {
    unsigned char c = _irda_rx_buffer->buffer[_irda_rx_buffer->tail];
    _irda_rx_buffer->tail = (unsigned int)(_irda_rx_buffer->tail + 1) % IrDA_BUFFER_SIZE;
    return c;
  }
}

// Записывает один символ в uart и увеличивает head
void IrDA_write(int c)
{
	if (c>=0)
	{
		int i = (_irda_tx_buffer->head + 1) % IrDA_BUFFER_SIZE;
		while (i == _irda_tx_buffer->tail);

		_irda_tx_buffer->buffer[_irda_tx_buffer->head] = (uint8_t)c;
		_irda_tx_buffer->head = i;

		__HAL_UART_ENABLE_IT(irda, UART_IT_TXE); // Enable UART transmission interrupt
	}
}

//Проверяет, доступны ли новые данные во входящем буфере
int IrDA_IsDataAvailable(void)
{
  return (uint16_t)(IrDA_BUFFER_SIZE + _irda_rx_buffer->head - _irda_rx_buffer->tail) % IrDA_BUFFER_SIZE;
}

//Отправляет строку в uart
void IrDA_sendstring (const char *s)
{
	while(*s) IrDA_write(*s++);
}

//Получить данные из буфера
void IrDA_GetDataFromBuffer (char *startString, char *endString, char *buffertocopyfrom, char *buffertocopyinto)
{
	int startStringLength = strlen (startString);
	int endStringLength   = strlen (endString);
	int so_far = 0;
	int indx = 0;
	int startposition = 0;
	int endposition = 0;

repeat1:
	while (startString[so_far] != buffertocopyfrom[indx]) indx++;
	if (startString[so_far] == buffertocopyfrom[indx])
	{
		while (startString[so_far] == buffertocopyfrom[indx])
		{
			so_far++;
			indx++;
		}
	}

	if (so_far == startStringLength) startposition = indx;
	else
	{
		so_far =0;
		goto repeat1;
	}

	so_far = 0;

repeat2:
	while (endString[so_far] != buffertocopyfrom[indx]) indx++;
	if (endString[so_far] == buffertocopyfrom[indx])
	{
		while (endString[so_far] == buffertocopyfrom[indx])
		{
			so_far++;
			indx++;
		}
	}

	if (so_far == endStringLength) endposition = indx-endStringLength;
	else
	{
		so_far =0;
		goto repeat2;
	}

	so_far = 0;
	indx=0;

	for (int i=startposition; i<endposition; i++)
	{
		buffertocopyinto[indx] = buffertocopyfrom[i];
		indx++;
	}
}

//Очистка буфера
void IrDA_flush (void)
{
	memset(_irda_rx_buffer->buffer,'\0', IrDA_BUFFER_SIZE);
	_irda_rx_buffer->head = 0;
	_irda_rx_buffer->tail = 0;
}

//Просмотр последнего принятого символа если он есть.
int IrDA_peek()
{
  if(_irda_rx_buffer->head == _irda_rx_buffer->tail)
  {
    return -1;
  }
  else
  {
    return _irda_rx_buffer->buffer[_irda_rx_buffer->tail];
  }
}

//Копирует данные из входящего буфера в наш буфер
//Необходимо использовать, если вы уверены, что данные получены
//Он будет скопирован независимо от того, есть ли конечная строка там или нет
//если конечная строка копируется, то возвращает 1 или иначе 0
//Используйте либо после (IsDataAvailable), либо после (Wait_for) функций
int IrDA_Copy_upto (char *string, char *buffertocopyinto)
{
	int so_far =0;
	int len = strlen (string);
	int indx = 0;

again:
	while (IrDA_peek() != string[so_far])
		{
			buffertocopyinto[indx] = _irda_rx_buffer->buffer[_irda_rx_buffer->tail];
			_irda_rx_buffer->tail = (unsigned int)(_irda_rx_buffer->tail + 1) % IrDA_BUFFER_SIZE;
			indx++;
			while (!IrDA_IsDataAvailable());

		}
	while (IrDA_peek() == string [so_far])
	{
		so_far++;
		buffertocopyinto[indx++] = IrDA_read();
		if (so_far == len) return 1;
		IrDA_timeout = TIMEOUT_DEF;
		while ((!IrDA_IsDataAvailable())&&IrDA_timeout);
		if (IrDA_timeout == 0) return 0;
	}

	if (so_far != len)
	{
		so_far = 0;
		goto again;
	}

	if (so_far == len) return 1;
	else return 0;
}

//Должен использоваться после того, как функция wait_for получит введенное количество символов после введенной строки
int IrDA_Get_after (char *string, uint8_t numberofchars, char *buffertosave)
{
	for (int indx=0; indx<numberofchars; indx++)
	{
		IrDA_timeout = TIMEOUT_DEF;
		while ((!IrDA_IsDataAvailable())&&IrDA_timeout);  // ждем пока данные станут доступны
		if (IrDA_timeout == 0) return 0;  //Если данные недоступны в течение определенного времени, то вернуть 0
		buffertosave[indx] = IrDA_read();  //Сохраним данные в буфере... увеличим длину хвоста
	}
	return 1;
}

//Ожидает поступления определенной строки во входящий буфер... Он также увеличивает значение tail, возвращаемое на 1, если строка обнаружена
//Добавлена функция тайм-аута, чтобы функция не блокировала обработку других функций
int IrDA_Wait_for (char *string)
{
	int so_far =0;
	int len = strlen (string);

again:
	IrDA_timeout = TIMEOUT_DEF;
	while ((!IrDA_IsDataAvailable())&&IrDA_timeout);  // подождем, пока появятся данные
	if (IrDA_timeout == 0) return 0;
	while (IrDA_peek() != string[so_far])  // заглянем в rx_buffer, чтобы посмотреть, получили ли мы строку
	{
		if (_irda_rx_buffer->tail != _irda_rx_buffer->head)
		{
			_irda_rx_buffer->tail = (unsigned int)(_irda_rx_buffer->tail + 1) % IrDA_BUFFER_SIZE;  // increment the tail
		}

		else
		{
			return 0;
		}
	}
	while (IrDA_peek() == string [so_far]) // если мы получили первую букву строки
	{
		// тогда будем искать и другие буквы тоже
		so_far++;
		_irda_rx_buffer->tail = (unsigned int)(_irda_rx_buffer->tail + 1) % IrDA_BUFFER_SIZE;  // increment the tail
		if (so_far == len) return 1;
		IrDA_timeout = TIMEOUT_DEF;
		while ((!IrDA_IsDataAvailable())&&IrDA_timeout);
		if (IrDA_timeout == 0) return 0;
	}

	if (so_far != len)
	{
		so_far = 0;
		goto again;
	}

	if (so_far == len) return 1;
	else return 0;
}

void IrDA_Uart_isr (UART_HandleTypeDef *huart)
{
  uint32_t isrflags   = READ_REG(huart->Instance->SR);
  uint32_t cr1its     = READ_REG(huart->Instance->CR1);
  if (((isrflags & USART_SR_RXNE) != RESET) && ((cr1its & USART_CR1_RXNEIE) != RESET))
  {
    huart->Instance->SR;
    unsigned char c = huart->Instance->DR;
    IrDA_store_char (c, _irda_rx_buffer);
    return;
  }
  if (((isrflags & USART_SR_TXE) != RESET) && ((cr1its & USART_CR1_TXEIE) != RESET))
  {
    if(IrDA_tx_buffer.head == IrDA_tx_buffer.tail)
    {
      __HAL_UART_DISABLE_IT(huart, UART_IT_TXE);
    }
    else
    {
      unsigned char c = IrDA_tx_buffer.buffer[IrDA_tx_buffer.tail];
      IrDA_tx_buffer.tail = (IrDA_tx_buffer.tail + 1) % IrDA_BUFFER_SIZE;
      huart->Instance->SR;
      huart->Instance->DR = c;
    }
    return;
  }
}
