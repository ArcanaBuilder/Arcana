#include "Jobs.h"
#include "Core.h"
#include "Debug.h" 
#include "Cache.h"
#include "Parser.h"
#include "Support.h"
#include "Defines.h"
#include "Semantic.h"
#include "Generator.h"
#include "TableHelper.h"

#include <variant>
#include <iostream>



USE_MODULE(Arcana);





//
//       ░███    ░█████████    ░██████     ░███    ░███    ░██    ░███    
//      ░██░██   ░██     ░██  ░██   ░██   ░██░██   ░████   ░██   ░██░██   
//     ░██  ░██  ░██     ░██ ░██         ░██  ░██  ░██░██  ░██  ░██  ░██  
//    ░█████████ ░█████████  ░██        ░█████████ ░██ ░██ ░██ ░█████████ 
//    ░██    ░██ ░██   ░██   ░██        ░██    ░██ ░██  ░██░██ ░██    ░██ 
//    ░██    ░██ ░██    ░██   ░██   ░██ ░██    ░██ ░██   ░████ ░██    ░██ 
//    ░██    ░██ ░██     ░██   ░██████  ░██    ░██ ░██    ░███ ░██    ░██ 
//     


static Semantic::Enviroment env;




static Arcana_Result Version(void)
{
    MSG("Arcana:  the make-like but user frendly builder");
    MSG("Version: " << __ARCANA__VERSION__STR__);

    return Arcana_Result::ARCANA_RESULT__OK;
}




static Arcana_Result Help(void)
{
    static const char* ARCANA_HELP = R"HELP(
                                                
         ▄████▄ █████▄  ▄█████ ▄████▄ ███  ██ ▄████▄ 
         ██▄▄██ ██▄▄██▄ ██     ██▄▄██ ██ ▀▄██ ██▄▄██ 
         ██  ██ ██   ██ ▀█████ ██  ██ ██   ██ ██  ██ 
                                         
         Arcana, the make-like but user frendly builder


DESCRIPTION
  Arcana lets you build your project in a simple and modern way.
  By defining tasks, statements, and variables, characterizing them with attributes, 
  you'll be able to define the main building steps of your project yourself, 
  in the cleanest possible form.


USAGE
  arcana [task] [options]
  arcana --help
  arcana --version


OPTIONS
  --help                Show this help message.
  --version             Print the arcana version.
  --debug-jobs          Show tasks that would run, without executing them.
  --flush-cache         Flush arcana cache, then exit.
  -p <profile>          Build with a specific profile. Profiles must be declared in the arcfile. 
  -s <arcfile>          Build with a specific arcfile. 
  --generate [stream]   Generate an arcfile template. If a stream is passed the template will be
                        saved into it.
                        If the stream is stdout, the template will be printed on int. 



LANGUAGE:
  It's a deliberately lightweight grammar, to avoid the complexities of other builders.
  It allows the use of native Arcana statements, variable declarations, and tasks, 
  with the ability to be customized through attributes that define their behavior and execution order.
  In particular, body tasks are grammar-less, meaning no control over their content is performed.
  This is because we wanted to offer users the freedom to use their preferred interpreter 
  to execute the instructions.
  This means no custom statements like if/for/while, no strange symbols, and no overly complex syntax.
  The only exception is the ability to expand variables declared in Arcana within task statements.

  NATIVE STATEMENTS:
    using profiles <Profile list>                   Allows the user to define a set of profiles to use 
                                                    in the arcana code.
                                                    Any use of profiles not declared in this way will 
                                                    raise an error.

    using default interpreter <path to interpreter> Allows the user to define the default interpreter 
                                                    for task bodies. 
                                                    By default, /bin/bash will be used.
    
    using threads <max threads nuumber>             Allows the user to define the number of threads on 
                                                    which to parallelize the execution of a specific task.
                                                    Omitting this statement will result in the use of all 
                                                    the cores on your machine.
  
  SYSTEM VARIABLES:
    In Arcana there are a few system variables that can be used in simple variable expansions.

    __profile__                     A system variable that identifies the currently selected profile. 
                                    If no profile exists, it will have the value 'None'.

    __version__                     A system variable that identifies the current version of arcana.


  VARIABLES:
    NAME = VALUE                    Simple assignment of VALUE into NAME
    GLOB = path/**.c                Simple assignment of path/**.c into GLOB, but at runtime
                                    the engine will try to expand the glob **.c
    @map GLOB
    VAR  = path2/**.o               Using the @map X attribute on a glob variable Y will generate 
                                    a mapping of X to Y


  TASKS:
    task Name(INPUT_PARAMS)         
    {
        instructions...
    }

    A task declaration follows the linear semantics of 'task NAME(OPTIONAL_INPUTS) { OPTIONAL_STATEMENTS }'.
    The inputs are not related to the body of the task itself; they only tell arcana that this task 
    handles these data sets.
    This is to keep track of which statements can be avoided in the cache because they have not changed.
    A task can have 0 inputs and 0 statements.
    If it has 0 statements, it will be optimized by eliminating the task itself, but through the use of 
    attributes like @then, @after, and @pub, it becomes a wrapper that allows the invocation of private tasks.
    As mentioned above, the only task statement management for arcana translates into the expansion 
    of arcana variables.
    Here too, the logic is quite simple.

    VARIABLES EXPANSION:
      There are various types of expansion:

      1) simple expansion, follows the grammar {arc:VARNAME}, results in a simple text replacing with 
         the contents of a variable.
      2) inline expansion, follows the grammar {arc:inline:VARNAME}, translates to an inline expansion 
         of the contents of a glob variable.
      3) list expansion, follows the grammar {arc:list:VARNAME}, translates into an expansion of the 
         statement into several sibling statements, each characterized by an entry of the glob type 
         variable.

      For glob expansions, if the passed variable is not a glob, its nominal content will be used.


  ATTRIBUTES:
    Attributes allow you to customize variables and, above all, tasks as much as possible.

    @map                            Valid only for variables. Allows you to map one glob to another.
    @pub                            Export task to the caller. By defaults all symbols are private.
    @main                           Mark the task as main task.
    @echo                           Prints at runtime on stdout the executed task instructions.
    @then        <task list>        After the execution of the task with the after attribute, 
                                    the specified tasks will be called.
    @requires    <task list>        Before the execution of the task with the after attribute, 
                                    the specified tasks will be called.
    @always                         Execute the task regardless of job scheduling.
    @profile     <profile>          Used only when the <profile> is given.
    @flushcache                     Clears cache, forces subsequent tasks to ignore it.
    @interpreter <interpreter>      Force the task to be executed with the specified interpreter.
    @multithread                    Enable the multithread for the selected task, not guaranteed.


EXAMPLES:
  arcana
  arcana <TASK>
  arcana <TASK> -p Debug
)HELP";

    MSG(ARCANA_HELP);

    return Arcana_Result::ARCANA_RESULT__OK;
}



static Arcana_Result Parse(const Support::Arguments& args)
{
    Arcana_Result result = Arcana_Result::ARCANA_RESULT__OK;
    Scan::Lexer          lexer(args.arcfile);
    Grammar::Engine      engine;
    Parsing::Parser      parser(lexer, engine);

    parser.Set_ParsingError_Handler    (Support::ParserError   {lexer} );
    parser.Set_AnalisysError_Handler   (Support::SemanticError {lexer} );
    parser.Set_PostProcessError_Handler(Support::PostProcError {lexer} );
    
    result = parser.Parse(env);

    if (result != Arcana_Result::ARCANA_RESULT__OK)
    {
        return result;
    }
    
    result = env.CheckArgs(args);

    if (result != Arcana_Result::ARCANA_RESULT__OK)
    {
        return result;
    }

    auto alignment_result = env.AlignEnviroment();
    
    if (alignment_result)
    {
        ERR(alignment_result.value());
        return Arcana_Result::ARCANA_RESULT__NOK;
    }
    
    auto expand_result = env.Expand();

    if (expand_result)
    {
        ERR(expand_result.value());
        return Arcana_Result::ARCANA_RESULT__NOK;
    }

    return Arcana_Result::ARCANA_RESULT__OK;
}



static Arcana_Result Execute(const Support::Arguments& args)
{
    Jobs::List           joblist;
    Core::RunOptions     runopt;

    const auto& jobs_result = Jobs::List::FromEnv(env, joblist);

    if (!jobs_result.ok)
    {
        ERR(jobs_result.msg);
        return Arcana_Result::ARCANA_RESULT__NOK;
    }

    if (args.debug_jobs)
    {
        Debug::DebugJobsList(joblist);
        return ARCANA_RESULT__OK;
    }

    runopt.max_parallelism = env.GetThreads();
    Core::run_jobs(joblist, runopt);

    return Arcana_Result::ARCANA_RESULT__OK;
}



                                                                                                                  
                                                                                                                                                                                      
                                                                    
int main(int argc, char** argv) 
{   
    Support::Arguments args;

    auto res = Support::ParseArgs(argc, argv);

    if (std::holds_alternative<std::string>(res)) 
    {
        ERR(std::get<std::string>(res));
        return Arcana_Result::ARCANA_RESULT__NOK;
    } 
    else 
    {
        args = std::get<Support::Arguments>(res);
    }

    if (args.version)
    {
        return Version();
    }

    if (args.help)
    {
        return Help();
    }

    if (args.flush_cache)
    {
        Cache::Manager::Instance().EraseCache();
        return Arcana_Result::ARCANA_RESULT__OK;
    }

    if (args.generator.found)
    {
        bool res = Generator::Generate_Template(args.generator.value);

        if (!res)
        {
            ERR("Cannot generate template!");
            return Arcana_Result::ARCANA_RESULT__NOK;
        }

        ARC("Generated template in " << args.generator.value << "!");
        return Arcana_Result::ARCANA_RESULT__OK;
    }

    if (!Support::file_exists(args.arcfile))
    {
        ERR("Script arcfile not found!");
        return Arcana_Result::ARCANA_RESULT__NOK;
    }

    Cache::Manager::Instance().LoadCache();

    auto result = Parse(args);

    if (result != Arcana_Result::ARCANA_RESULT__OK)
    {
        return result;
    }

    return Execute(args);
}