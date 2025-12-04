#ifndef __ARCANA_INSTRUCTION__H__
#define __ARCANA_INSTRUCTION__H__

///////////////////////////////////////////////////////////////////////////////
// INCLUDES
///////////////////////////////////////////////////////////////////////////////

#include <set>
#include <array>
#include <vector>
#include <variant>
#include <algorithm>
#include <filesystem>

#include "Defines.h"
#include "Grammar.h"

///////////////////////////////////////////////////////////////////////////////
// MODULE NAMESPACEs
///////////////////////////////////////////////////////////////////////////////

BEGIN_MODULE(Semantic)



//    ███████ ███████ ███    ███  █████  ███    ██ ████████ ██  ██████                                                         
//    ██      ██      ████  ████ ██   ██ ████   ██    ██    ██ ██                                                              
//    ███████ █████   ██ ████ ██ ███████ ██ ██  ██    ██    ██ ██                                                              
//         ██ ██      ██  ██  ██ ██   ██ ██  ██ ██    ██    ██ ██                                                              
//    ███████ ███████ ██      ██ ██   ██ ██   ████    ██    ██  ██████                                                         
//                                                                                                                             
//                                                                                                                             
//    ███████  ██████  ██████  ██     ██  █████  ██████  ██████  ███████ ██████      ████████ ██    ██ ██████  ███████ ███████ 
//    ██      ██    ██ ██   ██ ██     ██ ██   ██ ██   ██ ██   ██ ██      ██   ██        ██     ██  ██  ██   ██ ██      ██      
//    █████   ██    ██ ██████  ██  █  ██ ███████ ██████  ██   ██ █████   ██   ██        ██      ████   ██████  █████   ███████ 
//    ██      ██    ██ ██   ██ ██ ███ ██ ██   ██ ██   ██ ██   ██ ██      ██   ██        ██       ██    ██      ██           ██ 
//    ██       ██████  ██   ██  ███ ███  ██   ██ ██   ██ ██████  ███████ ██████         ██       ██    ██      ███████ ███████ 
//                                                                                                                             
//                                                                                                                             

struct Rule;




//    ██ ███    ██ ███    ██ ███████ ██████      ███████ ███████ ███    ███  █████  ███    ██ ████████ ██  ██████        
//    ██ ████   ██ ████   ██ ██      ██   ██     ██      ██      ████  ████ ██   ██ ████   ██    ██    ██ ██             
//    ██ ██ ██  ██ ██ ██  ██ █████   ██████      ███████ █████   ██ ████ ██ ███████ ██ ██  ██    ██    ██ ██             
//    ██ ██  ██ ██ ██  ██ ██ ██      ██   ██          ██ ██      ██  ██  ██ ██   ██ ██  ██ ██    ██    ██ ██             
//    ██ ██   ████ ██   ████ ███████ ██   ██     ███████ ███████ ██      ██ ██   ██ ██   ████    ██    ██  ██████        
//                                                                                                                       
//                                                                                                                       
//    ███    ██  █████  ███    ███ ███████ ███████ ██████   █████   ██████ ███████      █████  ████████ ████████ ██████  
//    ████   ██ ██   ██ ████  ████ ██      ██      ██   ██ ██   ██ ██      ██          ██   ██    ██       ██    ██   ██ 
//    ██ ██  ██ ███████ ██ ████ ██ █████   ███████ ██████  ███████ ██      █████       ███████    ██       ██    ██████  
//    ██  ██ ██ ██   ██ ██  ██  ██ ██           ██ ██      ██   ██ ██      ██          ██   ██    ██       ██    ██   ██ 
//    ██   ████ ██   ██ ██      ██ ███████ ███████ ██      ██   ██  ██████ ███████     ██   ██    ██       ██    ██   ██ 
//                                                                                                                       
//                                                                                                                       
//    ██████  ███████  ██████  ██ ███    ██                                                                              
//    ██   ██ ██      ██       ██ ████   ██                                                                              
//    ██████  █████   ██   ███ ██ ██ ██  ██                                                                              
//    ██   ██ ██      ██    ██ ██ ██  ██ ██                                                                              
//    ██████  ███████  ██████  ██ ██   ████                                                                              
//                                                                                                                       
//  

BEGIN_NAMESPACE(Attr)


//    ███    ██  █████  ███    ███ ███████ ███████ ██████   █████   ██████ ███████      █████  ████████ ████████ ██████  
//    ████   ██ ██   ██ ████  ████ ██      ██      ██   ██ ██   ██ ██      ██          ██   ██    ██       ██    ██   ██ 
//    ██ ██  ██ ███████ ██ ████ ██ █████   ███████ ██████  ███████ ██      █████       ███████    ██       ██    ██████  
//    ██  ██ ██ ██   ██ ██  ██  ██ ██           ██ ██      ██   ██ ██      ██          ██   ██    ██       ██    ██   ██ 
//    ██   ████ ██   ██ ██      ██ ███████ ███████ ██      ██   ██  ██████ ███████     ██   ██    ██       ██    ██   ██ 
//                                                                                                                       
//                                                                                                                       
//    ███████ ███    ██ ██    ██ ███    ███ ███████                                                                      
//    ██      ████   ██ ██    ██ ████  ████ ██                                                                           
//    █████   ██ ██  ██ ██    ██ ██ ████ ██ ███████                                                                      
//    ██      ██  ██ ██ ██    ██ ██  ██  ██      ██                                                                      
//    ███████ ██   ████  ██████  ██      ██ ███████                                                                      
//                                                                                                                       
//                                                                                                                       
enum class Type
{
    PROFILE          = 0,
    PUBLIC              ,
    ALWAYS              ,
    AFTER               ,
    THEN                ,
    MAP                 ,
    MULTITHREAD         ,
    MAIN                ,
    INTERPRETER         ,

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



//    ███    ██  █████  ███    ███ ███████ ███████ ██████   █████   ██████ ███████      █████  ████████ ████████ ██████        
//    ████   ██ ██   ██ ████  ████ ██      ██      ██   ██ ██   ██ ██      ██          ██   ██    ██       ██    ██   ██       
//    ██ ██  ██ ███████ ██ ████ ██ █████   ███████ ██████  ███████ ██      █████       ███████    ██       ██    ██████        
//    ██  ██ ██ ██   ██ ██  ██  ██ ██           ██ ██      ██   ██ ██      ██          ██   ██    ██       ██    ██   ██       
//    ██   ████ ██   ██ ██      ██ ███████ ███████ ██      ██   ██  ██████ ███████     ██   ██    ██       ██    ██   ██       
//                                                                                                                             
//                                                                                                                             
//    ███████  ██████  ██████  ██     ██  █████  ██████  ██████  ███████ ██████      ████████ ██    ██ ██████  ███████ ███████ 
//    ██      ██    ██ ██   ██ ██     ██ ██   ██ ██   ██ ██   ██ ██      ██   ██        ██     ██  ██  ██   ██ ██      ██      
//    █████   ██    ██ ██████  ██  █  ██ ███████ ██████  ██   ██ █████   ██   ██        ██      ████   ██████  █████   ███████ 
//    ██      ██    ██ ██   ██ ██ ███ ██ ██   ██ ██   ██ ██   ██ ██      ██   ██        ██       ██    ██      ██           ██ 
//    ██       ██████  ██   ██  ███ ███  ██   ██ ██   ██ ██████  ███████ ██████         ██       ██    ██      ███████ ███████ 
//                                                                                                                             
//                                                                                                                             

struct Attribute;


//    ███    ██  █████  ███    ███ ███████ ███████ ██████   █████   ██████ ███████      █████  ████████ ████████ ██████  
//    ████   ██ ██   ██ ████  ████ ██      ██      ██   ██ ██   ██ ██      ██          ██   ██    ██       ██    ██   ██ 
//    ██ ██  ██ ███████ ██ ████ ██ █████   ███████ ██████  ███████ ██      █████       ███████    ██       ██    ██████  
//    ██  ██ ██ ██   ██ ██  ██  ██ ██           ██ ██      ██   ██ ██      ██          ██   ██    ██       ██    ██   ██ 
//    ██   ████ ██   ██ ██      ██ ███████ ███████ ██      ██   ██  ██████ ███████     ██   ██    ██       ██    ██   ██ 
//                                                                                                                       
//                                                                                                                       
//    ██    ██ ███████ ██ ███    ██  ██████  ███████                                                                     
//    ██    ██ ██      ██ ████   ██ ██       ██                                                                          
//    ██    ██ ███████ ██ ██ ██  ██ ██   ███ ███████                                                                     
//    ██    ██      ██ ██ ██  ██ ██ ██    ██      ██                                                                     
//     ██████  ███████ ██ ██   ████  ██████  ███████                                                                     
//                                                                                                                       
//                                                                                                                       


using Properties = std::vector<std::string>;
using List       = std::vector<Attribute>;
using Targets    = std::vector<Target>;
using Rules      = std::array<Semantic::Rule, _I(Type::ATTRIBUTE__COUNT)>;



//    ███    ██  █████  ███    ███ ███████ ███████ ██████   █████   ██████ ███████      █████  ████████ ████████ ██████                   
//    ████   ██ ██   ██ ████  ████ ██      ██      ██   ██ ██   ██ ██      ██          ██   ██    ██       ██    ██   ██                  
//    ██ ██  ██ ███████ ██ ████ ██ █████   ███████ ██████  ███████ ██      █████       ███████    ██       ██    ██████                   
//    ██  ██ ██ ██   ██ ██  ██  ██ ██           ██ ██      ██   ██ ██      ██          ██   ██    ██       ██    ██   ██                  
//    ██   ████ ██   ██ ██      ██ ███████ ███████ ██      ██   ██  ██████ ███████     ██   ██    ██       ██    ██   ██                  
//                                                                                                                                        
//                                                                                                                                        
//    ██████  ██    ██ ██████  ██      ██  ██████      █████   ██████   ██████  ██████  ███████  ██████   █████  ████████ ███████ ███████ 
//    ██   ██ ██    ██ ██   ██ ██      ██ ██          ██   ██ ██       ██       ██   ██ ██      ██       ██   ██    ██    ██      ██      
//    ██████  ██    ██ ██████  ██      ██ ██          ███████ ██   ███ ██   ███ ██████  █████   ██   ███ ███████    ██    █████   ███████ 
//    ██      ██    ██ ██   ██ ██      ██ ██          ██   ██ ██    ██ ██    ██ ██   ██ ██      ██    ██ ██   ██    ██    ██           ██ 
//    ██       ██████  ██████  ███████ ██  ██████     ██   ██  ██████   ██████  ██   ██ ███████  ██████  ██   ██    ██    ███████ ███████ 
//                                                                                                                                        
//                                                                                                                                        

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


END_NAMESPACE(Attr)

//    ██ ███    ██ ███    ██ ███████ ██████      ███████ ███████ ███    ███  █████  ███    ██ ████████ ██  ██████        
//    ██ ████   ██ ████   ██ ██      ██   ██     ██      ██      ████  ████ ██   ██ ████   ██    ██    ██ ██             
//    ██ ██ ██  ██ ██ ██  ██ █████   ██████      ███████ █████   ██ ████ ██ ███████ ██ ██  ██    ██    ██ ██             
//    ██ ██  ██ ██ ██  ██ ██ ██      ██   ██          ██ ██      ██  ██  ██ ██   ██ ██  ██ ██    ██    ██ ██             
//    ██ ██   ████ ██   ████ ███████ ██   ██     ███████ ███████ ██      ██ ██   ██ ██   ████    ██    ██  ██████        
//                                                                                                                       
//                                                                                                                       
//    ███    ██  █████  ███    ███ ███████ ███████ ██████   █████   ██████ ███████      █████  ████████ ████████ ██████  
//    ████   ██ ██   ██ ████  ████ ██      ██      ██   ██ ██   ██ ██      ██          ██   ██    ██       ██    ██   ██ 
//    ██ ██  ██ ███████ ██ ████ ██ █████   ███████ ██████  ███████ ██      █████       ███████    ██       ██    ██████  
//    ██  ██ ██ ██   ██ ██  ██  ██ ██           ██ ██      ██   ██ ██      ██          ██   ██    ██       ██    ██   ██ 
//    ██   ████ ██   ██ ██      ██ ███████ ███████ ██      ██   ██  ██████ ███████     ██   ██    ██       ██    ██   ██ 
//                                                                                                                       
//                                                                                                                       
//    ███████ ███    ██ ██████                                                                                           
//    ██      ████   ██ ██   ██                                                                                          
//    █████   ██ ██  ██ ██   ██                                                                                          
//    ██      ██  ██ ██ ██   ██                                                                                          
//    ███████ ██   ████ ██████                                                                                           
//                                                                                                                       
//                                                                                                                       




                                                                                                                    

BEGIN_NAMESPACE(Task)
                                  
using Inputs = std::vector<std::string>;
using Instrs = std::vector<std::string>;

END_NAMESPACE(Task)



                                                                                                                               

BEGIN_NAMESPACE(Using)
                                                                                                                              
enum class Type
{
    PROFILES             = 0,
    INTERPRETER             ,
};

                                                                                                                                     

struct Rule
{
    std::vector<std::string> valid_attr;
    Type                     using_type;
};

END_NAMESPACE(Using)










//    ███████ ███████ ███    ███  █████  ███    ██ ████████ ██  ██████     ██████  ██    ██ ██████  ██      ██  ██████         
//    ██      ██      ████  ████ ██   ██ ████   ██    ██    ██ ██          ██   ██ ██    ██ ██   ██ ██      ██ ██              
//    ███████ █████   ██ ████ ██ ███████ ██ ██  ██    ██    ██ ██          ██████  ██    ██ ██████  ██      ██ ██              
//         ██ ██      ██  ██  ██ ██   ██ ██  ██ ██    ██    ██ ██          ██      ██    ██ ██   ██ ██      ██ ██              
//    ███████ ███████ ██      ██ ██   ██ ██   ████    ██    ██  ██████     ██       ██████  ██████  ███████ ██  ██████         
//                                                                                                                             
//                                                                                                                             
//    ███████  ██████  ██████  ██     ██  █████  ██████  ██████  ███████ ██████      ████████ ██    ██ ██████  ███████ ███████ 
//    ██      ██    ██ ██   ██ ██     ██ ██   ██ ██   ██ ██   ██ ██      ██   ██        ██     ██  ██  ██   ██ ██      ██      
//    █████   ██    ██ ██████  ██  █  ██ ███████ ██████  ██   ██ █████   ██   ██        ██      ████   ██████  █████   ███████ 
//    ██      ██    ██ ██   ██ ██ ███ ██ ██   ██ ██   ██ ██   ██ ██      ██   ██        ██       ██    ██      ██           ██ 
//    ██       ██████  ██   ██  ███ ███  ██   ██ ██   ██ ██████  ███████ ██████         ██       ██    ██      ███████ ███████ 
//                                                                                                                             
//                                                                                                                             

class  Engine;
struct Instruction;
struct InstructionAssign;
struct InstructionTask;
struct InstructionCall;
struct Enviroment;



//    ███████ ███████ ███    ███  █████  ███    ██ ████████ ██  ██████     ██    ██ ███████ ██ ███    ██  ██████  ███████ 
//    ██      ██      ████  ████ ██   ██ ████   ██    ██    ██ ██          ██    ██ ██      ██ ████   ██ ██       ██      
//    ███████ █████   ██ ████ ██ ███████ ██ ██  ██    ██    ██ ██          ██    ██ ███████ ██ ██ ██  ██ ██   ███ ███████ 
//         ██ ██      ██  ██  ██ ██   ██ ██  ██ ██    ██    ██ ██          ██    ██      ██ ██ ██  ██ ██ ██    ██      ██ 
//    ███████ ███████ ██      ██ ██   ██ ██   ████    ██    ██  ██████      ██████  ███████ ██ ██   ████  ██████  ███████ 
//                                                                                                                        
//                                                                                                                        


USE_MODULE(Arcana::Support);

template<typename T>
using Ref  = std::reference_wrapper<T>;

template<typename T> 
using CRef = std::reference_wrapper<const T>;

using VTable      = std::map<std::string, InstructionAssign>; 
using FTable      = std::map<std::string, InstructionTask>; 
using VList       = std::vector<InstructionAssign>;
using FList       = std::vector<InstructionTask>;
using FListCRef   = std::vector<CRef<InstructionTask>>;
using Interpreter = std::string;





//    ███████ ███████ ███    ███  █████  ███    ██ ████████ ██  ██████     ██████  ██    ██ ██████  ██      ██  ██████ 
//    ██      ██      ████  ████ ██   ██ ████   ██    ██    ██ ██          ██   ██ ██    ██ ██   ██ ██      ██ ██      
//    ███████ █████   ██ ████ ██ ███████ ██ ██  ██    ██    ██ ██          ██████  ██    ██ ██████  ██      ██ ██      
//         ██ ██      ██  ██  ██ ██   ██ ██  ██ ██    ██    ██ ██          ██      ██    ██ ██   ██ ██      ██ ██      
//    ███████ ███████ ██      ██ ██   ██ ██   ████    ██    ██  ██████     ██       ██████  ██████  ███████ ██  ██████ 
//                                                                                                                     
//                                                                                                                     
//     █████   ██████   ██████  ██████  ███████  ██████   █████  ████████ ███████ ███████                              
//    ██   ██ ██       ██       ██   ██ ██      ██       ██   ██    ██    ██      ██                                   
//    ███████ ██   ███ ██   ███ ██████  █████   ██   ███ ███████    ██    █████   ███████                              
//    ██   ██ ██    ██ ██    ██ ██   ██ ██      ██    ██ ██   ██    ██    ██           ██                              
//    ██   ██  ██████   ██████  ██   ██ ███████  ██████  ██   ██    ██    ███████ ███████                              
//                                                                                                                     
//                                                                                                                     

struct Rule 
{
    Attr::Qualificator qual;
    Attr::Count        count;
    Attr::Targets      targets;

    bool operator == (const Attr::Qualificator q) const { return this->qual == q; }
};



struct InstructionAssign
{
    std::string var_name;
    std::string var_value;
    Attr::List  attributes;

    std::vector<std::string> glob_expansion;

    InstructionAssign() = default;

    InstructionAssign(const std::string& var, const std::string& val)
        :
        var_name(var),
        var_value(val)
    {}

    // copy
    InstructionAssign(const InstructionAssign& other)             = default;
    InstructionAssign& operator=(const InstructionAssign & other) = default;

    bool hasAttribute(const Attr::Type attr) const
    {
        return (std::find(attributes.begin(), attributes.end(), attr) != attributes.end());
    }

    const Attr::Properties
    getProperties(const Attr::Type attr) const
    {
        for (const auto& a : attributes)
            if (a.type == attr)
                return a.props;

        return {};
    }
};


struct InstructionTask
{
    std::string  task_name;
    Task::Inputs task_inputs;
    Task::Instrs task_instrs;
    FListCRef    dependecies;
    FListCRef    thens;
    Attr::List   attributes;
    Interpreter  interpreter;

    InstructionTask() = default;

    InstructionTask(const std::string&  name,
                    const Task::Inputs& inputs,
                    const Task::Instrs& instrs)
        :
        task_name(name),
        task_inputs(inputs),
        task_instrs(instrs)
    {}

    // copy
    InstructionTask(const InstructionTask& other)            = default;
    InstructionTask& operator=(const InstructionTask& other) = default;

    bool hasAttribute(const Attr::Type attr) const
    {
        return (std::find(attributes.begin(), attributes.end(), attr) != attributes.end());
    }

    const Attr::Properties
    getProperties(const Attr::Type attr) const
    {
        for (const auto& a : attributes)
            if (a.type == attr)
                return a.props;

        return {};
    }

    void removeAttribute(const Attr::Type attr)
    {
        for (auto it = attributes.begin(); it != attributes.end(); ++it)
        {    
            if (it->type == attr)
            {
                attributes.erase(it);
                return;
            }
        }
    }
};



struct Profile
{
    std::vector<std::string> profiles;
    std::string              selected;

    void merge(Profile& other)
    {
        for (const auto& val : other.profiles)
            this->profiles.push_back(val);
    }
};



struct Enviroment
{
    friend class Engine;
    friend inline void EnvMerge(Enviroment& dst, Enviroment& src);

public:
    VTable   vtable;
    FTable   ftable;

    const std::optional<std::string> AlignEnviroment() noexcept;
          Arcana_Result              CheckArgs(const Arcana::Support::Arguments& args) noexcept;
    const std::optional<std::string> Expand() noexcept;
    
    Interpreter                      GetInterpreter() noexcept { return default_interpreter; }
    
private:
    Profile     profile;
    Interpreter default_interpreter;
};



inline void EnvMerge(Enviroment& dst, Enviroment& src)
{
    for (auto& [k, v] : src.vtable)
        dst.vtable[k] = std::move(v);

    for (auto& [k, v] : src.ftable)
        dst.ftable[k] = std::move(v);

    dst.profile.merge(src.profile);
}


class Engine
{
public:
    Engine();

    SemanticOutput Collect_Attribute (const std::string& name, const std::string&  prop);
    SemanticOutput Collect_Assignment(const std::string& name, const std::string&  val); 
    SemanticOutput Collect_Task      (const std::string& name, const std::string& inputs, const Task::Instrs& instrs);
    SemanticOutput Collect_Using     (const std::string& what, const std::string&  opt); 

    Enviroment                       GetEnvironment()  const noexcept { return _env; }
    Enviroment&                      EnvRef()                         { return _env; }

private:
    Attr::Rules  _attr_rules;
    Attr::List   _attr_pending;
    std::uint8_t _main_count;
    
    Enviroment   _env;
};


END_MODULE(Semantic)


#endif /* __ARCANA_INSTRUCTION__H__ */