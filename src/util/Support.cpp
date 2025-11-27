#include <sstream>

#include "Lexer.h"
#include "Support.h"
#include "Grammar.h"


USE_MODULE(Arcana);



Arcana_Result Support::ParserError::operator() (const Grammar::Match& match) const
{
    const auto& [token, found, semtypes, _] = match.Error;

    std::string       escaping;
    std::string       s(token.start + 1, '~');
    std::string       symbol(token.end, '^');
    std::stringstream ss;

    ss << "[" << ANSI_BRED << "ERROR" << ANSI_RESET << "] Semantic error on line " << token.line << ": ‘" << lexer[token] << "’" << std::endl;
    ss << ANSI_RED << "        +~~~~~~~~~~~~~~~~~~~~~~~~~" << s << symbol << ANSI_RESET << std::endl;
    
    escaping = (token.lexeme == "\n") ? "<New Line>" : token.lexeme;

    if (token.type == Scan::TokenType::UNKNOWN)
    {
        ss << "        Found undefined symbol: ‘" << escaping << "’" << std::endl;
    }
    else
    {
        ss << "        Found:    " << ANSI_RED << escaping << ANSI_RESET << " (" << Support::TokenTypeRepr(token.type) << ")" << std::endl;
        ss << "        Expected: " << Support::UniqueNonTerminalRepr(found);
        ss << " for statement(s)"  << std::endl;
        
        for (const auto& stmt : semtypes)
        {
            ss << "                  - ‘" << ANSI_CYAN << Support::RuleRepr(stmt) << ANSI_RESET << "’" << std::endl;
        }
    }
    
    std::cerr << ss.str();

    return Arcana_Result::ARCANA_RESULT__PARSING_ERROR;
}


std::size_t Support::StringViewHash::operator() (std::string_view s) const noexcept
{
    std::size_t h = 0;

    for (char c : s)
    {
        h = h * 131 + Support::toLowerAscii(c); 
    }

    return h;
}



bool Support::StringViewEq::operator() (std::string_view a, std::string_view b) const noexcept
{
    if (a.size() != b.size())
    {
        return false;
    }

    for (size_t i = 0; i < a.size(); ++i)
    {
        if (Support::toLowerAscii(a[i]) != Support::toLowerAscii(b[i]))
        {
            return false;
        }
    }
    return true;
}




Support::Arguments Support::ParseArgs(int argc, char** argv)
{
    Support::Arguments args;

    for (uint32_t i = 0; i < (uint32_t) argc; ++i)
    {
        args.push_back( Argument{ i, argv[i] } );
    }

    return args;
}


std::string Support::ltrim(const std::string& s)
{
    const std::string ws = " \t\r\n\f\v";

    size_t pos = s.find_first_not_of(ws);
    if (pos == std::string::npos)
    {
        return "";
    }

    return s.substr(pos);
}


inline char Support::toLowerAscii(char c) noexcept
{
    if (c >= 'A' && c <= 'Z')
        return c + ('a' - 'A');
    return c;
}


std::vector<std::string> Support::split(const std::string& s, char sep) noexcept
{
    std::vector<std::string> result;
    std::size_t i = 0;
    const std::size_t n = s.size();

    while (i < n)
    {
        // salta spazi
        while (i < n && s[i] == sep)
            ++i;

        if (i >= n)
            break;

        // inizio token
        std::size_t start = i;

        // avanza fino allo spazio
        while (i < n && s[i] != sep)
            ++i;

        result.emplace_back(s.substr(start, i - start));
    }

    return result;
}



std::string Support::TokenTypeRepr(const Scan::TokenType type)
{
    switch (type)
    {
        case Scan::TokenType::IDENTIFIER:  return "identifier";
        case Scan::TokenType::TASK:        return "task";     
        case Scan::TokenType::USING:       return "using";      
        case Scan::TokenType::NUMBER:      return "number";  
        case Scan::TokenType::ASSIGN:      return "assignment";  
        case Scan::TokenType::PLUS:        return "plus";
        case Scan::TokenType::MINUS:       return "minus"; 
        case Scan::TokenType::STAR:        return "star";
        case Scan::TokenType::SLASH:       return "slash"; 
        case Scan::TokenType::ROUNDLP:     return "left parenthesis";   
        case Scan::TokenType::ROUNDRP:     return "right parenthesis";   
        case Scan::TokenType::SQUARELP:    return "left bracket";    
        case Scan::TokenType::SQUARERP:    return "right bracket";    
        case Scan::TokenType::CURLYLP:     return "left brace";   
        case Scan::TokenType::CURLYRP:     return "right brace";   
        case Scan::TokenType::AT:          return "at sign";   
        case Scan::TokenType::SEMICOLON:   return "semicolon";   
        case Scan::TokenType::NEWLINE:     return "<new line>";   
        case Scan::TokenType::ENDOFFILE:   return "EOF";     
        case Scan::TokenType::UNKNOWN:     return "UNKNOWN";   
        case Scan::TokenType::ANY:         return "any";
        case Scan::TokenType::OPT_NEWLINE: return "<new line>";
        default:                           return "<INVALID>";
    }
}

std::string Support::TerminalRepr(const Grammar::Terminal& type)
{   
    std::stringstream ss;

    for (uint32_t i = 0; i < type.size(); ++i)
    {
        if (i > 0) ss << " or ";

        ss << "‘" << ANSI_GREEN << TokenTypeRepr(type[i]) << ANSI_RESET << "’";
    }

    return ss.str();
}


std::string Support::NonTerminalRepr(const Grammar::NonTerminal& type)
{   
    std::stringstream ss;

    for (uint32_t i = 0; i < type.size(); ++i)
    {
        if (i > 0) ss << " or ";

        ss << TerminalRepr(type[i]);
    }

    return ss.str();
}

std::string Support::UniqueNonTerminalRepr(const Grammar::UniqueNonTerminal& type)
{
    std::stringstream ss;
    uint32_t          i = 0;

    for (const auto& ref : type)
    {
        if (i > 0) ss << " or ";

        ss << TerminalRepr(ref);

        ++i;
    }

    return ss.str();
}



std::string Support::RuleRepr(const Grammar::Rule type)
{
    switch (type)
    {
        case Grammar::Rule::UNDEFINED:         return "UNDEFINED";
        case Grammar::Rule::VARIABLE_ASSIGN:   return "Assignment";
        case Grammar::Rule::EMPTY_LINE:        return "Empty Line";
        case Grammar::Rule::ATTRIBUTE:         return "Attribute";
        case Grammar::Rule::BUILTIN_TASK_DECL: return "Builtin Task Declaration";
        case Grammar::Rule::TASK_DECL:         return "Task Declaration";
        case Grammar::Rule::TASK_CALL:         return "Task Invoke";
        case Grammar::Rule::USING:             return "Using";
        default:                               return "<INVALID>";
    }
}
