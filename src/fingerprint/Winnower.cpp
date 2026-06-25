#include "Winnower.h"

#include <limits>

Winnower::Winnower(size_t windowSize)
    : window(windowSize)
{
}

std::vector<Fingerprint>

Winnower::select(

const std::vector<uint64_t>& hashes)

{

    std::vector<Fingerprint> fingerprints;

    if(hashes.size()<window)
        return fingerprints;

    for(size_t i=0;i+window<=hashes.size();i++)
    {

        uint64_t minimum=
            std::numeric_limits<uint64_t>::max();

        size_t position=i;

        for(size_t j=i;j<i+window;j++)
        {

            if(hashes[j]<=minimum)
            {
                minimum=hashes[j];

                position=j;
            }

        }

        if(fingerprints.empty() ||
           fingerprints.back().tokenIndex!=position)
        {

            fingerprints.push_back(
            {
                minimum,
                position
            });

        }

    }

    return fingerprints;

}