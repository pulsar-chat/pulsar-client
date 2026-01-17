#pragma once

#include "static"
#include "Random.hpp"

namespace PulsarCrypto {
    big gcd(big a, big b) { // НОД
        while (b != 0) {
            big t = b;
            b = a % b;
            a = t;
        }

        return a;
    }

    big inv_mod(big a, big mod) { // Обратный модуль
        if (mod == 1) return 0;

        verybig t = 0, newt = 1;
        verybig r = mod, newr = a;

        while (newr != 0) {
            verybig q = r / newr;
            verybig tmp = newt;
            newt = t - q * newt;
            t = tmp;
            tmp = newr;
            newr = r - q * newr;
            r = tmp;
        }

        if (r > 1) return 0;
        if (t < 0) t += mod;
        return (big)t;
    }

    big pow_mod(big base, big exp, big mod) { // (base ^ exp) % mod
        big result = 1;
        base = base % mod;

        while (exp > 0) {
            if (exp & 1) result = (big)((verybig)result * base % mod);

            exp = exp >> 1;
            base = (big)((verybig)base * base % mod);
        }

        return result;
    }

    bool is_prime(big n) { // Проверка на простоту
        if (n <= 1) return false;
        if (n <= 3) return true;
        if (n % 2 == 0) return false;

        big limit = (big)std::sqrt((long double)n);
        for (big i = 3; i <= limit; i += 2)
            if (n % i == 0)
                return false;
        return true;
    }

    big generate_prime(big min, big max) { // Генератор простых чисел в заданном диапазоне
        if (min < 3) min = 3;
        if (max <= min) max = min + 100;
        while (true) {
            big num = random_big(min, max);
            if (num % 2 == 0) ++num;
            for (big candidate = num; candidate <= max; candidate += 2) {
                if (is_prime(candidate)) return candidate;
            }
        }
        return 0;
    }
};