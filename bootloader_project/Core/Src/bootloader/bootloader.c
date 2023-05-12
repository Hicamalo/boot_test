/**
 * \file            bootloader.c
 * \brief           Основной файл загрузчика
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

#include <bootloader/bootloader.h>

const char command_word[] = "COMD\0";
const char header_word[] = "HEAD\0";
const char response_word[] = "RESP\0";
const char data_word[] = "DATA\0";
const char key_word[] = "PASS\0";
const char test_word[] = "TEST\0";
const char ack_word[] = "ACKW\0";
const char nack_word[] = "NACK\0";

/* После обновления проекта через CubeMX, необходимо удалять данные структуры
 * из функции main.c, чтобы избежать ошибки множественного определения */
UART_HandleTypeDef huart3; /* Структура, необходимая для передачи данных по UART */
CRC_HandleTypeDef hcrc; /* Структура, необходимая для аппаратного вычисления CRC32 */

uint32_t encryption_key = 0; /* Ключ шифрования прошивки */
uint32_t secret_encryption_key = 0; /* Ключ шифрования, использующийся для передачи пакета типа "key" */

/**
 * \brief       Функция, которая проверяет нажатие пользовательской клавиши в течение 10 секунд.
 * \return      flag: Результат: TRUE (кнопка нажата), FALSE (кнопка не нажата).
 */
uint8_t
check_bootloader_mode(void) {
    uint8_t flag = FALSE;
    GPIO_PinState user_btn_pin_state;
    uint32_t end_tick = HAL_GetTick() + 10000;

    printf("Нажмите кнопку User в течение 10 секунд чтобы перейти в режим загрузчика\n");
    do {
        user_btn_pin_state = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0);
        uint32_t current_tick = HAL_GetTick();

        /* Проверяем нажатие кнопки User в течение 10 секунд */
        if (user_btn_pin_state == GPIO_PIN_SET) {
            printf("Кнопка User была нажата, переходим в режим загрузчика\n");
            flag = TRUE;
            break;
        }
        if (current_tick > end_tick) {
            printf("Кнопка User не нажата, переходим к исполнению пользовательского приложения\n");
            break;
        }

    } while (1);

    return flag;
}

/**
 * \brief     Эта основная функция загрузчика
 */
void
bootloader_mode(void) {
    uint32_t connection_try = 0;

    do {
        /* Получаем настройки шифрования загрузчика */
        uint8_t get_settings_successfully = FALSE;
        uint8_t set_settings_by_default = FALSE;
        get_settings_successfully = check_settings();

        if (!get_settings_successfully) {
            printf("Ошибка получения ключей шифрования при запуске\n");
            printf("Происходит сброс до заводских настроек\n");

            erase_flash(FLASH_SECTOR_NUMBER);
            /* Устанавливаем настройки шифрования по умолчанию */
            set_settings_by_default = set_settings(DEFAULT_ENCRYPTION_KEY);

            if (!set_settings_by_default) {
                printf("Ошибка установки настроек по умолчанию!");
                break;
            }
        }

        cmd_t command = CMD_UNKNOWN;
        get_cmd_status_t get_cmd_result;
        uint8_t cmd_received_successfully = FALSE;

        /* Получаем пакет типа "command" */
        while (connection_try < MAX_USART_CONNECTION_TRY) {
            get_cmd_result = usart_get_cmd(&command);

            if (get_cmd_result != GET_CMD_OK) {
                usart_send_status(STATUS_NACK);
            } else if (command == CMD_UNKNOWN) {
                usart_send_status(STATUS_NACK);
            } else {
                usart_send_status(STATUS_ACK);
                cmd_received_successfully = TRUE;
                break;
            }

            connection_try++;
        }

        connection_try = 0;

        if (!cmd_received_successfully) {
            printf("Ошибка при получении пакета \"command\"\n");
            break;
        }

        /* Выполняем полученную команду */
        uint8_t execution_result = FALSE;
        execution_result = execute_command(command);

        if (!execution_result) {
            printf("Ошибка выполнения команды!\n");
            break;
        }

    } while (0);

    /* Программно перезагружаем микроконтроллер */
    restart();
}
