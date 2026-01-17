#pragma once

#include "static"
#include "Algorithms.hpp"

namespace PulsarCrypto {
    namespace Asymmetrical {
        namespace RSA {
            struct key {
                big n, s;
                key(big n, big s) : n(n), s(s) {}
                
                friend std::ostream& operator<<(std::ostream& os, const key& keys) {
                    os << std::hex << keys.n << ", " << std::hex << keys.s;
                    return os; 
                }
            };

            struct key_pair {
                key pub, priv;
            };

            big enc(big b, const key& key) {
                return pow_mod(b, key.s, key.n);
            }

            big dec(big b, const key& key) {
                return pow_mod(b, key.s, key.n);
            }

            class Generator {
            private:
                big p, q; // простые числа
                big n; // модуль
                big phi; // функция Эйлера
                big e; // открытый ключ
                big d; // закрытый ключ
            public:
                Generator() {
                    *this = Generator(generate_prime(3, 65535), generate_prime(3, 65535), 3);
                }

                Generator(big prime1, big prime2, big public_exp = 3) : p(prime1), q(prime2), e(public_exp) {
                    n = p * q;
                    phi = (p - 1) * (q - 1);

                    while (gcd(e, phi) != 1) e += 2;

                    d = inv_mod(e, phi);
                }

                Generator(const key& key, big p_val, big q_val) { // создать только закрытый ключ
                    p = p_val;
                    q = q_val;
                    n = key.n;
                    e = key.s;
                    phi = (p - 1) * (q - 1);
                    d = inv_mod(e, phi);
                }

                key getPublic() {
                    return {n, e};
                }

                key getPrivate() {
                    return {n, d};
                }
            };
        };

        bytes encrypt(const bytes& msg, RSA::key pub) {
            bytes res;
            for (ubyte c : msg) res.push_back(enc((big)c, pub));
            return res;
        }

        bytes decrypt(const bytes& msg, RSA::key priv) {
            bytes res;
            for (big c : msg) {
                big d = dec(c, priv);
                res.push_back(static_cast<ubyte>(d & 0xFF));
            }
            return res;
        }
    };
};