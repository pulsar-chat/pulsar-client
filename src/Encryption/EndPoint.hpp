#pragma once

#include "static"
#include "Asymmetrical.hpp"
#include "Symmetrical.hpp"

namespace PulsarCrypto {
    namespace end {
        Asymmetrical::RSA::key_pair generate_rsa() {
            Asymmetrical::RSA::Generator gen;
            return Asymmetrical::RSA::key_pair { gen.getPublic(), gen.getPrivate() };
        }

        Symmetrical::PESA generate_sym() {
            return Symmetrical::PESA { Symmetrical::random_symkey() } ;
        }

        std::string enc_rsa(std::string raw, Asymmetrical::RSA::key& pub) {
            auto enc_raw = Asymmetrical::encrypt(to_bytes(std::move(raw)), pub);
            return from_bytes(enc_raw);
        }

        std::string dec_rsa(std::string raw, Asymmetrical::RSA::key& priv) {
            auto dec_raw = Asymmetrical::decrypt(to_bytes(std::move(raw)), priv);
            return from_bytes(dec_raw);
        }

        std::string enc_sym(std::string raw, Symmetrical::PESA& key) {
            auto enc_raw = Symmetrical::encrypt(to_bytes(std::move(raw)), key);
            return from_bytes(enc_raw);
        }

        std::string dec_sym(std::string raw, Symmetrical::PESA& key) {
            auto dec_raw = Symmetrical::decrypt(to_bytes(std::move(raw)), key);
            return from_bytes(dec_raw);
        }
    };
};