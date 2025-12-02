#include "Grammar.h"
#include <algorithm>

USE_MODULE(Arcana);
USE_MODULE(Arcana::Grammar);



//     ██████  ██████   █████  ███    ███ ███    ███  █████  ██████      ██ ████████ ███████ ███    ███ ███████     
//    ██       ██   ██ ██   ██ ████  ████ ████  ████ ██   ██ ██   ██     ██    ██    ██      ████  ████ ██          
//    ██   ███ ██████  ███████ ██ ████ ██ ██ ████ ██ ███████ ██████      ██    ██    █████   ██ ████ ██ ███████     
//    ██    ██ ██   ██ ██   ██ ██  ██  ██ ██  ██  ██ ██   ██ ██   ██     ██    ██    ██      ██  ██  ██      ██     
//     ██████  ██   ██ ██   ██ ██      ██ ██      ██ ██   ██ ██   ██     ██    ██    ███████ ██      ██ ███████     
//                                                                                                                  
//                                                                                                                  
//    ███████ ████████ ██████  ██    ██  ██████ ████████                                                            
//    ██         ██    ██   ██ ██    ██ ██         ██                                                               
//    ███████    ██    ██████  ██    ██ ██         ██                                                               
//         ██    ██    ██   ██ ██    ██ ██         ██                                                               
//    ███████    ██    ██   ██  ██████   ██████    ██                                                               
//                                                                                                                  
//                                                                                                                  


struct Rules
{
    Rules& operator |  (const Scan::TokenType type); 
    Rules& operator |  (const Rules& streamer); 
    Rules& operator || (Scan::TokenType type);

    NonTerminal    buffer;
};

Rules operator | (Scan::TokenType lhs, Scan::TokenType rhs) 
{
    Rules b;
    Terminal     lnode { lhs };
    Terminal     rnode { rhs };

    b.buffer.push_back(lnode);
    b.buffer.push_back(rnode);
    return b;
}

Rules operator || (Scan::TokenType lhs, Scan::TokenType rhs) 
{
    Rules b;
    Terminal     node { lhs, rhs };

    b.buffer.push_back(node);
    return b;
}


Rules& Rules::operator | (const Scan::TokenType type)
{
    Terminal node { type };

    buffer.push_back(node);
    return *this;
}

Rules& Rules::operator | (const Rules& streamer)
{
    buffer.insert(buffer.end(), streamer.buffer.begin(), streamer.buffer.end());
    return *this;
}


Rules& Rules::operator || (const Scan::TokenType type)
{
    buffer.back().push_back(type);
    return *this;
}






//     ██████  ██████   █████  ███    ███ ███    ███  █████  ██████      
//    ██       ██   ██ ██   ██ ████  ████ ████  ████ ██   ██ ██   ██     
//    ██   ███ ██████  ███████ ██ ████ ██ ██ ████ ██ ███████ ██████      
//    ██    ██ ██   ██ ██   ██ ██  ██  ██ ██  ██  ██ ██   ██ ██   ██     
//     ██████  ██   ██ ██   ██ ██      ██ ██      ██ ██   ██ ██   ██     
//                                                                       
//                                                                       
//    ██████  ██    ██ ██      ███████ ███████                           
//    ██   ██ ██    ██ ██      ██      ██                                
//    ██████  ██    ██ ██      █████   ███████                           
//    ██   ██ ██    ██ ██      ██           ██                           
//    ██   ██  ██████  ███████ ███████ ███████                           
//                                                                       
//                                                                       




static Rules rule_VARIABLE_ASSIGNMENT = 
{
    Scan::TokenType::IDENTIFIER                           |
    Scan::TokenType::ASSIGN                               |
    Scan::TokenType::ANY                                  |
    ( Scan::TokenType::NEWLINE || Scan::TokenType::SEMICOLON || Scan::TokenType::ENDOFFILE )
};


static Rules rule_EMPTY_LINE = 
{
    ( Scan::TokenType::NEWLINE || Scan::TokenType::ENDOFFILE )
};


static Rules rule_ATTRIBUTE = 
{
    Scan::TokenType::AT           |
    Scan::TokenType::IDENTIFIER   |
    Scan::TokenType::ANY          |
    ( Scan::TokenType::NEWLINE || Scan::TokenType::SEMICOLON )
};


static Rules rule_TASK_DECL = 
{
    Scan::TokenType::TASK         |
    Scan::TokenType::IDENTIFIER   |
    Scan::TokenType::ROUNDLP      |
    Scan::TokenType::ROUNDRP      |
    Scan::TokenType::OPT_NEWLINE  |
    Scan::TokenType::CURLYLP      |
    Scan::TokenType::ANY          |
    Scan::TokenType::CURLYRP      |
    ( Scan::TokenType::NEWLINE || Scan::TokenType::SEMICOLON || Scan::TokenType::ENDOFFILE )  
};


static Rules rule_IMPORT = 
{
    Scan::TokenType::IMPORT       |
    Scan::TokenType::ANY          |
    ( Scan::TokenType::NEWLINE || Scan::TokenType::SEMICOLON || Scan::TokenType::ENDOFFILE )  
};


static Rules rule_USING = 
{
    Scan::TokenType::USING        |
    Scan::TokenType::IDENTIFIER   |
    Scan::TokenType::ANY          |
    ( Scan::TokenType::NEWLINE || Scan::TokenType::SEMICOLON || Scan::TokenType::ENDOFFILE )  
};






//     ██████  ██████   █████  ███    ███ ███    ███  █████  ██████                     
//    ██       ██   ██ ██   ██ ████  ████ ████  ████ ██   ██ ██   ██                    
//    ██   ███ ██████  ███████ ██ ████ ██ ██ ████ ██ ███████ ██████                     
//    ██    ██ ██   ██ ██   ██ ██  ██  ██ ██  ██  ██ ██   ██ ██   ██                    
//     ██████  ██   ██ ██   ██ ██      ██ ██      ██ ██   ██ ██   ██                    
//                                                                                      
//                                                                                      
//    ███████ ███    ██  ██████  ██ ███    ██ ███████     ██ ███    ███ ██████  ██      
//    ██      ████   ██ ██       ██ ████   ██ ██          ██ ████  ████ ██   ██ ██      
//    █████   ██ ██  ██ ██   ███ ██ ██ ██  ██ █████       ██ ██ ████ ██ ██████  ██      
//    ██      ██  ██ ██ ██    ██ ██ ██  ██ ██ ██          ██ ██  ██  ██ ██      ██      
//    ███████ ██   ████  ██████  ██ ██   ████ ███████     ██ ██      ██ ██      ███████ 
//                                                                                      
//                                                                                      


Engine::Engine()
{
    _rules[Rule::VARIABLE_ASSIGN  ] = rule_VARIABLE_ASSIGNMENT.buffer;
    _rules[Rule::EMPTY_LINE       ] = rule_EMPTY_LINE.buffer;
    _rules[Rule::ATTRIBUTE        ] = rule_ATTRIBUTE.buffer;
    _rules[Rule::TASK_DECL        ] = rule_TASK_DECL.buffer;
    _rules[Rule::IMPORT           ] = rule_IMPORT.buffer;
    _rules[Rule::USING            ] = rule_USING.buffer;

    _index[Rule::VARIABLE_ASSIGN  ] = std::vector<Index>(rule_VARIABLE_ASSIGNMENT.buffer.size());
    _index[Rule::EMPTY_LINE       ] = std::vector<Index>(rule_EMPTY_LINE.buffer.size());
    _index[Rule::ATTRIBUTE        ] = std::vector<Index>(rule_ATTRIBUTE.buffer.size());
    _index[Rule::TASK_DECL        ] = std::vector<Index>(rule_TASK_DECL.buffer.size());
    _index[Rule::IMPORT           ] = std::vector<Index>(rule_IMPORT.buffer.size());
    _index[Rule::USING            ] = std::vector<Index>(rule_USING.buffer.size());
}


void Engine::match(const Scan::Token& token, Match& match)
{
    bool                         error       = false;
    bool                         cached      = false;
    bool                         matched     = false;
    bool                         remove      = false;
    uint32_t                     position    = 0;
    Scan::TokenType              ttype       = token.type;
    Rule                         stype       = Rule::UNDEFINED;
    UniqueNonTerminal            estream;
    UniqueRule                   semtypes;
    std::set<Rule>               new_key_cache;
    Terminal::const_iterator     found;
    Terminal::const_iterator     wildcard;
    Terminal::const_iterator     opttoken;

    // FILL THE CACHE IF IT WAS FLUSHED OR UNINITIALIZED,
    // OTHERWISE, JUST USE IT 
    if (_cache.data.size() == 0)
    {
        for (const auto& pair : _rules) 
        {
            _cache.keys.insert(pair.first);
        }
    }
    else
    {
        cached = true;
    }

    // GET THE KEYS REF
    UniqueRule& keys = _cache.keys;

    // FOR EACH KEY IN KEYS UNTIL NO MATCH OCCURS
    for (auto it = keys.begin(); it != keys.end() && !matched; )
    {   
        // OBTAIN CURRENT KEY, RULE, RULE ITERATOR FROM CACHE
        const auto  key   = *it;
        const auto& value = _rules[key];
        position          = cached ? _cache.data[key] : 0;
        remove            = false;

        // IF THE RULE ITERATOR IS VALID
        if (position < value.size())
        {
            // OBTAIN RULE ITEM (LIST), THEN SEARCH THE PASSED TOKEN INTO IT
            // ALSO SEARCH IF THE CURRENT NODE HAS THE RULE 'ANY' OR 'OPT_NEWLINE' 
            auto& node     = value[position];
                  found    = std::find(node.begin(), node.end(), ttype);
                  wildcard = std::find(node.begin(), node.end(), Scan::TokenType::ANY);
                  opttoken = std::find(node.begin(), node.end(), Scan::TokenType::OPT_NEWLINE);
                  
            // APPEND THE CURRENT KEY MATCH RESULT
            // APPEND THE CURRENT NODE (COPY) TO THE ERROR STREAM
            estream.insert(node);
            semtypes.insert(key);

            // IF THERE IS A REGULAR MATCH WITH THE TOKEN AND THE RULE 
            if ( (found != node.end()))
            {
                // COLLECTS THE MATCHING POSITIONS 
                _collect_input(token, *found, key, position);

                // CACHE THE NEXT POSITION AND CHECK FOR A COMPLETE RULE MATCH
                _cache.data[key]  = ++position;
                matched           = (position == value.size());

                // SAVE THE CURRENT KEY IF THERE IS A COMPLETE MATCH
                if (matched)
                {
                    stype = key;
                }

                // IF THE CURRENT RULE IS A TASK DECLARATION
                if (key == Rule::TASK_DECL)
                {
                    // AND THE PASSED TOKEN IS A '{' OR '}' THE PROPER COUNTER MUST BE UPDATED 
                    if (ttype == Scan::TokenType::CURLYLP)
                    {
                        ++_cache.opened_curly_braces;
                    }
                    else if (ttype == Scan::TokenType::CURLYRP)
                    {
                        --_cache.opened_curly_braces;
                    }
                }

                // APPEND INTO THE NEW CACHE THE CURRENT KEY
                // THIS LOGIC WILL SAVE ONLY THE PROPER RULES FOR THE NEXT LOOP
                new_key_cache.insert(key);
            }

            // IF THE CURRENT RULE CONTAINS THE 'ANY'  
            else if (wildcard != node.end())
            {
                // JUST CHECK FOR THE LOOKAHEAD MATCHING 
                position++;
                auto& lookahead  = value[position];
                found            = std::find(lookahead.begin(), lookahead.end(), ttype);

                // IF THE CURRENT RULE IS A TASK DECLARATION
                if (key == Rule::TASK_DECL)
                {
                    // AND THE PASSED TOKEN IS A '{' OR '}' THE PROPER COUNTER MUST BE UPDATED 
                    if (ttype == Scan::TokenType::CURLYLP)
                    {
                        ++_cache.opened_curly_braces;
                    }
                    else if (ttype == Scan::TokenType::CURLYRP)
                    {
                        --_cache.opened_curly_braces;
                    }
                }

                // IF THE LOOKAHEAD FAILS, JUST COLLECT THE TOKEN
                if (found == lookahead.end())
                {
                    position -= 1;
                    _collect_input(token, *wildcard, key, position);
                }
                else
                {
                    // IF THE CURRENT RULE IS A TASK DECLARATION CHECK FOR THE CACHED BRACE COUNTER
                    // THE 'ANY' LOGIC MUST BE CONSIDER DONE.
                    if (key == Rule::TASK_DECL)
                    {
                        // IF THE CURLY BRACES COUNTER IS ZERO, A TASK BODY IS CLOSED
                        // THE 'ANY' LOGIC MUST BE CONSIDER DONE.
                        // OTHERWISE JUST COLLECT THE TOKEN 
                        if (_cache.opened_curly_braces == 0)
                        {
                            position += 1;
                            _collect_input(token, *found, key, position - 1);
                        }
                        else
                        {
                            position -= 1;
                            _collect_input(token, *wildcard, key, position);
                        }
                    }
                    else
                    {
                        position += 1;
                        _collect_input(token, *found, key, position - 1);
                    }
                }
                
                // FINALLY, CACHE THE NEW POSTION, AND CHECK FOR THE COMPLETE MATCH
                _cache.data[key]  = position;
                matched           = (position == value.size());

                // SAVE THE CURRENT KEY IF THERE IS A COMPLETE MATCH
                if (matched)
                {
                    stype = key;
                } 
            }

            // IF THE CURRENT RULES HAS 'OPT_NEWLINE' CHECK IF THE PASSED TOKEN
            // IS A 'NEWLINE'. IF ITS TRUE, COLLECT THEM, OTHERWISE JUST IGNORE THIS STATEMENT
            else if (opttoken != node.end())
            {
                _cache.data[key] = ++position;

                if (ttype == Scan::TokenType::NEWLINE)
                {
                    _collect_input(token, ttype, key, position);
                }
                else
                {
                    continue;
                }
            }

            // FINALLY, IF THERE ISNT ANY MATCH, THE CURRENT RULE MUST BE DELETED FROM TH CACHE 
            else
            {
                remove = true;
            }

            // IF THE RULE MUST BE DELETED, DO IT AND CHECK IF THE CACHE IS NOW EMPTY.
            // IF TRUE, NO RULE CAN MATCH THE STATEMENT: ERROR!
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

            // OTHERWISE, JUST INCR THE ITERATOR
            else
            {
                ++it;
            }
        }
    }

    // IF THE NEW CACHE WAS GENERATED, JUST UPDATE THE REGULAR CACHE
    if (new_key_cache.size())
    {
        _cache.keys = new_key_cache;
    }

    // POPULATE THE MATCH 
    match.valid          = matched;
    match.type           = stype;
    match.indexes        = _index[stype];
    match.Error.token    = token;
    match.Error.estream  = estream;
    match.Error.semtypes = semtypes;
    match.Error.presence = error;
    
    // IF THERE IS A MATCH, JUST RESET THE CACHE.
    if (matched) 
    {
        _reset();
    }

    return;
}


void Engine::_collect_input(const Scan::Token& token, const Scan::TokenType tt, const Rule st, const uint32_t pos)
{
    // OBTAIN THE REF FROM THE INDEXES VECTORS
    auto& index = _index[st][pos];

    // COMPUTE THE TOKEN AND THE END MATCH POSITION
    index.token = token;
    index.end   = token.start + token.lexeme.size();

    // IF THE TOKEN IS 'ANY':
    // FOR THE FIRST CALL: COMPUTE THE START POSITION AND FLAG THE STATUS
    if (tt == Scan::TokenType::ANY)
    {
        if (index.any == false)
        {
            index.start = token.start;
            index.any   = true;
        }
    }

    // IF A REGULAR TOKEN IS PARSED JUST COMPUTE THE START POSITION
    // IF THE ANY TOKEN WAS FLAGGED, JUST RESET IT
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


void Engine::_reset()
{
    _cache.reset();

    for (auto &pair : _index)
    {
        for (auto &idx : pair.second)
        {
            idx.reset();
        }
    }
}