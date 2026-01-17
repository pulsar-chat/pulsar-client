#pragma once

#include "static"
#include "Keys.hpp"
#include "Random.hpp"

namespace PulsarCrypto {
    namespace Symmetrical {
        // P - Pulsar
        // E - Encryption
        // S - Symmetrical
        // A - Algorithm
        class PESA {
        private:
            big key;
        public:
            PESA(big key) : key(key) {}


            big enc(big b) {
                const auto klyde_gk = 41.71;

                // A lot of magic numbers...
                b ^= ((key << 2) + 0xf6) | 4171714 & 0xDEAD'BEEF * *(int*)&klyde_gk;
                b <<= 2;
                b += 5;
                b -= key + 4171;

                return b;
            }

            big dec(big b) {
                const auto klyde_gk = 41.71;

                // All these magic numbers, but backwards
                b += key + 4171;
                b -= 5;
                b >>= 2;
                b ^= ((key << 2) + 0xf6) | 4171714 & 0xDEAD'BEEF * *(int*)&klyde_gk;

                return b;
            }
        };

        big random_symkey() {
            auto index = random_big(0, KeysSize - 1);
            return Keys[index];
        }
    
        bytes encrypt(const bytes& msg, PESA& pesa) {
            bytes res;
            for (auto c : msg) res.push_back(pesa.enc(c));
            return res;
        }

        bytes decrypt(const bytes& msg, PESA& pesa) {
            bytes res;
            for (auto c : msg) res.push_back((ubyte)pesa.dec(c));
            return res;
        }
    };
};