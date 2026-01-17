#pragma once

#include "../Encryption/EndPoint.hpp"
#include <iostream>

bool rsa_test(bool logs) {
    std::cout << "Выполняется проверка RSA..." << std::endl;
    
    PulsarCrypto::Asymmetrical::RSA::Generator client1_keys;
    PulsarCrypto::Asymmetrical::RSA::Generator client2_keys;

    auto public_1 = client1_keys.getPublic();
    auto public_2 = client2_keys.getPublic();

    auto private_1 = client1_keys.getPrivate();
    auto private_2 = client2_keys.getPrivate();

    if (logs) {
        std::cout << "Клиент 1:";
        std::cout << "\n\tОткрытый: " << public_1;
        std::cout << "\n\tЗакрытый: " << private_1;

        std::cout << "\nКлиент 2:";
        std::cout << "\n\tОткрытый: " << public_2;
        std::cout << "\n\tЗакрытый: " << private_2;
    }

    std::string raw_1 = "Hello, from client 1!";
    auto enc_to_2 = PulsarCrypto::Asymmetrical::encrypt(PulsarCrypto::to_bytes(raw_1), public_2);

    if (logs) {
        std::cout << "\nОтправлено с клиента 1:";
        std::cout << "\n\tСообщение: " << raw_1;
        std::cout << "\n\tЗашифрованное (HEX): ";
        for (auto i : enc_to_2) std::cout << std::hex << i << ' ';
    }

    std::string raw_2 = "Hello, from client 2!";
    auto enc_to_1 = PulsarCrypto::Asymmetrical::encrypt(PulsarCrypto::to_bytes(raw_2), public_1);

    if (logs) {
        std::cout << "\nОтправлено с клиента 2:";
        std::cout << "\n\tСообщение: " << raw_2;
        std::cout << "\n\tЗашифрованное (HEX): ";
        for (auto i : enc_to_1) std::cout << std::hex << i << ' ';
    }

    auto dec_1 = PulsarCrypto::from_bytes(PulsarCrypto::Asymmetrical::decrypt(enc_to_1, private_1));
    if (logs) std::cout << "\nПолучено клиентом 1 (расшифрованно): " << dec_1;

    auto dec_2 = PulsarCrypto::from_bytes(PulsarCrypto::Asymmetrical::decrypt(enc_to_2, private_2));
    if (logs) std::cout << "\nПолучено клиентом 2 (расшифрованно): " << dec_2;

    if (raw_1 == dec_2 && raw_2 == dec_1) {
        std::cout << "\nТест RSA пройден" << std::endl;
        return true;
    } else {
        std::cout << "\nТест RSA не пройден" << std::endl;
        return false;
    }
}