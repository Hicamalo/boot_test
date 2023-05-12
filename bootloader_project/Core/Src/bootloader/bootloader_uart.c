/**
 * \file            bootloader.c
 * \brief           Файл с функциями для работы в UART
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

#include "bootloader/bootloader_uart.h"

//TODO: продумать этот момент, не явно всегда 12 байт
/**
 * \brief       Функция, посылает по USART пакет типа "response".
 * \param[in]   response_data: Данные, которые необходимо послать в пакете "response".
 */
void
usart_send_response(uint32_t response_data) {
    uint8_t response_buff_u8[RESPONSE_SIZE];
    uint32_t response_word_plus_data[(RESPONSE_SIZE / 4) - 1];
    uint32_t response_crc = 0;

    /* Формируем начало посылки */
    response_buff_u8[0] = response_word[0];
    response_buff_u8[1] = response_word[1];
    response_buff_u8[2] = response_word[2];
    response_buff_u8[3] = response_word[3];

    /* Формируем данные посылки */
    response_buff_u8[4] = response_data & 0xFF;
    response_buff_u8[5] = (response_data >> 8) & 0xFF;
    response_buff_u8[6] = (response_data >> 16) & 0xFF;
    response_buff_u8[7] = (response_data >> 24) & 0xFF;

    response_word_plus_data[0] = four_uint8t_to_one_uint32t(&response_buff_u8[0]);
    response_word_plus_data[1] = four_uint8t_to_one_uint32t(&response_buff_u8[4]);

    response_crc = HAL_CRC_Calculate(&hcrc, response_word_plus_data, (RESPONSE_SIZE / 4) - 1);

    /* Формируем CRC32 посылки */
    response_buff_u8[8] = response_crc & 0xFF;
    response_buff_u8[9] = (response_crc >> 8) & 0xFF;
    response_buff_u8[10] = (response_crc >> 16) & 0xFF;
    response_buff_u8[11] = (response_crc >> 24) & 0xFF;

    HAL_UART_Transmit(&huart3, response_buff_u8, RESPONSE_SIZE, HAL_MAX_DELAY);
    HAL_UART_Transmit(&huart3, (uint8_t*)"\n", 1, HAL_MAX_DELAY);
}

/**
 * \brief       Функция, которая отправляет по USART пакет типа "status".
 * \param[in]  status: Одно из значений типа \ref status_t.
 */
void
usart_send_status(status_t status) {
    /* Учитывая нуль терминатор - + 1 байт */
    if (status == STATUS_ACK) {
        HAL_UART_Transmit(&huart3, (uint8_t*)ack_word, NUMBER_OF_BYTES_ACK + 1, HAL_MAX_DELAY);
    } else {
        HAL_UART_Transmit(&huart3, (uint8_t*)nack_word, NUMBER_OF_BYTES_NACK + 1, HAL_MAX_DELAY);
    }
    HAL_UART_Transmit(&huart3, (uint8_t*)"\n", 1, HAL_MAX_DELAY);
}

/**
 * \brief       Функция, которая получает по USART пакет типа "command".
 * \param[out]  *received_command: Указатель на полученный пакет.
 * \return     status: Статус выполнения операции, если \ref GET_CMD_OK, то пакет получен успешно,
 *             если что-то другое, произошла ошибка.
 */
get_cmd_status_t
usart_get_cmd(cmd_t* received_command) {

    /* Буфер, в который будуд приходить сырые данные */
    uint8_t local_rx_buffer[CMD_SIZE];
    /* Индекс local_rx_buffer */
    size_t index = 0;
    get_cmd_status_t status = GET_CMD_OK;

    do {
        if (received_command == NULL) {
            printf("Пустой указатель\n");
            status = GET_CMD_ERROR_EMPTY_POINTER;
            break;
        }

        int16_t receiving_status = HAL_UART_Receive(&huart3, &local_rx_buffer[index], CMD_SIZE, HAL_MAX_DELAY);

        if (receiving_status != HAL_OK) {
            printf("При использовании HAL функции для получения данных по UART возникла ошибка\n");
            status = GET_CMD_ERROR_RECEIVING_STATUS;
            break;
        }

        /* Определяем начало пакета типа "command" */
        uint8_t cmd_word_finded = find_word(&local_rx_buffer[index], command_word);

        if (!cmd_word_finded) {
            printf("Переданные данные не соответствуют пакету типа \"command\"\n");
            status = GET_CMD_ERROR_NOT_COMMAND;
            break;
        }

        index += NUMBER_OF_BYTES_COMMAND_WORD;

        uint32_t cmd_type_bytes = four_uint8t_to_one_uint32t(&local_rx_buffer[index]);

        /* Определяем, какая команда пришла в пакете */
        *received_command = check_cmd_type(cmd_type_bytes);

        index += NUMBER_OF_BYTES_COMMAND_DATA;

        uint32_t crc_bytes = four_uint8t_to_one_uint32t(&local_rx_buffer[index]);

        /* Делаем проверку целостности входных данных, путем вычисления CRC.
         * CRC сумма вычисляется по байтам:
         * "начало_пакета_command" (символы Ascii) + "данные_передаваемые_в_пакете_command" */
        uint8_t crc_result =
            check_crc(local_rx_buffer, (NUMBER_OF_BYTES_COMMAND_WORD + NUMBER_OF_BYTES_COMMAND_DATA), crc_bytes);

        if (!crc_result) {
            printf("Ошибка CRC, полученная и вычисленная сумма отличаются\n");
            status = GET_CMD_ERROR_CRC;
            break;
        }

    } while (0);

    return status;
}

/**
 * \brief       Функция, которая получает по USART пакет типа "header".
 * \param[out]  *firmware_size: Указатель на полученный размер прошивки.
 * \return     status: Статус выполнения операции, если \ref GET_HEADER_OK, то пакет получен успешно,
 *             если что-то другое, произошла ошибка.
 */
get_header_status_t
usart_get_header(uint32_t* firmware_size) {

    /* Буфер, в который будут приходить данные */
    uint8_t local_rx_buffer[HEADER_SIZE];
    /* Индекс local_rx_buffer */
    size_t index = 0;

    get_header_status_t status = GET_HEADER_OK;

    do {
        if (firmware_size == NULL) {
            printf("Пустой указатель\n");
            status = GET_HEADER_ERROR_EMPTY_POINTER;
            break;
        }

        int16_t receiving_status = HAL_UART_Receive(&huart3, &local_rx_buffer[index], HEADER_SIZE, HAL_MAX_DELAY);

        if (receiving_status != HAL_OK) {
            printf("При использовании HAL функции для получения данных по UART возникла ошибка\n");
            status = GET_HEADER_ERROR_RECEIVING_STATUS;
            break;
        }

        /* Определяем начало пакета типа "header" */
        uint8_t header_word_finded = find_word(&local_rx_buffer[index], header_word);

        if (!header_word_finded) {
            /* Переданные данные не соответствуют пакету типа "header" */
            printf("Переданные данные не соответствуют пакету типа \"header\"\n");
            status = GET_HEADER_ERROR_NOT_HEADER;
            break;
        }

        index += NUMBER_OF_BYTES_HEADER_WORD;

        /* Получаем размер прошивки */
        *firmware_size = four_uint8t_to_one_uint32t(&local_rx_buffer[index]);

        index += NUMBER_OF_BYTES_HEADER_DATA;

        uint32_t crc_bytes = four_uint8t_to_one_uint32t(&local_rx_buffer[index]);
        ;

        /* Делаем проверку целостности входных данных, путем вычисления CRC.
         * CRC сумма вычисляется по байтам:
        * "начало_пакета_header" (символы Ascii) + "данные_передаваемые_в_пакете_header" */
        uint8_t crc_result =
            check_crc(local_rx_buffer, (NUMBER_OF_BYTES_HEADER_WORD + NUMBER_OF_BYTES_HEADER_DATA), crc_bytes);

        if (!crc_result) {
            printf("Ошибка CRC, полученная и вычисленная сумма отличаются\n");
            status = GET_HEADER_ERROR_CRC;
            break;
        }

    } while (0);

    return status;
}

/**
 * \brief       Функция, которая получает по USART пакет типа "data".
 * \param[out]  *data_buffer: Указатель на полученный буфер с зашифрованными данными прошивки.
 * \return     status: Статус выполнения операции, если \ref GET_DATA_OK, то пакет получен успешно,
 *             если что-то другое, произошла ошибка.
 */
get_data_status_t
usart_get_data(uint8_t* data_buffer) {

    /* Буфер, в который будуд приходить сырые данные */
    uint8_t local_rx_buffer[DATA_SIZE];
    /* Индекс local_rx_buffer */
    size_t index = 0;
    get_data_status_t status = GET_DATA_OK;

    do {
        if (data_buffer == NULL) {
            printf("Пустой указатель\n");
            status = GET_DATA_ERROR_EMPTY_POINTER;
            break;
        }

        uint16_t receiving_status = HAL_UART_Receive(&huart3, &local_rx_buffer[index], DATA_SIZE, HAL_MAX_DELAY);

        if (receiving_status != HAL_OK) {
            printf("При использовании HAL функции для получения данных по UART возникла ошибка\n");
            status = GET_DATA_ERROR_RECEIVING_STATUS;
            break;
        }

        /* Определяем начало пакета типа "data" */
        uint8_t data_word_finded = find_word(&local_rx_buffer[index], data_word);

        if (!data_word_finded) {
            printf("Переданные данные не соответствуют пакету типа \"data\"\n");
            status = GET_DATA_ERROR_NOT_DATA;
            break;
        }

        index += NUMBER_OF_BYTES_DATA_WORD;

        /* Получаем даннные, которые пришли */
        memcpy(data_buffer, &local_rx_buffer[index], NUMBER_OF_BYTES_DATA_DATA);

        index += NUMBER_OF_BYTES_DATA_DATA;

        uint32_t crc_bytes = four_uint8t_to_one_uint32t(&local_rx_buffer[index]);

        /* Делаем проверку целостности входных данных, путем вычисления CRC.
        * CRC сумма вычисляется по байтам:
        * "начало_пакета_data" (символы Ascii) + "данные_передаваемые_в_пакете_data" */
        uint8_t crc_result =
            check_crc(local_rx_buffer, (NUMBER_OF_BYTES_DATA_WORD + NUMBER_OF_BYTES_DATA_DATA), crc_bytes);

        if (!crc_result) {
            printf("Ошибка CRC, полученная и вычисленная сумма отличаются\n");
            status = GET_DATA_ERROR_CRC;
            break;
        }

    } while (0);

    return status;
}

/**
 * \brief       Функция, которая получает по USART пакет типа "key".
 * \param[out]  *coded_key: Указатель на полученный ключ, который зашифрован \ref DEFAULT_SECRET_ENCRYPTION_KEY.
 * \return     status: Статус выполнения операции, если \ref GET_KEY_OK, то пакет получен успешно,
 *             если что-то другое, произошла ошибка.
 */
get_key_status_t
usart_get_key(uint32_t* coded_key) {
    /* Буфер, в который будут приходить данные */
    uint8_t local_rx_buffer[KEY_SIZE];
    /* Индекс local_rx_buffer */
    size_t index = 0;

    get_key_status_t status = GET_KEY_OK;

    do {
        if (coded_key == NULL) {
            printf("Пустой указатель\n");
            status = GET_KEY_ERROR_EMPTY_POINTER;
            break;
        }

        int16_t receiving_status = HAL_UART_Receive(&huart3, &local_rx_buffer[index], KEY_SIZE, HAL_MAX_DELAY);

        if (receiving_status != HAL_OK) {
            printf("При использовании HAL функции для получения данных по UART возникла ошибка\n");
            status = GET_KEY_ERROR_RECEIVING_STATUS;
            break;
        }

        /* Определяем начало пакета типа "ключ" */
        uint8_t key_word_finded = find_word(&local_rx_buffer[index], key_word);

        if (!key_word_finded) {
            printf("Переданные данные не соответствуют пакету типа \"key\"\n");
            status = GET_KEY_ERROR_NOT_KEY;
            break;
        }

        index += NUMBER_OF_BYTES_KEY_WORD;

        *coded_key = four_uint8t_to_one_uint32t(&local_rx_buffer[index]);

        index += NUMBER_OF_BYTES_KEY_DATA;

        uint32_t crc_bytes = four_uint8t_to_one_uint32t(&local_rx_buffer[index]);
        ;

        /* Делаем проверку целостности входных данных, путем вычисления CRC.
         * CRC сумма вычисляется по байтам:
        * "начало_пакета_key" (символы Ascii) + "данные_передаваемые_в_пакете_key" */
        uint8_t crc_result =
            check_crc(local_rx_buffer, (NUMBER_OF_BYTES_KEY_WORD + NUMBER_OF_BYTES_KEY_DATA), crc_bytes);

        if (!crc_result) {
            printf("Ошибка CRC, полученная и вычисленная сумма отличаются\n");
            status = GET_KEY_ERROR_CRC;
            break;
        }

    } while (0);

    return status;
}
