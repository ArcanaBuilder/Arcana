#ifndef __ARCANA_CACHE_H__
#define __ARCANA_CACHE_H__


#include "Defines.h"



BEGIN_MODULE(Cache)
USE_MODULE(Arcana);




//     ██████╗██╗      █████╗ ███████╗███████╗███████╗███████╗
//    ██╔════╝██║     ██╔══██╗██╔════╝██╔════╝██╔════╝██╔════╝
//    ██║     ██║     ███████║███████╗███████╗█████╗  ███████╗
//    ██║     ██║     ██╔══██║╚════██║╚════██║██╔══╝  ╚════██║
//    ╚██████╗███████╗██║  ██║███████║███████║███████╗███████║
//     ╚═════╝╚══════╝╚═╝  ╚═╝╚══════╝╚══════╝╚══════╝╚══════╝
// 

class Manager
{
public:
    Manager(const Manager&)              = delete;
    Manager& operator = (const Manager&) = delete;
    
    Manager(Manager&&)                    noexcept  = delete;
    Manager& operator = (const Manager&&) noexcept  = delete;
    
    static Manager& Instance() 
    {  
        static Manager m;
        return m;
    }

    bool HasFileChanged(const std::string& path) noexcept;

private:
    Manager()  = default;
    ~Manager() = default;
};



END_MODULE(Cache)



#endif /* __ARCANA_CACHE_H__ */