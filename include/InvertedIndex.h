#pragma once

#include <filesystem>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "Fingerprint.h"

namespace fs = std::filesystem;

struct CandidatePair
{
    fs::path fileA;
    fs::path fileB;
    size_t   sharedHashes;
};

class InvertedIndex
{
public:
    void insert(const fs::path& file,
                const std::vector<Fingerprint>& fingerprints);

    std::vector<CandidatePair> candidates() const;

    size_t size() const { return index_.size(); }

private:
    std::unordered_map<uint64_t, std::unordered_set<std::string>> index_;
};
