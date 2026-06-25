#include "CLI.h"
#include "FileScanner.h"
#include "Logger.h"
#include "Lexer.h"

std::vector<std::string>

convert(const std::vector<Token>& tokens)
{
    std::vector<std::string> output;

    for(const auto& t:tokens)
    {
        output.push_back(t.value);
    }

    return output;
}

int main(int argc,char** argv)
{
    if(argc<2)
    {
        Logger::error("Usage: detector <folder>");
        return 1;
    }

    try
    {
        FileScanner scanner(argv[1]);

        auto files = scanner.scan();

        Logger::info("Files Found: " + std::to_string(files.size()));

        for(const auto& file : files)
        {
            Logger::info(file.string());
        }
    }
    catch(const std::exception& e)
    {
        Logger::error(e.what());
    }
}