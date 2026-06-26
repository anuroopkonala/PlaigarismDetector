#include "Pipeline.h"

#include <algorithm>
#include <future>
#include <iomanip>
#include <iostream>
#include <mutex>

#include "ASTNormalizer.h"
#include "APTEDDistance.h"
#include "FileScanner.h"
#include "Lexer.h"
#include "Logger.h"
#include "Normalizer.h"
#include "ReportGenerator.h"
#include "SimilarityEngine.h"
#include "ThreadPool.h"



Pipeline::Pipeline(const PipelineConfig& cfg) : cfg_(cfg) {}



static std::vector<std::string>
toStringVec(const std::vector<Token>& tokens)
{
    std::vector<std::string> out;
    out.reserve(tokens.size());
    for (const auto& t : tokens) out.push_back(t.value);
    return out;
}


struct FileData
{
    std::string              path;
    std::vector<std::string> tokens;   
};

int Pipeline::run()
{
    try
    {
        FileScanner scanner(cfg_.inputDir);
        auto paths = scanner.scan();

        Logger::info("Files found: " + std::to_string(paths.size()));

        if (paths.size() < 2)
        {
            Logger::warning("Need at least 2 files to compare.");
            return 0;
        }

        const unsigned hwThreads = std::thread::hardware_concurrency();
        const unsigned nThreads  = cfg_.threads > 0
                                 ? cfg_.threads
                                 : std::max(1u, hwThreads);

        Logger::info("Thread pool: " + std::to_string(nThreads) + " threads");

        ThreadPool pool(nThreads);

        std::vector<std::future<FileData>> futures;
        futures.reserve(paths.size());

        for (const auto& p : paths)
        {
            futures.push_back(pool.enqueue([p, this]() -> FileData
            {
                Lexer      lexer;
                Normalizer norm;

                auto raw        = lexer.tokenize(p.string());
                auto normalized = norm.normalize(raw);
                auto words      = toStringVec(normalized);

                return { p.string(), std::move(words) };
            }));
        }

        std::vector<FileData> corpus;
        corpus.reserve(paths.size());

        SimilarityEngine simEngine;
        size_t skipCount = 0;

        for (auto& fut : futures)
        {
            try
            {
                auto fd = fut.get();

                if (cfg_.verbose)
                    Logger::info("  " + fs::path(fd.path).filename().string() +
                                 " — " +
                                 std::to_string(fd.tokens.size()) + " tokens");

                simEngine.addDocument(fd.path, fd.tokens);
                corpus.push_back(std::move(fd));
            }
            catch (const std::exception& ex)
            {
                Logger::warning("Skipping file: " + std::string(ex.what()));
                ++skipCount;
            }
        }

        Logger::info("Processed: " + std::to_string(corpus.size()) +
                     " files (" + std::to_string(skipCount) + " skipped)");

        std::vector<std::pair<std::string, std::string>> candidates;
        for (size_t i = 0; i < corpus.size(); ++i)
        {
            for (size_t j = i + 1; j < corpus.size(); ++j)
            {
                candidates.push_back({corpus[i].path, corpus[j].path});
            }
        }
        Logger::info("Candidate pairs: " + std::to_string(candidates.size()));

        std::vector<std::future<PairResult>> scoreFutures;
        scoreFutures.reserve(candidates.size());

        for (const auto& cand : candidates)
        {
            const std::string pathA = cand.first;
            const std::string pathB = cand.second;

            scoreFutures.push_back(pool.enqueue(
                [pathA, pathB, &simEngine, this]() -> PairResult
                {
                    PairResult r;
                    r.fileA             = pathA;
                    r.fileB             = pathB;
                    r.sharedFingerprints = 0;

                    r.lcsSim    = simEngine.lcsSimilarity(pathA, pathB);
                    r.cosineSim = simEngine.cosineSimilarity(pathA, pathB);
                    r.astSim    = 0.0;  

                    r.combinedScore = 0.4 * r.lcsSim
                                    + 0.4 * r.cosineSim
                                    + 0.2 * r.astSim;

                    return r;
                }));
        }

        std::vector<PairResult> results;
        results.reserve(candidates.size());

        for (auto& fut : scoreFutures)
        {
            try { results.push_back(fut.get()); }
            catch (...) {}
        }

        Logger::info("Computing AST similarity for " +
                     std::to_string(results.size()) + " pairs...");

        std::vector<std::future<double>> astFutures;
        astFutures.reserve(results.size());

        for (const auto& r : results)
        {
            astFutures.push_back(pool.enqueue(
                [pathA = r.fileA, pathB = r.fileB]() -> double
                {
                    try
                    {
                        ASTNormalizer an;
                        APTEDDistance apted;

                        auto treeA = an.normalize(pathA);
                        auto treeB = an.normalize(pathB);

                        return 1.0 - apted.normalizedDistance(treeA, treeB);
                    }
                    catch (...) { return 0.0; }
                }));
        }

        for (size_t i = 0; i < results.size(); ++i)
        {
            results[i].astSim = astFutures[i].get();

            results[i].combinedScore = 0.4 * results[i].lcsSim
                                     + 0.4 * results[i].cosineSim
                                     + 0.2 * results[i].astSim;
        }

        std::erase_if(results, [this](const PairResult& r)
        {
            return r.combinedScore < cfg_.threshold;
        });

        std::sort(results.begin(), results.end(),
                  [](const PairResult& a, const PairResult& b)
                  {
                      return a.combinedScore > b.combinedScore;
                  });

        Logger::info("Flagged pairs: " + std::to_string(results.size()));

        if (!results.empty())
        {
            std::cout << "\n┌─ Plagiarism Report (" << results.size()
                      << " pairs above threshold " << cfg_.threshold << ") ─\n";

            for (const auto& r : results)
            {
                std::cout
                    << "│\n"
                    << "│  " << fs::path(r.fileA).filename().string()
                    << "  ↔  " << fs::path(r.fileB).filename().string() << "\n"
                    << "│    LCS=" << std::fixed << std::setprecision(3) << r.lcsSim
                    << "  Cosine=" << r.cosineSim
                    << "  AST="    << r.astSim
                    << "  Combined=" << r.combinedScore << "\n";
            }
            std::cout << "└──────────────────────────────────────────\n\n";
        }
        else
        {
            std::cout << "\nNo suspicious pairs found above threshold "
                      << cfg_.threshold << ".\n";
        }

        ReportGenerator gen(cfg_.threshold);
        gen.generate(results, fs::path(cfg_.outputDir));

        Logger::info("Reports written to: " + cfg_.outputDir);
    }
    catch (const std::exception& e)
    {
        Logger::error(e.what());
        return 1;
    }

    return 0;
}
