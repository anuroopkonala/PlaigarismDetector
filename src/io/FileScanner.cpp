#include "FileScanner.h"

#include <iostream>

namespace fs = std::filesystem;

FileScanner::FileScanner(const std::string& rootDirectory)
    : root(rootDirectory)
{
}

bool FileScanner::isSupportedExtension(const fs::path& path) const
{
    std::string ext = path.extension().string();

    return ext == ".cpp"
        || ext == ".cc"
        || ext == ".c"
        || ext == ".hpp"
        || ext == ".h"
        || ext == ".cxx"
        || ext == ".hh";
}

std::vector<fs::path> FileScanner::scan()
{
    std::vector<fs::path> files;

    if (!fs::exists(root))
    {
        throw std::runtime_error("Directory does not exist.");
    }

    for (const auto& entry :
         fs::recursive_directory_iterator(root))
    {
        if (!entry.is_regular_file())
            continue;

        if (isSupportedExtension(entry.path()))
        {
            files.push_back(entry.path());
        }
    }

    return files;
}