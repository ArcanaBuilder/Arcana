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

COMPILER = g++

@profile Debug
FLAGS    = -std=c++17 -g3 -O0 -Wall -Wextra -pedantic -DDEBUG

@profile Release
FLAGS    = -std=c++17 -Os -Wall -Wextra -DRELEASE

INCLUDES = -Iinclude

TARGET   = {arc:BUILDDIR}/arcana
SOURCES  = {arc:SRCDIR}/**.cpp

@map SOURCES
OBJECTS  = {arc:BUILDDIR}/**.o

BUILDDIR = build
SRCDIR   = src


###########################
# PRIVATE TASKS
###########################


task MakeFolders() 
{
    cd {arc:SRCDIR}
    find . -type d -exec mkdir -p "../{arc:BUILDDIR}"/{} \;
}

@echo
@multithread
task Compile(SOURCES) 
{
    {arc:COMPILER} {arc:FLAGS} {arc:INCLUDES} -c {arc:list:SOURCES} -o {arc:list:OBJECTS}
}
    
    
task Link()
{        
    {arc:COMPILER} {arc:FLAGS} {arc:inline:OBJECTS} -o {arc:TARGET}
}


###########################
# PUBLIC TASKS
###########################

@pub
@flushcache
@then MakeFolders
task Clean() 
{ 
    rm -rf {arc:BUILDDIR}
}

@pub
@main
@requires Compile Link
task Build() {}

@pub
@requires Clean Build
task Rebuild() {}

@pub
@requires Clean Build
task Install()
{
    cp {arc:TARGET} /usr/bin/  
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