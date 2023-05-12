/**
 * \file            bootloader.c
 * \brief           Файл с функциями, работающими с Flash памятью
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

#include "bootloader/bootloader_flash.h"

/**
 * \brief       Функция, стирает определенный сектор flash памяти.
 * \param[in]   number_of_sector: Номер сектора flash памяти STM32F407xx для стирания.
 * \return      result: Результат стирания: TRUE (flash память сектора успешно очищена),
 *             FALSE (во время стирания flash памяти произошла ошибка).
 */
uint8_t
erase_flash(uint32_t number_of_sector) {
    HAL_StatusTypeDef hal_status;

    uint8_t result = FALSE;

    do {
        /* Разблокировка доступа к системной памяти */
        hal_status = HAL_FLASH_Unlock();

        if (hal_status != HAL_OK) {
            printf("Ошибка разблокировки памяти\n");
            break;
        }

        /* Определение структуры, которая будет использоваться для стирания сектора памяти */
        FLASH_EraseInitTypeDef EraseInitStruct;
        uint32_t sector_error = 0;

        /* Заполнение структуры */
        EraseInitStruct.TypeErase = FLASH_TYPEERASE_SECTORS;
        EraseInitStruct.Sector = number_of_sector;
        EraseInitStruct.NbSectors = 1;
        EraseInitStruct.VoltageRange = FLASH_VOLTAGE_RANGE_3;

        /* Стирание сектора памяти */
        hal_status = HAL_FLASHEx_Erase(&EraseInitStruct, &sector_error);

        if (hal_status != HAL_OK) {
            printf("Ошибка стирания памяти\n");
            break;
        }

        /* Блокировка доступа к системной памяти */
        hal_status = HAL_FLASH_Lock();

        if (hal_status != HAL_OK) {
            printf("Ошибка блокировки памяти\n");
            break;
        }

        printf("Стирание памяти - успешно!\n");
        result = TRUE;
    } while (0);

    return result;
}

/**
 * \brief       Функция которая записывает блок данных в flash память.
 * \note        Эта функция используется для записи расшифрованных блоков данных из пакета типа "data".
 * \param[in]  *data: Указатель на блок данных для записи.
 * \param[in]  data_len: Размер блока данных.
 * \param[in]  flash_address: Адрес flash памяти, в который происходит запись.
 * \return     result: Результат записи: TRUE (блок данных записан успешно), FALSE (ошибка записи)
 */
uint8_t
write_data_block_to_flash(uint8_t* data, uint32_t data_len, uint32_t flash_address) {
    HAL_StatusTypeDef hal_status;
    uint8_t result = FALSE;

    do {
        if (data == NULL) {
            printf("Пустой указатель\n");
            break;
        }

        /* Разблокировка доступа к системной памяти */
        hal_status = HAL_FLASH_Unlock();

        if (hal_status != HAL_OK) {
            printf("Ошибка разблокировки памяти\n");
            break;
        }

        /* Запись блока во flash */
        for (uint32_t i = 0; i < data_len; i++) {
            hal_status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_BYTE, (flash_address + i), data[i]);

            if (hal_status != HAL_OK) {
                printf("Ошибка при записи байта в память\n");
                break;
            }
        }

        /* Блокировка доступа к системной памяти */
        hal_status = HAL_FLASH_Lock();

        if (hal_status != HAL_OK) {
            printf("Ошибка блокировки памяти\n");
            break;
        }

        result = TRUE;
    } while (0);

    return result;
}

/**
 * \brief       Функция, которая считывает данные из flash памяти и проверяет их корректность путем вычисления CRC32.
 * \return      result: Результат выполение операции: TRUE (успех), FALSE (неудача).
 */
uint8_t
check_settings(void) {
    uint8_t result = FALSE;
    uint32_t settings_buffer[NUMBER_OF_SETTINGS_WORDS];
    uint32_t settings_address_offset = 0;

    /* Считывает данные с flash памяти по 4 байта */
    for (uint32_t i = 0; i < NUMBER_OF_SETTINGS_WORDS; i++) {
        settings_buffer[i] = (*(__IO uint32_t*)(MEMORY_ADDRESS_WITH_SETTINGS + settings_address_offset));
        settings_address_offset += FLASH_BLOCK_OFFSET;
    }

    /* Проверяем целостность данных путем вычисления CRC32, сам CRC32 в расчет не идет */
    uint32_t calculated_crc = HAL_CRC_Calculate(&hcrc, settings_buffer, NUMBER_OF_SETTINGS_WORDS - 1);

    if (calculated_crc == settings_buffer[NUMBER_OF_SETTINGS_WORDS - 1]) {
        encryption_key = settings_buffer[0];
        secret_encryption_key = settings_buffer[1];
        result = TRUE;
    }

    return result;
}

/**
 * \brief      Функция, которая производит сохранение настроек во flash память МК
 * \note       Настройки представляют собой значения uint32_t которые друг за другом располагаются в памяти:
 *             "ключ шифрования прошивки" (изменяем), "ключ шифрования для смены ключа шифрования прошивки" (неизменяем), CRC32 (для проверки)
 * \param[in]  key: Ключ шифрования прошивки.
 * \return     result: Результат записи настроек: TRUE (настройки успешно записаны), FALSE (произошла ошибка записи настроек)
 */
uint8_t
set_settings(uint32_t key) {
    HAL_StatusTypeDef hal_status;
    uint8_t result = FALSE;
    uint32_t settings_buffer[NUMBER_OF_SETTINGS_WORDS];
    uint32_t settings_address_offset = 0;

    do {
        printf("Начинаю запись настроек по умолчанию во flash\n");
        /* Стираем весь сектор с настройками, только после стирания мы можем записать данные в него */
        uint8_t erase_succesfull = erase_flash(MEMORY_SECTOR_WITH_SETTINGS);

        if (!erase_succesfull) {
            break;
        }

        /* Формируем буфер с настройками для записи во flash */
        settings_buffer[0] = key;
        settings_buffer[1] = DEFAULT_SECRET_ENCRYPTION_KEY;
        settings_buffer[2] = HAL_CRC_Calculate(&hcrc, settings_buffer, NUMBER_OF_SETTINGS_WORDS - 1);

        /* Записываем во flash память настройки по умолчанию */
        hal_status = HAL_FLASH_Unlock();

        if (hal_status != HAL_OK) {
            printf("Ошибка разблокировки памяти\n");
            break;
        }

        /* Запись настроек во flash */
        for (uint32_t i = 0; i < NUMBER_OF_SETTINGS_WORDS; i++) {
            hal_status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD,
                                           MEMORY_ADDRESS_WITH_SETTINGS + settings_address_offset, settings_buffer[i]);

            if (hal_status != HAL_OK) {
                printf("Ошибка при записи слова настроек в память\n");
                break;
            }

            settings_address_offset += FLASH_BLOCK_OFFSET;
        }

        /* Блокировка доступа к системной памяти */
        hal_status = HAL_FLASH_Lock();

        if (hal_status != HAL_OK) {
            printf("Ошибка блокировки памяти\n");
            break;
        }

        printf("Запись настроек в память - успешно!\n");

        result = TRUE;
    } while (0);

    return result;
}
