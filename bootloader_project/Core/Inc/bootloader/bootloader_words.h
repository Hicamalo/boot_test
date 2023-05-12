
/**
 * \file            bootloader_words.h
 * \brief           Файл с настройками обозначений пакетов
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

#ifndef BOOTLOADER_WORDS_H
#define BOOTLOADER_WORDS_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "all_includes.h"

/* + 2 байта нужно, чтобы учитывать нуль-терминатор */
extern const char command_word[NUMBER_OF_BYTES_COMMAND_WORD + 2];   /* Начало пакета типа "command" */
extern const char header_word[NUMBER_OF_BYTES_HEADER_WORD + 2];     /* Начало пакета типа "header" */
extern const char response_word[NUMBER_OF_BYTES_RESPONSE_WORD + 2]; /* Начало пакета типа "response" */
extern const char data_word[NUMBER_OF_BYTES_DATA_WORD + 2];         /* Начало пакета типа "data" */
extern const char key_word[NUMBER_OF_BYTES_KEY_WORD + 2];           /* Начало пакета типа "key" */
extern const char test_word[NUMBER_OF_BYTES_TEST_WORD + 2];           /* Слово для проверки ключей шифрования */

/* Условный пакет типа "status" */
/* + 2 байта нужно, чтобы учитывать нуль-терминатор */
extern const char ack_word[NUMBER_OF_BYTES_ACK + 2];   /* Начало пакета типа "ackw" */
extern const char nack_word[NUMBER_OF_BYTES_NACK + 2]; /* Начало пакета типа "nack" */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif //BOOTLOADER_WORDS_H
