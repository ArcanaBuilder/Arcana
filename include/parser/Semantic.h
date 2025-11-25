#ifndef __ARCANA_SEMANTIC__H__
#define __ARCANA_SEMANTIC__H__

#include <map>
#include <set>
#include <vector>

#include "Util.h"
#include "Lexer.h"


BEGIN_MODULE(Parser)



enum class SemanticType : uint32_t
{
    UNDEFINED          = 0,
    VARIABLE_ASSIGN       ,
    EMPTY_LINE            ,
    ATTRIBUTE             ,
    BUILTIN_TASK_DECL     ,
    TASK_DECL             ,
    TASK_CALL             ,
};



enum class Grammar_VARIABLE_ASSIGN : uint32_t 
{
    VARNAME            = 0,                          
    ASSIGN                ,               
    VALUE                 ,              
    GRAMMAR_END           ,
};


enum class Grammar_EMPTY_LINE : uint32_t 
{
    GRAMMAR_END        = 0,
};


enum class Grammar_ATTRIBUTE : uint32_t 
{
    AT                 = 0,         
    ATTRNAME              ,
    GRAMMAR_END           ,
};


enum class Grammar_BUILTIN_TASK_DECL : uint32_t 
{
    RESERVED           = 0,
    TASKNAME              ,
    ROUNDLP               ,
    PARAMS                ,
    ROUNDRP               ,
    CURLYLP               ,
    CURLYRP               ,
    GRAMMAR_END           ,
};

enum class Grammar_TASK_DECL : uint32_t 
{
    RESERVED           = 0,
    TASKNAME              ,
    ROUNDLP               ,
    PARAMS                ,
    ROUNDRP               ,
    NEWLINE               ,
    CURLYLP               ,
    INSTRUCTIONS          ,
    CURLYRP               ,
    GRAMMAR_END           ,
};


enum class Grammar_TASK_CALL : uint32_t 
{
    TASKNAME           = 0, 
    ROUNDLP               , 
    PARAMS                , 
    ROUNDRP               , 
    SEMICOLON             , 
    GRAMMAR_END           ,
};





std::string SemanticTypeRepr(const SemanticType type);


using SemanticNode        = std::vector<TokenType>;
using SemanticStream      = std::vector<SemanticNode>;
using SemanticStreams     = std::map<SemanticType, SemanticStream>;



struct SemanticCache
{
    std::set<SemanticType>              keys;
    std::map<SemanticType, uint32_t>    data; 
    std::map<SemanticType, std::string> items; 

    void reset() { keys.clear(); data.clear(); items.clear(); }
};


class Semantic;

class Index
{
    friend Semantic;
public:
    Index() : start(0), end(0), any(false) {}

    std::size_t start;
    std::size_t end;
    Token       token;

private:
    bool any;

    void reset() noexcept
    {
        start = 0;
        end   = 0;
        any   = false;
    }
};


struct Match
{
    bool               valid;
    SemanticType       type;
    std::vector<Index> indexes;
    
    struct
    {
        Token        token;
        SemanticNode found;
        bool         presence;
    } Error;

    bool isValid () const noexcept { return valid;          }
    bool isError () const noexcept { return Error.presence; }

    const Index* operator [] (const std::size_t s) { return &indexes[s]; }
};






class Semantic
{
public:
    Semantic();

    void match(const Token& token, Match& match);
    
private:
    void _collect_input(const Token& token, const TokenType tt, const SemanticType st, const uint32_t pos);
    void _reset();

    SemanticCache   _cache;
    SemanticStreams _streams;
    std::map<SemanticType, std::vector<Index>> _indexes;
};



END_MODULE(Parser)


#endif /* __ARCANA_SEMANTIC__H__ */
