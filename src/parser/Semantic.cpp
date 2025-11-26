#include "Semantic.h"
#include <algorithm>

USE_MODULE(Arcana::Parser);


#define ADD_STREAM(stream)              _streams [ #stream ] = stream.buffer


struct SemanticStreamer
{
    SemanticStreamer& operator |  (const TokenType type); 
    SemanticStreamer& operator |  (const SemanticStreamer& streamer); 
    SemanticStreamer& operator || (TokenType type);

    SemanticStream    buffer;
};

SemanticStreamer operator | (TokenType lhs, TokenType rhs) 
{
    SemanticStreamer b;
    SemanticNode     lnode { lhs };
    SemanticNode     rnode { rhs };

    b.buffer.push_back(lnode);
    b.buffer.push_back(rnode);
    return b;
}

SemanticStreamer operator || (TokenType lhs, TokenType rhs) 
{
    SemanticStreamer b;
    SemanticNode     node { lhs, rhs };

    b.buffer.push_back(node);
    return b;
}


SemanticStreamer& SemanticStreamer::operator | (const TokenType type)
{
    SemanticNode node { type };

    buffer.push_back(node);
    return *this;
}

SemanticStreamer& SemanticStreamer::operator | (const SemanticStreamer& streamer)
{
    buffer.insert(buffer.end(), streamer.buffer.begin(), streamer.buffer.end());
    return *this;
}


SemanticStreamer& SemanticStreamer::operator || (const TokenType type)
{
    buffer.back().push_back(type);
    return *this;
}






static SemanticStreamer VARIABLE_ASSIGNMENT = 
{
    TokenType::IDENTIFIER                           |
    TokenType::ASSIGN                               |
    TokenType::ANY                                  |
    ( TokenType::NEWLINE || TokenType::SEMICOLON || TokenType::ENDOFFILE )
};


static SemanticStreamer EMPTY_LINE = 
{
    ( TokenType::NEWLINE || TokenType::ENDOFFILE )
};


static SemanticStreamer ATTRIBUTE = 
{
    TokenType::AT           |
    TokenType::IDENTIFIER   |
    TokenType::NEWLINE
};


static SemanticStreamer BUILTIN_TASK_DECL = 
{
    TokenType::TASK         |
    TokenType::IDENTIFIER   |
    TokenType::ROUNDLP      |
    TokenType::ANY          |
    TokenType::ROUNDRP      |
    TokenType::SEMICOLON    |
    ( TokenType::NEWLINE || TokenType::ENDOFFILE )     
};

static SemanticStreamer TASK_DECL = 
{
    TokenType::TASK         |
    TokenType::IDENTIFIER   |
    TokenType::ROUNDLP      |
    TokenType::ANY          |
    TokenType::ROUNDRP      |
    TokenType::NEWLINE      |
    TokenType::CURLYLP      |
    TokenType::ANY          |
    TokenType::CURLYRP      |
    ( TokenType::NEWLINE || TokenType::ENDOFFILE )  
};


static SemanticStreamer TASK_CALL = 
{
    TokenType::IDENTIFIER   |
    TokenType::ROUNDLP      |
    TokenType::ANY          |
    TokenType::ROUNDRP      |
    TokenType::SEMICOLON    |
    ( TokenType::NEWLINE || TokenType::ENDOFFILE )  
};



std::string Arcana::Parser::SemanticTypeRepr(const SemanticType type)
{
    switch (type)
    {
        case SemanticType::UNDEFINED:         return "UNDEFINED";
        case SemanticType::VARIABLE_ASSIGN:   return "VARIABLE_ASSIGN";
        case SemanticType::EMPTY_LINE:        return "EMPTY_LINE";
        case SemanticType::ATTRIBUTE:         return "ATTRIBUTE";
        case SemanticType::BUILTIN_TASK_DECL: return "BUILTIN_TASK_DECL";
        case SemanticType::TASK_DECL:         return "TASK_DECL";
        case SemanticType::TASK_CALL:         return "TASK_CALL";
        default:                              return "<INVALID>";
    }
}



Semantic::Semantic() 
{
    _streams[SemanticType::VARIABLE_ASSIGN  ] = VARIABLE_ASSIGNMENT.buffer;
    _streams[SemanticType::EMPTY_LINE       ] = EMPTY_LINE.buffer;
    _streams[SemanticType::ATTRIBUTE        ] = ATTRIBUTE.buffer;
    _streams[SemanticType::BUILTIN_TASK_DECL] = BUILTIN_TASK_DECL.buffer;
    _streams[SemanticType::TASK_DECL        ] = TASK_DECL.buffer;
    _streams[SemanticType::TASK_CALL        ] = TASK_CALL.buffer;

    _indexes[SemanticType::VARIABLE_ASSIGN  ] = std::vector<Index>(VARIABLE_ASSIGNMENT.buffer.size());
    _indexes[SemanticType::EMPTY_LINE       ] = std::vector<Index>(EMPTY_LINE.buffer.size());
    _indexes[SemanticType::ATTRIBUTE        ] = std::vector<Index>(ATTRIBUTE.buffer.size());
    _indexes[SemanticType::BUILTIN_TASK_DECL] = std::vector<Index>(BUILTIN_TASK_DECL.buffer.size());
    _indexes[SemanticType::TASK_DECL        ] = std::vector<Index>(TASK_DECL.buffer.size());
    _indexes[SemanticType::TASK_CALL        ] = std::vector<Index>(TASK_CALL.buffer.size());
}


void Semantic::match(const Token& token, Match& match)
{
    bool                         error       = false;
    bool                         cached      = false;
    bool                         matched     = false;
    bool                         remove      = false;
    uint32_t                     position    = 0;
    TokenType                    ttype       = token.type;
    SemanticType                 stype       = SemanticType::UNDEFINED;
    SemanticStream               estream;
    std::set<SemanticType>       new_key_cache;
    SemanticNode::const_iterator found;
    SemanticNode::const_iterator wildcard;

    if (_cache.data.size() == 0)
    {
        for (const auto& pair : _streams) 
        {
            _cache.keys.insert(pair.first);
        }
    }
    else
    {
        cached = true;
    }

    std::set<SemanticType> &keys = _cache.keys;

    for (auto it = keys.begin(); it != keys.end() && !matched; )
    {   
        const auto  key   = *it;
        const auto& value = _streams[key];
        position          = cached ? _cache.data[key] : 0;
        remove            = false;

        if (position < value.size())
        {
            auto& node     = value[position];
                  found    = std::find(node.begin(), node.end(), ttype);
                  wildcard = std::find(node.begin(), node.end(), TokenType::ANY);
                  estream.push_back(node);

            if ( (found != node.end()))
            {
                _collect_input(token, *found, key, position);

                _cache.data[key]  = ++position;
                matched           = (position == value.size());

                if (matched)
                {
                    stype = key;
                }

                new_key_cache.insert(key);
            }
            else if (wildcard != node.end())
            {
                position++;
                auto& lookahead  = value[position];
                found            = std::find(lookahead.begin(), lookahead.end(), ttype);

                if (found == lookahead.end())
                {
                    position -= 1;
                    _collect_input(token, *wildcard, key, position);
                }
                else
                {
                    position += 1;
                    _collect_input(token, *found, key, position - 1);
                }
                
                _cache.data[key]  = position;
                matched           = (position == value.size());

                if (matched)
                {
                    stype = key;
                } 
            }
            else
            {
                remove = true;
            }

            if (remove)
            {
                it = keys.erase(it);

                if (keys.empty())
                {
                    error = true;
                    if (cached)
                    {
                        _cache.reset();
                    }

                    break;
                }
            }
            else
            {
                ++it;
            }
        }
    }

    if (new_key_cache.size())
    {
        _cache.keys = new_key_cache;
    }

    match.valid          = matched;
    match.type           = stype;
    match.indexes        = _indexes[stype];
    match.Error.token    = token;
    match.Error.estream  = estream;
    match.Error.presence = error;
    
    if (matched) 
    {
        _reset();
    }

    return;
}


void Semantic::_collect_input(const Token& token, const TokenType tt, const SemanticType st, const uint32_t pos)
{
    auto& index = _indexes[st][pos];

    index.token = token;
    index.end   = token.start + token.lexeme.size();

    if (tt == TokenType::ANY)
    {
        if (index.any == false)
        {
            index.start = token.start;
            index.any   = true;
        }
    }
    else
    {
        if (index.any == false)
        {
            index.start = token.start;
        }
        else
        {
            index.any   = false;
        }
    }
    
    return;
}


void Semantic::_reset()
{
    _cache.reset();

    for (auto &pair : _indexes)
    {
        for (auto &idx : pair.second)
        {
            idx.reset();
        }
    }
}