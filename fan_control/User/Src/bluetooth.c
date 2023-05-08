#include "bluetooth.h"
#include "bt_usart.h"
#include "main.h"

uint8_t HC_05_isConnected(void)//Проверка связи с модулем
{
  HAL_Delay (50);
  Uart_sendstring("AT\r\n");
  if (Wait_for("OK\r\n"))
  {
    HAL_Delay (50);
    Uart_sendstring("BT module is connected!\r\n");//Отладочная информация
    Wait_for("ERROR:(0)\r\n");
    return 1;
  }
  else
  {
    HAL_Delay (50);
    Uart_sendstring("BT module ERROR connected!\r\n");//Отладочная информация
    return 0;
  }
}

uint8_t HC_05_restoreDefault(void)
{
  
}

void HC_05_init(void)
{
  //Включаем режим приема АТ команд, подаем "1" на PIO11 модуля
  LL_GPIO_SetOutputPin(BT_AT_CMD_EN_GPIO_Port, BT_AT_CMD_EN_Pin);
  //Проверяем связь с модулем HC_05
  repeat1:
  if (!HC_05_isConnected())
  {
    HAL_Delay (5000);
    goto repeat1;
  }
  //Проверяем имя устройства
  HAL_Delay (50);
  Uart_sendstring("AT+NAME?\r\n");
  HAL_Delay (50);
  if (!Wait_for("+NAME:Vent_control\r\nOK\r\n"))
  {
    repeat3:
    //Если другое имя - делаем возврат к заводским настройкам
    HAL_Delay (50);
    Uart_sendstring("BT restore default\r\n");//Отладочная информация
    Wait_for("ERROR:(0)\r\n");
    uint8_t count = 3;
    HAL_Delay (50);
    Uart_sendstring("AT+ORGL\r\n");
    Wait_for("OK\r\n");
    HAL_Delay (1000);
    repeat2:
    if (!HC_05_isConnected())
    {
      if (count != 0)
      {
        count--;
        HAL_Delay (1000);
        goto repeat2;
      }
      else
      {
        HAL_Delay (1000);
        goto repeat1;
      }
    }
    //Устанавливаем новое имя устройства
    HAL_Delay (50);
    Uart_sendstring("AT+NAME=Vent_control\r\n");
    HAL_Delay (50);
    Wait_for("OK\r\n");
    HAL_Delay (50);
    //Устанавливаем новый пин-код устройства
    Uart_sendstring("AT+PSWD=35967200\r\n");
    HAL_Delay (50);
    Wait_for("OK\r\n");
    HAL_Delay (50);
    goto repeat1;
  }
  //Проверяем пин-код устройства
  HAL_Delay (50);
  Uart_sendstring("AT+PSWD?\r\n");
  HAL_Delay (50);
  if (!Wait_for("+PSWD:35967200\r\nOK\r\n"))
  {
    goto repeat3;
  }
  //Выходим из режима АТ команд
  repeat4:
  HAL_Delay (50);
  LL_GPIO_ResetOutputPin(BT_AT_CMD_EN_GPIO_Port, BT_AT_CMD_EN_Pin);
  HAL_Delay (50);
  Uart_sendstring("AT+RESET\r\n");
  if (!Wait_for("OK\r\n"))
  {
    goto repeat4;
  }
}
