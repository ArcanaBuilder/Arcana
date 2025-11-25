#include "Lexer.h"
#include "Semantic.h"

#include <cctype>
#include <algorithm>

USE_MODULE(Arcana::Parser);


std::string Arcana::Parser::TokenTypeRepr(const TokenType type)
{
    switch (type)
    {
        case TokenType::IDENTIFIER: return "IDENTIFIER";      
        case TokenType::NUMBER:     return "NUMBER";  
        case TokenType::ASSIGN:     return "ASSIGN";  
        case TokenType::PLUS:       return "PLUS";
        case TokenType::MINUS:      return "MINUS"; 
        case TokenType::STAR:       return "STAR";
        case TokenType::SLASH:      return "SLASH"; 
        case TokenType::ROUNDLP:    return "ROUNDLP";   
        case TokenType::ROUNDRP:    return "ROUNDRP";   
        case TokenType::SQUARELP:   return "SQUARELP";    
        case TokenType::SQUARERP:   return "SQUARERP";    
        case TokenType::CURLYLP:    return "CURLYLP";   
        case TokenType::CURLYRP:    return "CURLYRP";   
        case TokenType::AT:         return "AT";   
        case TokenType::SEMICOLON:  return "SEMICOLON";   
        case TokenType::NEWLINE:    return "NEWLINE";   
        case TokenType::ENDOFFILE:  return "ENDOFFILE";     
        case TokenType::UNKNOWN:    return "UNKNOWN";   
        case TokenType::ANY:        return "ANY";
        default:                    return "<INVALID>";
    }
}

std::string Arcana::Parser::TokenTypeListRepr(const std::vector<TokenType>& type)
{   
    std::stringstream ss;

    for (uint32_t i = 0; i < type.size(); ++i)
    {
        if (i > 0) ss << " or ";

        ss << TokenTypeRepr(type[i]);
    }

    return ss.str();
}


Lexer::Lexer(std::istream& in)
    : 
    in_(in),
    line_(1),
    col_(0),
    nlcol_(0),
    current_('\0')
{
    load_file_lines(in);
    advance();
}


Token Lexer::next() 
{
    skipWhitespace();

    if (!in_) 
    {
        return makeToken(TokenType::ENDOFFILE, "", line_, col_, col_);
    }

    if (std::isalpha(static_cast<unsigned char>(current_)) || current_ == '_') 
    {
        return identifier();
    }

    if (std::isdigit(static_cast<unsigned char>(current_))) 
    {
        return number();
    }

    switch (current_) 
    {
        case '=':  return simpleToken(TokenType::ASSIGN);
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
        case '@':  return simpleToken(TokenType::AT);
        case ';':  return simpleToken(TokenType::SEMICOLON);
        case '\n': return simpleToken(TokenType::NEWLINE);
        default:   return simpleToken(TokenType::UNKNOWN);
    }
}


void Lexer::load_file_lines(std::istream& in)
{
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
    for (;;)
    {
        int c = in_.get();
        if (c == EOF) 
        {
            in_.setstate(std::ios::eofbit);
            current_ = '\0';
            return;
        }

        if (c == '\\')
        {
            int n = in_.peek();
            if (n == '\n')
            {
                in_.get();

                ++line_;
                nlcol_ = col_;
                col_   = 0;

                continue;
            }
        }

        current_ = static_cast<char>(c);

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
    while (in_ && current_ != '\n' && ::isspace(static_cast<unsigned char>(current_))) 
    {
        advance();
    }
}

Token Lexer::simpleToken(TokenType type) 
{
    char c       = current_;
    auto tokLine = (type == TokenType::NEWLINE) ? line_  - 1 : line_;
    auto tokCol  = (type == TokenType::NEWLINE) ? nlcol_ - 1 : col_ - 1;
    advance();
    return makeToken(type, std::string(1, c), tokLine, tokCol, tokCol);
}

Token Lexer::identifier() 
{
    std::string lexeme;
    auto tokLine = line_;
    auto tokCol  = col_ - 1;

    while (in_ && (std::isalnum(static_cast<unsigned char>(current_)) || current_ == '_')) 
    {
        lexeme.push_back(current_);
        advance();
    }

    // Qui potresti fare lookup per parole chiave e cambiare TokenType
    return makeToken(TokenType::IDENTIFIER, std::move(lexeme), tokLine, tokCol, lexeme.size());
}

Token Lexer::number() 
{
    std::string lexeme;
    auto tokLine = line_;
    auto tokCol  = col_ - 1;

    while (in_ && std::isdigit(static_cast<unsigned char>(current_))) 
    {
        lexeme.push_back(current_);
        advance();
    }

    // Estendibile a numeri con punto, notazione scientifica, ecc.
    return makeToken(TokenType::NUMBER, std::move(lexeme), tokLine, tokCol, lexeme.size());
}

Token Lexer::makeToken(TokenType type, std::string lexeme,
                       std::size_t line, std::size_t start, std::size_t end) 
{
    return Token{ type, std::move(lexeme), line, start, end };
}
