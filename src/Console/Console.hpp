#pragma once

#include <iostream>
#include <string>
#include "console_defines"
#include "Fastfetch.hpp"
#include "../defines"
#include "../API/PulsarAPI.hpp"

class Console {
private:
    std::shared_ptr<PulsarAPI> api;
    std::string& dest;
    std::string& name;

    static bool checkLength(const std::string& str, size_t min) {
        return !(str.size() >= min);
    }
public:
    Console(std::shared_ptr<PulsarAPI> api_ptr, std::string& dest_ref, std::string& name_ref)
     : api(api_ptr), dest(dest_ref), name(name_ref) {}

    static void clear() {
        #ifdef _WIN32
            std::system("cls");
        #else
            std::system("clear");
        #endif
    }

    static void setTitle(const std::string& title) {
        #ifdef _WIN32
            std::string command = "title " + title;
            std::system(command.c_str());
        #else
            std::cout << "\033]0;" << title << "\007";
        #endif
    }

    void displayUnreadMessages() {
        auto msgs = api->getUnread();
        if (msgs.size() == 0) {
            std::cout << "У вас нет непрочитанных сообщений." << std::endl;
            return;
        }
        std::cout << "У вас есть " << msgs.size() << " непрочитанных сообщений: " << std::endl;
        for (auto i : msgs) {
            std::cout << i << std::endl;
        }
    }

    int run(const std::string& command) {
        auto split_view = split(command);
        auto com = split_view[0];
        if (com == "!exit") {
            api->disconnect();
        }
        else if (com == "!dest") {
            dest = split_view[1];
        }
        else if (com == "!join") {
            if (api->joinChannel(split_view[1])) {
                std::cout << "Вы успешно присоеденились к каналу '" << split_view[1] << "'." << std::endl;
            } else std::cout << "Не удалось присоедениться к каналу '" << split_view[1] << "'." << std::endl;
        } 
        else if (com == "!leave") {
            if (api->leaveChannel(split_view[1])) {
                std::cout << "Вы успешно покинули канал '" << split_view[1] << "'." << std::endl;
            } else std::cout << "Не удалось покинуть канал '" << split_view[1] << "'." << std::endl;
        }
        else if (com == "!create") {
            if (api->createChannel(split_view[1])) {
                std::cout << "Создан канал '" << split_view[1] << "'." << std::endl;
            } else std::cout << "Не удалось создать канал '" << split_view[1] << "'." << std::endl;
        }
        else if (com == "!chat") {
            auto chat = api->getChat(split_view[1]);
            std::cout << "Чат '" << split_view[1] << "':\n" << chat.to_stream().rdbuf();
        }
        else if (com == "!profile") {
            auto arg = split_view[1];

            if (arg == "edit") {
                std::string description, email, realName, birthday;

                std::cout << "Введите новое описание: ";
                std::getline(std::cin, description);
                std::cout << "Введите новый Email: ";
                std::getline(std::cin, email);
                std::cout << "Введите новое имя: ";
                std::getline(std::cin, realName);
                std::cout << "Введите новый день рождения (одно число): ";
                std::getline(std::cin, birthday);

                Profile p { description, email, realName, Datetime(std::stol(birthday)) };
                
                if (api->updateProfile(p)) {
                    std::cout << "Профиль обновлен." << std::endl;
                } else std::cout << "Не удалось обновить профиль." << std::endl;
            } else {
                auto profile = api->getProfile(arg);

                std::cout << "Профиль '" << arg << "':";
                std::cout << "\n\tОписание: " << profile.description();
                std::cout << "\n\tEmail: " << profile.email();
                std::cout << "\n\tИмя: " << profile.realName();
                std::cout << "\n\tДень рождения: " << profile.birthday().toTime() << std::endl;
            }
        }
        else if (com == "!contact") {
            auto act = split_view[1];
            auto username = split_view[2];

            if (act == "add") {
                if (api->createContact(username, split_view[3])) {
                    std::cout << "Создан контакт '" << split_view[3] << "'." << std::endl;
                } else std::cout << "Не удалось создать контакт '" << split_view[3] << "'." << std::endl;
            } else if (act == "rem") {
                if (api->removeContact(username)) {
                    std::cout << "Удален контакт для '" << username << "'." << std::endl;
                } else std::cout << "Не удалось удалить контакт для '" << username << "'." << std::endl;
            }
        }
        else if (com == "!unread") {
            displayUnreadMessages();
        }
        else if (com == "!read") {
            if (split_view.size() == 2 && split_view[1] == "all") api->readAll();

            auto chat = split_view[1];
            auto arg = split_view[2];

            if (arg == "all") {
                api->readAll(chat);
            } else {
                api->read(chat, std::stoll(arg));
            }
        }
        else if (command == "!fastfetch") {
            const std::vector<std::string> info = {
                "Pulsar Client " + std::string(PULSAR_VERSION),
                "Пользователь: " + name,
                "Сервер: " + api->getSocket()->getRemoteAddress()->toString() + ":" + std::to_string(api->getSocket()->getRemotePort())
            };
            fastfetch(info);
        }
        else if (command == "!help") {
            fullHelp();
        }
        else if (command.substr(0, 5) == "!help") {
            if (checkLength(command, 7)) help("!help");
            else help(command.substr(6));
        }
        else {
            if (command[0] == '!')
            return PULSAR_EXIT_CODE_INVALID_COMMAND;
        }
        return PULSAR_EXIT_CODE_SUCCESS;
    }

    void fullHelp() {
        std::cout << "\tДоступные команды:\n"
                        "!exit                                         - Отключиться и выйти\n"
                        "!dest <channel/user>                          - Сменить назначение сообщений\n"
                        "!join <channel>                               - Присоединиться к каналу\n"
                        "!leave <channel>                              - Покинуть канал\n"
                        "!create <channel>                             - Создать канал\n"
                        "!chat <channel/user>                          - Посмотреть историю чата\n"
                        "!profile <username>|edit                      - Посмотреть или изменить профиль\n"
                        "!contact <add/rem> <username> <contact>       - Добавить или удалить контакт\n"
                        "!unread                                       - Посмотреть непрочитанные сообщения\n"
                        "!read <chat> <id>|all                         - Прочитать сообщение по ID из чата\n"
                        "!fastfetch                                    - Вывести информацию о клиенте\n"
                        "!help                                         - Вывести это окно\n"
        << std::flush;
    }

    void help(const std::string& command) {
        if (command == "!help") {
            fullHelp();
        } else if (command == "!exit") {
            std::cout << "!exit - Отключиться от сервера и выйти из клиента." << std::endl;
        } else if (command == "!dest") {
            std::cout << "!dest <channel/user> - Сменить назначение для отправляемых сообщений." << std::endl;
        } else if (command == "!join") {
            std::cout << "!join <channel> - Присоединиться к указанному каналу." << std::endl;
        } else if (command == "!leave") {
            std::cout << "!leave <channel> - Покинуть указанный канал." << std::endl;
        } else if (command == "!create") {
            std::cout << "!create <channel> - Создать новый канал с указанным именем." << std::endl;
        } else if (command == "!chat") {
            std::cout << "!chat <channel/user> - Просмотреть историю чата с указанным каналом или пользователем." << std::endl;
        } else if (command == "!profile") {
            std::cout << "!profile <username>|edit - Просмотреть профиль пользователя или изменить свой собственный." << std::endl;
        } else if (command == "!contact") {
            std::cout << "!contact <add/rem> <username> <contact> - Добавить или удалить контакт для указанного пользователя." << std::endl;
        } else if (command == "!unread") {
            std::cout << "!unread - Просмотреть все непрочитанные сообщения." << std::endl;
        } else if (command == "!read") {
            std::cout << "!read <chat> <id>|all - Пометить сообщение по ID в указанном чате как прочитанное, или все сообщения." << std::endl;
        } else if (command == "!fastfetch") {
            std::cout << "!fastfetch - Вывести информацию о клиенте." << std::endl;
        } else {
            std::cout << "Нет справки по команде: " << command << std::endl;
        }
    }
};