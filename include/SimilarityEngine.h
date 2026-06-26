#pragma once

#include <string>
#include <unordered_map>
#include <vector>









class SimilarityEngine
{
public:
    
    void addDocument(const std::string&              fileId,
                     const std::vector<std::string>& tokens);

    
    double cosineSimilarity(const std::string& fileA,
                            const std::string& fileB) const;

    double lcsSimilarity(const std::string& fileA,
                         const std::string& fileB) const;

    size_t documentCount() const { return docs_.size(); }

private:
    using TermFreq = std::unordered_map<std::string, double>;
    using TokenVec = std::vector<std::string>;

    struct DocData
    {
        TokenVec tokens;
        TermFreq tf;
    };

    std::unordered_map<std::string, DocData> docs_;
    std::unordered_map<std::string, size_t>  df_;   

    TermFreq tfidfVector(const std::string& fileId) const;

    static double dot(const TermFreq& a, const TermFreq& b);
    static double norm(const TermFreq& v);

    static size_t lcsLength(const TokenVec& a, const TokenVec& b);
};
