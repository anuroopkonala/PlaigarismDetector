#include "RollingHash.h"

RollingHash::RollingHash(size_t k)
    : kGram(k)
{
}

std::vector<uint64_t>
RollingHash::compute(const std::vector<std::string>& tokens)
{
    std::vector<uint64_t> hashes;

    if(tokens.size() < kGram)
        return hashes;

    for(size_t i=0;i+kGram<=tokens.size();i++)
    {
        uint64_t h=0;

        for(size_t j=0;j<kGram;j++)
        {
            for(char c:tokens[i+j])
            {
                h=h*131+c;
            }

            h=h*BASE+17;
        }

        hashes.push_back(h);
    }

    return hashes;
}