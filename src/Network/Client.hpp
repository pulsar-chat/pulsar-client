#pragma once

#include <SFML/Network.hpp>
#include <iostream>
#include <thread>
#include <list>
#include <atomic>
#include <string>
#include "../lib/hash.h"
#include "../Other/Datetime.hpp"
#include "../Other/Chat.hpp"
#include "../Graphics/Window.hpp"
#include "../Other/Message.hpp"
#include "../Console/Console.hpp"
#include "../API/PulsarAPI.hpp"
#include <memory>

class Client {
private:
    std::string name, password;
    std::string ip;
    unsigned short port;

    std::shared_ptr<sf::TcpSocket> socket;
    std::shared_ptr<PulsarAPI> api;

    std::unique_ptr<Console> console;

    std::string dest = ":all";
public:
    Client(const std::string& name, const std::string& password_unhashed, const std::string& ip, unsigned short port)
     : ip(ip), port(port), name(name), password(hash(password_unhashed)) {
        api = std::make_shared<PulsarAPI>(socket, name);
        console = std::make_unique<Console>(api, dest, this->name);
    }

    ~Client() {
        api->disconnect();
    }

    void run() {
        if (!api->connect(ip, port)) return;
        api->startRecieverLoop();
        
        auto login = api->login(password);

        switch (login) {
            case PulsarAPI::Success: break;

            case PulsarAPI::Fail_Password: {
                std::cout << "Ошибка входа: неправильный пароль." << std::endl;
                return;
            } break;

            case PulsarAPI::Fail_Username: {
                std::cout << "Ошибка входа: неверное имя пользователя.\nЗарегистрировать нового пользвотеля? (y/N) ";
                std::string ans_;
                std::getline(std::cin, ans_);
                char ans = ans_[0];
                ans = ::tolower(ans);

                if (ans == 'n' || ans == '\n') return;
                else if (ans == 'y') {
                    api->registerUser(password);
                    break;
                }
                else return;
            } break;

            case PulsarAPI::Fail_Unknown: {
                std::cout << "Ошибка входа: неизвестная ошибка." << std::endl;
            } break;
        }

        api->requestUnread();

        std::cout << "Вы вошли в Pulsar как " << name << "." << std::endl;
        
        console->displayUnreadMessages();

        std::string message;
        while (api->isConnected()) {
            std::cout << "[" << name << "](" << dest << "): ";
            std::getline(std::cin, message);

            if (message[0] == '!') {
                console->run(message);
                continue;
            }
            if (!message.empty()) api->send(message, dest);

            sf::sleep(sf::milliseconds(20));
        }
    }
};