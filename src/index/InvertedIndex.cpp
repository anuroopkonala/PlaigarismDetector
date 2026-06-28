#include "InvertedIndex.h"

#include <algorithm>

void InvertedIndex::insert(const fs::path&                  file,
                           const std::vector<Fingerprint>& fingerprints)
{
    const std::string key = file.string();

    for (const auto& fp : fingerprints)
    {
        index_[fp.hash].insert(key);
    }
}

std::vector<CandidatePair> InvertedIndex::candidates() const
{
    std::unordered_map<std::string, CandidatePair> seen;

    for (const auto& [hash, files] : index_)
    {
        if (files.size() < 2)
            continue;

        const std::vector<std::string> fv(files.begin(), files.end());

        for (size_t i = 0; i < fv.size(); ++i)
        {
            for (size_t j = i + 1; j < fv.size(); ++j)
            {
                const std::string& a = (fv[i] < fv[j]) ? fv[i] : fv[j];
                const std::string& b = (fv[i] < fv[j]) ? fv[j] : fv[i];

                const std::string pairKey = a + "|" + b;

                auto it = seen.find(pairKey);

                if (it == seen.end())
                {
                    seen[pairKey] = {
                        fs::path(a),
                        fs::path(b),
                        1
                    };
                }
                else
                {
                    ++it->second.sharedHashes;
                }
            }
        }
    }

    std::vector<CandidatePair> result;
    result.reserve(seen.size());

    for (auto& [key, pair] : seen)
    {
        result.push_back(std::move(pair));
    }

    std::sort(result.begin(), result.end(),
              [](const CandidatePair& a, const CandidatePair& b)
              {
                  return a.sharedHashes > b.sharedHashes;
              });

    return result;
}
