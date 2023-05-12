import binascii

import serial
import sys
import struct
from serial import *
import serial.tools.list_ports
import crcmod
import time
import math

command_word = b"COMD"
header_word = b"HEAD"
data_word = b"DATA"
key_word = b"PASS"
response_word = b"RESP"
test_word = b"TEST"
ack_word = b"ACKW"
nack_word = b"NACK"

bootloader_responses = {"ok": 0xFFFFFFFF,
                        "fail": 0x33333333, }

persons = {
    0: "Разработчик",
    1: "Пользователь"
}

host_user_commands = {
    0: "Выход из программы",
    1: "Загрузить прошивку в микроконтроллер",
    2: "Узнать ID устройства"
}

host_developer_commands = {
    0: "Выход из программы",
    1: "Зашифровать прошивку",
    2: "Расшифровать прошивку",
    3: "Работа с загрузчиком",
}

# Этот словарь должен соответствовать enum cmd_t
host_developer_bootloader_commands = {
    0: "Выход из программы",
    1: "Загрузить прошивку в микроконтроллер",
    2: "Сменить ключ шифрования прошивки",
    3: "Проверить наличие защиты flash памяти",
    4: "Заблокировать flash память микроконтроллера",
    5: "Разблокировать flash память микроконтроллер",
    6: "Узнать UID микроконтроллера",
    7: "Проверить соответствие ключей шифрования",
    8: "Очистить flash память с прошивкой",
}

max_usart_connection_try = 10

# Аппаратный блок вычисления CRC в STM32F407 нельзя настроить, он всегда будет вычислять CRC32 по алгоритму MPEG-2
crc32mpeg2_func = crcmod.mkCrcFun(0x104c11db7, initCrc=0xFFFFFFFF, xorOut=0x0, rev=False)


def get_key(val, dictionary):
    for key, value in dictionary.items():
        if val == value:
            return key


def input_mode():
    print("Выберете человека, использующего загрузчик:")
    for mode_number, mode_name in persons.items():
        print(f"{mode_number}. {mode_name}")

    while True:
        input_number = input("Введите число: ")
        if not input_number.isnumeric():
            print("Ошибка: введите число")
            continue
        input_number = int(input_number)
        if input_number not in persons:
            print("Ошибка: такого числа нет")
            continue
        break

    return input_number


def input_user_bootloader_command():
    print("Команды загрузчика:")
    for command_number, command_description in host_user_commands.items():
        print(f"{command_number}. {command_description}")

    while True:
        command_number = input("Введите номер команды: ")
        if not command_number.isnumeric():
            print("Ошибка: введите число")
            continue
        command_number = int(command_number)
        if command_number not in host_user_commands:
            print("Ошибка: такой команды нет")
            continue
        break

    if command_number == 0:
        exit(1)

    return command_number


def developer_input_command():
    print("Режимы команды хост программы:")
    for mode_number, mode_description in host_developer_commands.items():
        print(f"{mode_number}. {mode_description}")

    while True:
        command_number = input("Введите номер команды: ")
        if not command_number.isnumeric():
            print("Ошибка: введите число")
            continue
        command_number = int(command_number)
        if command_number not in host_developer_commands:
            print("Ошибка: такой команды нет")
            continue
        break

    if command_number == 0:
        exit(1)

    return command_number


def input_developer_bootloader_command():
    print("Команды загрузчика:")
    for command_number, command_description in host_developer_bootloader_commands.items():
        print(f"{command_number}. {command_description}")

    while True:
        command_number = input("Введите номер команды: ")
        if not command_number.isnumeric():
            print("Ошибка: введите число")
            continue
        command_number = int(command_number)
        if command_number not in host_developer_bootloader_commands:
            print("Ошибка: такой команды нет")
            continue
        break

    if command_number == 0:
        exit(1)

    return command_number


def start_uart_connection():
    com_port_finded = False
    baudrate_finded = False
    supported_baudrates = [115200]

    try:
        # Получаем список всех доступных портов в системе
        ports = serial.tools.list_ports.comports()

        if len(ports) == 0:
            raise Exception('На компьютере не найден ни один COM-порт')
        else:
            print("Список доступных COM портов в системе:")
            for port in ports:
                print("Устройство: " + port.device + ". Название: " + port.name + ". Описание: " + port.description)

        while not com_port_finded:
            com_port = input(
                'Пожалуйста, введите название COM-порта (например COM1 для Windows или /dev/ttyS1 для Linux): ')
            com_port = com_port.strip()

            # Итерируемся по списку портов и выводим информацию о каждом из них
            for port in ports:
                if com_port == port.device:
                    com_port_finded = True
                    print('COM-порт найден:', com_port)

            if not com_port_finded:
                print('COM-порт не найден:', com_port)

        while not baudrate_finded:

            delimiter = ', '
            baudrate_string = delimiter.join(str(baudrate) for baudrate in supported_baudrates)

            input_baudrate = input(
                'Пожалуйста, выберете поддерживаемую скорость COM-порта из списка : ' + baudrate_string + '\n')
            input_baudrate = input_baudrate.strip()

            for baudrate in supported_baudrates:
                if int(input_baudrate) == baudrate:
                    baudrate_finded = True
                    print('Установлена скорость:', input_baudrate)

            if not baudrate_finded:
                print('Скорость не поддерживается:', input_baudrate)

        print(
            'Убедитесь, что UART устройства установлены со следующими настройками: World Length = 8 Bits; Parity = None; Stop Bits = 1;')

        serial_com = serial.Serial(com_port, input_baudrate, EIGHTBITS, PARITY_NONE, STOPBITS_ONE, timeout=1)

        if serial_com.is_open:
            print(f"UART соединение с {serial_com.name} успешно установлено!")
            print(f"Перезагрузите устройство, чтобы начать работать с загрузчиком")
        else:
            raise Exception(f"Ошибка установки UART соединения!")
        return serial_com

    except Exception as e:
        print("Произошла ошибка:", e)
        exit(0)


def wait_status(uart_serial):
    try:
        number_of_try = 0
        while number_of_try < max_usart_connection_try:
            response = uart_serial.readline().decode("utf-8").strip()
            index_ackw = response.find("ACKW")
            index_nack = response.find("NACK")
            if index_ackw != -1:
                return 1
            if index_nack != -1:
                return 0
            number_of_try = number_of_try + 1
        return 0
    except Exception as e:
        print(f"Ошибка wait_status: {e}")
        # TODO: нужно ли выходить из программы?


def send_header(uart_serial, firmware_size):
    try:
        header_data = firmware_size.to_bytes(4, "big")
        header_crc = struct.pack('>I', crc32mpeg2_func(header_word + header_data))
        header_packet = header_word + header_data + header_crc
        uart_serial.write(header_packet)
    except Exception as e:
        print(f"Ошибка send_header: {e}")
        exit(0)


def wait_response(uart_serial):
    number_of_try = 0
    try:
        while number_of_try < max_usart_connection_try:
            number_of_try = number_of_try + 1

            response_packet = uart_serial.readline()
            index = response_packet.find(response_word, 0)
            if index != -1:
                response_data = int.from_bytes(response_packet[4:8], byteorder="little")
                if response_data == bootloader_responses["ok"]:
                    print("Ответ - успех")
                    break
                elif response_data == bootloader_responses["fail"]:
                    raise Exception("Ответ - неудача")
                else:
                    return hex(response_data)
            if number_of_try + 1 == max_usart_connection_try:
                raise ValueError("За все попытки подключения получаем неизвестный ответ")
            number_of_try = number_of_try + 1
    except ValueError as e:
        print(f"Ошибка wait_response: {e}")
        exit(0)


def send_data(uart_serial, data_blocks):
    try:
        number_of_try = 0
        for data_block in data_blocks:
            number_of_block = 0
            while number_of_try < max_usart_connection_try:
                time.sleep(0.6)
                data_crc = struct.pack('>I', crc32mpeg2_func(data_word + data_block[1]))
                data_packet = data_word + data_block[1] + data_crc
                uart_serial.write(data_packet)
                status_result = wait_status(uart_serial)

                if status_result == 1:
                    print(f"[{number_of_block + 1} / {len(data_blocks)}]")
                    number_of_try = 0
                    break
                elif number_of_try + 1 == max_usart_connection_try:
                    return 0

                number_of_try = number_of_try + 1
        return 1
    except Exception as e:
        print(f"Ошибка send_data: {e}")
        exit(0)


def wait_bootloader_mode(uart_serial):
    # Ждем нажатия пользовательской кнопки
    number_of_try = 0
    try:
        while number_of_try < max_usart_connection_try:
            response = uart_serial.readline().decode('utf-8').strip()
            if response != "":
                print('Ответ загрузчика: {}'.format(response))
            if response == 'Кнопка User была нажата, переходим в режим загрузчика':
                print("Кнопка User была нажата, входим в режим загрузчика")
                break
            elif response == 'Кнопка User не нажата, переходим к исполнению пользовательского приложения':
                for i in range(10):
                    response = uart_serial.readline().decode("utf-8").strip()
                    if response != "":
                        print('Ответ загрузчика: {}'.format(response))
                raise Exception("Кнопка User не была нажата")
            elif number_of_try + 1 == max_usart_connection_try:
                raise Exception("Загрузчик не отвечает")
            number_of_try = number_of_try + 1
    except Exception as e:
        print("Ошибка wait_bootloader_mode:", e)
        exit(0)


def encrypt_firmware_file(encrypt_key):
    try:
        filepath = input("Введите путь до прошивки формата .bin: ").strip()
        with open(filepath, 'rb') as firmware_file:
            firmware_data = firmware_file.read()
            encrypted_data = bytearray(firmware_data)
            for i in range(0, len(firmware_data), 4):
                encrypted_data[i:i + 4] = bytes(
                    [b ^ (encrypt_key >> (8 * (j % 4))) & 0xFF for j, b in enumerate(firmware_data[i:i + 4])])
            encrypted_filename = os.path.splitext(filepath)[0] + '_encrypted.bin'
            with open(encrypted_filename, 'wb') as encrypted_file:
                encrypted_file.write(encrypted_data)
            print(f"Файл {filepath} успешно зашифрован в файл {encrypted_filename}")
    except FileNotFoundError:
        print(f"Ошибка encrypt_firmware_file: файл '{filepath}' не найден")
    except PermissionError:
        print(f"Ошибка encrypt_firmware_file: доступ к файлу '{filepath}' запрещен")
    except Exception as e:
        print(f"Ошибка encrypt_firmware_file: {e}")


def decrypt_firmware_file(encrypt_key):
    try:
        filepath = input("Введите путь до зашифрованной прошивки формата .bin: ").strip()
        with open(filepath, 'rb') as encrypted_file:
            encrypted_data = encrypted_file.read()
            decrypted_data = bytearray(encrypted_data)
            for i in range(0, len(encrypted_data), 4):
                decrypted_data[i:i + 4] = bytes(
                    [b ^ (encrypt_key >> (8 * (j % 4))) & 0xFF for j, b in enumerate(encrypted_data[i:i + 4])])
            decrypted_filename = os.path.splitext(filepath)[0] + '_decrypted.bin'
            with open(decrypted_filename, 'wb') as decrypted_file:
                decrypted_file.write(decrypted_data)
            print(f"Файл {filepath} успешно расшифрован в файл {decrypted_filename}")
    except FileNotFoundError:
        print(f"Ошибка decrypt_firmware_file: файл '{filepath}' не найден")
    except PermissionError:
        print(f"Ошибка decrypt_firmware_file: доступ к файлу '{filepath}' запрещен")
    except Exception as e:
        print(f"Ошибка decrypt_firmware_file: {e}")


def send_command(uart_serial, command):
    try:
        cmd_cmd = struct.pack(">i", command)
        cmd_crc = struct.pack('>I', crc32mpeg2_func(command_word + cmd_cmd))
        cmd_packet = command_word + cmd_cmd + cmd_crc
        uart_serial.write(cmd_packet)
    except Exception as e:
        print(f"Ошибка send_command: {e}")
        exit(0)


def input_key():
    while True:
        user_input = input("Введите 4 байтный ключ шифрования (например, '01020304'): ")
        # Проверить, что введенные данные состоят из 8 шестнадцатеричных символов
        if len(user_input) != 8 or not all(c in "0123456789abcdefABCDEF" for c in user_input):
            print("Ошибка: введите 4 байта в виде 8 шестнадцатеричных символов (например, '01020304').")
        else:
            # Преобразовать введенные данные в беззнаковое 32-битное целое число
            return int(user_input, 16)


def open_encrypted_firmware():
    # Число байт прошивки в блоке "данные"
    number_of_bytes_in_data_data = 1024
    result_data_blocks_to_send = []
    try:
        path_to_bin_file = input("Введите путь до прошивки формата .bin: ").strip()
        # path_to_bin_file = "test_app_red_11_04.bin"
        with open(path_to_bin_file, 'rb') as firmware_file:
            print("Файл прошивки успешно открыт, делим ее на блоки")
            firmware_data = firmware_file.read()
            firmware_size = len(firmware_data)
            print("Размер прошивки: " + str(firmware_size) + " байт")
            print("Количество блоков по " + str(number_of_bytes_in_data_data) + " байт: " + str(
                math.ceil(firmware_size / number_of_bytes_in_data_data)))

            for i in range(0, firmware_size, number_of_bytes_in_data_data):
                block_data = firmware_data[i:i + number_of_bytes_in_data_data]
                # Если файл нельзя разделить на блоки нацело, то заполняем оставшиеся в последнем блоке байты
                # числом 0xFF, что является обозначением "чистой" памяти в STM32
                if len(block_data) < number_of_bytes_in_data_data:
                    block_data = block_data.ljust(number_of_bytes_in_data_data, b'\xFF')
                crc = crc32mpeg2_func(data_word + block_data)
                result_data_blocks_to_send.append([data_word, block_data, crc])
        return result_data_blocks_to_send, firmware_size
    except FileNotFoundError:
        print(f"Ошибка open_and_encrypt_firmware: файл '{path_to_bin_file}' не найден")
        exit(0)
    except PermissionError:
        print(f"Ошибка open_and_encrypt_firmware: доступ к файлу '{path_to_bin_file}' запрещен")
        exit(0)
    except Exception as e:
        print(f"Ошибка open_and_encrypt_firmware: {e}")
        exit(0)


def update_firmware_command(uart_serial):
    try:
        number_of_try_connection = 0

        result_data_blocks_to_send, firmware_size = open_encrypted_firmware()
        print("Отправляю заголовок с размером прошивки")
        while number_of_try_connection < max_usart_connection_try:
            send_header(uart_serial, firmware_size)
            status_of_send_header = wait_status(uart_serial)
            if status_of_send_header == 1:
                print("Заголовок передан успешно")
                break
            elif status_of_send_header == 0 and (number_of_try_connection + 1 == max_usart_connection_try):
                raise Exception("Ошибка при передачи заголовка")
            else:
                number_of_try_connection += 1

        print("Ожидаю ответа о наличии свободного места во flash памяти")
        wait_response(uart_serial)

        print("Начинаю передачу прошивки")
        send_data_result = send_data(uart_serial, result_data_blocks_to_send)

        for i in range(10):
            response = uart_serial.readline().decode("utf-8").strip()
            if response == "Прошивка запрограммирована успешно!":
                print("Прошивка успешно запрограммирована!")
            if response == "Перезагрузка МК!":
                print("Перезагрузка микроконтроллера")

        if send_data_result != 1:
            raise Exception("Ошибка при передаче прошивки")

    except Exception as e:
        print(f"Ошибка update_firmware_command: {e}")
        exit(0)


def send_key(uart_serial, key):
    try:
        key_crc = struct.pack('>I', crc32mpeg2_func(key_word + key))
        key_packet = key_word + key + key_crc
        uart_serial.write(key_packet)
    except Exception as e:
        print(f"Ошибка при отправке пакета \"key\": {e}")
        exit(0)


def set_key_command(uart_serial):
    try:
        number_of_try_connection = 0

        uid = get_uid_command(uart_serial)
        secret_encryption_key = bytes.fromhex(uid[1][2:]) # убираем первые 2 символа "0x"
        secret_encryption_key = int.from_bytes(secret_encryption_key, byteorder="big")
        developer_input_key = input_key()

        developer_input_key = developer_input_key.to_bytes(4, "big")

        # Шифруем ключ при помощи secret_encryption_key
        encrypted_developer_key = bytes(
            [(b ^ (secret_encryption_key >> (8 * (j % 4))) & 0xFF) for j, b in enumerate(developer_input_key)])


        status_result = 0

        while number_of_try_connection < max_usart_connection_try:
            send_key(uart_serial, encrypted_developer_key)
            status_result = wait_status(uart_serial)
            if status_result == 1:
                print("Пакет \"key\" передан успешно!")
                break
            else:
                number_of_try_connection += 1

        if status_result == 0:
            raise Exception("Ошибка при передаче ключа прошивки")

        print("Ожидаю ответа о результате установки ключа шифрования")
        wait_response(uart_serial)

        print_developer_input_key = hex(int.from_bytes(developer_input_key, byteorder="big"))
        print(f"Ключ шифрования - {print_developer_input_key} для МК с UID - {uid[0]}-{uid[1]}-{uid[2]} установлен")

    except Exception as e:
        print(f"Ошибка set_key_command: {e}")
        exit(0)


def check_key_command(uart_serial):
    try:
        number_of_try_connection = 0

        develiper_input_key = input_key()

        # Шифруем тестовое слово полученным ключом шифрования
        encrypted_test_word = bytes(
            [(b ^ (develiper_input_key >> (8 * (j % 4))) & 0xFF) for j, b in enumerate(test_word)])

        status_result = 0

        while number_of_try_connection < max_usart_connection_try:
            send_key(uart_serial, encrypted_test_word)
            status_result = wait_status(uart_serial)
            if status_result == 1:
                print("Пакет \"key\" передан успешно!")
                break
            else:
                number_of_try_connection += 1

        if status_result == 0:
            raise Exception("Ошибка при передаче ключа прошивки")

        print("Ожидаю ответа о соответствии ключей шифрования")
        wait_response(uart_serial)

    except Exception as e:
        print(f"Ошибка check_key_command: {e}")
        exit(0)


def flash_ob_check_command(uart_serial):
    try:
        number_of_try = 0
        while number_of_try < 5:
            response = uart_serial.readline().decode("utf-8").strip()
            if response != "":
                print('Ответ загрузчика: {}'.format(response))
                number_of_try += 1
    except Exception as e:
        print(f"Ошибка flash_ob_check_command: {e}")


def flash_lock_command(uart_serial):
    try:
        number_of_try = 0
        while number_of_try < 5:
            response = uart_serial.readline().decode("utf-8").strip()
            if response != "":
                print('Ответ загрузчика: {}'.format(response))
                number_of_try += 1
    except Exception as e:
        print(f"Ошибка flash_lock_command: {e}")


def flash_unlock_command(uart_serial):
    try:
        number_of_try = 0
        while number_of_try < 5:
            response = uart_serial.readline().decode("utf-8").strip()
            if response != "":
                print('Ответ загрузчика: {}'.format(response))
                number_of_try += 1
    except Exception as e:
        print(f"Ошибка flash_unlock_command: {e}")


def get_uid_command(uart_serial):
    try:
        number_of_try_connection = 0

        uid_1 = wait_response(uart_serial)
        uid_2 = wait_response(uart_serial)
        uid_3 = wait_response(uart_serial)

        if uid_1 is not None and uid_2 is not None and uid_3 is not None:
            print(f"UID = {uid_1}-{uid_2}-{uid_3}")
            return uid_1, uid_2, uid_3
        else:
            raise ValueError("UID не может быть пустым")

    except Exception as e:
        print(f"Ошибка uart_serial: {e}")
        exit(0)


def erase_program_command(uart_serial):
    try:
        number_of_try = 0
        while number_of_try < 5:
            response = uart_serial.readline().decode("utf-8").strip()
            if response != "":
                print('Ответ загрузчика: {}'.format(response))
                number_of_try += 1
    except Exception as e:
        print(f"Ошибка erase_program_command: {e}")


def execute_develop_bootloader_command(uart_serial, command):
    # Должен соответствовать host_developer_bootloader_commands
    handlers = {
        1: update_firmware_command,
        2: set_key_command,
        3: flash_ob_check_command,
        4: flash_lock_command,
        5: flash_unlock_command,
        6: get_uid_command,
        7: check_key_command,
        8: erase_program_command,
    }

    try:
        handler = handlers.get(command, None)
        if handler is None:
            raise ValueError("Ошибка при выполнении команды - неизвестная команда")

        handler(uart_serial)
    except Exception as e:
        print(f"Ошибка execute_develop_bootloader_command: {e}")
        exit(0)


def execute_user_bootloader_command(uart_serial, command):
    handlers = {
        1: update_firmware_command,
        2: get_uid_command,
    }

    try:
        handler = handlers.get(command, None)
        if handler is None:
            raise ValueError("Ошибка при выполнении команды - неизвестная команда")

        handler(uart_serial)
    except Exception as e:
        print(f"Ошибка execute_user_bootloader_command: {e}")
        exit(0)
