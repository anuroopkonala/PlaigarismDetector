#include "Normalizer.h"

std::vector<Token>

Normalizer::normalize(

const std::vector<Token>& input)

{

    std::vector<Token> output;

    for(auto token:input)
    {
        if(token.type==TokenType::Comment)
            continue;

        if(token.type==TokenType::Literal)
        {
            token.value="NUM";
        }

        if(token.type==TokenType::Identifier)
        {
            token.value="VAR";
        }

        output.push_back(token);
    }

    return output;

}