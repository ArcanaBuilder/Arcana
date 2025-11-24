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
};



std::string SemanticTypeRepr(const SemanticType type);


using SemanticNode        = std::vector<TokenType>;
using SemanticStream      = std::vector<SemanticNode>;
using SemanticStreams     = std::map<SemanticType, SemanticStream>;



struct SemanticCache
{
    std::set<SemanticType>           keys;
    std::map<SemanticType, uint32_t> data; 

    void reset() { keys.clear(); data.clear(); }
};


struct Match
{
    bool         valid;
    SemanticType type;
    
    struct
    {
        Token        current;
        SemanticNode found;
        bool         presence;
    } Error;

    bool isValid () const noexcept { return valid;          }
    bool isError () const noexcept { return Error.presence; }
};






class Semantic
{
public:
    Semantic();

    Match match(const TokenType token);

private:
    SemanticStreams _streams;
    SemanticCache   _cache;
};



END_MODULE(Parser)


#endif /* __ARCANA_SEMANTIC__H__ */
