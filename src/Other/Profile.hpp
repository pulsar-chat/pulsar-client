#pragma once

#include <string>
#include "../defines"
#include <sstream>
#include <vector>
#include "Datetime.hpp"

class Profile {
private:
    std::string m_description;
    std::string m_email;
    std::string m_realName;
    Datetime m_birthday;
public:
    Profile(std::string description, std::string email, std::string realName, Datetime birthday)
     : m_description(description), m_email(email), m_realName(realName), m_birthday(birthday) {}

    // getters
    std::string description(void) const { return m_description; }
    std::string email(void) const { return m_email; }
    std::string realName(void) const { return m_realName; }
    Datetime birthday(void) const { return m_birthday; }

    // setters
    void description(const std::string& description) {
        m_description = description;
    }

    void email(const std::string& email) {
        m_email = email;
    }

    void realName(const std::string& realName) {
        m_realName = realName;
    }

    void birthday(const Datetime& birthday) {
        m_birthday = birthday;
    }

    std::string to_payload() const {
        std::ostringstream oss;

        oss << m_description << PULSAR_PROFILE_SEP;
        oss << m_email << PULSAR_PROFILE_SEP;
        oss << m_realName << PULSAR_PROFILE_SEP;
        oss << m_birthday.toTime();

        return oss.str();
    }

    static Profile from_payload(const std::string& payload) {
        std::vector<std::string> parts;
        std::istringstream iss(payload);
        std::string part;

        while (std::getline(iss, part, PULSAR_PROFILE_SEP)) {
            parts.push_back(part);
        }

        return Profile(parts[0], parts[1], parts[2], Datetime::fromString(parts[3]));
    }
};