#pragma once

#include "../defines"
#include "Datetime.hpp"
#include <string>
#include <ostream>
#include <sstream>
#include <iomanip>

template <typename _Tp>
inline std::string stringify(_Tp&& arg) {
    std::ostringstream oss;
    oss << std::forward<_Tp>(arg);
    return oss.str();
}

template <typename _Tp>
inline std::string align_right(size_t size, _Tp&& arg, char fill = ' ') {
    std::string str = stringify(std::forward<_Tp>(arg));
    auto s_size = str.size();
    auto delta = size - s_size;
    std::string res;

    for (size_t i = 0; i < delta; i++) res.push_back(fill);

    for (const auto& c : str) res.push_back(c);

    return res;
}

inline void remove_spaces(std::string& s) {
    s.erase(
        std::remove_if(s.begin(), s.end(),
            [](unsigned char c){ return std::isspace(c); }),
        s.end()
    );
}

class Message {
private:
    size_t id;
    Datetime time;
    std::string src;
    std::string dst;
    std::string msg;
public:
    Message() : id(0), time(0), src(""), dst(""), msg("") {}

    Message(size_t id, const std::string& src, const std::string& dst, const std::string& msg)
     : id(id), src(src), dst(dst), msg(msg), time(Datetime::now()) {}

     Message(size_t id, time_t time, const std::string& src, const std::string& dst, const std::string& msg)
     : id(id), src(src), dst(dst), msg(msg), time(time) {}

    inline size_t get_id() const { return id; }
    inline Datetime get_time() const { return time; }
    inline std::string get_src() const { return src; }
    inline std::string get_dst() const { return dst; }
    inline std::string get_msg() const { return msg; }

    std::string to_payload() const {
        std::ostringstream oss;

        oss << align_right(PULSAR_ID_SIZE, id, '0');
        oss << align_right(PULSAR_TIME_SIZE, time.toTime(), '0');
        oss << align_right(PULSAR_SRC_SIZE, src);
        oss << align_right(PULSAR_DST_SIZE, dst);
        oss << msg;

        return oss.str();
    }

    static std::string to_payload(const Message& message) {
        return message.to_payload();
    }

    static Message from_payload(const std::string& payload) {
        size_t id;
        time_t time;
        std::string src, dst, msg;
        
        id = std::stoul(payload.substr(0, PULSAR_ID_SIZE));
        time = std::stol(payload.substr(PULSAR_ID_SIZE, PULSAR_TIME_SIZE));
        src = payload.substr(PULSAR_ID_SIZE + PULSAR_TIME_SIZE, PULSAR_SRC_SIZE);
        dst = payload.substr(PULSAR_ID_SIZE + PULSAR_TIME_SIZE + PULSAR_SRC_SIZE, PULSAR_DST_SIZE);
        msg = payload.substr(PULSAR_ID_SIZE + PULSAR_TIME_SIZE + PULSAR_SRC_SIZE + PULSAR_DST_SIZE);

        remove_spaces(src);
        remove_spaces(dst);
        
        return { id, time, src, dst, msg };
    }
};

inline std::ostream& operator<<(std::ostream& os, const Message& m) {
    os << "<" << m.get_time().toFormattedString() << "> [id:" << m.get_id() << "] (от " << m.get_src() << " в " << m.get_dst() << "): " << m.get_msg();
    return os;
};