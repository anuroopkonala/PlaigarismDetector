#pragma once

#include <filesystem>
#include <string>
#include <vector>

#include "ReportGenerator.h"

namespace fs = std::filesystem;

struct PipelineConfig
{
    std::string inputDir;
    std::string outputDir  = ".";
    size_t      kGram      = 5;      
    size_t      window     = 4;      
    double      threshold  = 0.3;   
    unsigned    threads    = 0;      
    bool        verbose    = false;
};









class Pipeline
{
public:
    explicit Pipeline(const PipelineConfig& cfg);

    
    int run();

private:
    PipelineConfig cfg_;
};
