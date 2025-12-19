#include "Lexer.h"

#include <cctype>
#include <fstream>
#include <algorithm>

USE_MODULE(Arcana::Scan);


//     ██████╗██╗      █████╗ ███████╗███████╗    ██╗███╗   ███╗██████╗ ██╗     
//    ██╔════╝██║     ██╔══██╗██╔════╝██╔════╝    ██║████╗ ████║██╔══██╗██║     
//    ██║     ██║     ███████║███████╗███████╗    ██║██╔████╔██║██████╔╝██║     
//    ██║     ██║     ██╔══██║╚════██║╚════██║    ██║██║╚██╔╝██║██╔═══╝ ██║     
//    ╚██████╗███████╗██║  ██║███████║███████║    ██║██║ ╚═╝ ██║██║     ███████╗
//     ╚═════╝╚══════╝╚═╝  ╚═╝╚══════╝╚══════╝    ╚═╝╚═╝     ╚═╝╚═╝     ╚══════╝
//                                                                              

Lexer::Lexer(const std::string& arcscript)
    : 
    current_('\0'),
    line_(1),
    col_(0),
    nlcol_(0),
    file_(arcscript),
    in_(file_),
    arcscript_(arcscript)
{
    load_file_lines(in_);
    advance();
}



Token Lexer::next() 
{
    // IGNORE THE WHITE SPACES TOKEN, BUT \n IS PRESERVED
    skipWhitespace();

    // IF THE ISTREAM ISNT VALID RETURN EOF
    if (!in_) 
    {
        return makeToken(TokenType::ENDOFFILE, "", line_, col_, col_);
    }

    // IF STRING IS DETECTED 
    if (std::isalpha(static_cast<unsigned char>(current_)) || current_ == '_') 
    {
        return identifier();
    }

    // IF NUMBER IS DETECTED
    if (std::isdigit(static_cast<unsigned char>(current_))) 
    {
        return number();
    }

    // IF SINGLE CHAR AS TOKEN IS DETECTED
    switch (current_) 
    {
        case '=':  return simpleToken(TokenType::ASSIGN);
        case '"':  return simpleToken(TokenType::DQUOTE);
        case '+':  return simpleToken(TokenType::PLUS);
        case '-':  return simpleToken(TokenType::MINUS);
        case '*':  return simpleToken(TokenType::STAR);
        case '/':  return simpleToken(TokenType::SLASH);
        case '(':  return simpleToken(TokenType::ROUNDLP);
        case ')':  return simpleToken(TokenType::ROUNDRP);
        case '[':  return simpleToken(TokenType::SQUARELP);
        case ']':  return simpleToken(TokenType::SQUARERP);
        case '{':  return simpleToken(TokenType::CURLYLP);
        case '}':  return simpleToken(TokenType::CURLYRP);
        case '<':  return simpleToken(TokenType::ANGULARLP);
        case '>':  return simpleToken(TokenType::ANGULARRP);
        case '@':  return simpleToken(TokenType::AT);
        case ';':  return simpleToken(TokenType::SEMICOLON);
        case '\n': return simpleToken(TokenType::NEWLINE);
        default:   return simpleToken(TokenType::UNKNOWN);
    }
}


void Lexer::load_file_lines(std::istream& in)
{
    // LOAD THE ISTREAM AS ARRAYOF LINES
    std::string line;

    while (std::getline(in, line))
    {
        lines.push_back(line);
    }

    in.clear();
    in.seekg(0);

    return;
}



void Lexer::advance() 
{
    // ADVANCE THE ISTREAM POINTER
    for (;;)
    {
        // CHECK FOR THE EOF
        int c = in_.get();
        if (c == EOF) 
        {
            in_.setstate(std::ios::eofbit);
            current_ = '\0';
            return;
        }

        // IGNORE CR (CRLF normalization)
        if (c == '\r')
        {
            continue;
        }

        // if (c == '\\')
        // {
        //     int n = in_.peek();
        //     if (n == '\n')
        //     {
        //         in_.get();

        //         ++line_;
        //         nlcol_ = col_;
        //         col_   = 0;
 
        //         continue;
        //     }
        // }

        // CHECK FOR COMMENTS
        if (c == '#')
        {
            do
            {
                c = in_.get();
                ++nlcol_;
                ++col_;
            }
            while (c != '\n' && c != EOF);
        }

        // OBTAIN THE CURRENT CHAR
        current_ = static_cast<char>(c);

        // CHECK FOR NEW LINE, AND ALSO UPDATE THE POSITIONS
        if (current_ == '\n') 
        {
            ++line_;
            nlcol_ = col_;
            col_   = 0;
        } 
        else 
        {
            ++nlcol_;
            ++col_;
        }

        return;
    }
}



void Lexer::skipWhitespace() 
{
    // RUN ADVANCE UNTIL CURRRENT CHAR ISNT EOF, NEW LINE OR SPACE
    while (in_ && current_ != '\n' && ::isspace(static_cast<unsigned char>(current_))) 
    {
        advance();
    }
}



Token Lexer::simpleToken(TokenType type) 
{
    // GENERATE A SINGLE CHAR TOKEN
    char c       = current_;
    auto tokLine = (type == TokenType::NEWLINE) ? line_  - 1 : line_;
    auto tokCol  = (type == TokenType::NEWLINE) ? nlcol_ - 1 : col_ - 1;
    advance();
    return makeToken(type, std::string(1, c), tokLine, tokCol, 1);
}



Token Lexer::identifier() 
{
    // GENERATE A STRING TOKEN
    std::string lexeme;
    auto tokLine = line_;
    auto tokCol  = col_ - 1;
    TokenType tt = TokenType::IDENTIFIER; 

    while (in_ && (std::isalnum(static_cast<unsigned char>(current_)) || current_ == '_')) 
    {
        lexeme.push_back(current_);
        advance();
    }

    std::string lower = lexeme;
    std::transform(lower.begin(), lower.end(), lower.begin(),
                [](unsigned char c) { return std::tolower(c); });

    // CHECK FOR RESERVED KEYWORDS
    if (lower.compare("task") == 0)
    {
        tt = TokenType::TASK;
    }
    else if (lower.compare("import") == 0)
    {
        tt = TokenType::IMPORT;
    }
    else if (lower.compare("using") == 0)
    {
        tt = TokenType::USING;
    }
    else if (lower.compare("map") == 0)
    {
        tt = TokenType::MAPPING;
    }
    else if (lower.compare("assert") == 0)
    {
        tt = TokenType::ASSERT;
    }
    else if (lower.compare("ne") == 0)
    {
        tt = TokenType::NE;
    }
    else if (lower.compare("eq") == 0)
    {
        tt = TokenType::EQ;
    }

    // GENERATE THE TOKEN
    return makeToken(tt, std::move(lexeme), tokLine, tokCol, lexeme.size());
}



Token Lexer::number() 
{
    // GENERATE NUMERIC TOKEN 
    std::string lexeme;
    auto tokLine = line_;
    auto tokCol  = col_ - 1;

    while (in_ && std::isdigit(static_cast<unsigned char>(current_))) 
    {
        lexeme.push_back(current_);
        advance();
    }

    return makeToken(TokenType::NUMBER, std::move(lexeme), tokLine, tokCol, lexeme.size());
}



Token Lexer::makeToken(TokenType type, std::string lexeme,
                       std::size_t line, std::size_t start, std::size_t end) 
{
    // UTILITY TO GENERATE A SIMPLE TOKEN
    return Token{ type, std::move(lexeme), line, start, end };
}
