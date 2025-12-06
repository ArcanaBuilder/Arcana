#ifndef __ARCANA_UTIL_SUPPORT__H__
#define __ARCANA_UTIL_SUPPORT__H__


#include <map>
#include <set>
#include <cctype>
#include <string>
#include <vector>
#include <limits>
#include <climits> 
#include <variant>
#include <optional>
#include <string_view>


#include "Defines.h"



//    ███████╗ ██████╗ ██████╗ ██╗    ██╗ █████╗ ██████╗ ██████╗ ███████╗
//    ██╔════╝██╔═══██╗██╔══██╗██║    ██║██╔══██╗██╔══██╗██╔══██╗██╔════╝
//    █████╗  ██║   ██║██████╔╝██║ █╗ ██║███████║██████╔╝██║  ██║███████╗
//    ██╔══╝  ██║   ██║██╔══██╗██║███╗██║██╔══██║██╔══██╗██║  ██║╚════██║
//    ██║     ╚██████╔╝██║  ██║╚███╔███╔╝██║  ██║██║  ██║██████╔╝███████║
//    ╚═╝      ╚═════╝ ╚═╝  ╚═╝ ╚══╝╚══╝ ╚═╝  ╚═╝╚═╝  ╚═╝╚═════╝ ╚══════╝
//                                                                                                                                                                            

BEGIN_MODULE(Scan)

class Lexer;

enum class TokenType : uint32_t;

END_MODULE(Scan)


BEGIN_MODULE(Grammar)

class Index;

struct Match;

enum class Rule : uint32_t;

END_MODULE(Grammar)


BEGIN_MODULE(Support)

struct SemanticOutput;

END_MODULE(Support)





//    ███████╗████████╗██████╗ ██╗   ██╗ ██████╗████████╗███████╗
//    ██╔════╝╚══██╔══╝██╔══██╗██║   ██║██╔════╝╚══██╔══╝██╔════╝
//    ███████╗   ██║   ██████╔╝██║   ██║██║        ██║   ███████╗
//    ╚════██║   ██║   ██╔══██╗██║   ██║██║        ██║   ╚════██║
//    ███████║   ██║   ██║  ██║╚██████╔╝╚██████╗   ██║   ███████║
//    ╚══════╝   ╚═╝   ╚═╝  ╚═╝ ╚═════╝  ╚═════╝   ╚═╝   ╚══════╝
//           


BEGIN_MODULE(Support)

/// @brief structure used to hold command line arguments
struct Arguments
{
    std::string arcfile;

    struct
    {
        std::string value;
        bool        found;
    } 
    task,
    profile;

    bool debug_jobs;
    bool clear_cache;

    Arguments() 
        : 
        arcfile("arcfile"),
        task{"", false},
        profile{"", false},
        debug_jobs(false),
        clear_cache(false)
    {}
};


struct ParserError
{
    Scan::Lexer& lexer;
    
    Arcana_Result operator () (const std::string& ctx, const Grammar::Match& match) const;
};


struct SemanticError
{
    Scan::Lexer& lexer;
    
    Arcana_Result operator () (const std::string& ctx, const Support::SemanticOutput&, const Grammar::Match&) const;
};


struct PostProcError
{
    Scan::Lexer& lexer;
    
    Arcana_Result operator () (const std::string& ctx, const std::string& err) const;
};


END_MODULE(Support)




//    ██╗   ██╗███████╗██╗███╗   ██╗ ██████╗ ███████╗
//    ██║   ██║██╔════╝██║████╗  ██║██╔════╝ ██╔════╝
//    ██║   ██║███████╗██║██╔██╗ ██║██║  ███╗███████╗
//    ██║   ██║╚════██║██║██║╚██╗██║██║   ██║╚════██║
//    ╚██████╔╝███████║██║██║ ╚████║╚██████╔╝███████║
//     ╚═════╝ ╚══════╝╚═╝╚═╝  ╚═══╝ ╚═════╝ ╚══════╝
//                                                   

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


















//    ███╗   ███╗ ██████╗ ██████╗ ██╗   ██╗██╗     ███████╗    ███████╗██╗   ██╗██████╗ ██████╗  ██████╗ ██████╗ ████████╗
//    ████╗ ████║██╔═══██╗██╔══██╗██║   ██║██║     ██╔════╝    ██╔════╝██║   ██║██╔══██╗██╔══██╗██╔═══██╗██╔══██╗╚══██╔══╝
//    ██╔████╔██║██║   ██║██║  ██║██║   ██║██║     █████╗      ███████╗██║   ██║██████╔╝██████╔╝██║   ██║██████╔╝   ██║   
//    ██║╚██╔╝██║██║   ██║██║  ██║██║   ██║██║     ██╔══╝      ╚════██║██║   ██║██╔═══╝ ██╔═══╝ ██║   ██║██╔══██╗   ██║   
//    ██║ ╚═╝ ██║╚██████╔╝██████╔╝╚██████╔╝███████╗███████╗    ███████║╚██████╔╝██║     ██║     ╚██████╔╝██║  ██║   ██║   
//    ╚═╝     ╚═╝ ╚═════╝ ╚═════╝  ╚═════╝ ╚══════╝╚══════╝    ╚══════╝ ╚═════╝ ╚═╝     ╚═╝      ╚═════╝ ╚═╝  ╚═╝   ╚═╝   
//                                                                                                                        





BEGIN_MODULE(Support)





//    ███████╗████████╗██████╗ ██╗   ██╗ ██████╗████████╗███████╗
//    ██╔════╝╚══██╔══╝██╔══██╗██║   ██║██╔════╝╚══██╔══╝██╔════╝
//    ███████╗   ██║   ██████╔╝██║   ██║██║        ██║   ███████╗
//    ╚════██║   ██║   ██╔══██╗██║   ██║██║        ██║   ╚════██║
//    ███████║   ██║   ██║  ██║╚██████╔╝╚██████╗   ██║   ███████║
//    ╚══════╝   ╚═╝   ╚═╝  ╚═╝ ╚═════╝  ╚═════╝   ╚═╝   ╚══════╝
//                                                                                                               

struct StringViewHash
{
    using is_transparent = void;

    std::size_t operator()(std::string_view s) const noexcept;
};


struct StringViewEq
{
    using is_transparent = void;

    bool operator() (std::string_view a, std::string_view b) const noexcept;
};


struct SemanticOutput
{
    Semantic_Result  result;
    std::string      err;
    std::string      hint;

    SemanticOutput() 
        :
        result(Semantic_Result::AST_RESULT__OK),
        err(""),
        hint("")
    {}

    SemanticOutput(const Semantic_Result result, const std::string& err) 
        :
        result(result),
        err(err),
        hint("")
    {}

    SemanticOutput(const Semantic_Result result, const std::string& err, const std::string& hint) 
        :
        result(result),
        err(err),
        hint(hint)
    {}

    SemanticOutput(const Semantic_Result result, const std::string& err, const std::optional<std::string>& opt_hint) 
        :
        result(result),
        err(err)
    {
        if (opt_hint)
        {
            hint = opt_hint.value();
        }
    }
};



struct SplitResult
{
    bool                     ok;
    std::vector<std::string> tokens;
    std::string              error;
};






//    ██████╗ ██╗   ██╗██████╗ ██╗     ██╗ ██████╗    ███████╗██╗   ██╗███╗   ██╗ ██████╗███████╗
//    ██╔══██╗██║   ██║██╔══██╗██║     ██║██╔════╝    ██╔════╝██║   ██║████╗  ██║██╔════╝██╔════╝
//    ██████╔╝██║   ██║██████╔╝██║     ██║██║         █████╗  ██║   ██║██╔██╗ ██║██║     ███████╗
//    ██╔═══╝ ██║   ██║██╔══██╗██║     ██║██║         ██╔══╝  ██║   ██║██║╚██╗██║██║     ╚════██║
//    ██║     ╚██████╔╝██████╔╝███████╗██║╚██████╗    ██║     ╚██████╔╝██║ ╚████║╚██████╗███████║
//    ╚═╝      ╚═════╝ ╚═════╝ ╚══════╝╚═╝ ╚═════╝    ╚═╝      ╚═════╝ ╚═╝  ╚═══╝ ╚═════╝╚══════╝
//                                                                                                                                                                                                        


/// @brief Parse command line arguments
/// @param[in] argc argument count
/// @param[in] argv argument values
/// @return Arguments instance
std::variant<Support::Arguments, std::string>   ParseArgs(int argc, char** argv);


bool file_exists(const std::string& filename);



/// @brief remove whitespace character for the left of the string
/// @param s the input string
/// @return a copy left trimmed of @ref s
std::string ltrim(const std::string& s);


/// @brief remove whitespace character for the left of the string
/// @param s the input string
/// @return a copy left trimmed of @ref s
std::string rtrim(const std::string& s);


inline char toLowerAscii(char c) noexcept;


std::vector<std::string> split(const std::string& s, char sep = ' ') noexcept;

SplitResult split_quoted(const std::string& s, char sep = ' ') noexcept;

std::optional<long long> to_number(const std::string& s);

std::string generate_mangling(const std::string& target, const std::string& mangling);


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


std::optional<std::string> FindClosest(const std::vector<std::string> & list,
                                const std::string & target,
                                std::size_t max_distance = std::numeric_limits<std::size_t>::max()) noexcept;


END_MODULE(Support)


#endif /* __ARCANA_UTIL_SUPPORT__H__ */