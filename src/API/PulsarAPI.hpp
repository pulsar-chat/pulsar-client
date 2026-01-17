#pragma once

#include <SFML/Network.hpp>
#include <thread>
#include <list>
#include <iostream>
#include <mutex>

#include "../Other/Chat.hpp"
#include "../Network/Database.hpp"
#include "../Other/Message.hpp"
#include "../Other/Profile.hpp"
#include "../lib/hash.h"
#include "../Network/Checker.hpp"
#include "../Encryption/EndPoint.hpp"

class PulsarAPI {
private:
    struct ServerResponse {
        std::string req, rsp;
    };

    std::shared_ptr<sf::TcpSocket> socket;
    std::string username;
    Database db;
    std::thread recv_thr;
    std::atomic_bool connected = false;
    std::list<ServerResponse> responses;
    std::mutex responses_mtx;
public:
    enum LoginResult {
        Success,
        Fail_Username,
        Fail_Password,
        Fail_Unknown
    };

    PulsarAPI(std::shared_ptr<sf::TcpSocket> socket, const std::string& username)
     : socket(socket), username(username), db(username) {
        if (!Checker::checkUsername(username)) PULSAR_THROW UsernameFailed(username);
    }

    std::shared_ptr<sf::TcpSocket> getSocket() { return socket; }

    bool connect(const std::string& ip, unsigned short port) {
        if (!socket) socket = std::make_shared<sf::TcpSocket>();

        sf::Socket::Status status = socket->connect(*sf::IpAddress::resolve(ip), port, sf::milliseconds(PULSAR_TIMEOUT_MS));

        if (status != sf::Socket::Status::Done) {
            std::cout << "Невозможно подключиться к " << ip << ":" << port << std::endl;
            std::cout << "Удостоверьтесь, что сервер работает и доступен." << std::endl;
            return false;
        }

        std::cout << "Conneted to " << ip << ":" << port << std::endl;
        connected = true;
        return true;
    }

    void disconnect() {
        socket->disconnect();
        connected = false;
        std::cout << "Отключено от сервера." << std::endl;
    }

    bool isConnected() { return connected; }

    void sendRaw(const std::string& raw) {
        if (!connected) return;
        if (socket->send(raw.data(), raw.size()) != sf::Socket::Status::Done) {
            std::cout << "Не удалось отправить сообщение" << std::endl;
            disconnect();
        }
    }

    void send(const Message& msg) {
        sendRaw(msg.to_payload());
    }

    void send(const std::string& message, const std::string& dest) {
        send(Message {
            0, Datetime::now().toTime(),
            username, dest, message
        });
    }

    std::string recvRaw() {
        if (!connected) return {};

        char buffer[PULSAR_PACKET_SIZE];
        size_t recieved;
        if (socket->receive(buffer, sizeof(buffer), recieved) != sf::Socket::Status::Done) {
            std::cout << "Не удалось получить сообщение" << std::endl;
            disconnect();
        }

        return std::string { buffer, recieved };
    }

    Message recv() {
        return Message::from_payload(recvRaw());
    }

    void recieverLoop() {
        while (connected) {
            auto message = recv();

            if (message.get_src() == "!server.msg") {
                storeResponse(parseServer(message.get_msg()));
            }

            else {
                std::cout << message << std::endl;
            }

            sf::sleep(sf::milliseconds(20));
        }
    }

    void startRecieverLoop() {
        recv_thr = std::thread { &PulsarAPI::recieverLoop, this };
        recv_thr.detach();
    }

    ServerResponse parseServer(const std::string& message) {
        #ifdef PULSAR_DEBUG
            std::cout << "Server message:\n\traw: \"" << message << "\"\n\tparsed: ";
        #endif

        ServerResponse res;
        const char SEP = '\x1e';
        const std::string REQ_PREF = "REQ:";
        const std::string RSP_PREF = "RSP:";

        auto trim = [](std::string s){
            size_t a = 0, b = s.size();
            while (a < b && std::isspace((unsigned char)s[a])) ++a;
            while (b > a && std::isspace((unsigned char)s[b-1])) --b;
            return s.substr(a, b - a);
        };

        std::string left, right;
        auto pos = message.find(SEP);
        if (pos == std::string::npos) {
            auto rsp_pos = message.find("RSP:");
            if (rsp_pos != std::string::npos) {
                left = message.substr(0, rsp_pos);
                right = message.substr(rsp_pos);
            } else {
                left = message;
                right = "";
            }
        } else {
            left = message.substr(0, pos);
            right = message.substr(pos + 1);
        }

        left = trim(left);
        right = trim(right);

        if (left.rfind(REQ_PREF, 0) == 0) res.req = trim(left.substr(REQ_PREF.size()));
        else res.req = left;

        if (right.rfind(RSP_PREF, 0) == 0) res.rsp = trim(right.substr(RSP_PREF.size()));
        else res.rsp = right;

        #ifdef PULSAR_DEBUG
            std::cout << "REQ=\"" << res.req << "\" RSP=\"" << res.rsp << "\"" << std::endl;
        #endif

        return res;
    }

    void storeResponse(const ServerResponse& resp) {
        std::lock_guard lk(responses_mtx);

        responses.push_front(resp);

        if (responses.size() > PULSAR_MAX_RESPONSES) responses.pop_back();
    }

    std::string requestRaw(const std::string& req) {
        if (!connected) return {};

        send(req, "!server.req");

        sf::Clock clk;

        while (clk.getElapsedTime().asMilliseconds() < PULSAR_TIMEOUT_MS) {
            {
                std::lock_guard lk(responses_mtx);
    
                for (auto i = responses.begin(); i != responses.end(); i++) {
                    if (i->req == req) {
                        auto ans = i->rsp;
                        responses.erase(i);
                        return ans;
                    }
                }
            }

            sf::sleep(sf::milliseconds(20));
        }

        return {};
    }

    template<class... Args>
    std::string request(const std::string& req_command, Args&&... args) {
        std::ostringstream oss;
        oss << '!' << req_command;
        ((oss << ' ' << std::forward<Args>(args)), ...);

        return requestRaw(oss.str());
    }

    std::string request(const std::string& req_command) {
        return requestRaw("!" + req_command);
    }

    /// Common API

    void sendRsaKey(PulsarCrypto::Asymmetrical::RSA::key pub) {
        auto response = request("rsa", pub.n, pub.s);
    }

    bool joinChannel(const std::string& channel) {
        auto response = request("join", channel);

        if (response == "+") {
            db.join(channel);
            return true;
        } else {
            return false;
        }
    }

    bool leaveChannel(const std::string& channel) {
        auto response = request("leave", channel);

        if (response == "+") {
            db.leave(channel);
            return true;
        } else {
            return false;
        }
    }

    bool createChannel(const std::string& channel) {
        if (!Checker::checkChannelName(channel)) PULSAR_THROW ChannelNameFailed(channel);

        auto response = request("create", channel);

        if (response == "+") {
            joinChannel(channel);
            return true;
        } else {
            return false;
        }
    }

    LoginResult login(const std::string& password) {
        auto response = request("login", username, password);

        if (response == "success") {
            return LoginResult::Success;
        } else if (response == "fail_username") {
            return LoginResult::Fail_Username;
        } else if (response == "fail_password") {
            return LoginResult::Fail_Password;
        } else {
            return LoginResult::Fail_Unknown;
        }
    }

    LoginResult registerUser(const std::string& password) {
        auto response = request("register", username, password);

        if (response == "success") {
            return LoginResult::Success;
        } else if (response == "fail_username") {
            return LoginResult::Fail_Username;
        } else {
            return LoginResult::Fail_Unknown;
        }
    }

    Profile getProfile(const std::string& username) {
        auto response = request("profile", "get", username);

        return Profile::from_payload(response);
    }

    bool updateProfile(const Profile& profile) {
        auto response = request("profile", "set", profile.to_payload());

        return response == "+";
    }

    bool createContact(const std::string& username, const std::string& contact) {
        if (!Checker::checkUsername(username)) PULSAR_THROW UsernameFailed(username);

        auto response = request("contact", "add", username, contact);

        if (response == "+") {
            db.add_contact(username, contact);
            return true;
        } else return false;
    }

    bool removeContact(const std::string& contact) {
        if (!Checker::checkUsername(contact)) PULSAR_THROW UsernameFailed(contact);

        auto response = request("contact", "rem", contact);
        if (response == "+") {
            db.remove_contact(contact);
            return true;
        } else return false;
    }

    Chat getChat(const std::string& chat, int lines_count = 50) {
        if (!Checker::checkChannelName(chat) && chat[0] != '@') PULSAR_THROW ChannelNameFailed(chat);

        auto response = request("chat", chat, lines_count);
        return Chat { chat, split(response, PULSAR_SEP) };
    }

    bool isChannelMember(const std::string& channel) {
        if (!Checker::checkChannelName(channel) && channel[0] != '@' && channel[0] != '!' && channel != "") PULSAR_THROW ChannelNameFailed(channel);

        return db.is_channel_member(channel);
    }

    Message getMessageById(const std::string chat, size_t id) {
        auto response = request("msg", chat, id);

        auto msg = Message::from_payload(response);

        return Message { id, msg.get_time().toTime(), msg.get_src(), chat, msg.get_msg() };
    }

    std::vector<Message> getUnread() {
        return db.get_unread();
    }

    bool read(const std::string& chat, size_t id) {
        db.read(chat, id);
        return request("read", chat, id) == "+";
    }

    void readAll(const std::string& chat) {
        for (auto i : getUnread()) {
            if (chat == i.get_dst()) read(chat, i.get_id());
        }
    }

    void readAll() {
        for (auto i : getUnread()) read(i.get_dst(), i.get_id());
    }

    void requestUnread() {
        auto response = request("getUnread");
        
        auto splited = split(response, ';');

        for (auto unread : splited) {
            auto chat = split(unread, '|')[0];
            auto id = std::stoull(split(unread, '|')[1]);

            auto msg = getMessageById(chat, id);

            db.store_unread(msg);
        }
    }
};