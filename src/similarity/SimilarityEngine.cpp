#include "SimilarityEngine.h"

#include <algorithm>
#include <cmath>
#include <stdexcept>
#include <unordered_set>



void SimilarityEngine::addDocument(const std::string&              fileId,
                                   const std::vector<std::string>& tokens)
{
    DocData& doc = docs_[fileId];
    doc.tokens   = tokens;

    for (const auto& tok : tokens)
        doc.tf[tok] += 1.0;

    
    for (const auto& [term, _] : doc.tf)
        ++df_[term];
}



SimilarityEngine::TermFreq
SimilarityEngine::tfidfVector(const std::string& fileId) const
{
    auto it = docs_.find(fileId);
    if (it == docs_.end())
        throw std::runtime_error("Unknown document: " + fileId);

    const double N = static_cast<double>(docs_.size());
    TermFreq     tfidf;

    for (const auto& [term, rawCount] : it->second.tf)
    {
        auto dfIt    = df_.find(term);
        double df    = dfIt != df_.end() ? (double)dfIt->second : 1.0;
        double idf   = std::log(N / df + 1.0);
        tfidf[term]  = rawCount * idf;
    }
    return tfidf;
}

double SimilarityEngine::dot(const TermFreq& a, const TermFreq& b)
{
    const TermFreq& small = (a.size() <= b.size()) ? a : b;
    const TermFreq& large = (a.size() <= b.size()) ? b : a;
    double r = 0.0;
    for (const auto& [t, w] : small)
    {
        auto it = large.find(t);
        if (it != large.end()) r += w * it->second;
    }
    return r;
}

double SimilarityEngine::norm(const TermFreq& v)
{
    double s = 0.0;
    for (const auto& [_, w] : v) s += w * w;
    return std::sqrt(s);
}

double SimilarityEngine::cosineSimilarity(const std::string& fileA,
                                          const std::string& fileB) const
{
    auto vecA = tfidfVector(fileA);
    auto vecB = tfidfVector(fileB);
    double nA = norm(vecA), nB = norm(vecB);
    if (nA < 1e-12 || nB < 1e-12) return 0.0;
    return dot(vecA, vecB) / (nA * nB);
}

size_t SimilarityEngine::lcsLength(const TokenVec& a, const TokenVec& b)
{
    if (a.empty() || b.empty()) return 0;
    const TokenVec& X = (a.size() >= b.size()) ? a : b;
    const TokenVec& Y = (a.size() >= b.size()) ? b : a;
    const size_t m = Y.size();
    std::vector<size_t> prev(m + 1, 0), curr(m + 1, 0);
    for (size_t i = 1; i <= X.size(); ++i)
    {
        for (size_t j = 1; j <= m; ++j)
        {
            if (X[i - 1] == Y[j - 1])
                curr[j] = prev[j - 1] + 1;
            else
                curr[j] = std::max(prev[j], curr[j - 1]);
        }
        std::swap(prev, curr);
        std::fill(curr.begin(), curr.end(), 0);
    }
    return prev[m];
}

double SimilarityEngine::lcsSimilarity(const std::string& fileA,
                                       const std::string& fileB) const
{
    auto itA = docs_.find(fileA);
    auto itB = docs_.find(fileB);
    if (itA == docs_.end() || itB == docs_.end()) return 0.0;
    const TokenVec& tokA = itA->second.tokens;
    const TokenVec& tokB = itB->second.tokens;
    size_t maxLen = std::max(tokA.size(), tokB.size());
    if (maxLen == 0) return 0.0;
    return (double)lcsLength(tokA, tokB) / (double)maxLen;
}

