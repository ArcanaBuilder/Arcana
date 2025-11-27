#ifndef __ARCANA_INSTRUCTION__H__
#define __ARCANA_INSTRUCTION__H__

///////////////////////////////////////////////////////////////////////////////
// INCLUDES
///////////////////////////////////////////////////////////////////////////////

#include <set>
#include <array>
#include <vector>

#include "Defines.h"
#include "Grammar.h"

///////////////////////////////////////////////////////////////////////////////
// MODULE NAMESPACEs
///////////////////////////////////////////////////////////////////////////////

BEGIN_MODULE(Ast)




//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
// BEGIN INNER NAMESPACE Attr
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

BEGIN_NAMESPACE(Attr)


///////////////////////////////////////////////////////////////////////////////
// PUBLIC ENUMS
///////////////////////////////////////////////////////////////////////////////

enum class Type
{
    PRECOMPILER      = 0,
    POSTCOMPILER        ,
    BUILTIN             ,
    PROFILE             ,
    PUBLIC              ,
    PRIVATE             ,
    FOLDER              ,
    FILE                ,
    ALWAYS              ,
    DEPENDECY           ,
    CALLABLE            ,

    ATTRIBUTE__UNKNOWN  ,
    ATTRIBUTE__COUNT    ,
};


enum class Qualificator
{
    NO_PROPERY       = 0,
    OPTIONAL_PROPERY    ,
    REQUIRED_PROPERTY   ,
};


///////////////////////////////////////////////////////////////////////////////
// TYPES FORWARD
///////////////////////////////////////////////////////////////////////////////

struct Attribute;



///////////////////////////////////////////////////////////////////////////////
// USINGs
///////////////////////////////////////////////////////////////////////////////


using Properties = std::vector<std::string>;
using List       = std::vector<Attribute>;
using Rules      = std::array<Qualificator, _I(Type::ATTRIBUTE__COUNT)>;



///////////////////////////////////////////////////////////////////////////////
// PUBLIC AGGREGATES
///////////////////////////////////////////////////////////////////////////////

struct Attribute
{
    Type       type;
    Properties props;

    Attribute() 
        :
        type(Type::ATTRIBUTE__UNKNOWN)
    {}

    Attribute(const Type t, const Properties& p) 
        :
        type(t),
        props(p)
    {}
};


END_NAMESPACE(Attr)

//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
// END INNER NAMESPACE Attr
//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<





//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
// BEGIN INNER NAMESPACE Task
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

BEGIN_NAMESPACE(Task)


///////////////////////////////////////////////////////////////////////////////
// PUBLIC ENUMS
///////////////////////////////////////////////////////////////////////////////

enum class Type
{
    BUILTIN         = 0,
    CUSTOM             ,

    TASK__COUNT        ,
};


///////////////////////////////////////////////////////////////////////////////
// USINGs
///////////////////////////////////////////////////////////////////////////////

using Params = std::vector<std::string>;
using Instrs = std::vector<std::string>;


///////////////////////////////////////////////////////////////////////////////
// PUBLIC AGGREGATES
///////////////////////////////////////////////////////////////////////////////


END_NAMESPACE(Task)

//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
// END INNER NAMESPACE Task
//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<



///////////////////////////////////////////////////////////////////////////////
// TYPES FORWARD
///////////////////////////////////////////////////////////////////////////////

struct Instruction;
struct InstructionAssign;
struct InstructionTask;
struct InstructionCall;

///////////////////////////////////////////////////////////////////////////////
// USING
///////////////////////////////////////////////////////////////////////////////

USE_MODULE(Arcana::Support);


using VTable = std::map<std::string, InstructionAssign>; 
using FTable = std::map<std::string, InstructionTask>; 
using CTable = std::map<std::string, InstructionCall>; 
using Usings = std::vector<std::string>; 



///////////////////////////////////////////////////////////////////////////////
// PUBLIC AGGREGATES
///////////////////////////////////////////////////////////////////////////////


struct Instruction
{
    Instruction() {}
    virtual ~Instruction() = default;

    Attr::List attributes;
};


struct InstructionAssign : public Instruction
{
    std::string var_name;
    std::string var_value;

    InstructionAssign() = default;

    InstructionAssign(const std::string& var, const std::string& val) 
        :
        var_name(var),
        var_value(val)
    {}
};


struct InstructionTask : public Instruction
{
    std::string  task_name;
    Task::Params task_params;
    Task::Instrs task_instrs;

    InstructionTask() = default;

    InstructionTask(const std::string&  name, 
                    const Task::Params& params,
                    const Task::Instrs& instrs) 
        :
        task_name(name),
        task_params(params),
        task_instrs(instrs)
    {}
};


struct InstructionCall : public Instruction
{
    std::string  task_name;
    Task::Params task_params;

    InstructionCall() = default;

    InstructionCall(const std::string&  name, 
                    const Task::Params& params) 
        :
        task_name(name),
        task_params(params)
    {}
};



struct Enviroment
{
    VTable vtable;
    FTable ftable;
    CTable ctable;
    Usings usings;
};



class InstructionEngine
{
public:
    InstructionEngine();

    AstOutput Collect_Attribute (const std::string& name, const std::string&  prop);
    AstOutput Collect_Assignment(const std::string& name, const std::string&  val); 
    AstOutput Collect_Task      (const std::string& name, const Task::Params& params, const Task::Instrs& instrs); 
    AstOutput Collect_TaskCall  (const std::string& name, const Task::Params& params); 
    AstOutput Collect_Using     (const std::string& file); 

    const Enviroment Generate_Enviroment() const noexcept { return _env; }

private:
    Attr::Rules _attr_rules;
    Attr::List  _attr_pending;
    Enviroment  _env;
};


END_MODULE(Ast)


#endif /* __ARCANA_INSTRUCTION__H__ */