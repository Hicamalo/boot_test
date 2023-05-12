/**
 * \file            bootloader_settings.h
 * \brief           Файл с основными настройками загрузчика
 */

/*
 * Copyright (c) 2023 Дмитрий Соловьев
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE
 * AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 * This file is part of bootloader.
 *
 * Author:          Дмитрий Соловьев <sologodline@gmail.com>
 */

#ifndef BOOTLOADER_SETTINGS_H
#define BOOTLOADER_SETTINGS_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

extern UART_HandleTypeDef huart3;
extern CRC_HandleTypeDef hcrc;

extern uint32_t encryption_key; /*Ключ шифрования прошивки, использующийся в программе */
extern uint32_t secret_encryption_key; /* Ключ шифрования, необходимый для смены encryption_key */

#define TRUE                          1
#define FALSE                         0

/* Количество байт, которые определяют тип пакета */
#define NUMBER_OF_BYTES_COMMAND_WORD  4
#define NUMBER_OF_BYTES_HEADER_WORD   4
#define NUMBER_OF_BYTES_RESPONSE_WORD 4
#define NUMBER_OF_BYTES_DATA_WORD     4
#define NUMBER_OF_BYTES_KEY_WORD      4
#define NUMBER_OF_BYTES_TEST_WORD     4
#define NUMBER_OF_BYTES_ACK           4
#define NUMBER_OF_BYTES_NACK          4

/* Количество байт, которые определяют данные в пакете */
#define NUMBER_OF_BYTES_COMMAND_DATA  4
#define NUMBER_OF_BYTES_HEADER_DATA   4
#define NUMBER_OF_BYTES_RESPONSE_DATA 4
#define NUMBER_OF_BYTES_DATA_DATA     1024
#define NUMBER_OF_BYTES_KEY_DATA      4
#define NUMBER_OF_BYTES_CRC           4

/* Полный размер пакета */
#define CMD_SIZE                      (NUMBER_OF_BYTES_COMMAND_WORD + NUMBER_OF_BYTES_COMMAND_DATA + NUMBER_OF_BYTES_CRC)
#define HEADER_SIZE                   (NUMBER_OF_BYTES_HEADER_WORD + NUMBER_OF_BYTES_HEADER_DATA + NUMBER_OF_BYTES_CRC)
#define RESPONSE_SIZE                 (NUMBER_OF_BYTES_RESPONSE_WORD + NUMBER_OF_BYTES_RESPONSE_DATA + NUMBER_OF_BYTES_CRC)
#define DATA_SIZE                     (NUMBER_OF_BYTES_DATA_WORD + NUMBER_OF_BYTES_DATA_DATA + NUMBER_OF_BYTES_CRC)
#define KEY_SIZE                      (NUMBER_OF_BYTES_KEY_WORD + NUMBER_OF_BYTES_KEY_DATA + NUMBER_OF_BYTES_CRC)

/* Максимальная попытка получить от Host приложения пакет, если он получен с ошибкой*/
#define MAX_USART_CONNECTION_TRY      10U

#define MEMORY_ADDRESS_WITH_SETTINGS                                                                                   \
    0x0800C000U /* Адрес в памяти, где хранятся настройки загрузчика (должен совпадать с номером сектора) */
#define MEMORY_SECTOR_WITH_SETTINGS   3U /* Номер сектора памяти, где хранятся настройки */
#define NUMBER_OF_SETTINGS_WORDS      3U          /* Количество слов uint32_t настроек */
#define DEFAULT_ENCRYPTION_KEY        0x13121411U /* Ключ шифрования прошивки по умолчанию */
#define DEFAULT_SECRET_ENCRYPTION_KEY (*(uint32_t *)0x1FFF7A14) /* Ключ шифрования для смены encryption_key (Это вторые 32 бита UID) */

#define NUMBER_OF_BYTES_OF_FLASH_MEMORY_SECTOR                                                                         \
    65536U /* Максимальный размер сектора, в котором располагается загрузчик */

#define APP_FLASH_START_ADDRESS                                                                                        \
    0x08010000U /* Адрес начала пользовательского приложения (должен совпадать с номером сектора) */
#define FLASH_SECTOR_NUMBER 4U /* Номер сектора flash памяти, где находится пользовательское приложение */
#define FLASH_BLOCK_OFFSET  4U /* Количество байт данных типа слово, которое занято в памяти */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif //BOOTLOADER_SETTINGS_H
