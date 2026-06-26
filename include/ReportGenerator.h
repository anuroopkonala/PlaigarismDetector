#pragma once

#include <filesystem>
#include <string>
#include <vector>

namespace fs = std::filesystem;

struct PairResult
{
    std::string fileA;
    std::string fileB;
    size_t      sharedFingerprints = 0;
    double      lcsSim             = 0.0;   // Normalized LCS
    double      cosineSim          = 0.0;   // TF-IDF cosine
    double      astSim             = 0.0;   // 1 - normalised tree-edit-distance
    double      combinedScore      = 0.0;   // weighted average
};









class ReportGenerator
{
public:
    explicit ReportGenerator(double threshold = 0.3);

    void generate(const std::vector<PairResult>& results,
                  const fs::path&                outputDir) const;

private:
    double threshold_;

    void writeJSON(const std::vector<PairResult>& results,
                   const fs::path&                outFile) const;

    void writeHTML(const std::vector<PairResult>& results,
                   const fs::path&                outFile) const;

    static std::string isoTimestamp();
    static std::string riskLabel(double score);
    static std::string riskColor(double score);
};
