#include "defines"
#include "API/PulsarAPI.hpp"
#include "Network/Client.hpp"
#include "Network/Encryption.hpp"
#include "Network/Checker.hpp"
#include <exception>
#include <csignal>

#ifdef _WIN32
#   include <windows.h>
#endif

#ifndef PULSAR_CHECKER_VERSION
#   error Pulsar Checker not included!
#endif

int main(int argc, const char **argv) {
#ifdef _WIN32
    SetConsoleCP(65001); // Russian UTF-8 support
    SetConsoleOutputCP(65001);
#endif
    std::string serverIP;
    std::string name;
    std::string password;
#ifndef PULSAR_IP_PRESET
    std::cout << "Введите IP-адрес сервера (по умолчанию 127.0.0.1): ";
    std::getline(std::cin, serverIP);

    if (serverIP.empty())
    {
        serverIP = "127.0.0.1";
    }
#else
    serverIP = PULSAR_IP_PRESET;
#endif

    if (argc <= 1) {
        std::cout << "Введите имя пользователя: ";
        std::getline(std::cin, name);
    }
    else {
        name = "@" + std::string(argv[1]);
    }
    std::cout << "Введите пароль (нажмите ENTER если его нет): ";
    std::getline(std::cin, password);

    if (name[0] != '@')
        name = "@" + name;
    for (auto &c : name) c = tolower(c);

#ifdef PULSAR_RSA_TEST
    if (!rsa_test(PULSAR_RSA_TEST)) return -1;
#endif

    Client client(name, password, serverIP, PULSAR_PORT);
    client.run();

    std::cout << "Клиент завершил свою работу." << std::endl;

    return 0;
}