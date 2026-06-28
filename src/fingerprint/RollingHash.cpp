#include "RollingHash.h"

#include <cstring>

uint64_t RollingHash::mulmod(uint64_t a, uint64_t b)
{
    __uint128_t r = (__uint128_t)a * b;
    uint64_t lo = (uint64_t)(r & MOD);
    uint64_t hi = (uint64_t)(r >> 61);
    uint64_t res = lo + hi;
    if (res >= MOD) res -= MOD;
    return res;
}

uint64_t RollingHash::addmod(uint64_t a, uint64_t b)
{
    a += b;
    if (a >= MOD) a -= MOD;
    return a;
}

uint64_t RollingHash::submod(uint64_t a, uint64_t b)
{
    return addmod(a, MOD - b % MOD);
}

uint64_t RollingHash::hashToken(const std::string& tok)
{
    uint64_t h = 0;
    for (unsigned char c : tok)
        h = addmod(mulmod(h, BASE), c + 1);
    return h == 0 ? 1 : h;
}

RollingHash::RollingHash(size_t k) : kGram_(k) {}

std::vector<uint64_t>
RollingHash::compute(const std::vector<std::string>& tokens)
{
    const size_t n = tokens.size();
    std::vector<uint64_t> hashes;

    if (n < kGram_)
        return hashes;

    hashes.reserve(n - kGram_ + 1);

    std::vector<uint64_t> th(n);
    for (size_t i = 0; i < n; ++i)
        th[i] = hashToken(tokens[i]);

    uint64_t bk = 1;
    for (size_t i = 0; i < kGram_; ++i)
        bk = mulmod(bk, BASE);

    uint64_t h = 0;
    for (size_t i = 0; i < kGram_; ++i)
        h = addmod(mulmod(h, BASE), th[i]);
    hashes.push_back(h);

    for (size_t i = 1; i + kGram_ <= n; ++i)
    {
        h = submod(mulmod(h, BASE), mulmod(bk, th[i - 1]));
        h = addmod(h, th[i + kGram_ - 1]);
        hashes.push_back(h);
    }

    return hashes;
}
