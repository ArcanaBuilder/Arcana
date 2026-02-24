#include "Semantic.h"

#include "Glob.h"
#include "Core.h"
#include "Support.h"
#include "Cache.h"
#include "TableHelper.h"
#include "Profiler.h"

#include <regex>
#include <memory>
#include <thread>
#include <charconv>
#include <algorithm>
#include <filesystem>
#include <unordered_map>

USE_MODULE(Arcana::Semantic);

namespace fs = std::filesystem;


using AttributeMap = Arcana::Support::AbstractKeywordMap<Attr::Type>;
using UsingMap     = Arcana::Support::AbstractKeywordMap<Using::Rule>;



// ------------------------------
// STATIC TABLES
// ------------------------------


/**
 * @brief Map of attribute name -> normalized Attr::Type.
 */
static const AttributeMap Known_Attributes =
{
    { "profile"     , Attr::Type::PROFILE     },
    { "pub"         , Attr::Type::PUBLIC      },
    { "always"      , Attr::Type::ALWAYS      },
    { "requires"    , Attr::Type::REQUIRES    },
    { "then"        , Attr::Type::THEN        },
    { "map"         , Attr::Type::MAP         },
    { "multithread" , Attr::Type::MULTITHREAD },
    { "main"        , Attr::Type::MAIN        },
    { "interpreter" , Attr::Type::INTERPRETER },
    { "cache"       , Attr::Type::CACHE       },
    { "echo"        , Attr::Type::ECHO        },
    { "exclude"     , Attr::Type::EXCLUDE     },
    { "glob"        , Attr::Type::GLOB        },
    { "ifos"        , Attr::Type::IFOS        },
};



/**
 * @brief Map of using keyword -> semantic using rule.
 */
static const UsingMap Known_Usings =
{
    { "profiles", { {               }, Using::Type::PROFILES    } },
    { "default" , { { "interpreter" }, Using::Type::INTERPRETER } },
    { "threads" , { {               }, Using::Type::THREADS     } },
};



/**
 * @brief Canonical attribute names list for hinting/closest-match.
 */
static const std::vector<std::string> _attributes =
{
    "profile",
    "pub",
    "always",
    "requires",
    "then",
    "map",
    "multithread",
    "main",
    "interpreter",
    "cache",
    "echo",
    "exclude",
    "glob",
    "ifos",
};



/**
 * @brief Canonical using keywords list for hinting/closest-match.
 */
static const std::vector<std::string> _usings =
{
    "profiles",
    "default",
    "threads",
};



/**
 * @brief Canonical cache instructions
 */
static const std::vector<std::string> _cache =
{
    "track",
    "store",
    "untrack",
};



// ------------------------------
// OUTPUT MACROS
// ------------------------------

#define SEM_OK()                 SemanticOutput{}
#define SEM_NOK(err)             { Semantic_Result::AST_RESULT__NOK, err       }
#define SEM_NOK_HINT(err, hint)  { Semantic_Result::AST_RESULT__NOK, err, hint }







//    ███████╗███╗   ██╗ ██████╗ ██╗███╗   ██╗███████╗
//    ██╔════╝████╗  ██║██╔════╝ ██║████╗  ██║██╔════╝
//    █████╗  ██╔██╗ ██║██║  ███╗██║██╔██╗ ██║█████╗  
//    ██╔══╝  ██║╚██╗██║██║   ██║██║██║╚██╗██║██╔══╝  
//    ███████╗██║ ╚████║╚██████╔╝██║██║ ╚████║███████╗
//    ╚══════╝╚═╝  ╚═══╝ ╚═════╝ ╚═╝╚═╝  ╚═══╝╚══════╝
//                                                    



/**
 * @brief Construct semantic engine and initialize attribute rule table.
 */
Engine::Engine()
{
    // INITIALIZE ATTRIBUTE RULES TABLE
    _attr_rules[_I(Attr::Type::PROFILE     )] = { Attr::Qualificator::REQUIRED_PROPERTY, Attr::Count::ONE      , { Attr::Target::TASK, Attr::Target::VARIABLE } };
    _attr_rules[_I(Attr::Type::PUBLIC      )] = { Attr::Qualificator::NO_PROPERY       , Attr::Count::ZERO     , { Attr::Target::TASK, Attr::Target::VARIABLE } };
    _attr_rules[_I(Attr::Type::ALWAYS      )] = { Attr::Qualificator::NO_PROPERY       , Attr::Count::ZERO     , { Attr::Target::TASK,                        } };
    _attr_rules[_I(Attr::Type::REQUIRES    )] = { Attr::Qualificator::REQUIRED_PROPERTY, Attr::Count::UNLIMITED, { Attr::Target::TASK,                        } };
    _attr_rules[_I(Attr::Type::THEN        )] = { Attr::Qualificator::REQUIRED_PROPERTY, Attr::Count::UNLIMITED, { Attr::Target::TASK,                        } };
    _attr_rules[_I(Attr::Type::MAP         )] = { Attr::Qualificator::REQUIRED_PROPERTY, Attr::Count::ONE      , {                     Attr::Target::VARIABLE } };
    _attr_rules[_I(Attr::Type::EXCLUDE     )] = { Attr::Qualificator::REQUIRED_PROPERTY, Attr::Count::ONE      , {                     Attr::Target::VARIABLE } };
    _attr_rules[_I(Attr::Type::GLOB        )] = { Attr::Qualificator::NO_PROPERY       , Attr::Count::ZERO     , {                     Attr::Target::VARIABLE } };
    _attr_rules[_I(Attr::Type::MULTITHREAD )] = { Attr::Qualificator::NO_PROPERY       , Attr::Count::ZERO     , { Attr::Target::TASK,                        } };
    _attr_rules[_I(Attr::Type::MAIN        )] = { Attr::Qualificator::NO_PROPERY       , Attr::Count::ZERO     , { Attr::Target::TASK,                        } };
    _attr_rules[_I(Attr::Type::INTERPRETER )] = { Attr::Qualificator::REQUIRED_PROPERTY, Attr::Count::ONE      , { Attr::Target::TASK,                        } };
    _attr_rules[_I(Attr::Type::CACHE       )] = { Attr::Qualificator::REQUIRED_PROPERTY, Attr::Count::UNLIMITED, { Attr::Target::TASK,                        } };
    _attr_rules[_I(Attr::Type::ECHO        )] = { Attr::Qualificator::NO_PROPERY       , Attr::Count::ZERO     , { Attr::Target::TASK,                        } };
    _attr_rules[_I(Attr::Type::IFOS        )] = { Attr::Qualificator::REQUIRED_PROPERTY, Attr::Count::ONE      , {                     Attr::Target::VARIABLE } };
}



/**
 * @brief Collect an attribute statement and stage it for the next entity.
 * @param name Raw attribute name.
 * @param prop Raw properties string (tokenized via split()).
 * @return SemanticOutput with status and optional hint.
 */
SemanticOutput Engine::Collect_Attribute(const std::string& name, const std::string& prop)
{
    std::stringstream ss;

    Attr::Type       attr     = Attr::Type::ATTRIBUTE__UNKNOWN;
    Attr::Properties property = Arcana::Support::split(prop);

    // RESOLVE ATTRIBUTE NAME TO TYPE
    if (auto it = Known_Attributes.find(name); it != Known_Attributes.end())
    {
        attr = it->second;
    }

    // HANDLE UNKNOWN ATTRIBUTE
    if (attr == Attr::Type::ATTRIBUTE__UNKNOWN)
    {
        ss << "Attribute " << TOKEN_MAGENTA(name) << " not recognized";
        return SEM_NOK_HINT(ss.str(), Support::FindClosest(_attributes, name));
    }

    // VALIDATE PROPERTIES AGAINST RULES
    const auto& rule       = _attr_rules[_I(attr)];
    auto        props_count = property.size();

    if (rule == Attr::Qualificator::REQUIRED_PROPERTY)
    {
        // ENFORCE REQUIRED PROPERTIES
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
        // ENFORCE NO PROPERTIES
        if (props_count > 0)
        {
            ss << "Attribute " << TOKEN_MAGENTA(name) << " requires no option";
            return SEM_NOK(ss.str());
        }
    }

    // ATTRIBUTE-SPECIFIC VALIDATION
    if (attr == Attr::Type::PROFILE)
    {
        // VALIDATE PROFILE EXISTS
        const auto& profiles = _env.profile.profiles;

        if (std::find(profiles.begin(), profiles.end(), property[0]) == profiles.end())
        {
            ss << "Profile " << TOKEN_MAGENTA(property[0]) << " must be declared via " << ANSI_BMAGENTA << "using profile <profilenames>" << ANSI_RESET;
            return SEM_NOK_HINT(ss.str(), Support::FindClosest(profiles, property[0]));
        }
    }
    else if (attr == Attr::Type::MAP || attr == Attr::Type::EXCLUDE)
    {
        // VALIDATE REFERENCED VARIABLE EXISTS
        auto keys = Table::Keys(_env.vtable);

        if (std::find(keys.begin(), keys.end(), property[0]) == keys.end())
        {
            ss << "Invalid " << name << " " << TOKEN_MAGENTA(property[0]) << ": undeclared variable";
            return SEM_NOK_HINT(ss.str(), Support::FindClosest(keys, property[0]));
        }
    }
    else if (attr == Attr::Type::IFOS)
    {
        // VALIDATE OS NAME
        if (!Core::is_os(property[0]))
        {
            ss << "Invalid OS " << TOKEN_MAGENTA(property[0]);
            return SEM_NOK(ss.str());
        }
    }
    else if (attr == Attr::Type::CACHE)
    {
        auto keys = Table::Keys(_env.vtable);

        if (property.size() == 1)
        {
            ss << "Missing arguments for attribute " << TOKEN_MAGENTA(name);
            return SEM_NOK(ss.str());
        }

        if (std::find(_cache.begin(), _cache.end(), property[0]) == _cache.end())
        {
            ss << "Invalid keyword " << TOKEN_MAGENTA(property[0]) << " for attribute @" << TOKEN_CYAN(name);
            return SEM_NOK_HINT(ss.str(), Support::FindClosest(_cache, property[0]));
        }

        /*
        if (std::find(keys.begin(), keys.end(), property[1]) == keys.end())
        {
            ss << "Invalid " << property[0] << " property for " << TOKEN_MAGENTA(property[1]) << ": undeclared variable";
            return SEM_NOK_HINT(ss.str(), Support::FindClosest(keys, property[1]));
        }
        */
    }

    // ENQUEUE ATTRIBUTE FOR NEXT ENTITY
    _attr_pending.push_back({
        name,
        attr,
        property
    });

    return SEM_OK();
}



/**
 * @brief Collect a variable assignment into the environment VTable.
 * @param name Variable name.
 * @param val  Raw value.
 * @return SemanticOutput with status.
 */
SemanticOutput Engine::Collect_Assignment(const std::string& name, const std::string& val, bool join)
{
    std::stringstream  ss;
    InstructionAssign  assign { name, val };

    // ATTACH PENDING ATTRIBUTES AND CLEAR PENDING QUEUE
    assign.attributes = _attr_pending;
    _attr_pending.clear();

    // VALIDATE ATTRIBUTES TARGET VARIABLES
    for (const auto& attr : assign.attributes)
    {
        const auto& rule = _attr_rules[_I(attr.type)];

        if (std::find(rule.targets.begin(), rule.targets.end(), Attr::Target::VARIABLE) == rule.targets.end())
        {
            ss << "Attribute " << TOKEN_MAGENTA(attr.name) << " is not valid for variable assignment";
            return SEM_NOK(ss.str());
        }
    }

    // APPLY MANGLING FOR PROFILE/IFOS
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
        if (join) 
        {
            const auto& keys = Table::Keys(_env.vtable);

            if (std::find(keys.begin(), keys.end(), name) == keys.end())
            {
                #warning handle error
            } 

            auto& var = _env.vtable[name];
            var.var_value.push_back(val);
        }
        else
        {
            _env.vtable[name] = assign;
        }
    }

    return SEM_OK();
}



/**
 * @brief Collect a task declaration into the environment FTable.
 * @param name   Task name.
 * @param inputs Raw inputs string (split into tokens).
 * @param instrs Instruction lines.
 * @return SemanticOutput with status.
 */
SemanticOutput Engine::Collect_Task(const std::string& name, const Task::Instrs& instrs)
{
    std::stringstream ss;

    // BUILD TASK INSTRUCTION
    InstructionTask task { name, instrs };
    FTable&         ftable = _env.ftable;

    // ATTACH PENDING ATTRIBUTES AND CLEAR PENDING QUEUE
    task.attributes = _attr_pending;
    _attr_pending.clear();

    // VALIDATE ATTRIBUTES TARGET TASKS
    for (const auto& attr : task.attributes)
    {
        const auto& rule = _attr_rules[_I(attr.type)];

        if (std::find(rule.targets.begin(), rule.targets.end(), Attr::Target::TASK) == rule.targets.end())
        {
            ss << "Attribute " << TOKEN_MAGENTA(attr.name) << " is not valid for tasks";
            return SEM_NOK(ss.str());
        }
    }

    // APPLY MANGLING FOR PROFILE
    if (task.hasAttribute(Attr::Type::PROFILE))
    {
        const auto profile = task.getProperties(Attr::Type::PROFILE);

        ftable[Support::generate_mangling(name, profile[0])] = task;

        if (task.hasAttribute(Attr::Type::MAIN))
        {
            if (!Core::is_symbol_set(Core::SymbolType::MAIN))
            {
                Core::update_symbol(Core::SymbolType::MAIN, name);
            }
            else
            {
                if (Core::symbol(Core::SymbolType::MAIN) != name)
                {
                    ss << "Cannot tag multiple tasks with attribute " << TOKEN_MAGENTA("main");
                    return SEM_NOK(ss.str());
                }
            }
        }
    }
    else
    {
        ftable[name] = task;

        if (task.hasAttribute(Attr::Type::MAIN))
        {
            if (!Core::is_symbol_set(Core::SymbolType::MAIN))
            {
                Core::update_symbol(Core::SymbolType::MAIN, name);
            }
            else
            {
                ss << "Cannot tag multiple tasks with attribute " << TOKEN_MAGENTA("main");
                return SEM_NOK(ss.str());
            }
        }
    }

    return SEM_OK();
}



/**
 * @brief Collect a `using` directive and update environment configuration.
 * @param what Directive keyword (e.g. profiles/default/threads).
 * @param opt  Directive option string.
 * @return SemanticOutput with status and optional hint.
 */
SemanticOutput Engine::Collect_Using(const std::string& what, const std::string& opt)
{
    std::stringstream ss;
    Attr::Properties  options = Arcana::Support::split(opt);
    Using::Rule       rule;

    // RESOLVE USING RULE
    if (auto it = Known_Usings.find(what); it != Known_Usings.end())
    {
        rule = it->second;
    }
    else
    {
        ss << "Unknown " << TOKEN_MAGENTA(what) << " for statement " << TOKEN_MAGENTA("using");
        return SEM_NOK_HINT(ss.str(), Support::FindClosest(_usings, what));
    }

    // HANDLE DEFAULT INTERPRETER
    if (rule.using_type == Using::Type::INTERPRETER)
    {
        // VALIDATE OPTIONS PRESENCE
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

            ss << "Statement " << TOKEN_MAGENTA("using " << what) << " must be followed by " << ss1.str();
            return SEM_NOK(ss.str());
        }

        // VALIDATE ATTRIBUTE NAME
        const auto& attr = std::find(rule.valid_attr.begin(), rule.valid_attr.end(), options[0]);

        if (attr == rule.valid_attr.end())
        {
            ss << "Unknown attribute " << TOKEN_MAGENTA(options[0]) << " for statement " << TOKEN_MAGENTA("using " << what);
            return SEM_NOK_HINT(ss.str(), Support::FindClosest(rule.valid_attr, options[0]));
        }

        // VALIDATE INTERPRETER PATH PRESENT
        if (options.size() == 1)
        {
            ss << "Statement " << TOKEN_MAGENTA("using default " << options[0]) << " must be followed by interpeter path";
            return SEM_NOK(ss.str());
        }

        // VALIDATE INTERPRETER EXISTS
        if (!Support::file_exists(options[1]))
        {
            ss << "Interpreter " << TOKEN_MAGENTA(options[1]) << " is missing or unknown";
            return SEM_NOK(ss.str());
        }

        // SET DEFAULT INTERPRETER
        _env.default_interpreter = options[1];
    }
    else if (rule.using_type == Using::Type::PROFILES)
    {
        // VALIDATE PROFILES PRESENT
        if (options.size() == 0)
        {
            ss << "Statement " << TOKEN_MAGENTA("using profiles") << " must be followed by profiles name";
            return SEM_NOK(ss.str());
        }

        // COLLECT UNIQUE PROFILES
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
                ss << "Duplicate item in statement " << TOKEN_MAGENTA("using profiles") << ": " << TOKEN_MAGENTA(options[iter]) << ANSI_RESET;
                return SEM_NOK(ss.str());
            }
        }
    }
    else if (rule.using_type == Using::Type::THREADS)
    {
        // VALIDATE THREADS ARGUMENT
        if (options.size() != 1)
        {
            ss << "Statement " << TOKEN_MAGENTA("using multithread") << " must be followed maximum threads allowed";
            return SEM_NOK(ss.str());
        }

        // PARSE INT VALUE
        int         max_threads = 0;
        const char* begin       = options[0].data();
        const char* end         = options[0].data() + options[0].size();

        auto [ptr, ec] = std::from_chars(begin, end, max_threads);

        if (ec != std::errc{} || ptr != end || max_threads <= 0)
        {
            ss << "Invalid value for multithread: " << TOKEN_MAGENTA(options[0]) << ". Expected a positive integer.";
            return SEM_NOK(ss.str());
        }

        // STORE THREADS CONFIG
        _env.max_threads = max_threads;
        Core::update_symbol(Core::SymbolType::THREADS, std::to_string(max_threads));
    }

    return SEM_OK();
}



/**
 * @brief Collect a mapping statement (item_1 -> item_2) and annotate item_2 with @map(item_1).
 * @param item_1 Source variable name.
 * @param item_2 Destination variable name.
 * @return SemanticOutput with status/hint.
 */
SemanticOutput Engine::Collect_Mapping(const std::string& item_1, const std::string& item_2)
{
    std::stringstream ss;

    // VALIDATE VARIABLES PRESENCE
    auto&      vtable   = _env.vtable;
    const auto keys     = Table::Keys(vtable);
    auto       it_item1 = vtable.find(item_1);
    auto       it_item2 = vtable.find(item_2);

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

    // ATTACH @map ATTRIBUTE TO DESTINATION VARIABLE
    it_item2->second.attributes.push_back({
        "map",
        Attr::Type::MAP,
        { item_1 }
    });

    return SEM_OK();
}



/**
 * @brief Collect an assert statement into the environment.
 * @param line   Line number.
 * @param stmt   Raw statement string.
 * @param lvalue Left side.
 * @param op     Operator token (eq/ne/in).
 * @param rvalue Right side.
 * @param reason Reason string.
 * @return SemanticOutput with status.
 */
SemanticOutput Engine::Collect_Assert(std::size_t line,
                                     const std::string& stmt,
                                     const std::string& lvalue,
                                     const std::string& op,
                                     const std::string& rvalue,
                                     const std::string& reason,
                                     const bool         actions)
{
    AssertCheck acheck;

    // FILL ASSERT_MSG STRUCT
    acheck.line   = line;
    acheck.stmt   = stmt;
    acheck.lvalue = lvalue;
    acheck.rvalue = rvalue;
    
    if (actions)
    {
        acheck.type    = AssertCheck::Type::ACTIONS;
        acheck.actions = Arcana::Support::split(reason);
    }
    else
    {
        acheck.type   = AssertCheck::Type::MESSAGE;
        acheck.reason = reason;
    }
    
    // MAP OPERATOR
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

    // STORE ASSERT_MSG
    _env.atable.push_back(acheck);

    return SEM_OK();
}



// ------------------------------
// ENVIRONMENT
// ------------------------------

/**
 * @brief Validate CLI arguments against the collected environment and apply overrides.
 * @param args Parsed CLI arguments.
 * @return ARCANA_RESULT__OK on success, ARCANA_RESULT__NOK on error.
 */
Arcana_Result Enviroment::CheckArgs(const Arcana::Support::Arguments& args) noexcept
{
    // HANDLE PROFILE OVERRIDE
    if (args.profile.found)
    {
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
        // DEFAULT PROFILE SELECTION
        if (profile.profiles.empty())
        {
            return Arcana_Result::ARCANA_RESULT__OK;
        }

        profile.selected = profile.profiles[0];
    }
    
    // UPDATE PROFILE SYMBOL
    Core::update_symbol(Core::SymbolType::PROFILE, profile.selected);

    // ALIGN TABLES AFTER PROFILE SELECTION
    Table::AlignOnProfile(vtable, profile.selected);
    Table::AlignOnProfile(ftable, profile.selected);
    Table::AlignOnOS(vtable);

    // HANDLE THREADS OVERRIDE
    if (args.threads)
    {
        max_threads = args.threads.ivalue;
        Core::update_symbol(Core::SymbolType::THREADS, args.threads.svalue);
    }

    // HANDLE TASK OVERRIDE
    if (args.task.found)
    {
        // RESOLVE TASK (PROFILE-AWARE)
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
        else if (!task.value().get().hasAttribute(Semantic::Attr::Type::PUBLIC))
        {
            // REQUIRE PUBLIC WHEN REQUESTED VIA CLI
            ERR("Requested task " << TOKEN_MAGENTA(args.task.value << ANSI_RESET << " does not have " << ANSI_BMAGENTA << "public") << " attribute");
            return Arcana_Result::ARCANA_RESULT__NOK;
        }

        // TOGGLE MAIN TO REQUESTED TASK
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
        // REQUIRE EXPLICIT MAIN IF NO CLI TASK
        auto main_task = Table::GetValues(ftable, profile.profiles, Attr::Type::MAIN);

        if (!main_task.size())
        {
            ERR("No main task specified, make it explicit in the arcfile with the @main attribute or pass a task on the command line");
            return Arcana_Result::ARCANA_RESULT__NOK;
        }
    }

    return Arcana_Result::ARCANA_RESULT__OK;
}



/**
 * @brief Resolve dependencies/then links and finalize interpreter defaults.
 * @return Empty optional on success, error string on failure.
 */
const std::optional<std::string> Enviroment::AlignEnviroment() noexcept
{
    std::stringstream ss;

    // RESOLVE REQUIRES/THEN LINKS
    const std::array<Attr::Type, 2> attributes = { Attr::Type::REQUIRES, Attr::Type::THEN };

    for (const auto attr : attributes)
    {
        auto tasks = Table::GetValues(ftable, profile.profiles, attr);

        for (auto& ref : tasks)
        {
            auto& task  = ref.get();
            auto  props = task.getProperties(attr);

            // VALIDATE AND LINK REFERENCED TASKS
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

                // APPEND LINK IN ORDER
                if (attr == Attr::Type::REQUIRES)
                {
                    task.dependencies.push_back(std::cref(ftable.at(p)));
                }
                else
                {
                    task.thens.push_back(std::cref(ftable.at(p)));
                }
            }
        }
    }

    // SET DEFAULT INTERPRETER IF MISSING
    if (default_interpreter.empty())
    {
#if defined(_WIN32)
        default_interpreter = "C:\\Windows\\System32\\cmd.exe";
#else
        default_interpreter = "/bin/bash";
#endif
    }

    // ASSIGN INTERPRETER TO EACH TASK
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



/**
 * @brief Expand variables/internals, compute glob expansions, expand tasks and asserts.
 * @return Empty optional on success, error string on failure.
 */
const std::optional<std::string> Enviroment::Expand() noexcept
{
    Expander ex(*this);

    // COMPUTE MAX THREADS DEFAULT
    auto machine_max_threads = std::thread::hardware_concurrency();

    if (max_threads == 0 || max_threads > machine_max_threads)
    {
        max_threads = machine_max_threads;
    }

    // COLLECT VARIABLE KEYS FOR INPUT VALIDATION
    auto var_keys = Table::Keys(vtable);

    if (var_keys.empty())
    {
        return std::nullopt;
    }

    // SORT BY LENGTH (LONGEST FIRST)
    std::sort(var_keys.begin(), var_keys.end(), [] (const std::string& a, const std::string& b) {
        return a.size() > b.size();
    });
    
    // EXPAND VTABLE AND COMPUTE GLOB EXPANSIONS
    Glob::ExpandOptions opt;

    for (auto& [name, var] : vtable)
    {
        // EXPAND GLOB TO LIST
        var.glob_expansion.clear();

        for (auto& value : var.var_value)
        {
            // EXPAND TEXT TOKENS
            if (auto err = ex.ExpandText(value, {}); err.has_value())
            {
                return err;
            }

            if (!var.hasAttribute(Attr::Type::GLOB)) continue;
    
            // PARSE GLOB PATTERN
            Glob::Pattern    pattern;
            Glob::ParseError error;

            if (!Glob::Parse(value, pattern, error))
            {
                std::stringstream ss;
                ss << "While expanding " << TOKEN_MAGENTA(name)
                   << " an invalid glob was detected " << TOKEN_MAGENTA(pattern.normalized)
                   << ": " << ParseErrorRepr(error);
    
                return ss.str();
            }

            Arcana::Glob::Expand(pattern, ".", var.glob_expansion, opt);
        }
    }

    
    // HANDLE MAPPED VARS EXPANSION
    auto map_required = Table::GetValues(vtable, Semantic::Attr::Type::MAP);

    if (!map_required.has_value()) return std::nullopt;

    for (auto& stmt : map_required.value())
    {
        Glob::ParseError e1, e2;
        Glob::MapError   m1;

        auto& map_to   = stmt.get();
        auto& map_from = vtable[map_to.getProperties(Semantic::Attr::Type::MAP).at(0)];

        if (!Arcana::Glob::MapGlobToGlob(map_from.var_value, map_to.var_value[0],
                                            map_from.glob_expansion, map_to.glob_expansion, e1, e2, m1))
        {
            std::stringstream ss;
            ss << "While mapping " << TOKEN_MAGENTA(map_from.var_name)
                << " to " << TOKEN_MAGENTA(map_to.var_name)
                << ": incompatible globs";

            return ss.str();
        }
    }

    // EXPAND ASSERTS
    auto keys = Table::Keys(ftable);

    for (auto& assert : atable)
    {
        for (const auto& action : assert.actions)
        {
            if (std::find(keys.begin(), keys.end(), action) == keys.end())
            {
                std::stringstream ss;
                ss << "Callback " << TOKEN_MAGENTA(action) << " is undefined in statement " << TOKEN_CYAN("assert");
                
                if (auto closest = Support::FindClosest(keys, action); closest.has_value() )
                {
                    ss << std::endl << "[" << TOKEN_GREEN("HINT") << "] Did you mean " << TOKEN_CYAN(closest.value()) << "?";
                }

                return ss.str();
            }
        }

        if (auto err = ex.ExpandAssertSide(assert.lvalue, assert); err.has_value())
        {
            return err;
        }

        if (auto err = ex.ExpandAssertSide(assert.rvalue, assert); err.has_value())
        {
            return err;
        }

        if (auto err = ex.ExpandText(assert.reason, {}); err.has_value())
        {
            return err;
        }
    }

    // EXPAND FTABLE
    for (auto& [name, task] : ftable)
    {
        if (task.cache.enabled = task.hasAttribute(Attr::Type::CACHE); task.cache.enabled)
        {
            auto properties = task.getProperties(Attr::Type::CACHE);

            if (properties[0] == "track")
            {
                task.cache.type = InstructionTask::Cache::Type::TRACK;
            }
            else if (properties[0] == "untrack")
            {
                task.cache.type = InstructionTask::Cache::Type::UNTRACK;
            }
            else
            {
                task.cache.type = InstructionTask::Cache::Type::STORE;

            }

            for (uint32_t i = 1; i < properties.size(); ++i)
            {
                std::size_t old_size = task.cache.data.size();

                if (auto err = ex.ExpandText(properties[i], {Expander::Algorithm::LIST}, &task.cache.data); err.has_value())
                {
                    return err;
                }

#warning SF: handle multi value vars
#if 0
                if (!task.cache.data.size())
                {
                    auto values = Support::split(task., ' ');
                    task.cache.data.insert(task.cache.data.end(), values.begin(), values.end());
                }
#endif
                for (std::size_t j = old_size; j < task.cache.data.size(); ++j)
                {
                    const auto& file = task.cache.data[j];

                    if (!Support::file_exists(file))
                    {
                        std::stringstream ss;
                        ss << "Cannot " << properties[0] << " " << TOKEN_CYAN(file) << " from instruction " << TOKEN_CYAN(properties[i]);
                        return ss.str();
                    }
                }
            }
        }

        // EXPAND TASK INTERPRETER OVERRIDE
        if (task.hasAttribute(Attr::Type::INTERPRETER))
        {
            std::stringstream ss;
            auto properties = task.getProperties(Attr::Type::INTERPRETER);

            if (auto err = ex.ExpandText(properties[0], {}); err.has_value())
            {
                return err;
            }
            
            task.interpreter = properties[0];

            if (!Support::file_exists(task.interpreter))
            {
                ss << "Interpreter " << TOKEN_MAGENTA(task.interpreter) << " is missing or unknown";
                return ss.str();
            }
        }


        std::vector<std::string> expanded_instrs;
        task.expanded = false;
        // EXPAND INSTRUCTION LINES
        for (auto& instr : task.task_instrs)
        {
            size_t old_val = expanded_instrs.size();

            if (auto err = ex.ExpandText(instr, {
                Expander::Algorithm::INLINE, 
                Expander::Algorithm::LIST
            }, &expanded_instrs); err.has_value())
            {
                return err;
            }
            
            if (expanded_instrs.size() - old_val > 1)
            {
                task.expanded = true;
            }
            else if (expanded_instrs.size() == old_val)
            {
                expanded_instrs.push_back(instr);
            }
        }

        task.task_instrs = expanded_instrs;
    }
    
    return std::nullopt;
}



/**
 * @brief Evaluate all collected asserts after expansion.
 * @return Empty optional on success, error string on first failure.
 */
const std::optional<std::string> Enviroment::ExecuteAsserts(std::vector<std::string>& reco_cb) noexcept
{
    bool assert_failed = false;
    std::stringstream ss;

    reco_cb.clear(); 

    for (const auto& assert : atable)
    {
        assert_failed = false;

        // EVALUATE ASSERT_MSG
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
            // BUILD ERROR MESSAGE
            ss << "Assert failed on line " << assert.line << ": " << TOKEN_CYAN(assert.stmt);

            if (assert.check != AssertCheck::CheckType::DEPENDENCIES)
            {
                ss << " with lvalue: " << TOKEN_MAGENTA(assert.lvalue) << ", rvalue: " << TOKEN_MAGENTA(assert.rvalue) << std::endl;
            }
            else
            {
                ss << " dependency " << TOKEN_MAGENTA(assert.search_path) << " not found!" << std::endl;
            }

            if (assert.type == AssertCheck::Type::MESSAGE)
            {
                ss << "Reason: " << assert.reason;
                return ss.str();
            }
            else
            {
                for (auto& task : assert.actions)
                {
                    ss << "Scheduling recovery callback " << TOKEN_MAGENTA(task);
                    reco_cb.push_back(task);
                } 
            }
        }
    }

    if (ss.str().size()) return ss.str();

    return std::nullopt;
}




//    ███████╗██╗  ██╗██████╗  █████╗ ███╗   ██╗██████╗ ███████╗██████╗ 
//    ██╔════╝╚██╗██╔╝██╔══██╗██╔══██╗████╗  ██║██╔══██╗██╔════╝██╔══██╗
//    █████╗   ╚███╔╝ ██████╔╝███████║██╔██╗ ██║██║  ██║█████╗  ██████╔╝
//    ██╔══╝   ██╔██╗ ██╔═══╝ ██╔══██║██║╚██╗██║██║  ██║██╔══╝  ██╔══██╗
//    ███████╗██╔╝ ██╗██║     ██║  ██║██║ ╚████║██████╔╝███████╗██║  ██║
//    ╚══════╝╚═╝  ╚═╝╚═╝     ╚═╝  ╚═╝╚═╝  ╚═══╝╚═════╝ ╚══════╝╚═╝  ╚═╝
//                                                                      


/**
 * @brief Expand internal symbols `{arc:__...__}`.
 * @param s String to expand in-place.
 * @return Empty optional on success, error string on failure.
 */
std::optional<std::string> Enviroment::Expander::ExpandInternals(std::string& s) noexcept
{
    for (int depth = 0; depth < 256; ++depth)
    {
        // SEARCH NEXT INTERNAL TOKEN
        std::smatch m;
        if (!std::regex_search(s, m, re_intern))
        {
            return std::nullopt;
        }

        const std::string sym = m[1].str();

        // RESOLVE SYMBOL AND REPLACE
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



/**
 * @brief Expand variable references `{arc:NAME}` using env.vtable.
 * @param s String to expand in-place.
 * @return Empty optional on success, error string on failure.
 */
std::optional<std::string> Enviroment::Expander::ExpandArcAll(std::string& s, const std::vector<Algorithm>& allowed_algorithms,
                                                              std::vector<std::string>* list_exp) noexcept
{
    std::size_t expected;

    List expanded;
    expanded.source  = s;

    std::vector<Arcana::Semantic::InstructionAssign*> list_expansions;

    std::sregex_iterator rx_it1(expanded.source.begin(), expanded.source.end(), re_arc_mode);
    std::sregex_iterator end;

    auto expand = [&] (std::string& src, uint32_t pos = 0) -> void
    {
        ssize_t span = 0;
        for (const auto& match : expanded.matches)
        {
            std::string content{};
            
            switch (match.algo)
            {
                case Algorithm::NORMAL: content = match.datasource->GetListValue();      break;
                case Algorithm::LIST:   content = match.datasource->glob_expansion[pos]; break;
                case Algorithm::INLINE: content = match.datasource->GetListGlob();       break;
            }   

            src.replace(match.start + span, match.count, content);
    
            span += content.length() - match.pattern_len;
        }
    };

    for (; rx_it1 != end; ++rx_it1)
    {
        const std::smatch& m         = *rx_it1;
        const std::string  algorithm = m[1].str();
        const std::string  name      = m[2].str();

        // LOOKUP VARIABLE VALUE
        auto it = env.vtable.find(name);
        if (it == env.vtable.end())
        {
            std::stringstream err;
            err << "Undefined variable " << TOKEN_MAGENTA(name) << " in statement " << TOKEN_CYAN(s);
            return err.str();
        }

        Algorithm algo;

        if (auto eit = Expansion_Map.find(algorithm); eit != Expansion_Map.end())
        {
            algo = eit->second;
        }

        if (std::find(allowed_algorithms.begin(), allowed_algorithms.end(), algo) != allowed_algorithms.end())
        {
            if (it->second.glob_expansion.size() == 0)
            {
                std::stringstream err;
                err << "Invalid expand algorithm " << TOKEN_MAGENTA(algorithm) << " for variable " << TOKEN_CYAN(name) << " in statement " << TOKEN_CYAN(s);
                return err.str();
            }

            if (algo == Algorithm::LIST) list_expansions.push_back(&it->second);

            expanded.matches.push_back( List::Match {algo, (size_t) m.position(0), (size_t) m.length(0), m[0].str().length(), &it->second} );
        }
        else
        {
            std::stringstream err;
            err << "Invalid expand algorithm " << TOKEN_MAGENTA(algorithm) << " for variable " << TOKEN_CYAN(name) << " in statement " << TOKEN_CYAN(s);
            return err.str();
        }
    }

    std::sregex_iterator rx_it2(expanded.source.begin(), expanded.source.end(), re_arc);

    for (; rx_it2 != end; ++rx_it2)
    {
        const std::smatch& m     = *rx_it2;
        std::size_t        start = m.position(0);
        std::size_t        len   = m.length(0);
        const std::string  name  = m[1].str();

        // LOOKUP VARIABLE VALUE
        auto it = env.vtable.find(name);
        if (it == env.vtable.end())
        {
            std::stringstream err;
            err << "Undefined variable " << TOKEN_MAGENTA(name) << " in statement " << TOKEN_CYAN(s);

            return err.str();
        }

        expanded.matches.push_back( List::Match {Algorithm::NORMAL, start, len, m[0].str().length(), &it->second});
    }

    std::sort(expanded.matches.begin(), expanded.matches.end(), [] (const List::Match& a, const List::Match& b)
    {
        return a.start < b.start;
    });

    if (list_expansions.size() > 0)
    {
        if (list_exp == nullptr)
        {
            std::stringstream err;
            err << "Invalid expansion algorithm " << TOKEN_CYAN("list") << " algorithm in statement " << TOKEN_MAGENTA(s);
            return err.str();
        }

        expected = list_expansions[0]->glob_expansion.size();

        const bool same_len = std::all_of(list_expansions.begin(), list_expansions.end(), [&] (const auto* list)
        {
            return list->glob_expansion.size() == expected;
        });

        if (!same_len)
        {
            std::stringstream err;
            err << "Invalid variables length for " << TOKEN_CYAN("list") << " algorithm in statement " << TOKEN_MAGENTA(s);
            return err.str();
        }

        for (uint32_t i = 0; i < expected; ++i)
        {
            std::string src = s;
            expand(src, i);
            list_exp->push_back(src);
        }
    } 
    
    expand(s);

    return std::nullopt;
}


std::optional<std::string> Enviroment::Expander::ExpandLists()
{

    return std::nullopt;
}
 

/**
 * @brief Expand a text using internals + variable expansion.
 * @param s String to expand in-place.
 * @return Empty optional on success, error string on failure.
 */
std::optional<std::string> Enviroment::Expander::ExpandText(std::string& s, const std::vector<Algorithm>& allowed_algorithms,
                                                            std::vector<std::string>* list_exp) noexcept
{
    // EXPAND INTERNALS
    if (auto err = ExpandInternals(s); err.has_value())
    {
        return err;
    }

    // EXPAND VARIABLES
    if (auto err = ExpandArcAll(s, allowed_algorithms, list_exp); err.has_value())
    {
        return err;
    }

    return std::nullopt;
}



/**
 * @brief Extract `{fs:...}` paths from a string.
 * @param s Input string.
 * @param out Output list of extracted paths.
 */
void Enviroment::Expander::ExtractFsPaths(const std::string& s, std::vector<fs::path>& out) noexcept
{
    std::smatch m;

    // ITERATE ALL FS TOKENS
    for (auto it = s.cbegin(); std::regex_search(it, s.cend(), m, re_fs); )
    {
        out.push_back(fs::path(m[1].str()));
        it = m.suffix().first;
    }
}



/**
 * @brief Expand one assert side and update dependency-mode if `{fs:...}` is present.
 * @param stmt Assert side string (lvalue/rvalue), expanded in-place.
 * @param assert Assert object to update.
 * @return Empty optional on success, error string on failure.
 */
std::optional<std::string> Enviroment::Expander::ExpandAssertSide(std::string& stmt, AssertCheck& assert) noexcept
{
    // EXPAND TEXT TOKENS
    if (auto err = ExpandText(stmt, {}); err.has_value())
    {
        return err;
    }

    // EXTRACT FS PATHS AND UPDATE ASSERT_MSG MODE
    std::vector<fs::path> paths;
    ExtractFsPaths(stmt, paths);

    if (!paths.empty())
    {
        assert.check       = AssertCheck::CheckType::DEPENDENCIES;
        assert.search_path = paths.back() / assert.lvalue;
    }

    return std::nullopt;
}
