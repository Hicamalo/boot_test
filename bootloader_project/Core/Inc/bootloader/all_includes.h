/**
 * \file            all_includes.h
 * \brief           Файл, в котором в правильном порядке структурированы #include
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

#ifndef BOOTLOADER_PROJECT_ALL_INCLUDES_H
#define BOOTLOADER_PROJECT_ALL_INCLUDES_H

#include <stdio.h>
#include <string.h>

#include "stm32f4xx.h"
#include "stm32f4xx_hal_rcc.h"

#include "bootloader_types.h"
#include "bootloader_settings.h"
#include "bootloader_words.h"

#include "bootloader_utilities.h"
#include "bootloader_uart.h"
#include "bootloader_flash.h"
#include "bootloader_execution.h"

#endif //BOOTLOADER_PROJECT_ALL_INCLUDES_H
