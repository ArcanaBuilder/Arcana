#ifndef __ARCANA_GENERATOR_H__
#define __ARCANA_GENERATOR_H__


#include "Defines.h"

#include <ostream>
#include <fstream>
#include <filesystem>


BEGIN_MODULE(Generator)



static const char* ARCANA_TEMPLATE = R"TEMPLATE(
#!/usr/bin/arcana

using profiles Debug Release;
using default interpreter /bin/bash;
using threads 1;

@profile Debug;   FLAGS = 
@profile Release; FLAGS = 

COMPILER = 
INCLUDES = 
TARGET   = 
SOURCES  = 
OBJECTS  =
BUILDDIR = 
SRCDIR   = 
SYSBINS  = /bin

map SOURCES -> OBJECTS;

assert "{arc:COMPILER}" in "{fs:{arc:SYSBINS}}" -> "{arc:COMPILER} is required for this project";


###########################
# PRIVATE TASKS
###########################



###########################
# PUBLIC TASKS
###########################

@pub
@flushcache
task Clean() 
{ 

}

@pub
@main
task Build() 
{

}

@pub
@requires Clean Build
task Rebuild() {}

@pub
@requires Rebuild
task Install()
{

}

)TEMPLATE";




inline bool Generate_Template(std::string& output)
{
    namespace fs = std::filesystem;

    if (output.empty())
    {
        output = "arcfile";
    }
    
    if (output.compare("stdout") == 0)
    {
        MSG(ARCANA_TEMPLATE);
        return true;
    }
    
    fs::path file = output;
    std::error_code ec;
    if (!file.parent_path().empty())
    {
        if (!fs::exists(file.parent_path(), ec))
        {
            if (!fs::create_directories(file.parent_path(), ec))
            {
                return false;
            }
        }
    }

    std::ofstream out(file, std::ios::binary | std::ios::trunc);
    if (!out)
    {
        return false;
    }

    out << ARCANA_TEMPLATE;
    return out.good();
}




END_MODULE(Generator)



#endif /* __ARCANA_GENERATOR_H__ */