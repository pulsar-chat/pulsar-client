#pragma once

#include "../Other/Profile.hpp"
#include "../Other/Message.hpp"
#include "../defines"
#include "SQLite3.hpp"
#include <string>
#include <vector>
#include <sstream>

class Database {
private:
    SQLite3Database db;
    std::string username;
public:
    Database(const std::string& username)
     : db(("pulsar_" + username + ".db")), username(username) {
        db.execute("PRAGMA foreign_keys = ON;");
        db.execute("CREATE TABLE IF NOT EXISTS profile (username TEXT PRIMARY KEY, name TEXT, email TEXT, description TEXT, birthday INTEGER, status TEXT);");
        db.execute("CREATE TABLE IF NOT EXISTS channels (username TEXT, channel TEXT, PRIMARY KEY(username, channel));");
        db.execute("CREATE TABLE IF NOT EXISTS contacts (username TEXT, contact_username TEXT, contact_name TEXT, PRIMARY KEY(username, contact_username));");
        db.execute("CREATE TABLE IF NOT EXISTS unread (username TEXT, id INTEGER, time INTEGER, src TEXT, dst TEXT, msg TEXT, PRIMARY KEY(username, id, src, dst));");
        try {
            db.execute("INSERT OR IGNORE INTO profile(username, name, email, description, birthday, status) VALUES ('" + username + "', 'NAME', '', '', 0, 'active');");
        } catch(...) {}
    }

    std::string getString() {
        return "sqlite3://pulsar_" + username + ".db";
    }

    void init(const std::string& name, const std::string& email, const std::string& description, time_t birthday, const std::string& status) {
        std::ostringstream oss;
        oss << "INSERT OR REPLACE INTO profile(username, name, email, description, birthday, status) VALUES ('" << username << "', '" << name << "', '" << email << "', '" << description << "', " << birthday << ", '" << status << "');";
        db.execute(oss.str());
    }

    bool is_channel_member(const std::string& channel) {
        if (channel.empty()) return false;
        if (channel[0] == '@' || channel[0] == '!') return false;
        bool exists = false;
        db.query("SELECT 1 FROM channels WHERE username='" + username + "' AND channel='" + channel + "' LIMIT 1;",
                 [&](const SQLite3Database::Row& row){ exists = true; });
        return exists;
    }

    void join(const std::string& channel) {
        db.execute("INSERT OR IGNORE INTO channels(username, channel) VALUES ('" + username + "', '" + channel + "');");
    }

    void leave(const std::string& channel) {
        db.execute("DELETE FROM channels WHERE username='" + username + "' AND channel='" + channel + "';");
    }

    void add_contact(const std::string& contact_username, const std::string& contact_name) {
        db.execute("INSERT OR REPLACE INTO contacts(username, contact_username, contact_name) VALUES ('" + username + "', '" + contact_username + "', '" + contact_name + "');");
    }

    void remove_contact(const std::string& contact_username) {
        db.execute("DELETE FROM contacts WHERE username='" + username + "' AND contact_username='" + contact_username + "';");
    }

    std::string contact_name(const std::string& contact_username) {
        std::string res;
        db.query("SELECT contact_name FROM contacts WHERE username='" + username + "' AND contact_username='" + contact_username + "' LIMIT 1;",
                 [&](const SQLite3Database::Row& row){ res = row[0]; });
        return res;
    }

    Message contact(const Message& msg) {
        std::string cname = contact_name(msg.get_src());
        if (cname.empty()) return msg;
        return Message(msg.get_id(), msg.get_time().toTime(), cname, msg.get_dst(), msg.get_msg());
    }

    void update_profile(const Profile& profile) {
        std::ostringstream oss;
        oss << "INSERT OR REPLACE INTO profile(username, name, email, description, birthday, status) VALUES ('" << username << "', '" << profile.realName() << "', '" << profile.email() << "', '" << profile.description() << "', " << profile.birthday().toTime() << ", 'active');";
        db.execute(oss.str());
    }

    void store_unread(const Message& msg) {
        std::ostringstream oss;
        oss << "INSERT OR REPLACE INTO unread(username, id, time, src, dst, msg) VALUES ('" << username << "', " << msg.get_id() << ", " << msg.get_time().toTime() << ", '" << msg.get_src() << "', '" << msg.get_dst() << "', '" << msg.get_msg() << "');";
        db.execute(oss.str());
    }

    std::vector<Message> get_unread() {
        std::vector<Message> out;
        db.query("SELECT id, time, src, dst, msg FROM unread WHERE username='" + username + "' ORDER BY time ASC;",
                 [&](const SQLite3Database::Row& row){
                     size_t id = std::stoull(row[0]);
                     time_t t = static_cast<time_t>(std::stoull(row[1]));
                     std::string src = row[2];
                     std::string dst = row[3];
                     std::string text = row[4];
                     out.emplace_back(id, t, src, dst, text);
                 });
        return out;
    }

    void read(const std::string& chat, size_t id) {
        std::ostringstream oss;
        oss << "DELETE FROM unread WHERE username='" << username << "' AND id=" << id << " AND dst='" << chat << "';";
        db.execute(oss.str());
    }

    void clear_unread() {
        db.execute("DELETE FROM unread WHERE username='" + username + "';");
    }
};