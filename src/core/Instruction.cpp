#include "Instruction.h"

#include "Support.h"

#include <unordered_map>


USE_MODULE(Arcana::Ast);



using KeywordMap = std::unordered_map<
    std::string_view,
    Attr::Type,
    Arcana::Support::StringViewHash,
    Arcana::Support::StringViewEq
>;


static const KeywordMap Known_Attributes = 
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



InstructionEngine::InstructionEngine()
{
    _attr_rules[_I(Attr::Type::PRECOMPILER )] = Attr::Qualificator::OPTIONAL_PROPERY;    
    _attr_rules[_I(Attr::Type::POSTCOMPILER)] = Attr::Qualificator::OPTIONAL_PROPERY;
    _attr_rules[_I(Attr::Type::PROFILE     )] = Attr::Qualificator::REQUIRED_PROPERTY;
    _attr_rules[_I(Attr::Type::PUBLIC      )] = Attr::Qualificator::NO_PROPERY;
    _attr_rules[_I(Attr::Type::PRIVATE     )] = Attr::Qualificator::NO_PROPERY;
    _attr_rules[_I(Attr::Type::FOLDER      )] = Attr::Qualificator::NO_PROPERY;
    _attr_rules[_I(Attr::Type::FILE        )] = Attr::Qualificator::NO_PROPERY;
    _attr_rules[_I(Attr::Type::ALWAYS      )] = Attr::Qualificator::NO_PROPERY;
    _attr_rules[_I(Attr::Type::DEPENDECY   )] = Attr::Qualificator::REQUIRED_PROPERTY;
    _attr_rules[_I(Attr::Type::CALLABLE    )] = Attr::Qualificator::NO_PROPERY;    
}


AstOutput InstructionEngine::Collect_Attribute(const std::string& name, const std::string&  prop)
{
    Attr::Type attr = Attr::Type::ATTRIBUTE__UNKNOWN;

    if (auto it = Known_Attributes.find(name); it != Known_Attributes.end())
    {
        attr = it->second;
    }

    if (attr == Attr::Type::ATTRIBUTE__UNKNOWN)
    {
        return { Ast_Result::AST_RESULT__INVALID_ATTR, name };
    }

    _attr_pending.push_back({ 
        attr, 
        Arcana::Support::split(prop, ' ') 
    });

    return { Ast_Result::AST_RESULT__OK, "" };
}


AstOutput InstructionEngine::Collect_Assignment(const std::string& name, const std::string&  val)
{
    InstructionAssign assign { name, val };

    assign.attributes = _attr_pending;
    _attr_pending.clear();
    
    _env.vtable[name] = assign;

    return { Ast_Result::AST_RESULT__OK, "" };
} 


AstOutput InstructionEngine::Collect_Task(const std::string& name, const Task::Params& params, const Task::Instrs& instrs)
{
    InstructionTask task { name, params, instrs };

    task.attributes = _attr_pending;
    _attr_pending.clear();
    
    _env.ftable[name] = task;

    return { Ast_Result::AST_RESULT__OK, "" };
} 


AstOutput InstructionEngine::Collect_TaskCall(const std::string& name, const Task::Params& params)
{
    InstructionCall call { name, params };

    call.attributes = _attr_pending;
    _attr_pending.clear();
    
    _env.ctable[name] = call;

    return { Ast_Result::AST_RESULT__OK, "" };
} 


AstOutput InstructionEngine::Collect_Using(const std::string& file)
{
    _env.usings.push_back(file);

    return { Ast_Result::AST_RESULT__OK, "" };
}

