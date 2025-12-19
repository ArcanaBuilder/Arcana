#ifndef __ARCANA_LEXER__H__
#define __ARCANA_LEXER__H__


#include <set>
#include <vector>
#include <sstream>
#include <istream>
#include <fstream>

#include "Defines.h"



BEGIN_MODULE(Scan)



//    ██████╗ ██╗   ██╗██████╗ ██╗     ██╗ ██████╗    ███████╗███╗   ██╗██╗   ██╗███╗   ███╗███████╗
//    ██╔══██╗██║   ██║██╔══██╗██║     ██║██╔════╝    ██╔════╝████╗  ██║██║   ██║████╗ ████║██╔════╝
//    ██████╔╝██║   ██║██████╔╝██║     ██║██║         █████╗  ██╔██╗ ██║██║   ██║██╔████╔██║███████╗
//    ██╔═══╝ ██║   ██║██╔══██╗██║     ██║██║         ██╔══╝  ██║╚██╗██║██║   ██║██║╚██╔╝██║╚════██║
//    ██║     ╚██████╔╝██████╔╝███████╗██║╚██████╗    ███████╗██║ ╚████║╚██████╔╝██║ ╚═╝ ██║███████║
//    ╚═╝      ╚═════╝ ╚═════╝ ╚══════╝╚═╝ ╚═════╝    ╚══════╝╚═╝  ╚═══╝ ╚═════╝ ╚═╝     ╚═╝╚══════╝
//                                                                                                                                                                                            

/// @brief Enum used to define lexer tokens
enum class TokenType : uint32_t
{
    IDENTIFIER           =  0,
    TASK                     ,
    IMPORT                   ,
    USING                    ,
    MAPPING                  ,
    ASSERT                   ,
    NUMBER                   ,
    ASSIGN                   ,
    DQUOTE                   ,
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
    ANGULARLP                ,
    ANGULARRP                ,
    AT                       ,
    NE                       ,
    EQ                       ,
    SEMICOLON                ,
    NEWLINE                  ,
    ENDOFFILE                ,

    // SPECIAL TOKENS
    UNKNOWN                  ,
    ANY                      ,
    OPT_NEWLINE              ,        
};




//    ██████╗ ██╗   ██╗██████╗ ██╗     ██╗ ██████╗    ███████╗████████╗██████╗ ██╗   ██╗ ██████╗████████╗███████╗
//    ██╔══██╗██║   ██║██╔══██╗██║     ██║██╔════╝    ██╔════╝╚══██╔══╝██╔══██╗██║   ██║██╔════╝╚══██╔══╝██╔════╝
//    ██████╔╝██║   ██║██████╔╝██║     ██║██║         ███████╗   ██║   ██████╔╝██║   ██║██║        ██║   ███████╗
//    ██╔═══╝ ██║   ██║██╔══██╗██║     ██║██║         ╚════██║   ██║   ██╔══██╗██║   ██║██║        ██║   ╚════██║
//    ██║     ╚██████╔╝██████╔╝███████╗██║╚██████╗    ███████║   ██║   ██║  ██║╚██████╔╝╚██████╗   ██║   ███████║
//    ╚═╝      ╚═════╝ ╚═════╝ ╚══════╝╚═╝ ╚═════╝    ╚══════╝   ╚═╝   ╚═╝  ╚═╝ ╚═════╝  ╚═════╝   ╚═╝   ╚══════╝
//                                                                                                                                                                                                         


/// @brief Struct used to hold the lexer output
struct Token 
{
    TokenType    type;         //<! Type of token
    std::string  lexeme;       //<! lexeme matched   
    std::size_t  line;         //<! Line of match
    std::size_t  start;        //<! Lexeme start index match  
    std::size_t  end;          //<! Lexeme end index match
};




//    ██████╗ ██╗   ██╗██████╗ ██╗     ██╗ ██████╗     ██████╗██╗      █████╗ ███████╗███████╗███████╗███████╗
//    ██╔══██╗██║   ██║██╔══██╗██║     ██║██╔════╝    ██╔════╝██║     ██╔══██╗██╔════╝██╔════╝██╔════╝██╔════╝
//    ██████╔╝██║   ██║██████╔╝██║     ██║██║         ██║     ██║     ███████║███████╗███████╗█████╗  ███████╗
//    ██╔═══╝ ██║   ██║██╔══██╗██║     ██║██║         ██║     ██║     ██╔══██║╚════██║╚════██║██╔══╝  ╚════██║
//    ██║     ╚██████╔╝██████╔╝███████╗██║╚██████╗    ╚██████╗███████╗██║  ██║███████║███████║███████╗███████║
//    ╚═╝      ╚═════╝ ╚═════╝ ╚══════╝╚═╝ ╚═════╝     ╚═════╝╚══════╝╚═╝  ╚═╝╚══════╝╚══════╝╚══════╝╚══════╝
//                                                                                                            


class Lexer 
{
public:
    explicit Lexer(const std::string& arcscript);

    Token next();

    inline std::string operator[] (const size_t pos) 
    {
        return lines[pos];
    }

    inline std::string& operator[] (const Token& token) 
    {
        return lines[token.line - 1];
    }

    inline const std::string& source() const { return arcscript_; }

private:
    char                     current_;
    std::size_t              line_;
    std::size_t              col_;
    std::size_t              nlcol_;
    std::ifstream            file_;
    std::istream&            in_;
    const std::string&       arcscript_;
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




END_MODULE(Scan)



#endif /* __ARCANA_LEXER__H__ */