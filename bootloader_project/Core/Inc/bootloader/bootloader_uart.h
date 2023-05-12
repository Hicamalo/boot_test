/**
 * \file            bootloader_uart.h
 * \brief           Заголовочный файл, для bootloader_uart.c
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

#ifndef BOOTLOADER_PROJECT_BOOTLOADER_UART_H
#define BOOTLOADER_PROJECT_BOOTLOADER_UART_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "all_includes.h"

extern void usart_send_response(uint32_t response_data);
extern void usart_send_status(status_t status);
extern get_cmd_status_t usart_get_cmd(cmd_t* received_command);
extern get_header_status_t usart_get_header(uint32_t* firmware_size);
extern get_data_status_t usart_get_data(uint8_t* data_buffer);
extern get_key_status_t usart_get_key(uint32_t* coded_key);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif //BOOTLOADER_PROJECT_BOOTLOADER_UART_H
