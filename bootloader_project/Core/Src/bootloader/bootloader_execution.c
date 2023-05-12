/**
 * \file            bootloader_execution.c
 * \brief           Файл с командами, которые должен выполнять загрузчик
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

#include "bootloader/bootloader_execution.h"

/**
 * \brief     Эта функция служит для отправки UID микроконтроллера
 */
//TODO: описание, h
void send_uid(void) {
    usart_send_response((*(uint32_t *)RESPONSE_UID_1_ADDRESS));
    usart_send_response((*(uint32_t *)RESPONSE_UID_2_ADDRESS));
    usart_send_response((*(uint32_t *)RESPONSE_UID_3_ADDRESS));
}

/**
 * \brief      Эта функция служит для изменения ключа шифрования прошивки.
 * \return     result: Результат изменения ключа: TRUE (ключ изменен), FALSE (произошла ошибка изменения ключа)
 */
uint8_t
set_key(void) {
    uint8_t connection_try = 0;
    uint8_t result = FALSE;

    uint32_t coded_key = 0;

    do {

        get_key_status_t get_key_result;
        uint8_t key_received_successfully = FALSE;

        send_uid();

        /* Получаем пакет типа "key" */
        while (connection_try < MAX_USART_CONNECTION_TRY) {
            get_key_result = usart_get_key(&coded_key);

            if (get_key_result != GET_KEY_OK) {
                usart_send_status(STATUS_NACK);
            } else {
                usart_send_status(STATUS_ACK);
                key_received_successfully = TRUE;
                break;
            }

            connection_try++;
        }

        connection_try = 0;

        if (!key_received_successfully) {
            printf("Ошибка при получении пакета \"key\"\n");
            break;
        }

        /* Расшифровываем ключ секретным ключом */
        uint8_t temp_arr[NUMBER_OF_BYTES_KEY_DATA];
        temp_arr[0] = (coded_key >> 24) & 0xFF;
        temp_arr[1] = (coded_key >> 16) & 0xFF;
        temp_arr[2] = (coded_key >> 8) & 0xFF;
        temp_arr[3] = coded_key & 0xFF;

        decrypt_data(secret_encryption_key, temp_arr, NUMBER_OF_BYTES_KEY_DATA);

        encryption_key = four_uint8t_to_one_uint32t(temp_arr);

        /* Записываем навый ключ шифрования во flash */
        uint8_t set_settings_successfully = FALSE;
        set_settings_successfully = set_settings(encryption_key);

        if (!set_settings_successfully) {
            printf("Ошибка установки настроек!");
            usart_send_response(RESPONSE_FAIL);
            break;
        }

        usart_send_response(RESPONSE_OK);
        result = TRUE;
    } while (0);

    return result;
}

/**
 * \brief       Функция которая производит обновление прошивки.
 * \return     update_successfull: Результат обновления: TRUE (обновление прошло успешно), FALSE (произошла ошибка обновления)
 */
uint8_t
update_firmware(void) {
    uint32_t connection_try = 0;

    uint8_t update_successfull = FALSE;

    do {
        /* Получаем header с размером прошивки */
        uint32_t firmware_size = 0;
        uint32_t max_flash_size_b = NUMBER_OF_BYTES_OF_FLASH_MEMORY_SECTOR;
        get_header_status_t get_header_status;
        uint8_t get_header_successfull = FALSE;

        while (connection_try < MAX_USART_CONNECTION_TRY) {
            get_header_status = usart_get_header(&firmware_size);

            if (get_header_status != GET_HEADER_OK) {
                usart_send_status(STATUS_NACK);
            } else if (firmware_size == 0) {
                printf("Размер прошивки равен нулю\n");
                usart_send_status(STATUS_ACK);
                usart_send_response(RESPONSE_FAIL);
                break;
            } else if (firmware_size > max_flash_size_b) {
                printf("Размер прошивки больше %lu байт\n", max_flash_size_b);
                usart_send_status(STATUS_ACK);
                usart_send_response(RESPONSE_FAIL);
                break;
            } else {
                usart_send_status(STATUS_ACK);
                usart_send_response(RESPONSE_OK);
                get_header_successfull = TRUE;
                break;
            }

            connection_try++;
        }

        connection_try = 0;

        if (!get_header_successfull) {
            printf("Ошибка при получении header\n");
            break;
        }

        /*TODO: Сделать проверку на соответствие ключей шифрования */

        /* Получаем прошивку */
        get_data_status_t get_data_status;
        uint32_t number_of_data_blocks = 0;

        uint8_t coded_data[NUMBER_OF_BYTES_DATA_DATA];
        uint8_t decoded_data[NUMBER_OF_BYTES_DATA_DATA];

        if (firmware_size % NUMBER_OF_BYTES_DATA_DATA == 0) {
            number_of_data_blocks = firmware_size / NUMBER_OF_BYTES_DATA_DATA;
        } else {
            number_of_data_blocks = firmware_size / NUMBER_OF_BYTES_DATA_DATA;
            number_of_data_blocks++;
        }

        uint8_t all_data_blocks_received = FALSE;
        uint32_t address_write_to_memory = APP_FLASH_START_ADDRESS;

        for (uint32_t i = 0; i < number_of_data_blocks; i++) {

            uint8_t one_data_block_received = FALSE;

            while (connection_try < MAX_USART_CONNECTION_TRY) {
                /* Получаем блок с прошивкой */
                get_data_status = usart_get_data(coded_data);

                if (get_data_status != GET_DATA_OK) {
                    printf("Ошибка при выполнении usart_get_data\n");
                    usart_send_status(STATUS_NACK);
                } else {
                    printf("[%lu / %lu]\n", i + 1, number_of_data_blocks);
                    usart_send_status(STATUS_ACK);
                    one_data_block_received = TRUE;
                    break;
                }

                connection_try++;
            }

            if (!one_data_block_received) {
                printf("Ошибка при получении блока данных с прошивкой\n");
                all_data_blocks_received = FALSE;
                break;
            }

            one_data_block_received = FALSE;

            /* 1 раз стираем flash память, только после того, как получили первый блок */
            if (i == 0) {
                uint8_t erase_successfull = erase_flash(FLASH_SECTOR_NUMBER);

                if (!erase_successfull) {
                    printf("Ошибка при стирании flash памяти\n");
                    break;
                }
            }

            /* Копируем зашифрованные данные для дальнейшей расшифровки */
            memcpy(decoded_data, coded_data, NUMBER_OF_BYTES_DATA_DATA);

            /* Расшифровываем данные */
            decrypt_data(encryption_key, decoded_data, NUMBER_OF_BYTES_DATA_DATA);

            /* Записываем блок данных во flash */
            uint8_t write_status =
                write_data_block_to_flash(decoded_data, NUMBER_OF_BYTES_DATA_DATA, address_write_to_memory);
            address_write_to_memory += NUMBER_OF_BYTES_DATA_DATA;

            if (!write_status) {
                printf("Ошибка при записи блока flash памяти\n");
                break;
            }

            /* Последний блок передан успешно */
            if (i + 1 == number_of_data_blocks) {
                printf("Прошивка запрограммирована успешно!\n");
                all_data_blocks_received = TRUE;
            }
        }

        if (!all_data_blocks_received) {
            printf("Ошибка при обновлении прошивки\n");
            break;
        }

        update_successfull = TRUE;
    } while (0);

    return update_successfull;
}

/**
 * \brief      Эта функция служит проверки защиты flash памяти. (Регистра с RDP байтами)
 * \return     result: Результат проверки: TRUE (Protection Level = 1 - есть защита от чтения или записи),
 *             FALSE (Protection Level = 0 - нет защиты от чтения или записи)
 */
uint8_t
flash_ob_check(void) {
    FLASH_OBProgramInitTypeDef OBconfig;
    HAL_FLASHEx_OBGetConfig(&OBconfig);

    uint8_t result;

    if (((OBconfig.OptionType & OPTIONBYTE_RDP) != OPTIONBYTE_RDP) || OBconfig.RDPLevel != OB_RDP_LEVEL_1) {
        printf("Защита flash памяти выключена\n");
        result = FALSE;
    } else {
        printf("Защита flash памяти включена (отключение приведет к полному стиранию flash памяти!)\n");
        result = TRUE;
    }

    return result;
}

/**
 * \brief     Эта функция служит для установки защиты flash памяти. (RDP Protection Level = 1)
 */
uint8_t
flash_lock(void) {

    uint8_t result = TRUE;

    FLASH_OBProgramInitTypeDef OBconfig;
    HAL_FLASHEx_OBGetConfig(&OBconfig);

    printf("Провожу проверку на наличие защиты flash памяти\n");
    uint8_t flash_already_lock = flash_ob_check();
    if (!flash_already_lock) {
        HAL_FLASH_Unlock();
        HAL_FLASH_OB_Unlock();

        OBconfig.OptionType = OPTIONBYTE_RDP;
        /* Защита от чтения прошивки */
        OBconfig.RDPLevel = ((uint8_t)0xBB);

//        /* Защита от случайного изменения прошивки */
//        OBconfig.WRPState = OB_WRPSTATE_ENABLE;
//        OBconfig.WRPSector = OB_WRP_SECTOR_All;

        HAL_FLASHEx_OBProgram(&OBconfig);

        if (HAL_FLASHEx_OBProgram(&OBconfig) != HAL_OK) {
            result = FALSE;
        }

        HAL_FLASH_OB_Lock();
        HAL_FLASH_Lock();
//        /* Перезагрузка регистров option bytes */
        HAL_FLASH_OB_Launch();

        HAL_FLASHEx_OBGetConfig(&OBconfig);
        printf("Защита flash памяти успешно включена\n");
    } else {
        printf("Защита flash памяти уже включена, отмена команды\n");
    }

    return result;
}

/**
 * \brief     Эта функция служит для снятия защиты flash памяти. (RDP Protection Level = 0)
 * \note      После снятия защиты происходит обязательное полное стирания flash памяти,
 *            как следствие необходимо заново прошивать прошивку и загрузчик.
 *            Стирание происходит аппаратно, и его невозможно предотвратить
 */
void
flash_unlock(void) {
    FLASH_OBProgramInitTypeDef OBconfig;

    printf("Провожу проверку на наличие защиты flash памяти\n");
    uint8_t flash_lock = flash_ob_check();
    if (flash_lock) {
        printf("Отключаю защиту flash памяти\n");

        HAL_FLASH_Unlock();
        HAL_FLASH_OB_Unlock();

        OBconfig.OptionType = OPTIONBYTE_RDP | OPTIONBYTE_WRP;

        /* Защита от чтения прошивки */
        OBconfig.RDPLevel = OB_RDP_LEVEL_0;

        /* Защита от случайного изменения прошивки */
        OBconfig.WRPState = OB_WRPSTATE_DISABLE;

        HAL_FLASHEx_OBProgram(&OBconfig);
        HAL_FLASH_OB_Lock();
        HAL_FLASH_Lock();
        /* Перезагрузка регистров option bytes */
        HAL_FLASH_OB_Launch();
    } else {
        printf("Защита flash памяти уже выключена, отмена команды\n");
    }
}


/**
 * \brief     Эта функция служит для стирания пользовательской прошивки из flash памяти
 */
// TODO: описание, h
void erase_program(void) {
    erase_flash(FLASH_SECTOR_NUMBER);
}

// TODO: описание, h
uint8_t check_key(void) {
    uint8_t connection_try = 0;
    uint8_t result = FALSE;

    uint32_t encrypted_test_word = 0;

    do {

        get_key_status_t get_key_result;
        uint8_t key_received_successfully = FALSE;

        /* Получаем пакет типа "key" */
        while (connection_try < MAX_USART_CONNECTION_TRY) {
            get_key_result = usart_get_key(&encrypted_test_word);

            if (get_key_result != GET_KEY_OK) {
                usart_send_status(STATUS_NACK);
            } else {
                usart_send_status(STATUS_ACK);
                key_received_successfully = TRUE;
                break;
            }

            connection_try++;
        }

        connection_try = 0;

        if (!key_received_successfully) {
            printf("Ошибка при получении пакета \"key\"\n");
            break;
        }

        /* Расшифровываем тестовое слово ключом шифрования */
        uint8_t temp_arr[NUMBER_OF_BYTES_TEST_WORD];
        temp_arr[0] = (encrypted_test_word >> 24) & 0xFF;
        temp_arr[1] = (encrypted_test_word >> 16) & 0xFF;
        temp_arr[2] = (encrypted_test_word >> 8) & 0xFF;
        temp_arr[3] = encrypted_test_word & 0xFF;

        decrypt_data(encryption_key, temp_arr, NUMBER_OF_BYTES_TEST_WORD);

        /* Делаем проверку тестового слова */
        uint8_t test_word_finded = find_word(temp_arr, test_word);

        if (test_word_finded) {
            usart_send_response(RESPONSE_OK);
            printf("Ключи шифрования соответствуют!\n");
            result = TRUE;
        } else {
            usart_send_response(RESPONSE_FAIL);
            printf("Ключи шифрования НЕ соответствуют!\n");
            break;
        }

    } while (0);

    return result;
}

/**
 * \brief     Эта функция служит для выполнения выбранной команды
 * \param[in] command: Какую команду нужно выполнить
 * \return    result: Результат выполнения команды: TRUE (команда выполнена успешно), FALSE (ошибка во время выполнения команды)
 */
uint8_t
execute_command(cmd_t command) {
    uint8_t result = FALSE;

    switch (command) {
        case CMD_UPDATE_FIRMWARE:
            printf("Выбрана команда для обновления прошивки, выполняю...\n");
            result = update_firmware();
            break;
        case CMD_SET_KEY:
            printf("Выбрана команда для задания пароля, выполняю...\n");
            result = set_key();
            break;
        case CMD_FLASH_OB_CHECK:
            printf("Выбрана команда для проверки защиты flash памяти, выполняю...\n");
            flash_ob_check();
            result = TRUE;
            break;
        case CMD_FLASH_LOCK:
            printf("Выбрана команда для установки защиты flash памяти, выполняю...\n");
            result = flash_lock();;
            break;
        case CMD_FLASH_UNLOCK:
            printf("Выбрана команда для снятия защиты flash памяти, выполняю...\n");
            flash_unlock();
            result = TRUE;
            break;
        case CMD_GET_ID:
            printf("Выбрана команда для получения UID, выполняю...\n");
            send_uid();
            result = TRUE;
            break;
        case CMD_CHECK_KEY:
            printf("Выбрана команда для проверки соответствия ключей шифрования, выполняю...\n");
            result = check_key();
            break;
        case CMD_ERASE_PROGRAM:
            printf("Выбрана команда для стирания пользовательской прошивки из flash памяти, выполняю...\n");
            erase_program();
            result = TRUE;
            break;
        default:
            printf("Неизвестная команда\n");
            break;
    }

    return result;
}
