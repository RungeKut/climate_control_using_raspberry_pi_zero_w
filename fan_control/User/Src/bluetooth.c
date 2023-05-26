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

void HC_05_init(void)
{
  uint8_t attemptConnect = 0;
  //Включаем режим приема АТ команд, подаем "1" на PIO11 модуля
  HAL_GPIO_WritePin(BT_AT_CMD_EN_GPIO_Port, BT_AT_CMD_EN_Pin, GPIO_PIN_SET);
  //Проверяем связь с модулем HC_05
  repeat1:
  attemptConnect++;
  if (!HC_05_isConnected())
  {
    HAL_Delay (5000);
    if (attemptConnect > 10) //Если больше 10-ти попыток - выходим
    {
      HAL_GPIO_WritePin(BT_AT_CMD_EN_GPIO_Port, BT_AT_CMD_EN_Pin, GPIO_PIN_RESET);
      return;
    }
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
    //Устанавливаем новое имя устройства (не более 32 символов)
    HAL_Delay (50);
    Uart_sendstring("AT+NAME=Vent_control\r\n");
    HAL_Delay (50);
    Wait_for("OK\r\n");
    HAL_Delay (50);
    //Устанавливаем новый пин-код устройства (не более 16 символов)
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
  //repeat4:
  HAL_Delay (50);
  HAL_GPIO_WritePin(BT_AT_CMD_EN_GPIO_Port, BT_AT_CMD_EN_Pin, GPIO_PIN_RESET);
  HAL_Delay (50);
  Uart_sendstring("AT+RESET\r\n");
  //if (!Wait_for("OK\r\n"))
  //{
  //  goto repeat4;
  //}
}

//http://know.smartelements.ru/main:comm:hc05_at
//Описание ошибок выдаваемых модулем (ERROR №):
//0 — Неправильная AT команда (нет такой команды)
//1 — Результат по умолчанию
//2 — Ошибка сохранения пароля
//3 — Слишком длинное имя устройства (более 32 байт)
//4 — Имя устройства не указано
//5 — Часть адреса NAP слишком длинная (более 4 разрядов в шестнадцатиричной системе)
//6 — Часть адреса UAP слишком длинная (более 2 разрядов в шестнадцатиричной системе)
//7 — Часть адреса LAP слишком длинная (более 6 разрядов в шестнадцатиричной системе)
//8 — Не указана маска порта PIO
//9 — Не указан номер вывода PIO
//A — Не указан тип (класс) устройства
//B — Слишком длинный тип (класс) устройства
//C — Не указан общий код доступа IAC (Inquire Access Code)
//D — Слишком длинный общий код доступа IAC (Inquire Access Code)
//E — Недопустимый общий код доступа IAC (Inquire Access Code)
//F — Не указан пароль (или пароль пуст)
//10 — Слишком длинный пароль (более 16 байт)
//11 — Недопустимая роль модуля
//12 — Недопустимая скорость передачи данных
//13 — Недопустимый размер стоп-бита
//14 — Недопустимая настройка бита четности
//15 — Устройство отсутствует в списке пар (списке сопряжённых Bluetooth устройств)
//16 — Профиль последовательного порта (SPP, Serial Port Profile) не инициализирован
//17 — Повторная инициализация профиля SPP (SPP, Serial Port Profile)
//18 — Недопустимый режим опроса Bluetooth устройств
//19 — Слишком большое время опроса
//1A — Не указан адрес Bluetooth устройства
//1B — Недопустимый режим безопасности (секретности)
//1C — Недопустимый режим шифрования
