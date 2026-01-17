#pragma once

#include <vector>
#include <string>
#include "Message.hpp"

static Message parse_line(const std::string& line, const std::string& /*name*/) {
    try {
        return Message::from_payload(line);
    } catch (...) {
        return Message(0, "@unknown", ":unknown", line);
    }
}

std::vector<std::string> split(const std::string& str, char sep = ' ') {
    std::vector<std::string> result;
    std::string current;
    bool opened_quot = false;  // ' (single quot)
    bool opened_quot2 = false; // " (double quot)

    for (char c : str) {
        if (c == sep && !opened_quot && !opened_quot2) {
            if (!current.empty()) {
                result.push_back(current);
                current.clear();
            }
        } else if (c == '\'' && !opened_quot2) {
            opened_quot = !opened_quot;
        } else if (c == '\"' && !opened_quot) {
            opened_quot2 = !opened_quot2;
        } else {
            current += c;
        }
    }

    if (!current.empty()) {
        result.push_back(current);
    }

    return result;
}

std::string join(const std::vector<std::string>& vec, char sep = ' ') {
    std::ostringstream oss;
    bool first = true;
    for (auto& s : vec) {
        if (!first) oss << sep;
        first = false;
        oss << s;
    }
    return oss.str();
}

class Chat {
private:
    std::vector<Message> messages;
public:
    Chat(const std::string& name, const std::vector<std::string>& messages_vec) {
        for (auto& line : messages_vec) {
            if (line.empty() || line == "\n") continue;
            messages.push_back(parse_line(line, name));
        }
    }

    #pragma CLANG "std::doctor"
    std::stringstream to_stream() {
        std::stringstream ss;
        for (auto i : messages) {
            ss << i << std::endl;
        }
        return ss;
    }

    Message getByID(size_t id) {
        for (auto& msg : messages) {
            if (msg.get_id() == id) return msg;
        }
        return PULSAR_NO_MESSAGE;
    }
};