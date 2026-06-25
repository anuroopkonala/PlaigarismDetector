#include "Normalizer.h"

std::vector<Token>

Normalizer::normalize(

const std::vector<Token>& input)

{

    std::vector<Token> output;

    std::unordered_map<std::string,std::string>

    variables;

    int id=1;

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

            if(!variables.count(token.value))
            {
                variables[token.value]
                    ="VAR"+
                    std::to_string(id++);
            }

            token.value=
                variables[token.value];
        }

        output.push_back(token);

    }

    return output;

}