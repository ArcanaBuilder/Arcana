#include "Semantic.h"

#include "Glob.h"
#include "Core.h"
#include "Support.h"
#include "TableHelper.h"

#include <regex>
#include <memory>
#include <thread>
#include <charconv>
#include <algorithm>
#include <filesystem>
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


using AttributeMap   = Arcana::Support::AbstractKeywordMap<Attr::Type>;
using UsingMap       = Arcana::Support::AbstractKeywordMap<Using::Rule>;

namespace fs = std::filesystem;



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
    { "exclude"        , Attr::Type::EXCLUDE      },   
    { "ifos"           , Attr::Type::IFOS         },          
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
    "exclude"       ,
    "ifos"          ,
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
    _attr_rules[_I(Attr::Type::EXCLUDE     )] = { Attr::Qualificator::REQUIRED_PROPERTY, Attr::Count::ONE      , {                     Attr::Target::VARIABLE } };     
    _attr_rules[_I(Attr::Type::MULTITHREAD )] = { Attr::Qualificator::NO_PROPERY       , Attr::Count::ZERO     , { Attr::Target::TASK,                        } };    
    _attr_rules[_I(Attr::Type::MAIN        )] = { Attr::Qualificator::NO_PROPERY       , Attr::Count::ZERO     , { Attr::Target::TASK,                        } };     
    _attr_rules[_I(Attr::Type::INTERPRETER )] = { Attr::Qualificator::REQUIRED_PROPERTY, Attr::Count::ONE      , { Attr::Target::TASK,                        } };       
    _attr_rules[_I(Attr::Type::FLUSHCACHE  )] = { Attr::Qualificator::NO_PROPERY       , Attr::Count::ZERO     , { Attr::Target::TASK,                        } }; 
    _attr_rules[_I(Attr::Type::ECHO        )] = { Attr::Qualificator::NO_PROPERY       , Attr::Count::ZERO     , { Attr::Target::TASK,                        } };   
    _attr_rules[_I(Attr::Type::IFOS        )] = { Attr::Qualificator::REQUIRED_PROPERTY, Attr::Count::ONE      , {                     Attr::Target::VARIABLE } };         
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
        ss << "Attribute " << TOKEN_MAGENTA(name) << " not recognized";
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
            ss << "Attribute " << TOKEN_MAGENTA(name) << " requires at least one option";
            return SEM_NOK(ss.str());
        }
        else if (props_count != 1 && rule.count == Attr::Count::ONE)
        {
            ss << "Attribute " << TOKEN_MAGENTA(name) << " requires one option, not " << props_count;
            return SEM_NOK(ss.str());
        }
    }
    else
    {
        // CHECK FOR PROPERTIES PRESENCE, BUT THE RULE NOT ADMIT THEM
        if (props_count > 0)
        {
            ss << "Attribute " << TOKEN_MAGENTA(name) << " requires no option";
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
            ss << "Profile " << TOKEN_MAGENTA(property[0]) << " must be declared via " << ANSI_BMAGENTA << "using profile <profilenames>" << ANSI_RESET;
            return SEM_NOK_HINT(ss.str(), Support::FindClosest(profiles, property[0]));
        }
    }
    // IF THE ATTRBUTE IS 'MAP' or 'EXCLUDE'
    else if (attr == Attr::Type::MAP || attr == Attr::Type::EXCLUDE)
    {
        // CHECK THE VTABLE FOR THE MAPPING VARIABLE
        auto keys = Table::Keys(_env.vtable);

        if (std::find(keys.begin(), keys.end(), property[0]) == keys.end())
        {
            ss << "Invalid " << name << " " << TOKEN_MAGENTA(property[0]) << ": undeclared variable";
            return SEM_NOK_HINT(ss.str(), Support::FindClosest(keys, name));
        }
    }
    // IF THE ATTRIBUTE IS 'MAIN'
    else if (attr == Attr::Type::MAIN)
    {
        // CHECK FOR THE UNIQUENESS
        if (_main_count > 0)
        {
            ss << "Cannot tag multiple tasks with attribute " << TOKEN_MAGENTA(name);
            return SEM_NOK(ss.str());
        }

        _main_count = 1;
    }
    // IF THE ATTRIBUTE IS 'IFOS'
    else if (attr == Attr::Type::IFOS)
    {
        if (!Core::is_os(property[0]))
        {
            ss << "Invalid OS " << TOKEN_MAGENTA(property[0]);
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
            ss << "Attribute " << TOKEN_MAGENTA(attr.name) << " is not valid for variable assignment";
            return SEM_NOK(ss.str());
        }
    }
    
    // GENERATE A MANGLING IF THE ASSIGNMENT IS TAGGED WITH THE ATTRIBUTE PROFILE
    const auto& attr_profile = std::find(assign.attributes.begin(), assign.attributes.end(), Attr::Type::PROFILE);
    const auto& attr_if      = std::find(assign.attributes.begin(), assign.attributes.end(), Attr::Type::IFOS);

    if (attr_profile != assign.attributes.end())
    {
        _env.vtable[Support::generate_mangling(name, (*attr_profile).props[0])] = assign;
    }
    else if (attr_if != assign.attributes.end())
    {
        _env.vtable[Support::generate_mangling(name, (*attr_if).props[0])] = assign;
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
            ss << "Attribute " << TOKEN_MAGENTA(attr.name) << " is not valid for tasks";
            return SEM_NOK(ss.str());
        }
    }

    if (task.hasAttribute(Attr::Type::REQUIRES))
    {
        auto& properties = task.getProperties(Attr::Type::REQUIRES);

        if (std::find(properties.begin(), properties.end(), name) != properties.end())
        {
            ss << "Attribute " << TOKEN_MAGENTA("@requires") << " with property " << TOKEN_MAGENTA(name) << " cannot be auto referencing";
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
        ss << "Unknown " << TOKEN_MAGENTA(what) << " for statement " << TOKEN_MAGENTA("using") ;
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

                ss1 << TOKEN_MAGENTA(rule.valid_attr[iter]); 
            }

            ss << "Statement " << TOKEN_MAGENTA("using " << what ) << " must be followed by " << ss1.str();
            return SEM_NOK(ss.str());
        }

        // CHECK IF PROPERTY[0] (ATTRIBUTE) IS KNOWN
        const auto& attr = std::find(rule.valid_attr.begin(), rule.valid_attr.end(), options[0]);

        if (attr == rule.valid_attr.end())
        {
            ss << "Unknown attribute " << TOKEN_MAGENTA(options[0]) << " for statement " << TOKEN_MAGENTA("using " << what ) ;
            return SEM_NOK_HINT(ss.str(), Support::FindClosest(rule.valid_attr, options[0]));
        }
        
        // CHECK IF THE ATTRIBUTE IS FOLLOWER BY TASKS NAME
        if (options.size() == 1)
        {
            ss << "Statement " << TOKEN_MAGENTA("using default " << options[0] ) << " must be followed by interpeter path";
            return SEM_NOK(ss.str());
        }

        // CHECK IF THE INTERPRETER EXISTS
        if (!Support::file_exists(options[1]))
        {
            ss << "Interpreter " << TOKEN_MAGENTA(options[1]) << " is missing or unknown";
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
            ss << "Statement " << TOKEN_MAGENTA("using profiles") << " must be followed by profiles name";
            return SEM_NOK(ss.str());
        }

        // COLLECT THEM AND CHECK FOR ERRORS
        for (uint32_t iter = 0; iter < options.size(); ++iter)
        {
            if (Core::is_os(options[iter]) || Core::is_arch(options[iter]))
            {
                ss << "Profile " << TOKEN_MAGENTA(options[iter]) << " cannot be the OS or ARCH name";
                return SEM_NOK(ss.str());
            }
            else if (std::find(_env.profile.profiles.begin(), _env.profile.profiles.end(), options[iter]) == _env.profile.profiles.end())
            {
                _env.profile.profiles.push_back(options[iter]);
            }
            else
            {
                ss << "Duplicate item in statement " << TOKEN_MAGENTA("using profiles") << ": " << TOKEN_MAGENTA(options[iter]) << ANSI_RESET ;
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
            ss << "Statement " << TOKEN_MAGENTA("using multithread") << " must be followed maximum threads allowed";
            return SEM_NOK(ss.str());
        }

        // CHECK IF ITS A NUMBER
        int max_threads = 0;
        const char* begin = options[0].data();
        const char* end   = options[0].data() + options[0].size();

        auto [ptr, ec] = std::from_chars(begin, end, max_threads);

        if (ec != std::errc{} || ptr != end || max_threads <= 0)
        {
            ss << "Invalid value for multithread: " << TOKEN_MAGENTA(options[0]) << ". Expected a positive integer.";
            return SEM_NOK(ss.str());
        }

        _env.max_threads = max_threads;

        Core::update_symbol(Core::SymbolType::THREADS, std::to_string(max_threads));
    }

    return SemanticOutput{};
}



SemanticOutput Engine::Collect_Mapping(const std::string& item_1, const std::string& item_2)
{
    std::stringstream ss;

    auto&       vtable   = _env.vtable;
    const auto  keys     = Table::Keys(vtable);
    auto        it_item1 = vtable.find(item_1);
    auto        it_item2 = vtable.find(item_2);

    if (it_item1 == vtable.end())
    {
        ss << "Invalid mapping " << TOKEN_MAGENTA(item_1) << " -> " << TOKEN_MAGENTA(item_2) << "! " << item_1 << ": undeclared variable";
        return SEM_NOK_HINT(ss.str(), Support::FindClosest(keys, item_1));
    }

    if (it_item2 == vtable.end())
    {
        ss << "Invalid mapping " << TOKEN_MAGENTA(item_1) << " -> " << TOKEN_MAGENTA(item_2) << "! " << item_2 << ": undeclared variable";
        return SEM_NOK_HINT(ss.str(), Support::FindClosest(keys, item_2));
    }

    it_item2->second.attributes.push_back({
        "map",
        Attr::Type::MAP,
        {item_1}
    });

    return SemanticOutput{};
}



SemanticOutput Engine::Collect_Assert(std::size_t line, const std::string& stmt, const std::string& lvalue, 
                                      const std::string& op, const std::string& rvalue, const std::string& reason)
{
    AssertCheck acheck;
    acheck.line   = line;
    acheck.stmt   = stmt;
    acheck.lvalue = lvalue;
    acheck.rvalue = rvalue;
    acheck.reason = reason;

    if (op == "eq")
    {
        acheck.check = AssertCheck::CheckType::EQUAL;
    }
    else if (op == "ne")
    {
        acheck.check = AssertCheck::CheckType::NOT_EQUAL;
    }
    else if (op == "in")
    {
        acheck.check = AssertCheck::CheckType::IN;
    }

    _env.atable.push_back(acheck);

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
            ERR("Unknown task " << TOKEN_MAGENTA(args.task.value));

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
            ERR("Requested task " << TOKEN_MAGENTA(args.task.value << ANSI_RESET << " does not have " << ANSI_BMAGENTA << "public" ) << " attribute");
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

        Core::update_symbol(Core::SymbolType::MAIN, task.value().get().task_name);
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
            ERR("Requested profile " << TOKEN_MAGENTA(args.profile.value) << " is invalid!");

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

    if (args.threads)
    {
        Core::update_symbol(Core::SymbolType::THREADS, args.threads.svalue);
    }

    Core::update_symbol(Core::SymbolType::PROFILE, profile.selected);
    
    // AFTER THE PROFILE SELECTION, REMOVE THE UNWANTED KEYS FROM THE TABLES AND
    // REMOVE THE MANGLINGS   
    Table::AlignOnProfile(vtable, profile.selected);
    Table::AlignOnProfile(ftable, profile.selected);
    Table::AlignOnOS(vtable);
    
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
    
                    ss << "Invalid dependecy " << TOKEN_MAGENTA(p) << " for task " << TOKEN_MAGENTA(task.task_name) << std::endl;
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
#if defined(_WIN32)
        default_interpreter = "C:\\Windows\\System32\\cmd.exe";
#else
        default_interpreter = "/bin/bash";
#endif
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



const std::optional<std::string>
Enviroment::Expand() noexcept
{
    Expander ex(*this);

    // ------------------------------------------------------------
    // HANDLE MAX THREADS
    // ------------------------------------------------------------
    auto machine_max_threads = std::thread::hardware_concurrency();

    if (max_threads == 0 || max_threads > machine_max_threads)
    {
        max_threads = machine_max_threads;
    }

    // ------------------------------------------------------------
    // FOR EACH KEY IN VTABLE (per validare task_inputs)
    // ------------------------------------------------------------
    auto var_keys = Table::Keys(vtable);

    if (var_keys.empty())
    {
        return std::nullopt;
    }

    std::sort(var_keys.begin(), var_keys.end(), [] (const std::string& a, const std::string& b) {
        return a.size() > b.size();
    });

    // ------------------------------------------------------------
    // EXPAND VTABLE + GLOB EXPAND
    // ------------------------------------------------------------
    Glob::ExpandOptions opt;

    for (auto& [name, var] : vtable)
    {
        if (auto err = ex.ExpandText(var.var_value); err.has_value())
        {
            return err;
        }

        Glob::Pattern    pattern;
        Glob::ParseError error;

        if (!Glob::Parse(var.var_value, pattern, error))
        {
            std::stringstream ss;
            ss << "While expanding " << TOKEN_MAGENTA(name)
               << " an invalid glob was detected " << TOKEN_MAGENTA(pattern.normalized)
               << ": " << ParseErrorRepr(error);

            return ss.str();
        }

        var.glob_expansion.clear();

        if (!Arcana::Glob::Expand(pattern, ".", var.glob_expansion, opt))
        {
            // qui la tua Glob::Expand ritorna bool; se false significa "nessun match"? o errore?
            // Io non posso inventare semantica: se vuoi trattarlo come errore, fai return.
        }
    }

    // ------------------------------------------------------------
    // ASSERTS
    // ------------------------------------------------------------
    for (auto& assert : atable)
    {
        if (auto err = ex.ExpandAssertSide(assert.lvalue, assert); err.has_value())
        {
            return err;
        }

        if (auto err = ex.ExpandAssertSide(assert.rvalue, assert); err.has_value())
        {
            return err;
        }

        if (auto err = ex.ExpandText(assert.reason); err.has_value())
        {
            return err;
        }
    }

    // ------------------------------------------------------------
    // FTABLE
    // ------------------------------------------------------------
    for (auto& [name, task] : ftable)
    {
        if (task.hasAttribute(Attr::Type::INTERPRETER))
        {
            std::stringstream ss;
            auto properties = task.getProperties(Attr::Type::INTERPRETER);

            if (!properties.empty())
            {
                if (auto err = ex.ExpandText(properties[0]); err.has_value())
                {
                    return err;
                }

                task.interpreter = properties[0];

                if (!Support::file_exists(properties[0]))
                {
                    ss << "Interpreter " << TOKEN_MAGENTA(properties[0]) << " is missing or unknown";
                    return ss.str();
                }
            }
        }

        for (const auto& input : task.task_inputs)
        {
            if (std::find(var_keys.begin(), var_keys.end(), input) == var_keys.end())
            {
                std::stringstream ss;
                ss << "Invalid input " << TOKEN_MAGENTA(input << ANSI_RESET << " for task " << ANSI_BMAGENTA << name)
                   << ": Undefined variable";

                return ss.str();
            }
        }

        for (auto& instr : task.task_instrs)
        {
            if (auto err = ex.ExpandText(instr); err.has_value())
            {
                return err;
            }
        }
    }

    // ------------------------------------------------------------
    // HANDLE MAPPED VARS EXPANSION
    // ------------------------------------------------------------
    auto map_required = Table::GetValues(vtable, Semantic::Attr::Type::MAP);

    for (auto& stmt : map_required.value())
    {
        Glob::ParseError e1, e2;
        Glob::MapError   m1;

        auto& map_to   = stmt.get();
        auto& map_from = vtable[map_to.getProperties(Semantic::Attr::Type::MAP).at(0)];

        if (!Arcana::Glob::MapGlobToGlob(map_from.var_value,      map_to.var_value,
                                         map_from.glob_expansion, map_to.glob_expansion, e1, e2, m1))
        {
            std::stringstream ss;
            ss << "While mapping " << TOKEN_MAGENTA(map_from.var_name)
               << " to " << TOKEN_MAGENTA(map_to.var_name)
               << ": incompatible globs";

            return ss.str();
        }
    }

    return std::nullopt;
}


const std::optional<std::string> Enviroment::ExecuteAsserts() noexcept
{
    bool assert_failed = false;

    for (const auto& assert : atable)
    {
        assert_failed = false;

        switch (assert.check)
        {
            case AssertCheck::CheckType::EQUAL:     
                assert_failed = (assert.lvalue != assert.rvalue);                         
                break;
            case AssertCheck::CheckType::NOT_EQUAL: 
                assert_failed = (assert.lvalue == assert.rvalue);                         
                break;
            case AssertCheck::CheckType::IN:        
                assert_failed = (assert.rvalue.find(assert.lvalue) == std::string::npos); 
                break;

            case AssertCheck::CheckType::DEPENDENCIES:
                assert_failed = !fs::exists(assert.search_path);
                break;
        }

        if (assert_failed)
        {
            std::stringstream ss;
            ss << "Assert failed on line " << assert.line << ": " << TOKEN_CYAN(assert.stmt);

            if (assert.check != AssertCheck::CheckType::DEPENDENCIES)
            {
                ss << " with lvalue: " << TOKEN_MAGENTA(assert.lvalue) << ", rvalue: " << TOKEN_MAGENTA(assert.rvalue) << std::endl;
            }
            else
            {
                ss << " dependency " << TOKEN_MAGENTA(assert.search_path) << " not found!" << std::endl;
            }

            ss << "Reason: " << assert.reason;
            return ss.str();
        }
    }

    return std::nullopt;
}




//    ███████╗██╗  ██╗██████╗  █████╗ ███╗   ██╗██████╗ ███████╗██████╗ 
//    ██╔════╝╚██╗██╔╝██╔══██╗██╔══██╗████╗  ██║██╔══██╗██╔════╝██╔══██╗
//    █████╗   ╚███╔╝ ██████╔╝███████║██╔██╗ ██║██║  ██║█████╗  ██████╔╝
//    ██╔══╝   ██╔██╗ ██╔═══╝ ██╔══██║██║╚██╗██║██║  ██║██╔══╝  ██╔══██╗
//    ███████╗██╔╝ ██╗██║     ██║  ██║██║ ╚████║██████╔╝███████╗██║  ██║
//    ╚══════╝╚═╝  ╚═╝╚═╝     ╚═╝  ╚═╝╚═╝  ╚═══╝╚═════╝ ╚══════╝╚═╝  ╚═╝
//                                                                      



std::optional<std::string> Enviroment::Expander::ExpandInternals(std::string& s) noexcept
{
    for (int depth = 0; depth < 32; ++depth)
    {
        std::smatch m;
        if (!std::regex_search(s, m, re_intern))
        {
            return std::nullopt;
        }

        const std::string sym = m[1].str();

        if (auto st = Core::is_symbol(sym); st != Core::SymbolType::UNDEFINED)
        {
            const std::string rep = Core::symbol(st);

            s.replace(static_cast<std::size_t>(m.position(0)), static_cast<std::size_t>(m.length(0)), rep);
        }
        else
        {
            return "Internal symbol expansion failed for {arc:" + sym + "}";
        }
    }

    return "Too deep internal symbol expansion (depth limit reached)";
}



std::optional<std::string> Enviroment::Expander::ExpandArcAll(std::string& s) noexcept
{
    for (int depth = 0; depth < 32; ++depth)
    {
        std::smatch m;
        if (!std::regex_search(s, m, re_arc))
        {
            return std::nullopt;
        }

        const std::string name = m[1].str();

        auto it = env.vtable.find(name);
        if (it == env.vtable.end())
        {
            std::stringstream err;
            err << "Undefined variable " << ANSI_BMAGENTA << name << ANSI_RESET
                << " while trying to expand " << ANSI_BMAGENTA << "{arc:" << name << "}" << ANSI_RESET;

            return err.str();
        }

        const std::string& value = it->second.var_value;

        s.replace(static_cast<std::size_t>(m.position(0)), static_cast<std::size_t>(m.length(0)), value);
    }

    return "Too deep / cyclic {arc:...} expansion (depth limit reached)";
}



std::optional<std::string> Enviroment::Expander::ExpandText(std::string& s) noexcept
{
    if (auto err = ExpandInternals(s); err.has_value())
    {
        return err;
    }

    if (auto err = ExpandArcAll(s); err.has_value())
    {
        return err;
    }

    return std::nullopt;
}



void Enviroment::Expander::ExtractFsPaths(const std::string& s, std::vector<fs::path>& out) noexcept
{
    std::smatch m;

    for (auto it = s.cbegin(); std::regex_search(it, s.cend(), m, re_fs); )
    {
        out.push_back(fs::path(m[1].str()));
        it = m.suffix().first;
    }
}



std::optional<std::string> Enviroment::Expander::ExpandAssertSide(std::string& stmt, AssertCheck& assert) noexcept
{ 
    if (auto err = ExpandText(stmt); err.has_value())
    {
        return err;
    }
    
    std::vector<fs::path> paths;
    ExtractFsPaths(stmt, paths);

    if (!paths.empty())
    {
        assert.check       = AssertCheck::CheckType::DEPENDENCIES;
        assert.search_path = paths.back() / assert.lvalue;
    }

    return std::nullopt;
}