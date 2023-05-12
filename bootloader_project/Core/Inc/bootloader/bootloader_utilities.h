/**
 * \file            bootloader_utilities.h
 * \brief           Заголовочный файл, для bootloader_utilities.c
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

#ifndef BOOTLOADER_PROJECT_BOOTLOADER_UTILITIES_H
#define BOOTLOADER_PROJECT_BOOTLOADER_UTILITIES_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "all_includes.h"

extern uint32_t four_uint8t_to_one_uint32t(uint8_t* p_bytes);
extern void decrypt_data(uint32_t key, uint8_t* data, size_t size);
extern uint8_t find_word(uint8_t* source, const char* destination);
extern uint8_t check_crc(uint8_t* data, size_t data_size, uint32_t input_crc);
extern cmd_t check_cmd_type(uint32_t cmd_type_bytes);
extern void restart(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif //BOOTLOADER_PROJECT_BOOTLOADER_UTILITIES_H
