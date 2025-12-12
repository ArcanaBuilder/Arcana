#ifndef __ARCANA_CORE_H__
#define __ARCANA_CORE_H__


#include "Jobs.h"
#include "Defines.h"

#include <thread>


BEGIN_MODULE(Core)


USE_MODULE(Arcana);



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


std::vector<Result> run_jobs(const Jobs::List& jobs, const RunOptions& opt) noexcept;



END_MODULE(Core)


#endif /* __ARCANA_CORE_H__ */