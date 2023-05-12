import time

import serial
from serial import *

from uart_functions import *

try:
    # Ждем выбор режима пользователь\разработчик
    person = input_mode()

    if person == get_key("Разработчик", persons):
        # Режим разработчика
        developer_command = developer_input_command()
        if developer_command == get_key("Зашифровать прошивку", host_developer_commands):
            key = input_key()
            encrypt_firmware_file(key)
        elif developer_command == get_key("Расшифровать прошивку", host_developer_commands):
            key = input_key()
            decrypt_firmware_file(key)
        elif developer_command == get_key("Работа с загрузчиком", host_developer_commands):

            # Устанавливаем uart соединение
            ser = start_uart_connection()

            # Ждем перехода прошивки в режим загрузчика
            wait_bootloader_mode(ser)

            # Разработчик вводит команду для загрузчика
            command = input_developer_bootloader_command()

            # Ждем ответа о принятии команды (статуса)
            status_result = 0
            number_of_try_connection = 0

            while number_of_try_connection < max_usart_connection_try:
                send_command(ser, command)
                status_result = wait_status(ser)
                if status_result == 1:
                    print("Команда передана успешно")
                    break
                else:
                    number_of_try_connection += 1

            if status_result == 0:
                raise Exception("Загрузчик не отвечает пакетом \"статус\"")

            number_of_try_connection = 0

            time.sleep(1)
            execute_develop_bootloader_command(ser, command)

        else:
            raise Exception("Ошибка при выборе команды хост программы")
    elif person == get_key("Пользователь", persons):
        # Режим пользователя

        # Устанавливаем uart соединение
        ser = start_uart_connection()

        # Ждем перехода прошивки в режим загрузчика
        wait_bootloader_mode(ser)

        # Пользователь вводит команду для загрузчика
        command = input_user_bootloader_command()

        command_to_bootloader = 0
        # Соотносим словари host_developer_bootloader_commands и host_user_commands
        if command == 1:
            command_to_bootloader = 1
        if command == 2:
            command_to_bootloader = 6

        # Ждем ответа о принятии команды (статуса)
        status_result = 0
        number_of_try_connection = 0

        while number_of_try_connection < max_usart_connection_try:
            send_command(ser, command_to_bootloader)
            status_result = wait_status(ser)
            if status_result == 1:
                print("Команда передана успешно")
                break
            else:
                number_of_try_connection += 1

        if status_result == 0:
            raise Exception("Загрузчик не отвечает пакетом \"статус\"")

        number_of_try_connection = 0

        time.sleep(1)
        execute_user_bootloader_command(ser, command)


    else:
        raise Exception("Ошибка при выборе человека, использующего хост программу")

except Exception as e:
    print(f"Ошибка main: {e}")
    exit(0)

# for i in range(100):
#     response = ser.readline().decode("utf-8").strip()
#     if response != "":
#         print('Ответ загрузчика: {}'.format(response))
print("Закрываю программу")
