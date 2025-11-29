#include "Semantic.h"

#include "Support.h"

#include <algorithm>
#include <unordered_map>


USE_MODULE(Arcana::Semantic);


template < typename T >
using AbstractKeywordMap = std::unordered_map<
    std::string_view,
    T,
    Arcana::Support::StringViewHash,
    Arcana::Support::StringViewEq
>;


using AttributeMap   = AbstractKeywordMap<Attr::Type>;
using UsingMap       = AbstractKeywordMap<Using>;


static const AttributeMap Known_Attributes = 
{
    { "precompiler" , Attr::Type::PRECOMPILER  },          
    { "postcompiler", Attr::Type::POSTCOMPILER },          
    { "profile"     , Attr::Type::PROFILE      },          
    { "public"      , Attr::Type::PUBLIC       },          
    { "private"     , Attr::Type::PRIVATE      },          
    { "folder"      , Attr::Type::FOLDER       },          
    { "file"        , Attr::Type::FILE         },          
    { "always"      , Attr::Type::ALWAYS       },          
    { "dependecy"   , Attr::Type::DEPENDECY    },          
    { "callable"    , Attr::Type::CALLABLE     },          
};


static const UsingMap Known_Usings = 
{
    { "profile"       , Using::PROFILE        },   
    { "precompiler"   , Using::PRECOMPILER    },          

};



Engine::Engine()
{
    _attr_rules[_I(Attr::Type::PRECOMPILER )] = { Attr::Qualificator::NO_PROPERY       , Attr::Count::ZERO     , { Attr::Target::TASK                         } };              
    _attr_rules[_I(Attr::Type::POSTCOMPILER)] = { Attr::Qualificator::NO_PROPERY       , Attr::Count::ZERO     , { Attr::Target::TASK,                        } };          
    _attr_rules[_I(Attr::Type::PROFILE     )] = { Attr::Qualificator::REQUIRED_PROPERTY, Attr::Count::ONE      , { Attr::Target::TASK, Attr::Target::VARIABLE } };           
    _attr_rules[_I(Attr::Type::PUBLIC      )] = { Attr::Qualificator::NO_PROPERY       , Attr::Count::ZERO     , { Attr::Target::TASK, Attr::Target::VARIABLE } };    
    _attr_rules[_I(Attr::Type::PRIVATE     )] = { Attr::Qualificator::NO_PROPERY       , Attr::Count::ZERO     , { Attr::Target::TASK, Attr::Target::VARIABLE } };    
    _attr_rules[_I(Attr::Type::FOLDER      )] = { Attr::Qualificator::NO_PROPERY       , Attr::Count::ZERO     , { Attr::Target::TASK,                        } };    
    _attr_rules[_I(Attr::Type::FILE        )] = { Attr::Qualificator::NO_PROPERY       , Attr::Count::ZERO     , { Attr::Target::TASK,                        } };    
    _attr_rules[_I(Attr::Type::ALWAYS      )] = { Attr::Qualificator::NO_PROPERY       , Attr::Count::ZERO     , { Attr::Target::TASK,                        } };    
    _attr_rules[_I(Attr::Type::DEPENDECY   )] = { Attr::Qualificator::REQUIRED_PROPERTY, Attr::Count::UNLIMITED, { Attr::Target::TASK,                        } };           
    _attr_rules[_I(Attr::Type::CALLABLE    )] = { Attr::Qualificator::NO_PROPERY       , Attr::Count::ZERO     , { Attr::Target::TASK,                        } };        
}


SemanticOutput Engine::Collect_Attribute(const std::string& name, const std::string&  prop)
{
    std::stringstream ss;

    Attr::Type       attr     = Attr::Type::ATTRIBUTE__UNKNOWN;
    Attr::Properties property = Arcana::Support::split(prop);

    if (auto it = Known_Attributes.find(name); it != Known_Attributes.end())
    {
        attr = it->second;
    }

    if (attr == Attr::Type::ATTRIBUTE__UNKNOWN)
    {
        ss << "Attribute " << "‘" << ANSI_BMAGENTA << "@" << name << ANSI_RESET << "’" << " not recognized";
        return { Semantic_Result::AST_RESULT__INVALID_ATTR, ss.str() };
    }

    const auto& rule = _attr_rules[_I(attr)];

    if (rule == Attr::Qualificator::REQUIRED_PROPERTY)
    {
        auto props_count = property.size();

        if (props_count == 0)
        {
            ss << "Attribute " << "‘" << ANSI_BMAGENTA << "@" << name << ANSI_RESET << "’" << " requires at least one option";
            return { Semantic_Result::AST_RESULT__INVALID_ATTR_PROP, ss.str() };
        }
        else if (props_count != 1 && rule.count == Attr::Count::ONE)
        {
            ss << "Attribute " << "‘" << ANSI_BMAGENTA << "@" << name << ANSI_RESET << "’" << " requires one option, not " << props_count;
            return { Semantic_Result::AST_RESULT__INVALID_ATTR_PROP, ss.str() };
        }
    }

    if (attr == Attr::Type::PROFILE)
    {
        const auto& profiles = _env.profile.profiles;

        if (std::find(profiles.begin(), profiles.end(), property[0]) == profiles.end())
        {
            ss << "Profile " << "‘" << ANSI_BMAGENTA << property[0] << ANSI_RESET << "’" << " must be declared via " << ANSI_BMAGENTA << "‘using profile <profilenames>’" << ANSI_RESET;
            return { Semantic_Result::AST_RESULT__INVALID_ATTR_PROP, ss.str() };
        }

        _env.profile.profiles.insert(property[0]);
    }

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

    assign.attributes = _attr_pending;
    _attr_pending.clear();

    for (const auto& attr : assign.attributes)
    {
        const auto& rule = _attr_rules[_I(attr.type)];

        if (std::find(rule.targets.begin(), rule.targets.end(), Attr::Target::VARIABLE) == rule.targets.end())
        {
            ss << "Attribute " << "‘" << ANSI_BMAGENTA << "@" << attr.name << ANSI_RESET << "’" << " is not valid for variable assignment";
            return { Semantic_Result::AST_RESULT__INVALID_ATTR_PROP, ss.str() };
        }
    }
    
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

    for (const auto& attr : task.attributes)
    {
        const auto& rule = _attr_rules[_I(attr.type)];

        if (std::find(rule.targets.begin(), rule.targets.end(), Attr::Target::TASK) == rule.targets.end())
        {
            ss << "Attribute " << "‘" << ANSI_BMAGENTA << "@" << attr.name << ANSI_RESET << "’" << " is not valid for tasks";
            return { Semantic_Result::AST_RESULT__INVALID_ATTR_PROP, ss.str() };
        }
    }

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

    sr = Support::split_quoted(param);

    if (!sr.ok)
    {
        ss << "Invalid argument(s) for task call " << "‘" << ANSI_BMAGENTA << name << ANSI_RESET << "’";
        return { Semantic_Result::AST_RESULT__INVALID_ARGUMENTS, ss.str() };
    }

    if (_attr_pending.size())
    {
        ss << "Cannot use attibutes for task call: " << "‘" << ANSI_BMAGENTA << name << ANSI_RESET << "’";
        return { Semantic_Result::AST_RESULT__ATTR_NOT_ALLOWED, ss.str() };
    }
    
    const auto& task = _env.ftable.find(name);

    if (task == _env.ftable.end())
    {
        ss << "Cannot call non-existent task: " << "‘" << ANSI_BMAGENTA << name << ANSI_RESET << "’";
        return { Semantic_Result::AST_RESULT__INVALID_TASK_CALL, ss.str() };
    }

    const auto& attributes = (*task).second.attributes;

    if (std::find(attributes.begin(), attributes.end(), Attr::Type::CALLABLE) == attributes.end())
    {
        ss << "Cannot call task marked: " << "‘" << ANSI_BMAGENTA << name << ANSI_RESET << "’ without the attribute" << ANSI_BMAGENTA << " @callable" << ANSI_RESET;
        return { Semantic_Result::AST_RESULT__INVALID_TASK_CALL, ss.str() };
    }

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
    Attr::Type        attr    = Attr::Type::ATTRIBUTE__UNKNOWN;

    if (auto it = Known_Attributes.find(what); it != Known_Attributes.end())
    {
        attr = it->second;
    }

    if (attr == Attr::Type::ATTRIBUTE__UNKNOWN)
    {
        ss << "Attribute " << "‘" << ANSI_BMAGENTA << what << ANSI_RESET << "’" << " not recognized";
        return { Semantic_Result::AST_RESULT__INVALID_ATTR, ss.str() };
    }

    if (auto it = Known_Usings.find(what); it == Known_Usings.end())
    {
        ss << "Unknown attribute " << "‘" << ANSI_BMAGENTA << what << ANSI_RESET << "’ for statement " << ANSI_BMAGENTA << "using" << ANSI_RESET ;
        return { Semantic_Result::AST_RESULT__INVALID_ATTR, ss.str() };
    }

    if (attr == Attr::Type::PROFILE)
    {
        for (const auto& opt : options)
        {
            _env.profile.profiles.insert(opt);
        }
    }

    return SemanticOutput{};
}