/**
 * \file            bootloader_types.h
 * \brief           Файл с основными стуктурами и перечислениями
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

#ifndef BOOTLOADER_TYPES_H
#define BOOTLOADER_TYPES_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * \brief       Перечисление, отвечающее за команды, поддерживаемые загрузчиком
 */
typedef enum {
    CMD_UNKNOWN = 0,         /* Неизвестная команда */
    CMD_UPDATE_FIRMWARE = 1, /* Команда на обновление пользовательской прошивки */
    CMD_SET_KEY = 2, /* Команда на смену 4 байтного ключа шифрования прошивки */
    CMD_FLASH_OB_CHECK = 3, /* Команда на проверку наличия защиты на flash памяти */
    CMD_FLASH_LOCK = 4, /* Команда на блокировку чтения или записи flash памяти */
    CMD_FLASH_UNLOCK = 5, /* Команда на разблокировку чтения или записи flash памяти */
    CMD_GET_ID = 6, /* Команда на получение уникального ID микроконтроллера */
    CMD_CHECK_KEY = 7, /* Команда на проверку соответствия ключей шифрования */
    CMD_ERASE_PROGRAM = 8, /* Команда на стирание пользовательской прошивки */
} cmd_t;

/**
 * \brief     Перечисление, отвечающее за данные, которые будут передаваться в пакете "response"
 * \note    UID - это уникальный ID микроконтроллера (96 бит)
 */
typedef enum {
    RESPONSE_FAIL = 0x33333333, /* Код ошибки */
    RESPONSE_OK = 0xFFFFFFFF,   /* Код успеха */
    RESPONSE_UID_1_ADDRESS = 0x1FFF7A10, /* Первые 32 бита */
    RESPONSE_UID_2_ADDRESS = 0x1FFF7A14, /* Вторые 32 бита */
    RESPONSE_UID_3_ADDRESS = 0x1FFF7A18, /* Третьи 32 бита */
} response_t;

/**
 * \brief     Перечисление, отвечающее за данные, которые будут передаваться в пакете "status"
 */
typedef enum {
    STATUS_NACK = 0, /* Ошибка при передаче пакета загрузчику */
    STATUS_ACK = 1,  /* Пакет передался загрузчику успешно */
} status_t;

/**
 * \brief     Перечисление, в котором указан статус выполнения функции usart_get_cmd
 */
typedef enum {
    GET_CMD_OK = 0,                     /* Выполнение функции успешно */
    GET_CMD_ERROR_RECEIVING_STATUS = 1, /* Ошибка при использовании HAL_UART_Receive */
    GET_CMD_ERROR_NOT_COMMAND = 2,      /* Ошибка при определении типа пакета */
    GET_CMD_ERROR_CRC = 3,              /* Ошибка проверки CRC32 */
    GET_CMD_ERROR_EMPTY_POINTER = 4,    /* Указатель на входные данные == NULL */
} get_cmd_status_t;

/**
 * \brief     Перечисление, в котором указан статус выполнения функции get_header_status_t
 */
typedef enum {
    GET_HEADER_OK = 0,                     /* Выполнение функции успешно */
    GET_HEADER_ERROR_RECEIVING_STATUS = 1, /* Ошибка при использовании HAL_UART_Receive */
    GET_HEADER_ERROR_NOT_HEADER = 2,       /* Ошибка при определении типа пакета */
    GET_HEADER_ERROR_CRC = 3,              /* Ошибка проверки CRC32 */
    GET_HEADER_ERROR_EMPTY_POINTER = 4,    /* Указатель на входные данные == NULL */
} get_header_status_t;

/**
 * \brief     Перечисление, в котором указан статус выполнения функции get_data_status_t
 */
typedef enum {
    GET_DATA_OK = 0,                     /* Выполнение функции успешно */
    GET_DATA_ERROR_RECEIVING_STATUS = 1, /* Ошибка при использовании HAL_UART_Receive */
    GET_DATA_ERROR_NOT_DATA = 2,         /* Ошибка при определении типа пакета */
    GET_DATA_ERROR_CRC = 3,              /* Ошибка проверки CRC32 */
    GET_DATA_ERROR_EMPTY_POINTER = 4,    /* Указатель на входные данные == NULL */
} get_data_status_t;

/**
 * \brief     Перечисление, в котором указан статус выполнения функции get_key_status_t
 */
typedef enum {
    GET_KEY_OK = 0,                     /* Выполнение функции успешно */
    GET_KEY_ERROR_RECEIVING_STATUS = 1, /* Ошибка при использовании HAL_UART_Receive */
    GET_KEY_ERROR_NOT_KEY = 2,          /* Ошибка при определении типа пакета */
    GET_KEY_ERROR_CRC = 3,              /* Ошибка проверки CRC32 */
    GET_KEY_ERROR_EMPTY_POINTER = 4,    /* Указатель на входные данные == NULL */
} get_key_status_t;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif //BOOTLOADER_TYPES_H
