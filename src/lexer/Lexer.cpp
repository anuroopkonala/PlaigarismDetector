#include "Lexer.h"

#include <clang-c/Index.h>

#include <iostream>

static TokenType convert(CXTokenKind kind)
{
    switch(kind)
    {
        case CXToken_Keyword:

            return TokenType::Keyword;

        case CXToken_Identifier:

            return TokenType::Identifier;

        case CXToken_Literal:

            return TokenType::Literal;

        case CXToken_Punctuation:

            return TokenType::Punctuation;

        case CXToken_Comment:

            return TokenType::Comment;

        default:

            return TokenType::Unknown;
    }
}

std::vector<Token> Lexer::tokenize(const std::string& filename)
{
    std::vector<Token> tokens;

    CXIndex index =
        clang_createIndex(0,0);

    CXTranslationUnit tu =
        clang_parseTranslationUnit(
            index,
            filename.c_str(),
            nullptr,
            0,
            nullptr,
            0,
            CXTranslationUnit_None);

    if(!tu)
        throw std::runtime_error("Cannot parse source.");

    CXFile file =
        clang_getFile(tu,
                      filename.c_str());

    CXSourceRange range =
        clang_getRange(

            clang_getLocationForOffset(
                tu,
                file,
                0),

            clang_getLocationForOffset(
                tu,
                file,
                clang_getFileContents(
                    tu,
                    file,
                    nullptr
                )
                ? strlen(
                    clang_getFileContents(
                        tu,
                        file,
                        nullptr))
                : 0
            ));

    CXToken* tokenArray;

    unsigned numTokens;

    clang_tokenize(

        tu,

        range,

        &tokenArray,

        &numTokens

    );

    for(unsigned i=0;i<numTokens;i++)
    {
        CXToken t =
            tokenArray[i];

        CXString spelling =
            clang_getTokenSpelling(
                tu,
                t);

        CXSourceLocation loc =
            clang_getTokenLocation(
                tu,
                t);

        unsigned line;

        unsigned col;

        clang_getExpansionLocation(

            loc,

            nullptr,

            &line,

            &col,

            nullptr

        );

        tokens.push_back(

            {

                convert(

                    clang_getTokenKind(
                        t)),

                clang_getCString(
                    spelling),

                line,

                col

            }

        );

        clang_disposeString(spelling);
    }

    clang_disposeTokens(
        tu,
        tokenArray,
        numTokens);

    clang_disposeTranslationUnit(tu);

    clang_disposeIndex(index);

    return tokens;
}

