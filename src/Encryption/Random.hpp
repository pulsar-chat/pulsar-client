#pragma once

#include "static"

namespace PulsarCrypto {
    static inline bytes random_bytes(size_t n) {
        static std::random_device rd;
        static std::mt19937_64 gen(rd());
        static std::uniform_int_distribution<big> dist64(0, UINT64_MAX);

        bytes out(n);
        size_t i = 0;

        while (i + 8 <= n) {
            big v = dist64(gen);

            for (int k = 0; k < 8; k++)
                out[i++] = ubyte((v >> (k * 8)) & 0xFF);
        }
        
        while (i < n)
            out[i++] = ubyte(dist64(gen) & 0xFF);

        return out;
    }

    static inline big random_big(big min, big max) {
        static std::random_device rd;
        static std::mt19937_64 gen(rd());
        std::uniform_int_distribution<big> dist(min, max);
        return dist(gen);
    }
};