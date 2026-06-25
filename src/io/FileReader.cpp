#include "FileReader.h"

#include <fstream>
#include <sstream>

std::string FileReader::read(const std::filesystem::path& file)
{
    std::ifstream fin(file);

    if (!fin)
        throw std::runtime_error("Cannot open file.");

    std::stringstream buffer;

    buffer << fin.rdbuf();

    return buffer.str();
}