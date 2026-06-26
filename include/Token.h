#pragma once

#include <string>

enum class TokenType
{
    Keyword,
    Identifier,
    Literal,
    Operator,
    Punctuation,
    Comment,
    Unknown
};

struct Token
{
    TokenType   type;
    std::string value;
    unsigned    line;
    unsigned    column;
};
