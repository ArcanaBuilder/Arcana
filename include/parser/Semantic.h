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

BEGIN_MODULE(Semantic)


struct Rule;

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
    REQUIRED_PROPERTY   ,
};

enum class Count
{
    ZERO             = 0,
    ONE                 ,
    UNLIMITED           ,
};

enum class Target
{
    TASK             = 0,
    VARIABLE            ,
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
using Targets    = std::vector<Target>;
using Rules      = std::array<Semantic::Rule, _I(Type::ATTRIBUTE__COUNT)>;


///////////////////////////////////////////////////////////////////////////////
// PUBLIC AGGREGATES
///////////////////////////////////////////////////////////////////////////////

struct Attribute
{
    std::string name;
    Type        type;
    Properties  props;

    Attribute() 
        :
        type(Type::ATTRIBUTE__UNKNOWN)
    {}

    Attribute(const std::string& name, const Type t, const Properties& p) 
        :
        name(name),
        type(t),
        props(p)
    {}

    bool operator == (const Type t) const { return this->type == t; }
};


#if DEBUG
inline std::string print_attr(const Type t)
{
    switch (t)
    {
        case Type::PRECOMPILER:  return "PRECOMPILER";      
        case Type::POSTCOMPILER: return "POSTCOMPILER";       
        case Type::BUILTIN:      return "BUILTIN";  
        case Type::PROFILE:      return "PROFILE";  
        case Type::PUBLIC:       return "PUBLIC"; 
        case Type::PRIVATE:      return "PRIVATE";  
        case Type::FOLDER:       return "FOLDER"; 
        case Type::FILE:         return "FILE"; 
        case Type::ALWAYS:       return "ALWAYS"; 
        case Type::DEPENDECY:    return "DEPENDECY";    
        case Type::CALLABLE:     return "CALLABLE";    
        default:                 return "NOT_A_ATTR";
    }
}
#endif

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


using VTable   = std::map<std::string, InstructionAssign>; 
using FTable   = std::map<std::string, InstructionTask>; 
using CTable   = std::map<std::string, InstructionCall>; 




///////////////////////////////////////////////////////////////////////////////
// PUBLIC AGGREGATES
///////////////////////////////////////////////////////////////////////////////

enum class Using
{
    PROFILE             = 0,
    PRECOMPILER            ,
};

struct Rule 
{
    Attr::Qualificator qual;
    Attr::Count        count;
    Attr::Targets      targets;

    bool operator == (const Attr::Qualificator q) const { return this->qual == q; }
};


struct Instruction
{
    Instruction() = default;
    virtual ~Instruction() = default;

    // copy
    Instruction(const Instruction & other)            = default;
    Instruction & operator=(const Instruction & other) = default;

    // move
    Instruction(Instruction && other) noexcept            = default;
    Instruction & operator=(Instruction && other) noexcept = default;

    Attr::List attributes;

#if DEBUG
    void print() const
    {
        this->pre_print();
        this->do_print();
    }

protected:
    virtual void do_print() const = 0;

    void pre_print() const
    {
        for (const auto & attr : attributes)
        {
            DBG("Attribute {");
            DBG("  Type:  " << Attr::print_attr(attr.type));

            for (const auto & prop : attr.props)
            {
                DBG("  Prop:  " << prop);
            }
            DBG("}");
        }
    }

#endif
};


struct InstructionAssign : public Instruction
{
    std::string var_name;
    std::string var_value;

    InstructionAssign() = default;

    InstructionAssign(const std::string & var, const std::string & val)
        :
        var_name(var),
        var_value(val)
    {
    }

    // copy
    InstructionAssign(const InstructionAssign & other)            = default;
    InstructionAssign & operator=(const InstructionAssign & other) = default;

    // move
    InstructionAssign(InstructionAssign && other) noexcept
        :
        Instruction(std::move(other)),
        var_name(std::move(other.var_name)),
        var_value(std::move(other.var_value))
    {
    }

    InstructionAssign & operator=(InstructionAssign && other) noexcept
    {
        if (this != &other)
        {
            Instruction::operator=(std::move(other));
            var_name  = std::move(other.var_name);
            var_value = std::move(other.var_value);
        }
        return *this;
    }

#if DEBUG
    void do_print() const override
    {
        DBG("Name:  " << var_name);
        DBG("Value: " << var_value);
    }
#endif
};


struct InstructionTask : public Instruction
{
    std::string  task_name;
    Task::Params task_params;
    Task::Instrs task_instrs;

    InstructionTask() = default;

    InstructionTask(const std::string &  name,
                    const Task::Params & params,
                    const Task::Instrs & instrs)
        :
        task_name(name),
        task_params(params),
        task_instrs(instrs)
    {
    }

    // copy
    InstructionTask(const InstructionTask & other)            = default;
    InstructionTask & operator=(const InstructionTask & other) = default;

    // move
    InstructionTask(InstructionTask && other) noexcept
        :
        Instruction(std::move(other)),
        task_name(std::move(other.task_name)),
        task_params(std::move(other.task_params)),
        task_instrs(std::move(other.task_instrs))
    {
    }

    InstructionTask & operator=(InstructionTask && other) noexcept
    {
        if (this != &other)
        {
            Instruction::operator=(std::move(other));
            task_name   = std::move(other.task_name);
            task_params = std::move(other.task_params);
            task_instrs = std::move(other.task_instrs);
        }
        return *this;
    }

#if DEBUG
    void do_print() const override
    {
        DBG("Name:  " << task_name);

        for (const auto & param : task_params)
        {
            DBG("Param: " << param);
        }

        for (const auto & instr : task_instrs)
        {
            DBG("Instr: " << instr);
        }
    }
#endif
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

    InstructionCall(InstructionCall&& other) noexcept
        :
        Instruction(std::move(other)), // importante per la base
        task_name(std::move(other.task_name)),
        task_params(std::move(other.task_params))
    {
    }

    InstructionCall& operator=(InstructionCall&& other) noexcept
    {
        if (this != &other)
        {
            Instruction::operator=(std::move(other));
            task_name  = std::move(other.task_name);
            task_params = std::move(other.task_params);
        }
        return *this;
    }

    InstructionCall(const InstructionCall&)            = default;
    InstructionCall& operator=(const InstructionCall&) = default;

#if DEBUG
    void do_print() const override
    {
        DBG("Name:  " << task_name);

        for (const auto& param : task_params)
        {
            DBG("Param: " << param);
        }
    }
#endif

};



struct Profile
{
    std::set<std::string> profiles;
    std::string           selected;

    void merge(Profile& other)
    {
        for (const auto& val : other.profiles)
            this->profiles.insert(val);
    }
};





struct Enviroment
{
    VTable   vtable;
    FTable   ftable;
    CTable   ctable;
    Profile  profile;

#ifdef DEBUG
    void print()
    {
        DBG("");
        DBG("########################################################################");
        DBG("## ENV");
        DBG("########################################################################");
        DBG("");

        DBG("------------------------------------------------------------------------");
        for (const auto& p : profile.profiles)
        {
            DBG("(PROFILE       ) => " << p);
        }
        DBG("(SEL PROFILE   ) => " << profile.selected);

        for (const auto& [key, val] : vtable)
        {
            DBG("------------------------------------------------------------------------");
            DBG("(VTABLE        ) => " << key);
            val.print();
        }
        
        for (const auto& [key, val] : ftable)
        {
            DBG("------------------------------------------------------------------------");
            DBG("(FTABLE        ) => " << key);
            val.print();
        }

        for (const auto& [key, val] : ctable)
        {
            DBG("------------------------------------------------------------------------");
            DBG("(CTABLE        ) => " << key);
            val.print();
        }
    }
#endif

};


inline void EnvMerge(Enviroment& dst, Enviroment& src)
{
    for (auto& [k, v] : src.vtable)
        dst.vtable[k] = std::move(v);

    for (auto& [k, v] : src.ftable)
        dst.ftable[k] = std::move(v);

    for (auto& [k, v] : src.ctable)
        dst.ctable[k] = std::move(v);

    dst.profile.merge(src.profile);
}



class Engine
{
public:
    Engine();

    SemanticOutput Collect_Attribute (const std::string& name, const std::string&  prop);
    SemanticOutput Collect_Assignment(const std::string& name, const std::string&  val); 
    SemanticOutput Collect_Task      (const std::string& name, const std::string& param, const Task::Instrs& instrs); 
    SemanticOutput Collect_TaskCall  (const std::string& name, const std::string&  param);  
    SemanticOutput Collect_Using     (const std::string& what, const std::string& opt); 

    const Enviroment Generate_Enviroment() const noexcept { return _env; }

    Enviroment&      EnvRef() { return _env; }

private:
    Attr::Rules _attr_rules;
    Attr::List  _attr_pending;

    Enviroment  _env;
};


END_MODULE(Semantic)


#endif /* __ARCANA_INSTRUCTION__H__ */