#ifndef __ARCANA_INSTRUCTION__H__
#define __ARCANA_INSTRUCTION__H__


#include "Util.h"


BEGIN_MODULE(Core)



enum class Attribute
{
    PRECOMPILER      = 0,
    POSTCOMPILER        ,
    BUILTIN             ,
    PROFILING           ,
};


enum class BuiltinTask
{
    CLEAN            = 0,
    BUILD               ,
    INSTALL             ,
};


enum class BuiltinVars
{
    MODE             = 0,
    COMPILER            ,
    FLAGS               ,
    LIBS                ,
    TARGET              ,
};


enum class InstructionType
{
    ASSIGN           = 0,
    EXPAND              ,
    INVOKE              ,
};


struct InstructionBase
{
    InstructionBase() {}
    
    virtual ~InstructionBase() = default;
};


struct InstructionAssign
{
    std::string var;
    std::string val;

    InstructionAssign(const std::string& var, const std::string& val)
        :
        var(var),
        val(val)
    {}
};


END_MODULE(Core)


#endif /* __ARCANA_INSTRUCTION__H__ */