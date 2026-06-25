#pragma once

#include <vector>

#include "Fingerprint.h"

class Winnower
{

public:

    Winnower(size_t windowSize=4);

    std::vector<Fingerprint>

    select(const std::vector<uint64_t>& hashes);

private:

    size_t window;

};