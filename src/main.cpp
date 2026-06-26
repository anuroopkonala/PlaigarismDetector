#include <iostream>
#include <string>
#include <thread>

#include "Logger.h"
#include "Pipeline.h"

static void usage(const char* prog)
{
    std::cerr
        << "Usage: " << prog << " <input_dir> [options]\n\n"
        << "Options:\n"
        << "  --output  <dir>    Output directory for reports (default: .)\n"
        << "  --threshold <f>    Minimum combined score to flag [0-1] (default: 0.3)\n"
        << "  --threads  <n>     Worker threads (default: hardware concurrency)\n"
        << "  --kgram    <n>     Rolling hash k-gram size (default: 5)\n"
        << "  --window   <n>     Winnowing window size (default: 4)\n"
        << "  --verbose          Print per-file processing info\n"
        << "  --help             Show this message\n";
}

int main(int argc, char** argv)
{
    if (argc < 2)
    {
        usage(argv[0]);
        return 1;
    }

    PipelineConfig cfg;
    cfg.inputDir = argv[1];

    for (int i = 2; i < argc; ++i)
    {
        std::string arg = argv[i];

        if (arg == "--help" || arg == "-h") { usage(argv[0]); return 0; }

        auto nextStr = [&]() -> std::string {
            if (i + 1 >= argc) { Logger::error("Missing value for " + arg); exit(1); }
            return argv[++i];
        };
        auto nextDbl = [&]() { return std::stod(nextStr()); };
        auto nextUL  = [&]() { return (unsigned)std::stoul(nextStr()); };

        if      (arg == "--output")    cfg.outputDir  = nextStr();
        else if (arg == "--threshold") cfg.threshold  = nextDbl();
        else if (arg == "--threads")   cfg.threads    = nextUL();
        else if (arg == "--kgram")     cfg.kGram      = (size_t)nextUL();
        else if (arg == "--window")    cfg.window     = (size_t)nextUL();
        else if (arg == "--verbose")   cfg.verbose    = true;
        else
        {
            Logger::error("Unknown option: " + arg);
            usage(argv[0]);
            return 1;
        }
    }

    Pipeline pipeline(cfg);
    return pipeline.run();
}
