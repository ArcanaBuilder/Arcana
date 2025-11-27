#ifndef __ARCANA_SUPPORT__H__
#define __ARCANA_SUPPORT__H__


///////////////////////////////////////////////////////////////////////////////
// INCLUDES
///////////////////////////////////////////////////////////////////////////////

#include <map>
#include <set>
#include <vector>

#include "Defines.h"



///////////////////////////////////////////////////////////////////////////////
// FORWARDED DECLARATIONS
///////////////////////////////////////////////////////////////////////////////

BEGIN_MODULE(Scan)

class Lexer;

enum class TokenType : uint32_t;

END_MODULE(Scan)


BEGIN_MODULE(Grammar)

class Index;

struct Match;

enum class Rule : uint32_t;

END_MODULE(Grammar)



///////////////////////////////////////////////////////////////////////////////
// PUBLIC STRUCTS
///////////////////////////////////////////////////////////////////////////////

BEGIN_MODULE(Support)

/// @brief structure used to hold command line arguments
struct Argument
{
    uint32_t pos;
    char*    arg;
};


struct ParserError
{
    Scan::Lexer& lexer;
    
    void operator () (const Grammar::Match& match) const;
};

END_MODULE(Support)

///////////////////////////////////////////////////////////////////////////////
// USINGS
///////////////////////////////////////////////////////////////////////////////

BEGIN_MODULE(Support)

using Arguments         = std::vector<Argument>;

END_MODULE(Support)


BEGIN_MODULE(Grammar)

using Terminal          = std::vector<Scan::TokenType>;
using NonTerminal       = std::vector<Terminal>;
using Production        = std::map<Grammar::Rule, NonTerminal>;

using UniqueRule        = std::set<Grammar::Rule>;
using UniqueNonTerminal = std::set<Terminal>;


END_MODULE(Grammar)

using Point             = const Arcana::Grammar::Index*;
using Input             = std::string&;
using Lexeme            = std::string;
using Statement         = std::vector<std::string>;




///////////////////////////////////////////////////////////////////////////////
// MODULE NEMASPACEs
///////////////////////////////////////////////////////////////////////////////

BEGIN_MODULE(Support)



///////////////////////////////////////////////////////////////////////////////
// PUBLIC FUNCTIONS
///////////////////////////////////////////////////////////////////////////////


/// @brief Parse command line arguments
/// @param[in] argc argument count
/// @param[in] argv argument values
/// @return Arguments instance
Arguments   ParseArgs(int argc, char** argv);


/// @brief remove whitespace character for the left of the string
/// @param s the input string
/// @return a copy left trimmed of @ref s
std::string ltrim(const std::string& s);


/// @brief Function used to represent a TokenType
/// @param[in] type 
/// @return string representation of TokenType
std::string TokenTypeRepr(const Scan::TokenType type);


/// @brief Function used to represent a vector of TokenType 
/// @param[in] type 
/// @return string representation of TokenType separeted by 'or'
std::string TerminalRepr(const Grammar::Terminal& type);


/// @brief Function used to represent a vector of vector of TokenType
/// @param[in] type 
/// @return string representation of TokenType separeted by 'or'
std::string NonTerminalRepr(const Grammar::NonTerminal& type);


/// @brief Function used to represent a set of vector of TokenType
/// @param[in] type 
/// @return string representation of TokenType separeted by 'or'
std::string UniqueNonTerminalRepr(const Grammar::UniqueNonTerminal& type);


/// @brief Function used to represent a Rule
/// @param[in] type 
/// @return string representation of Rule
std::string RuleRepr(const Grammar::Rule type);



END_MODULE(Support)


#endif /* __ARCANA_SUPPORT__H__ */