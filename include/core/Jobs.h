#ifndef __ARCANA_JOBS_H__
#define __ARCANA_JOBS_H__


#include "Defines.h"
#include "Semantic.h"

#include <variant>
#include <unordered_set>


BEGIN_MODULE(Jobs)
USE_MODULE(Arcana);






//    ███████╗ ██████╗ ██████╗ ██╗    ██╗ █████╗ ██████╗ ██████╗ ███████╗
//    ██╔════╝██╔═══██╗██╔══██╗██║    ██║██╔══██╗██╔══██╗██╔══██╗██╔════╝
//    █████╗  ██║   ██║██████╔╝██║ █╗ ██║███████║██████╔╝██║  ██║███████╗
//    ██╔══╝  ██║   ██║██╔══██╗██║███╗██║██╔══██║██╔══██╗██║  ██║╚════██║
//    ██║     ╚██████╔╝██║  ██║╚███╔███╔╝██║  ██║██║  ██║██████╔╝███████║
//    ╚═╝      ╚═════╝ ╚═╝  ╚═╝ ╚══╝╚══╝ ╚═╝  ╚═╝╚═╝  ╚═╝╚═════╝ ╚══════╝
//                                                                                                                                                                                       



class Job;
class List;










//    ███████╗████████╗██████╗ ██╗   ██╗ ██████╗████████╗███████╗
//    ██╔════╝╚══██╔══╝██╔══██╗██║   ██║██╔════╝╚══██╔══╝██╔════╝
//    ███████╗   ██║   ██████╔╝██║   ██║██║        ██║   ███████╗
//    ╚════██║   ██║   ██╔══██╗██║   ██║██║        ██║   ╚════██║
//    ███████║   ██║   ██║  ██║╚██████╔╝╚██████╗   ██║   ███████║
//    ╚══════╝   ╚═╝   ╚═╝  ╚═╝ ╚═════╝  ╚═════╝   ╚═╝   ╚══════╝
//                                                                                                                                                  


struct ExpansionError
{
    bool        ok;
    std::string msg;
    std::string hint;

    ExpansionError() : ok(false) {}
};


struct Job
{
    std::string            name;
    Semantic::Task::Instrs instructions;
    Semantic::Interpreter  interpreter;
    bool                   parallelizable;
    bool                   echo;
};








//     ██████╗██╗      █████╗ ███████╗███████╗███████╗███████╗
//    ██╔════╝██║     ██╔══██╗██╔════╝██╔════╝██╔════╝██╔════╝
//    ██║     ██║     ███████║███████╗███████╗█████╗  ███████╗
//    ██║     ██║     ██╔══██║╚════██║╚════██║██╔══╝  ╚════██║
//    ╚██████╗███████╗██║  ██║███████║███████║███████╗███████║
//     ╚═════╝╚══════╝╚═╝  ╚═╝╚══════╝╚══════╝╚══════╝╚══════╝
//                                                            



class List
{
public:
    List()            = default;
    List(const List&) = default;

    static ExpansionError    FromEnv(Semantic::Enviroment& environment, List& out) noexcept;
    const  std::vector<Job>& All() const noexcept { return data; }

private:
    void Insert(const std::optional<Job>& j);
    void Insert(std::vector<Job>& vj);
    
    std::unordered_set<std::string> index;
    std::vector<Job>                data;
};
                                                                                   


END_MODULE(Jobs)




#endif /* __ARCANA_JOBS_H__ */