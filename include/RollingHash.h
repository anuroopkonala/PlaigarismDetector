#pragma once

#include <cstdint>
#include <string>
#include <vector>

class RollingHash
{
public:
    explicit RollingHash(size_t k = 5);

    std::vector<uint64_t> compute(const std::vector<std::string>& tokens);

private:
    size_t          kGram_;
    static constexpr uint64_t BASE  = 131;
    static constexpr uint64_t MOD   = (1ULL << 61) - 1;

    static uint64_t hashToken(const std::string& tok);

    static uint64_t mulmod(uint64_t a, uint64_t b);
    static uint64_t addmod(uint64_t a, uint64_t b);
    static uint64_t submod(uint64_t a, uint64_t b);
};
