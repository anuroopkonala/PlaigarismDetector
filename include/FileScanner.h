#pragma once

#include <filesystem>
#include <string>
#include <vector>

class FileScanner
{
public:

    explicit FileScanner(const std::string& rootDirectory);

    std::vector<std::filesystem::path> scan();

private:

    std::filesystem::path root;

    bool isSupportedExtension(const std::filesystem::path& path) const;
};