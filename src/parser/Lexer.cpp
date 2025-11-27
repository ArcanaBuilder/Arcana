#include "Lexer.h"

#include <cctype>
#include <algorithm>

USE_MODULE(Arcana::Scan);


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
    return makeToken(type, std::string(1, c), tokLine, tokCol, 1);
}

Token Lexer::identifier() 
{
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

    if (lower.compare("task") == 0)
    {
        tt = TokenType::TASK;
    }
    else if (lower.compare("using") == 0)
    {
        tt = TokenType::USING;
    }

    // Qui potresti fare lookup per parole chiave e cambiare TokenType
    return makeToken(tt, std::move(lexeme), tokLine, tokCol, lexeme.size());
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
