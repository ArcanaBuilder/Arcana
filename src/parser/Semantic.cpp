#include "Semantic.h"

#include "Support.h"
#include "TableHelper.h"

#include <regex>
#include <memory>
#include <thread>
#include <charconv>
#include <algorithm>
#include <unordered_map>


USE_MODULE(Arcana::Semantic);



//    ███████╗████████╗██████╗ ██╗   ██╗ ██████╗████████╗███████╗
//    ██╔════╝╚══██╔══╝██╔══██╗██║   ██║██╔════╝╚══██╔══╝██╔════╝
//    ███████╗   ██║   ██████╔╝██║   ██║██║        ██║   ███████╗
//    ╚════██║   ██║   ██╔══██╗██║   ██║██║        ██║   ╚════██║
//    ███████║   ██║   ██║  ██║╚██████╔╝╚██████╗   ██║   ███████║
//    ╚══════╝   ╚═╝   ╚═╝  ╚═╝ ╚═════╝  ╚═════╝   ╚═╝   ╚══════╝
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



//    ██╗   ██╗███████╗██╗███╗   ██╗ ██████╗ ███████╗
//    ██║   ██║██╔════╝██║████╗  ██║██╔════╝ ██╔════╝
//    ██║   ██║███████╗██║██╔██╗ ██║██║  ███╗███████╗
//    ██║   ██║╚════██║██║██║╚██╗██║██║   ██║╚════██║
//    ╚██████╔╝███████║██║██║ ╚████║╚██████╔╝███████║
//     ╚═════╝ ╚══════╝╚═╝╚═╝  ╚═══╝ ╚═════╝ ╚══════╝
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




//    ██████╗ ██╗   ██╗██╗     ███████╗███████╗
//    ██╔══██╗██║   ██║██║     ██╔════╝██╔════╝
//    ██████╔╝██║   ██║██║     █████╗  ███████╗
//    ██╔══██╗██║   ██║██║     ██╔══╝  ╚════██║
//    ██║  ██║╚██████╔╝███████╗███████╗███████║
//    ╚═╝  ╚═╝ ╚═════╝ ╚══════╝╚══════╝╚══════╝
//                                                                                                                                                            


static const AttributeMap Known_Attributes = 
{         
    { "profile"        , Attr::Type::PROFILE      },          
    { "pub"            , Attr::Type::PUBLIC       },                   
    { "always"         , Attr::Type::ALWAYS       },          
    { "requires"       , Attr::Type::REQUIRES     },   
    { "then"           , Attr::Type::THEN         },          
    { "map"            , Attr::Type::MAP          },    
    { "multithread"    , Attr::Type::MULTITHREAD  },     
    { "main"           , Attr::Type::MAIN         },  
    { "interpreter"    , Attr::Type::INTERPRETER  },  
    { "flushcache"     , Attr::Type::FLUSHCACHE   },       
    { "echo"           , Attr::Type::ECHO         },          
};



static const UsingMap Known_Usings = 
{        
    { "profiles"       , { {               },  Using::Type::PROFILES    } },   
    { "default"        , { { "interpreter" },  Using::Type::INTERPRETER } },   
    { "threads"        , { {               },  Using::Type::THREADS     } },   
};



static const std::vector<std::string> _attributes =
{
    "profile"       ,
    "pub"           ,
    "always"        ,
    "requires"      ,
    "then"          ,
    "map"           ,
    "multithread"   ,
    "main"          ,
    "interpreter"   ,
    "flushcache"    ,
    "echo"          ,
};



static const std::vector<std::string> _usings =
{
    "profiles",
    "default" ,
};





//    ███╗   ███╗ █████╗  ██████╗██████╗  ██████╗ ███████╗
//    ████╗ ████║██╔══██╗██╔════╝██╔══██╗██╔═══██╗██╔════╝
//    ██╔████╔██║███████║██║     ██████╔╝██║   ██║███████╗
//    ██║╚██╔╝██║██╔══██║██║     ██╔══██╗██║   ██║╚════██║
//    ██║ ╚═╝ ██║██║  ██║╚██████╗██║  ██║╚██████╔╝███████║
//    ╚═╝     ╚═╝╚═╝  ╚═╝ ╚═════╝╚═╝  ╚═╝ ╚═════╝ ╚══════╝
//                                                        


#define SEM_OK()                 SemanticOutput{}
#define SEM_NOK(err)             { Semantic_Result::AST_RESULT__NOK, err       }
#define SEM_NOK_HINT(err, hint)  { Semantic_Result::AST_RESULT__NOK, err, hint }








//     ██████╗██╗      █████╗ ███████╗███████╗    ██╗███╗   ███╗██████╗ ██╗     
//    ██╔════╝██║     ██╔══██╗██╔════╝██╔════╝    ██║████╗ ████║██╔══██╗██║     
//    ██║     ██║     ███████║███████╗███████╗    ██║██╔████╔██║██████╔╝██║     
//    ██║     ██║     ██╔══██║╚════██║╚════██║    ██║██║╚██╔╝██║██╔═══╝ ██║     
//    ╚██████╗███████╗██║  ██║███████║███████║    ██║██║ ╚═╝ ██║██║     ███████╗
//     ╚═════╝╚══════╝╚═╝  ╚═╝╚══════╝╚══════╝    ╚═╝╚═╝     ╚═╝╚═╝     ╚══════╝
//                                                                              


Engine::Engine()
    :
    _main_count(0)
{
    // ATTRIBUTE RULES      
    _attr_rules[_I(Attr::Type::PROFILE     )] = { Attr::Qualificator::REQUIRED_PROPERTY, Attr::Count::ONE      , { Attr::Target::TASK, Attr::Target::VARIABLE } };           
    _attr_rules[_I(Attr::Type::PUBLIC      )] = { Attr::Qualificator::NO_PROPERY       , Attr::Count::ZERO     , { Attr::Target::TASK, Attr::Target::VARIABLE } };    
    _attr_rules[_I(Attr::Type::ALWAYS      )] = { Attr::Qualificator::NO_PROPERY       , Attr::Count::ZERO     , { Attr::Target::TASK,                        } };    
    _attr_rules[_I(Attr::Type::REQUIRES    )] = { Attr::Qualificator::REQUIRED_PROPERTY, Attr::Count::UNLIMITED, { Attr::Target::TASK,                        } }; 
    _attr_rules[_I(Attr::Type::THEN        )] = { Attr::Qualificator::REQUIRED_PROPERTY, Attr::Count::UNLIMITED, { Attr::Target::TASK,                        } };           
    _attr_rules[_I(Attr::Type::MAP         )] = { Attr::Qualificator::REQUIRED_PROPERTY, Attr::Count::ONE      , {                     Attr::Target::VARIABLE } };     
    _attr_rules[_I(Attr::Type::MULTITHREAD )] = { Attr::Qualificator::NO_PROPERY       , Attr::Count::ZERO     , { Attr::Target::TASK,                        } };    
    _attr_rules[_I(Attr::Type::MAIN        )] = { Attr::Qualificator::NO_PROPERY       , Attr::Count::ZERO     , { Attr::Target::TASK,                        } };     
    _attr_rules[_I(Attr::Type::INTERPRETER )] = { Attr::Qualificator::REQUIRED_PROPERTY, Attr::Count::ONE      , { Attr::Target::TASK,                        } };       
    _attr_rules[_I(Attr::Type::FLUSHCACHE  )] = { Attr::Qualificator::NO_PROPERY       , Attr::Count::ZERO     , { Attr::Target::TASK,                        } }; 
    _attr_rules[_I(Attr::Type::ECHO        )] = { Attr::Qualificator::NO_PROPERY       , Attr::Count::ZERO     , { Attr::Target::TASK,                        } };         
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
        return SEM_NOK_HINT(ss.str(), Support::FindClosest(_attributes, name));
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
            return SEM_NOK(ss.str());
        }
        else if (props_count != 1 && rule.count == Attr::Count::ONE)
        {
            ss << "Attribute " << "‘" << ANSI_BMAGENTA << name << ANSI_RESET << "’" << " requires one option, not " << props_count;
            return SEM_NOK(ss.str());
        }
    }
    else
    {
        // CHECK FOR PROPERTIES PRESENCE, BUT THE RULE NOT ADMIT THEM
        if (props_count > 0)
        {
            ss << "Attribute " << "‘" << ANSI_BMAGENTA << name << ANSI_RESET << "’" << " requires no option";
            return SEM_NOK(ss.str());
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
            return SEM_NOK_HINT(ss.str(), Support::FindClosest(profiles, property[0]));
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
            return SEM_NOK_HINT(ss.str(), Support::FindClosest(keys, name));
        }
    }
    // IF THE ATTRIBUTE IS 'MAIN'
    else if (attr == Attr::Type::MAIN)
    {
        // CHECK FOR THE UNIQUENESS
        if (_main_count > 0)
        {
            ss << "Cannot tag multiple tasks with attribute " << "‘" << ANSI_BMAGENTA << name << ANSI_RESET << "’";
            return SEM_NOK(ss.str());
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
            return SEM_NOK(ss.str());
        }
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
            return SEM_NOK(ss.str());
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



SemanticOutput Engine::Collect_Task(const std::string& name, const std::string& inputs, const Task::Instrs& instrs)
{
    std::stringstream ss;

    Task::Inputs      task_inputs = Support::split(inputs);

    InstructionTask   task { name, task_inputs, instrs };
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
            return SEM_NOK(ss.str());
        }
    }

    if (task.hasAttribute(Attr::Type::REQUIRES))
    {
        auto& properties = task.getProperties(Attr::Type::REQUIRES);

        if (std::find(properties.begin(), properties.end(), name) != properties.end())
        {
            ss << "Attribute " << ANSI_BMAGENTA << "@dependency" << ANSI_RESET << " with property " << "‘" << ANSI_BMAGENTA << name << ANSI_RESET << "’ cannot be auto referencing";
            return SEM_NOK(ss.str());
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



SemanticOutput Engine::Collect_Using(const std::string& what, const std::string& opt)
{
    std::stringstream ss;
    Attr::Properties  options = Arcana::Support::split(opt);
    Using::Rule       rule;

    // CHECK IF THE USING IS KNOWN 
    if (auto it = Known_Usings.find(what); it != Known_Usings.end())
    {
        rule = it->second;
    }
    else
    {
        ss << "Unknown " << "‘" << ANSI_BMAGENTA << what << ANSI_RESET << "’ for statement " << ANSI_BMAGENTA << "using" << ANSI_RESET ;
        return SEM_NOK_HINT(ss.str(), Support::FindClosest(_usings, what));
    }

    // IF 'INTERPRETER' IS SELECTED
    if (rule.using_type == Using::Type::INTERPRETER)
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
            return SEM_NOK(ss.str());
        }

        // CHECK IF PROPERTY[0] (ATTRIBUTE) IS KNOWN
        const auto& attr = std::find(rule.valid_attr.begin(), rule.valid_attr.end(), options[0]);

        if (attr == rule.valid_attr.end())
        {
            ss << "Unknown attribute " << "‘" << ANSI_BMAGENTA << options[0] << ANSI_RESET << "’ for statement " << ANSI_BMAGENTA << "using " << what << ANSI_RESET ;
            return SEM_NOK_HINT(ss.str(), Support::FindClosest(rule.valid_attr, options[0]));
        }
        
        // CHECK IF THE ATTRIBUTE IS FOLLOWER BY TASKS NAME
        if (options.size() == 1)
        {
            ss << "Statement " << "‘" << ANSI_BMAGENTA << "using default " << options[0] << ANSI_RESET << "’ must be followed by interpeter path";
            return SEM_NOK(ss.str());
        }

        // CHECK IF THE INTERPRETER EXISTS
        if (!Support::file_exists(options[1]))
        {
            ss << "Interpreter " << "‘" << ANSI_BMAGENTA << options[1] << ANSI_RESET << "’ is missing or unknown";
            return SEM_NOK(ss.str());
        }

        _env.default_interpreter = options[1];
    }

    // IF 'PROFILES' IS SELECTED
    else if (rule.using_type == Using::Type::PROFILES)
    {
        // CHECK FOR THE SIZE
        if (options.size() == 0)
        {
            ss << "Statement " << "‘" << ANSI_BMAGENTA << "using profiles" << ANSI_RESET << "’ must be followed by profiles name";
            return SEM_NOK(ss.str());
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
                return SEM_NOK(ss.str());
            }
        }
    }

    // IF 'THREADS' IS SELECTED
    else if (rule.using_type == Using::Type::THREADS)
    {
        // CHECK FOR THE SIZE
        if (options.size() != 1)
        {
            ss << "Statement " << "‘" << ANSI_BMAGENTA << "using multithread" << ANSI_RESET << "’ must be followed maximum threads allowed";
            return SEM_NOK(ss.str());
        }

        // CHECK IF ITS A NUMBER
        int max_threads = 0;
        const char* begin = options[0].data();
        const char* end   = options[0].data() + options[0].size();

        auto [ptr, ec] = std::from_chars(begin, end, max_threads);

        if (ec != std::errc{} || ptr != end || max_threads <= 0)
        {
            ss << "Invalid value for multithread: " << ANSI_BMAGENTA  << options[0] << ANSI_RESET << ". Expected a positive integer.";
            return SEM_NOK(ss.str());
        }

        _env.max_threads = max_threads;
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

            return Arcana_Result::ARCANA_RESULT__NOK;
        }
        // CHECK IF THE SELECTED TASK HAS THE ATTRIBUTE 'PUBLIC'
        else if (!task.value().get().hasAttribute(Semantic::Attr::Type::PUBLIC))
        {
            ERR("Requested task " << ANSI_BMAGENTA << args.task.value << ANSI_RESET << " does not have " << ANSI_BMAGENTA << "public" << ANSI_RESET << " attribute");
            return Arcana_Result::ARCANA_RESULT__NOK;
        }
        
        // TOOGLE ATTRIBUTE 'MAIN' IF TASK IS SPECIFIED
        auto old_main_task = Table::GetValue(ftable, Attr::Type::MAIN);
        
        if (old_main_task)
        {
            old_main_task.value().get().removeAttribute(Attr::Type::MAIN);
        }

        task.value().get().attributes.push_back({
            "main",
            Attr::Type::MAIN, 
            {}
        });
    }
    else
    {
        auto main_task = Table::GetValues(ftable, profile.profiles, Attr::Type::MAIN);

        if (!main_task.size())
        {
            ERR("No main task specified, make it explicit in the arcfile with the @main attribute or pass a task on the command line");
            return Arcana_Result::ARCANA_RESULT__NOK;
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

            return Arcana_Result::ARCANA_RESULT__NOK;
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

    const std::array<Attr::Type, 2> attributes = { Attr::Type::REQUIRES, Attr::Type::THEN };
    
    for (const auto attr : attributes)
    {
        // IF ATTRIBUTE IS FOUND FROM FTABLE
        auto tasks = Table::GetValues(ftable, profile.profiles, attr);
        
        // FOR EACH TASK 
        for (auto& ref : tasks)
        {
            auto& task = ref.get();
            auto props = task.getProperties(attr);
    
            // CHECK IF THE DEPENDECY IS CONGRUENT, OTHERWISE RAISE AN ERROR
            for (auto& p : props)
            {
                auto it = ftable.find(p);
    
                if (it == ftable.end())
                {
                    auto closest = Support::FindClosest(Table::Keys(ftable), p);
    
                    ss << "Invalid dependecy ‘" << ANSI_BMAGENTA << p << ANSI_RESET << "’ for task " << ANSI_BMAGENTA << task.task_name << ANSI_RESET << std::endl;
                    if (closest) ss << "[" << ANSI_BGREEN << "HINT" << ANSI_RESET << "]  Did you mean " << ANSI_BCYAN << closest.value() << ANSI_RESET << "?";
                    return ss.str();
                }
        
                // ENQUEUE THE DEPENDECY IN ORDER
                if (attr == Attr::Type::REQUIRES)
                {
                    task.dependecies.push_back(std::cref(ftable.at(p)));
                }
                else
                {
                    task.thens.push_back(std::cref(ftable.at(p)));
                }
            }
        }
    }

    // CHECK FOR INTERPRETERS
    if (default_interpreter.empty())
    {
        default_interpreter = "/bin/bash";
    }

    for (auto& [_, task] : ftable)
    {
        if (task.hasAttribute(Attr::Type::INTERPRETER))
        {
            task.interpreter = task.getProperties(Attr::Type::INTERPRETER).at(0);
        }
        else
        {
            task.interpreter = default_interpreter;
        }
    }

    return std::nullopt;
}



const std::optional<std::string> Enviroment::Expand() noexcept
{
    // LAMBDA USED TO EXPAND A STATEMENT
    auto expand_one = [this](std::string& stmt, std::vector<std::string>& vars) noexcept
    {
        std::stringstream        ss;
        std::vector<ExpandMatch> matches;
        bool                     intern_satisfied = false;
        
        for (const auto& var : vars)
        {
            std::regex intern_re(R"(\{arc:(__profile__|__version__)\})");
            for (std::sregex_iterator it(stmt.begin(), stmt.end(), intern_re), end; it != end && !intern_satisfied; ++it)
            {   
                std::string new_stmt  = stmt.substr(0, it->position());

                if ((*it)[1].compare("__profile__") == 0)
                {
                    new_stmt += (profile.selected.empty()) ? "None" : profile.selected;
                } 
                else if ((*it)[1].compare("__version__") == 0)
                {
                    new_stmt += __ARCANA__VERSION__STR__;
                }

                new_stmt             += stmt.substr(it->position() + (*it)[0].length(), stmt.length() - it->position() + (*it)[0].length());
                stmt = new_stmt;
            }

            intern_satisfied = true;

            std::regex var_re("(\\{arc:)" + var + "(\\})");
            for (std::sregex_iterator it(stmt.begin(), stmt.end(), var_re), end; it != end; ++it)
            {   
                std::size_t start = it->position();
                matches.push_back({var, start, start + var.size() + 6});
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
    
    if (var_keys.empty()) return std::nullopt;

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
        for (const auto& input : task.task_inputs)
        {
            if (std::find(var_keys.begin(), var_keys.end(), input) == var_keys.end())
            {
                std::stringstream ss;
                ss << "Invalid input " << ANSI_BMAGENTA << input << ANSI_RESET << " for task " << ANSI_BMAGENTA << name << ANSI_RESET << ": Undefined variable";
                return ss.str();
            }
        }

        for (auto& instr : task.task_instrs) 
        {
            expand_one(instr, var_keys);
        }
    }

    // HANDLE GLOB EXPANSION
    if (auto res = Table::ExpandGlob(vtable); !res.empty())
    {
        return res;
    }

    // HANDLE MAPPED VARS EXPANSION
    if (auto res = Table::Map(vtable); !res.empty())
    {
        return res;
    }

    // HANDLE MAX THREADS   
    auto machine_max_threads = std::thread::hardware_concurrency();

    if (max_threads == 0 || max_threads > machine_max_threads)
    {
        max_threads = machine_max_threads;
    }

    return std::nullopt;
}