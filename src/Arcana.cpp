#include "Parser.h"

#include <fstream>
#include <iostream>

USE_MODULE(Arcana);



void ParserErrorCallback(const std::string& msg)
{
    std::cerr << msg << std::endl;
}


int run(const Arguments& args)
{   
    int result = -1;

    if (args.size() < 2)
    {
        std::cerr << "Required at least one argument, the arcana-script file!" << std::endl;
        return result;
    }

    std::ifstream file(args[1].arg);

    ::Parser::Lexer    lex(file);
    ::Parser::Semantic sem;
    ::Parser::Parser   p(lex, sem);

    p.AddErrorCallback( [&lex] (const ::Parser::Match& m) {
        const auto& [token, found, _] = m.Error;

        std::string       escaping;
        std::string       s(token.start + 1, '-');
        std::string       symbol(token.end, '^');
        std::stringstream ss;

        ss << "[" << ANSI_BRED << "ERROR" << ANSI_RESET << "] Semantic error on line " << token.line << ": '" << lex[token] << "'" << std::endl;
        ss << ANSI_RED << "        +-------------------------" << s << symbol << ANSI_RESET << std::endl;
        
        escaping = (token.lexeme == "\n") ? "<New Line>" : token.lexeme;

        if (token.type == ::Parser::TokenType::UNKNOWN)
        {
            ss << "        Found undefined symbol: '" << escaping << "'" << std::endl;
        }
        else
        {
            ss << "        Found:    " << ANSI_RED << escaping << ANSI_RESET << std::endl;
            ss << "        Expected: " << ::Parser::TokenTypeStreamRepr(found) << std::endl;
        }
        
        std::cerr << ss.str();
    });

    p.parse();

    result = 0;
    return result;
}





int main(int argc, char** argv) 
{
    Arguments args = ParseArgs(argc, argv);

    return run(args);
}