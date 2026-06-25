#pragma once

#include <vector>
#include <string>
#include <cstdint>

class RollingHash
{
public:

    RollingHash(size_t k = 5);

    std::vector<uint64_t>
    compute(const std::vector<std::string>& tokens);

private:

    size_t kGram;

    const uint64_t BASE = 911382323;
};