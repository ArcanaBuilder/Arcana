#ifndef __ARCANA_CORE_H__
#define __ARCANA_CORE_H__


#include "Jobs.h"
#include "Defines.h"

#include <thread>


BEGIN_MODULE(Core)


USE_MODULE(Arcana);




//    ███████╗███╗   ██╗██╗   ██╗███╗   ███╗███████╗
//    ██╔════╝████╗  ██║██║   ██║████╗ ████║██╔════╝
//    █████╗  ██╔██╗ ██║██║   ██║██╔████╔██║███████╗
//    ██╔══╝  ██║╚██╗██║██║   ██║██║╚██╔╝██║╚════██║
//    ███████╗██║ ╚████║╚██████╔╝██║ ╚═╝ ██║███████║
//    ╚══════╝╚═╝  ╚═══╝ ╚═════╝ ╚═╝     ╚═╝╚══════╝
//        



enum class SymbolType : std::uint8_t
{
    MAIN        = 0,
    ROOT           ,
    VERSION        ,
    PROFILE        ,
    THREADS        ,
    MAX_THREADS    ,
    OS             ,
    ARCH           ,

    UNDEFINED      ,
};



//    ███████╗████████╗██████╗ ██╗   ██╗ ██████╗████████╗███████╗
//    ██╔════╝╚══██╔══╝██╔══██╗██║   ██║██╔════╝╚══██╔══╝██╔════╝
//    ███████╗   ██║   ██████╔╝██║   ██║██║        ██║   ███████╗
//    ╚════██║   ██║   ██╔══██╗██║   ██║██║        ██║   ╚════██║
//    ███████║   ██║   ██║  ██║╚██████╔╝╚██████╗   ██║   ███████║
//    ╚══════╝   ╚═╝   ╚═╝  ╚═╝ ╚═════╝  ╚═════╝   ╚═╝   ╚══════╝
//                                                               


struct InstructionResult
{
    std::string command;
    int         exit_code;
};


struct Result
{
    std::string                    name;
    bool                           ok;
    int                            first_error;
    std::vector<InstructionResult> results;
};


struct RunOptions
{
    bool     silent          = false;
    bool     stop_on_error   = true;
    unsigned max_parallelism = std::thread::hardware_concurrency();
};




//    ██████╗ ██╗   ██╗██████╗     ███████╗██╗   ██╗███╗   ██╗ ██████╗████████╗██╗ ██████╗ ███╗   ██╗███████╗
//    ██╔══██╗██║   ██║██╔══██╗    ██╔════╝██║   ██║████╗  ██║██╔════╝╚══██╔══╝██║██╔═══██╗████╗  ██║██╔════╝
//    ██████╔╝██║   ██║██████╔╝    █████╗  ██║   ██║██╔██╗ ██║██║        ██║   ██║██║   ██║██╔██╗ ██║███████╗
//    ██╔═══╝ ██║   ██║██╔══██╗    ██╔══╝  ██║   ██║██║╚██╗██║██║        ██║   ██║██║   ██║██║╚██╗██║╚════██║
//    ██║     ╚██████╔╝██████╔╝    ██║     ╚██████╔╝██║ ╚████║╚██████╗   ██║   ██║╚██████╔╝██║ ╚████║███████║
//    ╚═╝      ╚═════╝ ╚═════╝     ╚═╝      ╚═════╝ ╚═╝  ╚═══╝ ╚═════╝   ╚═╝   ╚═╝ ╚═════╝ ╚═╝  ╚═══╝╚══════╝
//                                                                                                           


std::vector<Result>       run_jobs(const Jobs::List& jobs, const RunOptions& opt) noexcept;

std::string&              symbol(Core::SymbolType type) noexcept;
SymbolType                is_symbol(const std::string& symbol) noexcept;
void                      update_symbol(SymbolType type, const std::string& val) noexcept;
bool                      is_os(const std::string& param) noexcept;
bool                      is_arch(const std::string& param) noexcept;



END_MODULE(Core)


#endif /* __ARCANA_CORE_H__ */