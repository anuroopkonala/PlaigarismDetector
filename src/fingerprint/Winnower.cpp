#include "Winnower.h"

#include <deque>

Winnower::Winnower(size_t windowSize) : window_(windowSize) {}

std::vector<Fingerprint>
Winnower::select(const std::vector<uint64_t>& hashes)
{
    std::vector<Fingerprint> fps;

    const size_t n = hashes.size();
    if (n < window_)
        return fps;

    std::deque<std::pair<uint64_t, size_t>> dq;

    size_t lastPos = SIZE_MAX;

    for (size_t i = 0; i < n; ++i)
    {
        while (!dq.empty() && dq.back().first >= hashes[i])
            dq.pop_back();

        dq.emplace_back(hashes[i], i);

        if (dq.front().second + window_ <= i)
            dq.pop_front();

        if (i + 1 < window_)
            continue;

        if (dq.front().second != lastPos)
        {
            lastPos = dq.front().second;
            fps.push_back({ dq.front().first, lastPos });
        }
    }

    return fps;
}
