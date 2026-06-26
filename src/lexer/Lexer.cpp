#include "Lexer.h"

#include <clang-c/Index.h>
#include <cstring>
#include <stdexcept>

static TokenType convert(CXTokenKind kind)
{
    switch (kind)
    {
        case CXToken_Keyword:     return TokenType::Keyword;
        case CXToken_Identifier:  return TokenType::Identifier;
        case CXToken_Literal:     return TokenType::Literal;
        case CXToken_Punctuation: return TokenType::Punctuation;
        case CXToken_Comment:     return TokenType::Comment;
        default:                  return TokenType::Unknown;
    }
}

std::vector<Token> Lexer::tokenize(const std::string& filename)
{
    std::vector<Token> tokens;

    CXIndex idx = clang_createIndex(0, 0);

    
    const bool isCpp = filename.ends_with(".cpp") ||
                       filename.ends_with(".cc")  ||
                       filename.ends_with(".cxx") ||
                       filename.ends_with(".hpp");

    
    std::vector<const char*> args = {
        "-std=c++20",
        "-w",
        "-fno-delayed-template-parsing",
    };
    if (!isCpp)
        args = {"-std=c11", "-w"};

    CXTranslationUnit tu = clang_parseTranslationUnit(
        idx,
        filename.c_str(),
        args.data(), (int)args.size(),
        nullptr, 0,
        CXTranslationUnit_None);

    if (!tu)
    {
        clang_disposeIndex(idx);
        throw std::runtime_error("Cannot parse: " + filename);
    }

    CXFile file = clang_getFile(tu, filename.c_str());

    size_t fileSize = 0;
    const char* contents = clang_getFileContents(tu, file, &fileSize);

    CXSourceRange range = clang_getRange(
        clang_getLocationForOffset(tu, file, 0),
        clang_getLocationForOffset(tu, file, contents ? (unsigned)fileSize : 0));

    CXToken*  tokenArray = nullptr;
    unsigned  numTokens  = 0;
    clang_tokenize(tu, range, &tokenArray, &numTokens);

    for (unsigned i = 0; i < numTokens; ++i)
    {
        CXString spelling = clang_getTokenSpelling(tu, tokenArray[i]);
        CXSourceLocation loc = clang_getTokenLocation(tu, tokenArray[i]);

        unsigned line = 0, col = 0;
        clang_getExpansionLocation(loc, nullptr, &line, &col, nullptr);

        tokens.push_back({
            convert(clang_getTokenKind(tokenArray[i])),
            clang_getCString(spelling),
            line,
            col
        });

        clang_disposeString(spelling);
    }

    clang_disposeTokens(tu, tokenArray, numTokens);
    clang_disposeTranslationUnit(tu);
    clang_disposeIndex(idx);

    return tokens;
}
