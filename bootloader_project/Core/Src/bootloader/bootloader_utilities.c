/**
 * \file            bootloader.c
 * \brief           Файл с функциями-помощниками для загрузчика
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

#include "bootloader/bootloader_utilities.h"

/**
 * \brief           Функция, которая превращает массив uint8_t с 4 элементами
 *                  в uint32_t с сохранением порядка байт (big-endian).
 * \note            p_bytes должен указывать не на начало массива, а на начало
 *                  последовательности из 4 байт, которую мы хотим сделать uin32_t.
 * \param[in]       *p_bytes: Указатель на массив.
 * \return          result: Выровненное число uint32_t, или же 0, если указатель на массив пуст.
 */
uint32_t
four_uint8t_to_one_uint32t(uint8_t* p_bytes) {
    uint32_t result;

    /* Проверка на пустой указатель */
    if (p_bytes == NULL) {
        printf("four_uint8t_to_one_uint32t - Пустой указатель\n");
        result = 0;
        return result;
    }

    union {
        uint32_t u32;
        uint8_t u8[4];
    } temp_bytes;

    /* Выравниваем байты правильным образом */
    temp_bytes.u8[0] = *(p_bytes + 3);
    temp_bytes.u8[1] = *(p_bytes + 2);
    temp_bytes.u8[2] = *(p_bytes + 1);
    temp_bytes.u8[3] = *p_bytes;

    return temp_bytes.u32;
}

/**
 * \brief       Функция которая расшифровывает данные.
 * \note        Алгоритм шифрования - XOR, с ключом в 4 байта.
 * \param[in]  key: Ключ шифрования.
 * \param[in,out] *data: Указатель на зашифрованные данные, которые после выполнения функции расшифровываются.
 * \param[out] size: Размер зашифрованных данных.
 */
void
decrypt_data(uint32_t key, uint8_t* data, size_t size) {
    if (data != NULL) {
        for (size_t i = 0; i < size; i++) {
            data[i] ^= (key >> (8 * (i % 4))) & 0xFF;
        }
    }
}

/**
 * \brief       Функция, которая ищет в входных данных последовательность символов.
 * \note        Входные данные должны быть в кодировки Ascii.
 * \param[in]  *source: Указатель на входные данные.
 * \param[in]  *destination: Указатель на последовательность, которую хотим найти.
 * \return     result: Результат поиска: TRUE (входные данные совпадают с последовательностью),
 *             FALSE (входные данные не совпадают с последовательностью).
 */
uint8_t
find_word(uint8_t* source, const char* destination) {

    /* Проверка на пустой указатель */
    if (source == NULL || destination == NULL) {
        return FALSE;
    }

    /* Опираясь на пакетную структуру передаваемых данных, априори знаем, что первые байты пакета,
     * это всегда тип передаваемого сообщения, поэтому вести поиск дальше этих байт не имеет смысла */
    size_t destination_size = strlen(destination);

    /* Выдаем ответ */
    uint8_t search_result;
    if (memcmp(source, destination, destination_size) == 0) {
        search_result = TRUE;
    } else {
        search_result = FALSE;
    }

    return search_result;
}

/**
 * \brief       Функция, которая проверяет переданные данные путем вычисления CRC32 MPEG-2.
 * \note        CRC32 вычисляется аппаратно.
 * \param[in]  *data: Указатель на исходные данные.
 * \param[in]  data_size: Размер исходных данных.
 * \param[in]  input_crc: Полученный вместе с данными CRC32.
 * \return     result: Результат проверки: TRUE (данные переданы корректно), FALSE (возникла ошибка при передачи данных).
 */
uint8_t
check_crc(uint8_t* data, size_t data_size, uint32_t input_crc) {

    uint8_t result = FALSE;

    /* Проверка на пустой указатель */
    if (data == NULL) {
        return result;
    }

    size_t temp_data_size = data_size / sizeof(uint32_t);
    uint32_t temp_data[temp_data_size];

    /* Создаем временный массив uint32_t, чтобы использовать правильный порядок
     * байт в памяти для дальнейшего использования аппаратного вычисления CRC */
    memcpy(temp_data, data, data_size);
    for (int i = 0; i < data_size / 4; i++) {
        temp_data[i] = ((temp_data[i] & 0x000000FF) << 24) | ((temp_data[i] & 0x0000FF00) << 8)
                       | ((temp_data[i] & 0x00FF0000) >> 8) | ((temp_data[i] & 0xFF000000) >> 24);
    }

    /* Используем аппаратные средства для вычисления CRC. В STM32F407xx нет возможности
     * настроить алгоритм вычисления CRC, используются только настройки по умолчанию, а именно:
     * Алгоритм CRC32 MPEG-2 */
    uint32_t calculated_crc = HAL_CRC_Calculate(&hcrc, temp_data, temp_data_size);

//    printf("Входные CRC байты - %08lx\n", input_crc);
//    printf("Вычисленные STM32 CRC байты - %08lx\n", calculated_crc);

    /* Выдаем ответ */
    if (input_crc == calculated_crc) {
        result = TRUE;
    } else {
        result = FALSE;
    }

    return result;
}

/**
 * \brief       Функция, которая проверяет наличие последовательности байт
 *              в списке поддерживаемых загрузчиком команд.
  * \param[in]  cmd_type_bytes: Байты, которые необходимо проверить на соответствие.
 * \return      type: Один из соответствующих типов команд из \ref cmd_t, если команда неизвестна, то \ref CMD_UNKNOWN.
 */
cmd_t
check_cmd_type(uint32_t cmd_type_bytes) {
    cmd_t type;

    switch (cmd_type_bytes) {
        case CMD_UPDATE_FIRMWARE:
            type = CMD_UPDATE_FIRMWARE;
            break;
        case CMD_SET_KEY:
            type = CMD_SET_KEY;
            break;
        case CMD_FLASH_OB_CHECK:
            type = CMD_FLASH_OB_CHECK;
            break;
        case CMD_FLASH_LOCK:
            type = CMD_FLASH_LOCK;
            break;
        case CMD_FLASH_UNLOCK:
            type = CMD_FLASH_UNLOCK;
            break;
        case CMD_GET_ID:
            type = CMD_GET_ID;
            break;
        case CMD_CHECK_KEY:
            type = CMD_CHECK_KEY;
            break;
        case CMD_ERASE_PROGRAM:
            type = CMD_ERASE_PROGRAM;
            break;
        default:
            type = CMD_UNKNOWN;
            break;
    }

    return type;
}

/**
 * \brief     Эта функция производить программную перезагрузку МК через 3 секунды
 */
void
restart(void) {
    printf("Перезагрузка микроконтроллера через 3 секунды\n");

    uint32_t end_tick = HAL_GetTick() + 3000;

    do {
        uint32_t current_tick = HAL_GetTick();

        if (current_tick > end_tick) {
            printf("Перезагрузка МК!\n");
            NVIC_SystemReset();
            break;
        }

    } while (1);
}
