#ifndef __ARCANA_SEMANTIC__H__
#define __ARCANA_SEMANTIC__H__


///////////////////////////////////////////////////////////////////////////////
// INCLUDES
///////////////////////////////////////////////////////////////////////////////

#include <map>
#include <set>
#include <vector>

#include "Lexer.h"
#include "Defines.h"
#include "Support.h"


///////////////////////////////////////////////////////////////////////////////
// MODULE NEMASPACEs
///////////////////////////////////////////////////////////////////////////////

BEGIN_MODULE(Grammar)


///////////////////////////////////////////////////////////////////////////////
// FORWARDED DECLARATIONS
///////////////////////////////////////////////////////////////////////////////

class Engine;


///////////////////////////////////////////////////////////////////////////////
// PUBLIC ENUMS 
///////////////////////////////////////////////////////////////////////////////

/// @brief Enum used to define rules
enum class Rule : uint32_t
{
    UNDEFINED          = 0,
    VARIABLE_ASSIGN       ,
    EMPTY_LINE            ,
    ATTRIBUTE             ,
    BUILTIN_TASK_DECL     ,
    TASK_DECL             ,
    TASK_CALL             ,
    USING                 ,
};


enum class VARIABLE_ASSIGN : uint32_t 
{
    VARNAME            = 0,                          
    ASSIGN                ,               
    VALUE                 ,              
    GRAMMAR_END           ,
};


enum class EMPTY_LINE : uint32_t 
{
    GRAMMAR_END        = 0,
};


enum class ATTRIBUTE : uint32_t 
{
    AT                 = 0,         
    ATTRNAME              ,
    ATTROPTION            ,
    GRAMMAR_END           ,
};


enum class BUILTIN_TASK_DECL : uint32_t 
{
    RESERVED           = 0,
    TASKNAME              ,
    ROUNDLP               ,
    PARAMS                ,
    ROUNDRP               ,
    SEMICOLON             ,
    GRAMMAR_END           ,
};


enum class TASK_DECL : uint32_t 
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


enum class TASK_CALL : uint32_t 
{
    TASKNAME           = 0, 
    ROUNDLP               , 
    PARAMS                , 
    ROUNDRP               , 
    SEMICOLON             , 
    GRAMMAR_END           ,
};


enum class USING : uint32_t 
{
    RESERVED           = 0, 
    SCRIPT                , 
    GRAMMAR_END           ,
};


///////////////////////////////////////////////////////////////////////////////
// PUBLIC STRUCTS & CLASSES
///////////////////////////////////////////////////////////////////////////////

class Index
{
    friend Engine;
public:
    Index() : start(0), end(0), any(false) {}

    std::size_t start;
    std::size_t end;
    Scan::Token token;

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
    Rule               type;
    std::vector<Index> indexes;
    
    struct
    {
        Scan::Token       token;
        UniqueNonTerminal estream;
        UniqueRule        semtypes;   
        bool              presence;
    } Error;

    bool isValid () const noexcept { return valid;          }
    bool isError () const noexcept { return Error.presence; }

    const Index* operator [] (const std::size_t s) { return &indexes[s]; }
};



class Engine
{
public:
    Engine();

    void match(const Scan::Token& token, Match& match);
    
private:
    void _collect_input(const Scan::Token& token, const Scan::TokenType tt, const Rule st, const uint32_t pos);
    void _reset();

    struct EngineCache
    {
        UniqueRule                  keys;
        std::map<Rule, uint32_t>    data; 
        std::map<Rule, std::string> items; 

        void reset() { keys.clear(); data.clear(); items.clear(); }
    } _cache;
    
    Production                         _rules;
    std::map<Rule, std::vector<Index>> _index;
};



END_MODULE(Grammar)


#endif /* __ARCANA_SEMANTIC__H__ */
