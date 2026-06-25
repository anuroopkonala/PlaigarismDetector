#pragma once

#include <vector>
#include <string>

#include "Token.h"

class Lexer
{

public:

    std::vector<Token> tokenize(const std::string& filename);

};