#ifndef __ARCANA_CACHE_H__
#define __ARCANA_CACHE_H__


#include "Defines.h"

#include <map>
#include <filesystem>


BEGIN_MODULE(Cache)
USE_MODULE(Arcana);





namespace fs = std::filesystem;







std::string MD5(const std::string& data) noexcept;




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

    void     EraseCache()                            noexcept;
    void     ClearCache()                            noexcept;
    void     LoadCache()                             noexcept;
    bool     HasFileChanged(const std::string& path) noexcept;
    fs::path WriteScript(const std::string& path)    noexcept;


private:
    Manager();
    ~Manager() = default;

    fs::path _cache_folder;
    fs::path _script_path;
    fs::path _input_path;
    std::map<std::string, std::string> _cached_inputs;
};



END_MODULE(Cache)



#endif /* __ARCANA_CACHE_H__ */