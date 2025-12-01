#include "Semantic.h"

#include "Support.h"
#include "TableHelper.h"

#include <regex>
#include <memory>
#include <algorithm>
#include <unordered_map>


USE_MODULE(Arcana::Semantic);



//    ███████ ███████ ███    ███  █████  ███    ██ ████████ ██  ██████     ██████  ██████  ██ ██    ██  █████  ████████ ███████     
//    ██      ██      ████  ████ ██   ██ ████   ██    ██    ██ ██          ██   ██ ██   ██ ██ ██    ██ ██   ██    ██    ██          
//    ███████ █████   ██ ████ ██ ███████ ██ ██  ██    ██    ██ ██          ██████  ██████  ██ ██    ██ ███████    ██    █████       
//         ██ ██      ██  ██  ██ ██   ██ ██  ██ ██    ██    ██ ██          ██      ██   ██ ██  ██  ██  ██   ██    ██    ██          
//    ███████ ███████ ██      ██ ██   ██ ██   ████    ██    ██  ██████     ██      ██   ██ ██   ████   ██   ██    ██    ███████     
//                                                                                                                                  
//                                                                                                                                  
//    ████████ ██    ██ ██████  ███████ ███████                                                                                     
//       ██     ██  ██  ██   ██ ██      ██                                                                                          
//       ██      ████   ██████  █████   ███████                                                                                     
//       ██       ██    ██      ██           ██                                                                                     
//       ██       ██    ██      ███████ ███████                                                                                     
//                                                                                                                                  
//                                                                                                                                  
                                                                                                      

struct ExpandMatch
{
    const std::string* text;  
    std::size_t        start;  
    std::size_t        end;  

    ExpandMatch(const std::string& s, std::size_t a, std::size_t b) noexcept
        : text(&s), start(a), end(b)
    {}
};



//    ███████ ███████ ███    ███  █████  ███    ██ ████████ ██  ██████     ██████  ██████  ██ ██    ██  █████  ████████ ███████     
//    ██      ██      ████  ████ ██   ██ ████   ██    ██    ██ ██          ██   ██ ██   ██ ██ ██    ██ ██   ██    ██    ██          
//    ███████ █████   ██ ████ ██ ███████ ██ ██  ██    ██    ██ ██          ██████  ██████  ██ ██    ██ ███████    ██    █████       
//         ██ ██      ██  ██  ██ ██   ██ ██  ██ ██    ██    ██ ██          ██      ██   ██ ██  ██  ██  ██   ██    ██    ██          
//    ███████ ███████ ██      ██ ██   ██ ██   ████    ██    ██  ██████     ██      ██   ██ ██   ████   ██   ██    ██    ███████     
//                                                                                                                                  
//                                                                                                                                  
//    ██    ██ ███████ ██ ███    ██  ██████  ███████                                                                                
//    ██    ██ ██      ██ ████   ██ ██       ██                                                                                     
//    ██    ██ ███████ ██ ██ ██  ██ ██   ███ ███████                                                                                
//    ██    ██      ██ ██ ██  ██ ██ ██    ██      ██                                                                                
//     ██████  ███████ ██ ██   ████  ██████  ███████                                                                                
//                                                                                                                                  
//                                                                                                                                  

template < typename T >
using AbstractKeywordMap = std::unordered_map<
    std::string_view,
    T,
    Arcana::Support::StringViewHash,
    Arcana::Support::StringViewEq
>;


using AttributeMap   = AbstractKeywordMap<Attr::Type>;
using UsingMap       = AbstractKeywordMap<Using::Rule>;




//    ███████ ███████ ███    ███  █████  ███    ██ ████████ ██  ██████     ██████  ██    ██ ██      ███████ ███████ 
//    ██      ██      ████  ████ ██   ██ ████   ██    ██    ██ ██          ██   ██ ██    ██ ██      ██      ██      
//    ███████ █████   ██ ████ ██ ███████ ██ ██  ██    ██    ██ ██          ██████  ██    ██ ██      █████   ███████ 
//         ██ ██      ██  ██  ██ ██   ██ ██  ██ ██    ██    ██ ██          ██   ██ ██    ██ ██      ██           ██ 
//    ███████ ███████ ██      ██ ██   ██ ██   ████    ██    ██  ██████     ██   ██  ██████  ███████ ███████ ███████ 
//                                                                                                                  
//                                                                                                                  

static const AttributeMap Known_Attributes = 
{
    { "precompiler" , Attr::Type::PRECOMPILER  },          
    { "postcompiler", Attr::Type::POSTCOMPILER },          
    { "profile"     , Attr::Type::PROFILE      },          
    { "public"      , Attr::Type::PUBLIC       },          
    { "private"     , Attr::Type::PRIVATE      },          
    { "always"      , Attr::Type::ALWAYS       },          
    { "dependecy"   , Attr::Type::DEPENDECY    },          
    { "callable"    , Attr::Type::CALLABLE     },    
    { "map"         , Attr::Type::MAP          },    
    { "multithread" , Attr::Type::MULTITHREAD  },     
    { "main"        , Attr::Type::MAIN         },  
    { "interpreter" , Attr::Type::INTERPRETER  },          
};


static const UsingMap Known_Usings = 
{
    { "order"          , { { "precompiler", "postcompiler" },  Using::Type::ORDER       } },          
    { "profiles"       , { {                               },  Using::Type::PROFILES    } },   
    { "default"        , { { "interpreter"                 },  Using::Type::INTERPRETER } },   
};






//    ███████ ███████ ███    ███  █████  ███    ██ ████████ ██  ██████     ██ ███    ███ ██████  ██      
//    ██      ██      ████  ████ ██   ██ ████   ██    ██    ██ ██          ██ ████  ████ ██   ██ ██      
//    ███████ █████   ██ ████ ██ ███████ ██ ██  ██    ██    ██ ██          ██ ██ ████ ██ ██████  ██      
//         ██ ██      ██  ██  ██ ██   ██ ██  ██ ██    ██    ██ ██          ██ ██  ██  ██ ██      ██      
//    ███████ ███████ ██      ██ ██   ██ ██   ████    ██    ██  ██████     ██ ██      ██ ██      ███████ 
//                                                                                                       
// 


Engine::Engine()
    :
    _main_count(0)
{
    // ATTRIBUTE RULES
    _attr_rules[_I(Attr::Type::PRECOMPILER )] = { Attr::Qualificator::NO_PROPERY       , Attr::Count::ZERO     , { Attr::Target::TASK                         } };              
    _attr_rules[_I(Attr::Type::POSTCOMPILER)] = { Attr::Qualificator::NO_PROPERY       , Attr::Count::ZERO     , { Attr::Target::TASK,                        } };          
    _attr_rules[_I(Attr::Type::PROFILE     )] = { Attr::Qualificator::REQUIRED_PROPERTY, Attr::Count::ONE      , { Attr::Target::TASK, Attr::Target::VARIABLE } };           
    _attr_rules[_I(Attr::Type::PUBLIC      )] = { Attr::Qualificator::NO_PROPERY       , Attr::Count::ZERO     , { Attr::Target::TASK, Attr::Target::VARIABLE } };    
    _attr_rules[_I(Attr::Type::PRIVATE     )] = { Attr::Qualificator::NO_PROPERY       , Attr::Count::ZERO     , { Attr::Target::TASK, Attr::Target::VARIABLE } };    
    _attr_rules[_I(Attr::Type::ALWAYS      )] = { Attr::Qualificator::NO_PROPERY       , Attr::Count::ZERO     , { Attr::Target::TASK,                        } };    
    _attr_rules[_I(Attr::Type::DEPENDECY   )] = { Attr::Qualificator::REQUIRED_PROPERTY, Attr::Count::UNLIMITED, { Attr::Target::TASK,                        } };           
    _attr_rules[_I(Attr::Type::CALLABLE    )] = { Attr::Qualificator::NO_PROPERY       , Attr::Count::ZERO     , { Attr::Target::TASK,                        } };
    _attr_rules[_I(Attr::Type::MAP         )] = { Attr::Qualificator::REQUIRED_PROPERTY, Attr::Count::ONE      , {                     Attr::Target::VARIABLE } };     
    _attr_rules[_I(Attr::Type::MULTITHREAD )] = { Attr::Qualificator::NO_PROPERY       , Attr::Count::ZERO     , { Attr::Target::TASK,                        } };    
    _attr_rules[_I(Attr::Type::MAIN        )] = { Attr::Qualificator::NO_PROPERY       , Attr::Count::ZERO     , { Attr::Target::TASK,                        } };     
    _attr_rules[_I(Attr::Type::INTERPRETER )] = { Attr::Qualificator::REQUIRED_PROPERTY, Attr::Count::ONE      , { Attr::Target::TASK,                        } };        
}


SemanticOutput Engine::Collect_Attribute(const std::string& name, const std::string&  prop)
{
    std::stringstream ss;

    Attr::Type       attr     = Attr::Type::ATTRIBUTE__UNKNOWN;
    Attr::Properties property = Arcana::Support::split(prop);

    // CHECK IF THE ATTRIBUTE IS KNOWN
    if (auto it = Known_Attributes.find(name); it != Known_Attributes.end())
    {
        attr = it->second;
    }

    if (attr == Attr::Type::ATTRIBUTE__UNKNOWN)
    {
        ss << "Attribute " << "‘" << ANSI_BMAGENTA << name << ANSI_RESET << "’" << " not recognized";
        return { Semantic_Result::AST_RESULT__INVALID_ATTR, ss.str() };
    }

    // ITERATE THE RULE OF THE ATTRIBUTE AND CHECK FOR THE CONGRUENCY
    const auto& rule = _attr_rules[_I(attr)];
    auto props_count = property.size();

    // IF THE PROPERTY OF THE ATTRIBUTE IS REQUIRED
    if (rule == Attr::Qualificator::REQUIRED_PROPERTY)
    {
        // CHECK THE NUMBER OF PROPERTIES
        if (props_count == 0)
        {
            ss << "Attribute " << "‘" << ANSI_BMAGENTA << name << ANSI_RESET << "’" << " requires at least one option";
            return { Semantic_Result::AST_RESULT__INVALID_ATTR_PROP, ss.str() };
        }
        else if (props_count != 1 && rule.count == Attr::Count::ONE)
        {
            ss << "Attribute " << "‘" << ANSI_BMAGENTA << name << ANSI_RESET << "’" << " requires one option, not " << props_count;
            return { Semantic_Result::AST_RESULT__INVALID_ATTR_PROP, ss.str() };
        }
    }
    else
    {
        // CHECK FOR PROPERTIES PRESENCE, BUT THE RULE NOT ADMIT THEM
        if (props_count > 0)
        {
            ss << "Attribute " << "‘" << ANSI_BMAGENTA << name << ANSI_RESET << "’" << " requires no option";
            return { Semantic_Result::AST_RESULT__INVALID_ATTR_PROP, ss.str() };
        }
    }

    // IF THE ATTRIBUTE IS 'PROFILE'
    if (attr == Attr::Type::PROFILE)
    {
        // CHECK IF THE PROFILE IS KNOWN
        const auto& profiles = _env.profile.profiles;

        if (std::find(profiles.begin(), profiles.end(), property[0]) == profiles.end())
        {
            ss << "Profile " << "‘" << ANSI_BMAGENTA << property[0] << ANSI_RESET << "’" << " must be declared via " << ANSI_BMAGENTA << "‘using profile <profilenames>’" << ANSI_RESET;
            return { Semantic_Result::AST_RESULT__INVALID_ATTR_PROP, ss.str() };
        }
    }
    // IF THE ATTRBUTE IS 'MAP'
    else if (attr == Attr::Type::MAP)
    {
        // CHECK THE VTABLE FOR THE MAPPING VARIABLE
        auto keys = Table::Keys(_env.vtable);

        if (std::find(keys.begin(), keys.end(), property[0]) == keys.end())
        {
            ss << "Invalid mapping " << "‘" << ANSI_BMAGENTA << property[0] << ANSI_RESET << "’" << ": undeclared variable";
            return { Semantic_Result::AST_RESULT__INVALID_ATTR_PROP, ss.str() };
        }
    }
    // IF THE ATTRIBUTE IS 'MAIN'
    else if (attr == Attr::Type::MAIN)
    {
        // CHECK FOR THE UNIQUENESS
        if (_main_count > 0)
        {
            ss << "Cannot tag multiple tasks with attribute " << "‘" << ANSI_BMAGENTA << name << ANSI_RESET << "’";
            return { Semantic_Result::AST_RESULT__INVALID_ATTR, ss.str() };
        }

        _main_count = 1;
    }
    // IF THE ATTRIBUTE IS 'INTERPRETER'
    else if (attr == Attr::Type::INTERPRETER)
    {
        // CHECK IF THE INTERPRETER EXISTS
        if (!Support::file_exists(property[0]))
        {
            ss << "Interpreter " << "‘" << ANSI_BMAGENTA << property[0] << ANSI_RESET << "’ is missing or unknown";
            return { Semantic_Result::AST_RESULT__INVALID_ATTR, ss.str() };
        }

        _main_count = 1;
    }

    // ENQUEUE THE ATTRIBUTE
    _attr_pending.push_back({ 
        name,
        attr, 
        property
    });

    return SemanticOutput{};
}


SemanticOutput Engine::Collect_Assignment(const std::string& name, const std::string&  val)
{
    std::stringstream ss;
    InstructionAssign assign { name, val };

    // DEQUE THE ATTRIBUTES
    assign.attributes = _attr_pending;
    _attr_pending.clear();

    // CHECK IF THE ATTRIBUTES ARE VALID FOR ASSIGN STATEMENT
    for (const auto& attr : assign.attributes)
    {
        const auto& rule = _attr_rules[_I(attr.type)];

        if (std::find(rule.targets.begin(), rule.targets.end(), Attr::Target::VARIABLE) == rule.targets.end())
        {
            ss << "Attribute " << "‘" << ANSI_BMAGENTA << attr.name << ANSI_RESET << "’" << " is not valid for variable assignment";
            return { Semantic_Result::AST_RESULT__INVALID_ATTR_PROP, ss.str() };
        }
    }
    
    // GENERATE A MANGLING IF THE ASSIGNMENT IS TAGGED WITH THE ATTRIBUTE PROFILE
    const auto& attr = std::find(assign.attributes.begin(), assign.attributes.end(), Attr::Type::PROFILE);

    if (attr != assign.attributes.end())
    {
        _env.vtable[Support::generate_mangling(name, (*attr).props[0])] = assign;
    }
    else
    {
        _env.vtable[name] = assign;
    }

    return SemanticOutput{};
} 


SemanticOutput Engine::Collect_Task(const std::string& name, const std::string& param, const Task::Instrs& instrs)
{
    std::stringstream    ss;
    Support::SplitResult sr;

    // GENERATE PARAMS
    sr = Support::split_quoted(param);

    if (!sr.ok)
    {
        ss << "Invalid argument(s) for task " << "‘" << ANSI_BMAGENTA << name << ANSI_RESET << "’";
        return { Semantic_Result::AST_RESULT__INVALID_ARGUMENTS, ss.str() };
    }
    
    InstructionTask   task { name, sr.tokens, instrs };
    FTable&           ftable = _env.ftable;

    task.attributes = _attr_pending;
    _attr_pending.clear();

    // CHECK FOR ATTRIBUTES ARE VALID FOR TASK DECLARATIONS STATEMENT
    for (const auto& attr : task.attributes)
    {
        const auto& rule = _attr_rules[_I(attr.type)];

        if (std::find(rule.targets.begin(), rule.targets.end(), Attr::Target::TASK) == rule.targets.end())
        {
            ss << "Attribute " << "‘" << ANSI_BMAGENTA << attr.name << ANSI_RESET << "’" << " is not valid for tasks";
            return { Semantic_Result::AST_RESULT__INVALID_ATTR_PROP, ss.str() };
        }
    }

    // GENERATE A MANGLING IF THE ASSIGNMENT IS TAGGED WITH THE ATTRIBUTE PROFILE
    const auto& attr = std::find(task.attributes.begin(), task.attributes.end(), Attr::Type::PROFILE);

    if (attr != task.attributes.end())
    {
        ftable[Support::generate_mangling(name, (*attr).props[0])] = task;
    }
    else
    {
        ftable[name] = task;
    }

    return SemanticOutput{};
} 


SemanticOutput Engine::Collect_TaskCall(const std::string& name, const std::string& param)
{
    std::stringstream    ss;
    Support::SplitResult sr;

    // GENERATE PARAMS
    sr = Support::split_quoted(param);

    if (!sr.ok)
    {
        ss << "Invalid argument(s) for task call " << "‘" << ANSI_BMAGENTA << name << ANSI_RESET << "’";
        return { Semantic_Result::AST_RESULT__INVALID_ARGUMENTS, ss.str() };
    }

    // CHECK IF THE TASK CALL IS TAGGED, NOT ADMITTED
    if (_attr_pending.size())
    {
        ss << "Cannot use attibutes for task call: " << "‘" << ANSI_BMAGENTA << name << ANSI_RESET << "’";
        return { Semantic_Result::AST_RESULT__ATTR_NOT_ALLOWED, ss.str() };
    }
    
    // CHECK IF THE TASK CALL CAN BE PERFORMED: TASK PRESENCE
    const auto& task = _env.ftable.find(name);

    if (task == _env.ftable.end())
    {
        ss << "Cannot call non-existent task: " << "‘" << ANSI_BMAGENTA << name << ANSI_RESET << "’";
        return { Semantic_Result::AST_RESULT__INVALID_TASK_CALL, ss.str() };
    }

    // CHECK IF THE OBTAINED TASK IS MARKER 'CALLABLE'
    const auto& attributes = (*task).second.attributes;

    if (std::find(attributes.begin(), attributes.end(), Attr::Type::CALLABLE) == attributes.end())
    {
        ss << "Cannot call task marked: " << "‘" << ANSI_BMAGENTA << name << ANSI_RESET << "’ without the attribute" << ANSI_BMAGENTA << " @callable" << ANSI_RESET;
        return { Semantic_Result::AST_RESULT__INVALID_TASK_CALL, ss.str() };
    }

    // CHECK FOR THE PARAM CONGRUENCY
    const auto expected_count = (*task).second.task_params.size();
    const auto computed_count = sr.tokens.size();

    if (expected_count != computed_count)
    {
        ss << "Cannot call task " << "‘" << ANSI_BMAGENTA << name << ANSI_RESET << "’" << " with a different argument count" << std::endl;
        ss << "        Expected: " << expected_count << ", found: " << computed_count;
        return { Semantic_Result::AST_RESULT__INVALID_ARGUMENTS, ss.str() };
    }
    
    InstructionCall call { name, sr.tokens };

    _env.ctable[name] = call;

    return SemanticOutput{};
} 


SemanticOutput Engine::Collect_Using(const std::string& what, const std::string& opt)
{
    std::stringstream ss;
    Attr::Properties  options = Arcana::Support::split(opt);
    Attr::Type        attr_type;
    Using::Rule       rule;

    // CHECK IF THE USING IS KNOWN 
    if (auto it = Known_Usings.find(what); it != Known_Usings.end())
    {
        rule = it->second;
    }
    else
    {
        ss << "Unknown " << "‘" << ANSI_BMAGENTA << what << ANSI_RESET << "’ for statement " << ANSI_BMAGENTA << "using" << ANSI_RESET ;
        return { Semantic_Result::AST_RESULT__INVALID_ATTR, ss.str() };
    }

    // IF 'ORDER' IS SELECTED
    if (rule.using_type == Using::Type::ORDER || rule.using_type == Using::Type::INTERPRETER)
    {
        // CHECK FOR THE PROPERTIES SIZE
        if (options.size() == 0)
        {
            std::stringstream ss1;
            for (uint32_t iter = 0; iter < rule.valid_attr.size(); iter++)
            {
                if (iter > 0)
                {
                    ss1 << ", or ";
                }

                ss1 << ANSI_BMAGENTA << rule.valid_attr[iter] << ANSI_RESET; 
            }

            ss << "Statement " << "‘" << ANSI_BMAGENTA << "using " << what << ANSI_RESET << "’ must be followed by " << ss1.str();
            return { Semantic_Result::AST_RESULT__INVALID_ATTR, ss.str() };
        }

        // CHECK IF PROPERTY[0] (ATTRIBUTE) IS KNOWN
        const auto& attr = std::find(rule.valid_attr.begin(), rule.valid_attr.end(), options[0]);

        if (attr == rule.valid_attr.end())
        {
            ss << "Unknown attribute " << "‘" << ANSI_BMAGENTA << options[0] << ANSI_RESET << "’ for statement " << ANSI_BMAGENTA << "using " << what << ANSI_RESET ;
            return { Semantic_Result::AST_RESULT__INVALID_ATTR, ss.str() };
        }

        // GET THE ATTRIBUTE
        attr_type = Known_Attributes.at(options[0]);
        

        if (attr_type == Attr::Type::INTERPRETER)
        {
            // CHECK IF THE ATTRIBUTE IS FOLLOWER BY TASKS NAME
            if (options.size() == 1)
            {
                ss << "Statement " << "‘" << ANSI_BMAGENTA << "using default " << options[0] << ANSI_RESET << "’ must be followed by interpeter path";
                return { Semantic_Result::AST_RESULT__INVALID_ATTR, ss.str() };
            }

            // CHECK IF THE INTERPRETER EXISTS
            if (!Support::file_exists(options[1]))
            {
                ss << "Interpreter " << "‘" << ANSI_BMAGENTA << options[1] << ANSI_RESET << "’ is missing or unknown";
                return { Semantic_Result::AST_RESULT__INVALID_ATTR, ss.str() };
            }

            _env.default_interpreter = options[1];
        }
        else
        {
            auto& order = (attr_type == Attr::Type::PRECOMPILER) ? _env.preorder : _env.postorder;
            
            // CHECK IF THE ATTRIBUTE IS FOLLOWER BY TASKS NAME
            if (options.size() == 1)
            {
                ss << "Statement " << "‘" << ANSI_BMAGENTA << "using order " << options[0] << ANSI_RESET << "’ must be followed by tasks name";
                return { Semantic_Result::AST_RESULT__INVALID_ATTR, ss.str() };
            }
    
            // GET THE TASKS AND CHECK FOR ERRORS
            for (uint32_t iter = 1; iter < options.size(); ++iter)
            {
                if (std::find(order.begin(), order.end(), options[iter]) == order.end())
                {
                    order.push_back(options[iter]);
                }
                else
                {
                    ss << "Duplicate item in statement " << "‘" << ANSI_BMAGENTA << "using order" << ANSI_RESET << "’: ‘" << ANSI_BMAGENTA << options[iter] << ANSI_RESET <<"’" << ANSI_RESET ;
                    return { Semantic_Result::AST_RESULT__INVALID_ATTR, ss.str() };
                }
            }
        }
    }

    // IF 'PROFILES' IS SELECTED
    else if (rule.using_type == Using::Type::PROFILES)
    {
        // CHECK FOR THE SIZE
        if (options.size() == 0)
        {
            ss << "Statement " << "‘" << ANSI_BMAGENTA << "using profiles" << ANSI_RESET << "’ must be followed by profiles name";
            return { Semantic_Result::AST_RESULT__INVALID_ATTR, ss.str() };
        }

        // COLLECT THEM AND CHECK FOR ERRORS
        for (uint32_t iter = 0; iter < options.size(); ++iter)
        {
            if (std::find(_env.profile.profiles.begin(), _env.profile.profiles.end(), options[iter]) == _env.profile.profiles.end())
            {
                _env.profile.profiles.push_back(options[iter]);
            }
            else
            {
                ss << "Duplicate item in statement " << "‘" << ANSI_BMAGENTA << "using profiles" << ANSI_RESET << "’: ‘" << ANSI_BMAGENTA << options[iter] << ANSI_RESET <<"’" << ANSI_RESET ;
                return { Semantic_Result::AST_RESULT__INVALID_ATTR, ss.str() };
            }
        }
    }

    return SemanticOutput{};
}



Arcana_Result Enviroment::CheckArgs(const Arcana::Support::Arguments& args) noexcept
{
    // CHECK IF TASK IS PASSED VIA CLI
    if (args.task.found)
    {
        // CHECK IF THE TASK IS KNOWN, IF NOT THEN GIVA AN HINT
        auto task = Table::GetValue(ftable, args.task.value, profile.profiles);

        if (!task)
        {
            ERR("Unknown task " << ANSI_BMAGENTA << args.task.value << ANSI_RESET);

            auto keys    = Table::Keys(ftable);             
            auto closest = Support::FindClosest(keys, args.task.value);

            if (closest)
            {
                HINT("Did you mean " << ANSI_BCYAN << closest.value() << ANSI_RESET << "?");
            }

            return Arcana_Result::ARCANA_RESULT__INVALID_ARGS;
        }
        // CHECK IF THE SELECTED TASK HAS THE ATTRIBUTE 'PUBLIC'
        else if (!task.value().get().hasAttribute(Semantic::Attr::Type::PUBLIC))
        {
            ERR("Requested task " << ANSI_BMAGENTA << args.task.value << ANSI_RESET << " does not have " << ANSI_BMAGENTA << "public" << ANSI_RESET << " attribute");
            return Arcana_Result::ARCANA_RESULT__INVALID_ARGS;
        }
    }

    // IF A PROFILE IS PASSED VIA CLI
    if (args.profile.found)
    {
        // CHECK FOR THE PRESENCE, IF NOT THEN GIVA AN HINT
        if (std::find(profile.profiles.begin(), profile.profiles.end(), args.profile.value) == profile.profiles.end())
        {
            ERR("Requested profile " << ANSI_BMAGENTA << args.profile.value << ANSI_RESET << " is invalid!");

            auto closest = Support::FindClosest(profile.profiles, args.profile.value);

            if (closest)
            {
                HINT("Did you mean " << ANSI_BCYAN << closest.value() << ANSI_RESET << "?");
            }

            return Arcana_Result::ARCANA_RESULT__INVALID_ARGS;
        }

        profile.selected = args.profile.value;
    }
    else
    {
        // IF NO PROFILES ARE SPECFIED, JUST RETURN
        if (profile.profiles.empty()) return Arcana_Result::ARCANA_RESULT__OK;

        // FOR THE DEFAULT GET THE FIRST OF THEM
        profile.selected = profile.profiles[0];
    }
    
    // AFTER THE PROFILE SELECTION, REMOVE THE UNWANTED KEYS FROM THE TABLES AND
    // REMOVE THE MANGLINGS   
    Table::AlignOnProfile(vtable, profile.selected);
    Table::AlignOnProfile(ftable, profile.selected);
    
    return Arcana_Result::ARCANA_RESULT__OK;
}


const std::optional<std::string> Enviroment::AlignEnviroment() noexcept
{
    std::stringstream ss;

    const std::array<Ref<Order>,  2> orders = { preorder               , postorder                };
    const std::array<Ref<FList>,  2> flist  = { pretask                , posttask                 };
    const std::array<Attr::Type,  2> atype  = { Attr::Type::PRECOMPILER, Attr::Type::POSTCOMPILER };
    const std::array<std::string, 2> arepr  = { "precompiler"          , "postcompiler"           };

    // IF 'DEPENDENCY' ATTRIBUTE IS FOUND FROM FTABLE
    auto tasks = Table::GetValues(ftable, profile.profiles, Attr::Type::DEPENDECY);

    // FOR EACH TASK 
    for (auto& ref : tasks)
    {
        auto& task = ref.get();
        auto props = task.getProperties(Attr::Type::DEPENDECY);

        // CHECK IF THE DEPENDECY IS CONGRUENT, OTHERWISE RAISE AN ERROR
        for (auto& p : props->get())
        {
            if (ftable.find(p) == ftable.end())
            {
                ss << "Invalid dependecy ‘" << ANSI_BMAGENTA << p << ANSI_RESET << "’ for task " << ANSI_BMAGENTA << task.task_name << ANSI_RESET;
                return ss.str();
            }

            // ENQUEUE THE DEPENDECY IN ORDER
            task.dependecies.push_back(std::cref(ftable.at(p)));
        }
    }

    // ALIGN THE PRECOMPILER AND POSTCOMPLER TASKS
    for (uint32_t iter = 0; iter < orders.size(); ++iter)
    {
        if (orders[iter].get().size() > 0)
        {
            // FOR EACH MARKED TASK
            for (const auto& task : orders[iter].get())
            {
                // POP THE TASK FROM THE FTABLE AND ENQUEUE IN ORDER IN THE PROPER LIST
                // ALSO CHECK FOR ERRORS
                auto result = Table::TakeValue(ftable, task, profile.profiles, atype[iter]);
        
                if (result)
                {
                    flist[iter].get().push_back(result.value());
                }
                else
                {
                    ss << "Task ‘" << ANSI_BMAGENTA << task << ANSI_RESET << "’ is not marked with attribute " << ANSI_BMAGENTA << arepr[iter] << ANSI_RESET << " or does not exists";
                    return ss.str();
                }
            }
        }
        else
        {
            // FOR DEFAULT, THE DECLARATION ORDER IS USED
            flist[iter].get() = Table::TakeValues(ftable, profile.profiles, atype[iter]);
        }
    }


    // CHECK FOR DEFAULT INTERPRETER
    if (default_interpreter.empty())
    {
        default_interpreter = "/bin/bash";
    }

    return std::nullopt;
}


void Enviroment::Expand() noexcept
{
    // LAMBDA USED TO EXPAND A STATEMENT
    auto expand_one = [this](std::string& stmt, std::vector<std::string>& vars) noexcept
    {
        std::stringstream        ss;
        std::vector<ExpandMatch> matches;

        for (const auto& var : vars)
        {
            std::regex re("(^|\\{)" + var + "($|\\})");
            for (std::sregex_iterator it(stmt.begin(), stmt.end(), re); it != std::sregex_iterator(); ++it)
            {   
                std::size_t start = it->position();
                matches.push_back({var, start, start + var.size() + 2});
            }
        }

        if (!matches.empty())
        {
            std::size_t iterator = 0;

            std::sort(matches.begin(), matches.end(), [] (const ExpandMatch& a, const ExpandMatch& b) {
                return a.start < b.start;
            });
    
            for (const auto& match: matches)
            {
                const std::string& replacement = vtable[*match.text].var_value;

                ss << stmt.substr(iterator, match.start - iterator);
                ss << replacement;
                iterator = match.end;
            }

            auto& last = matches.back(); 

            if (last.end < stmt.length())
            {
                ss << stmt.substr(last.end, stmt.length() - iterator);
            }

            stmt = ss.str();
        }

    };

    // FOR EACH KEY IN VTABLE
    auto var_keys = Table::Keys(vtable);
    
    if (var_keys.empty()) return;

    // SORT BY VARIABLE NAME LENGTH
    std::sort(var_keys.begin(), var_keys.end(), [] (const std::string& a, const std::string& b) {
        return a.size() > b.size();
    });

    // ITERATE THE VTABLE AND TRY TO EXPAND
    for (auto& [name, var] : vtable)
    {
        expand_one(var.var_value, var_keys);
    }

    // ITERATE THE FTABLE AND TRY TO EXPAND
    for (auto& [name, task] : ftable)
    {
        for (auto& instr : task.task_instrs) 
        {
            expand_one(instr, var_keys);
        }
    }

    // ITERATE THE CTABLE AND TRY TO EXPAND
    for (auto& [name, call] : ctable)
    {
        for (auto& p : call.task_params) 
        {
            expand_one(p, var_keys);
        }
    }

    // ITERATE THE pretask LIST AND TRY TO EXPAND
    for (auto& pre : pretask)
    {
        for (auto& instr : pre.task_instrs) 
        {
            expand_one(instr, var_keys);
        }
    }

    // ITERATE THE posttask LIST AND TRY TO EXPAND
    for (auto& post : posttask)
    {
        for (auto& instr : post.task_instrs) 
        {
            expand_one(instr, var_keys);
        }
    }
}