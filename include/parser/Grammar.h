#ifndef __ARCANA_SEMANTIC__H__
#define __ARCANA_SEMANTIC__H__


#include <map>
#include <set>
#include <vector>

#include "Lexer.h"
#include "Defines.h"
#include "Support.h"



BEGIN_MODULE(Grammar)




//    ███████╗ ██████╗ ██████╗ ██╗    ██╗ █████╗ ██████╗ ██████╗ ███████╗
//    ██╔════╝██╔═══██╗██╔══██╗██║    ██║██╔══██╗██╔══██╗██╔══██╗██╔════╝
//    █████╗  ██║   ██║██████╔╝██║ █╗ ██║███████║██████╔╝██║  ██║███████╗
//    ██╔══╝  ██║   ██║██╔══██╗██║███╗██║██╔══██║██╔══██╗██║  ██║╚════██║
//    ██║     ╚██████╔╝██║  ██║╚███╔███╔╝██║  ██║██║  ██║██████╔╝███████║
//    ╚═╝      ╚═════╝ ╚═╝  ╚═╝ ╚══╝╚══╝ ╚═╝  ╚═╝╚═╝  ╚═╝╚═════╝ ╚══════╝
//                                                                                                                                                                                                                                                                                                                                            

class Engine;





//    ██████╗ ██╗   ██╗██████╗ ██╗     ██╗ ██████╗    ███████╗███╗   ██╗██╗   ██╗███╗   ███╗███████╗
//    ██╔══██╗██║   ██║██╔══██╗██║     ██║██╔════╝    ██╔════╝████╗  ██║██║   ██║████╗ ████║██╔════╝
//    ██████╔╝██║   ██║██████╔╝██║     ██║██║         █████╗  ██╔██╗ ██║██║   ██║██╔████╔██║███████╗
//    ██╔═══╝ ██║   ██║██╔══██╗██║     ██║██║         ██╔══╝  ██║╚██╗██║██║   ██║██║╚██╔╝██║╚════██║
//    ██║     ╚██████╔╝██████╔╝███████╗██║╚██████╗    ███████╗██║ ╚████║╚██████╔╝██║ ╚═╝ ██║███████║
//    ╚═╝      ╚═════╝ ╚═════╝ ╚══════╝╚═╝ ╚═════╝    ╚══════╝╚═╝  ╚═══╝ ╚═════╝ ╚═╝     ╚═╝╚══════╝
//                                                                                                                                                                                                                  


/// @brief Enum used to define rules
enum class Rule : uint32_t
{
    UNDEFINED          = 0,
    VARIABLE_ASSIGN       ,
    EMPTY_LINE            ,
    ATTRIBUTE             ,
    TASK_DECL             ,
    IMPORT                ,
    USING                 ,
    MAPPING               ,
    ASSERT                ,
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



enum class TASK_DECL : uint32_t 
{
    RESERVED           = 0,
    TASKNAME              ,
    ROUNDLP               ,
    INPUTS                ,
    ROUNDRP               ,
    NEWLINE               ,
    CURLYLP               ,
    INSTRUCTIONS          ,
    CURLYRP               ,
    GRAMMAR_END           ,
};



enum class IMPORT : uint32_t 
{
    RESERVED           = 0, 
    SCRIPT                , 
    GRAMMAR_END           ,
};



enum class USING : uint32_t 
{
    RESERVED           = 0, 
    WHAT                  ,
    OPT                   , 
    GRAMMAR_END           ,
};



enum class MAPPING : uint32_t 
{
    RESERVED1          = 0, 
    ITEM_1                ,
    RESERVED2             ,
    RESERVED3             ,
    ITEM_2                , 
    GRAMMAR_END           ,
};


enum class ASSERT : uint32_t 
{
    RESERVED1          = 0, 
    RESERVED2             , 
    ITEM_1                ,
    RESERVED3             , 
    OP                    ,
    RESERVED4             , 
    ITEM_2                , 
    RESERVED5             , 
    GRAMMAR_END           ,
};




//    ██████╗ ██╗   ██╗██████╗ ██╗     ██╗ ██████╗     █████╗  ██████╗  ██████╗ ██████╗ ███████╗ ██████╗  █████╗ ████████╗███████╗███████╗
//    ██╔══██╗██║   ██║██╔══██╗██║     ██║██╔════╝    ██╔══██╗██╔════╝ ██╔════╝ ██╔══██╗██╔════╝██╔════╝ ██╔══██╗╚══██╔══╝██╔════╝██╔════╝
//    ██████╔╝██║   ██║██████╔╝██║     ██║██║         ███████║██║  ███╗██║  ███╗██████╔╝█████╗  ██║  ███╗███████║   ██║   █████╗  ███████╗
//    ██╔═══╝ ██║   ██║██╔══██╗██║     ██║██║         ██╔══██║██║   ██║██║   ██║██╔══██╗██╔══╝  ██║   ██║██╔══██║   ██║   ██╔══╝  ╚════██║
//    ██║     ╚██████╔╝██████╔╝███████╗██║╚██████╗    ██║  ██║╚██████╔╝╚██████╔╝██║  ██║███████╗╚██████╔╝██║  ██║   ██║   ███████╗███████║
//    ╚═╝      ╚═════╝ ╚═════╝ ╚══════╝╚═╝ ╚═════╝    ╚═╝  ╚═╝ ╚═════╝  ╚═════╝ ╚═╝  ╚═╝╚══════╝ ╚═════╝ ╚═╝  ╚═╝   ╚═╝   ╚══════╝╚══════╝
//                                                                                                                                                                                                                                                         


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

    bool isValid () noexcept { bool v = valid         ; valid          = false; return v; }
    bool isError () noexcept { bool v = Error.presence; Error.presence = false; return v; }

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
        EngineCache() : opened_curly_braces(0) {}

        UniqueRule                  keys;
        std::map<Rule, uint32_t>    data; 
        std::map<Rule, std::string> items; 

        std::size_t                 opened_curly_braces;

        void reset() { keys.clear(); data.clear(); items.clear(); opened_curly_braces = 0; }
    } _cache;
    
    Production                         _rules;
    std::map<Rule, std::vector<Index>> _index;
};



END_MODULE(Grammar)


#endif /* __ARCANA_SEMANTIC__H__ */
