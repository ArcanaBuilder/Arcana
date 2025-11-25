#ifndef __ARCANA_LEXER__H__
#define __ARCANA_LEXER__H__

#include <sstream>
#include <istream>
#include <optional>

#include "Util.h"


BEGIN_MODULE(Parser)





enum class TokenType : uint32_t
{
    IDENTIFIER           =  0,
    NUMBER                   ,
    ASSIGN                   ,
    PLUS                     ,
    MINUS                    ,
    STAR                     ,
    SLASH                    ,
    ROUNDLP                  ,
    ROUNDRP                  ,
    SQUARELP                 ,
    SQUARERP                 ,
    CURLYLP                  ,
    CURLYRP                  ,
    AT                       ,
    SEMICOLON                ,
    NEWLINE                  ,
    ENDOFFILE                ,

    // SPECIAL TOKENS
    UNKNOWN                  ,
    ANY                      ,
};

struct Token 
{
    TokenType    type;
    std::string  lexeme;
    std::size_t  line;
    std::size_t  start;
    std::size_t  end;
};


std::string TokenTypeRepr(const TokenType type);
std::string TokenTypeListRepr(const std::vector<TokenType>& type);


using Tokens = std::vector<Token>;


class Lexer 
{
public:
    explicit Lexer(std::istream& in);

    Token next();

    inline std::string operator[] (const size_t pos) 
    {
        return lines[pos];
    }

    inline std::string& operator[] (const Token& token) 
    {
        return lines[token.line - 1];
    }

private:
    std::istream& in_;
    std::size_t   line_;
    std::size_t   col_;
    std::size_t   nlcol_;
    char          current_;

    std::vector<std::string> lines;
    void  load_file_lines(std::istream& in);

    void  advance();
    void  skipWhitespace();
    Token simpleToken(TokenType type);
    Token identifier();
    Token number();
    Token makeToken(TokenType type, std::string lexeme,
                    std::size_t line, std::size_t start, std::size_t end);
};




END_MODULE(Parser)



#endif /* __ARCANA_LEXER__H__ */