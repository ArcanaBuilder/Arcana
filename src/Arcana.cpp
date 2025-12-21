#include "Glob.h"
#include "Jobs.h"
#include "Core.h"
#include "Cache.h"
#include "Parser.h"
#include "Support.h"
#include "Defines.h"
#include "Semantic.h"
#include "TableHelper.h"

#include <cerrno>
#include <cstring>
#include <iostream>
#include <unistd.h>
#include <filesystem>


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



#define CHECK_RESULT(op)                    if (auto res = op; res != Arcana_Result::ARCANA_RESULT__OK) { return res; }
#define CHECK_STR_RESULT(op)                if (auto res = op; res.has_value())                         { ERR(res.value()); return Arcana_Result::ARCANA_RESULT__NOK; }


static Semantic::Enviroment env;




/**
 * @brief Parse and process the Arcana source file.
 *
 * This function performs lexical analysis, parsing, semantic validation,
 * environment alignment, variable expansion, and assertion execution.
 *
 * @param args Parsed command-line arguments.
 * @return Arcana_Result::ARCANA_RESULT__OK on success, NOK on failure.
 */
static Arcana_Result Parse(const Support::Arguments& args)
{
    // INITIALIZE LEXER, GRAMMAR ENGINE, AND PARSER.
    Scan::Lexer          lexer(args.arcfile);
    Grammar::Engine      engine;
    Parsing::Parser      parser(lexer, engine);

    // REGISTER PARSING, SEMANTIC, AND POST-PROCESS ERROR HANDLERS.
    parser.Set_ParsingError_Handler    (Support::ParserError   {lexer} );
    parser.Set_AnalisysError_Handler   (Support::SemanticError {lexer} );
    parser.Set_PostProcessError_Handler(Support::PostProcError {lexer} );
    
    // PARSE INPUT FILE AND BUILD ENVIRONMENT STATE.
    CHECK_RESULT(parser.Parse(env));

    // VALIDATE CLI ARGUMENTS AGAINST ENVIRONMENT (PROFILES, THREADS, MAIN TASK, ETC).
    CHECK_RESULT(env.CheckArgs(args));

    // ALIGN ENVIRONMENT TABLES AND DEFAULTS.
    CHECK_STR_RESULT(env.AlignEnviroment());

    // EXPAND VARIABLES, GLOBS, AND ATTRIBUTE-DRIVEN TRANSFORMS.
    CHECK_STR_RESULT(env.Expand());

    // EXECUTE ASSERT STATEMENTS.
    CHECK_STR_RESULT(env.ExecuteAsserts());

    // CHECK FOR PUBLIC TASKS PRESENCE.
    if (!Table::GetValues(env.ftable, Semantic::Attr::Type::PUBLIC))
    {
        std::stringstream ss;
        ss << "Arcfile " << TOKEN_MAGENTA(args.arcfile) << " has no public tasks"; 
        ERR(ss.str());
        return Arcana_Result::ARCANA_RESULT__NOK;
    }

    return Arcana_Result::ARCANA_RESULT__OK;
}



/**
 * @brief Build the job list from the environment and execute it.
 *
 * @param args Parsed command-line arguments.
 * @return Arcana_Result::ARCANA_RESULT__OK on success, NOK on failure.
 */
static Arcana_Result Execute(const Support::Arguments& args)
{
    Jobs::List           joblist;
    Core::RunOptions     runopt;

    // BUILD JOBLIST FROM CURRENT ENVIRONMENT.
    CHECK_RESULT(Jobs::List::FromEnv(env, joblist));

    // CONFIGURE RUNTIME EXECUTION OPTIONS.
    runopt.silent          = args.silent;
    runopt.max_parallelism = env.GetThreads();

    // EXECUTE JOBS AND PROPAGATE RESULT.
    return Core::run_jobs(joblist, runopt);
}
                                                                                                   
                                                                                                                                                                                      
                                                                    
/**
 * @brief Arcana program entry point.
 *
 * @param argc Argument count.
 * @param argv Argument vector.
 * @return Process exit code.
 */
int main(int argc, char** argv) 
{   
    Support::Arguments args;

    // PARSE COMMAND-LINE ARGUMENTS INTO `args`.
    CHECK_RESULT(Support::ParseArgs(argc, argv, args));

    // HANDLE PRE PARSE EARLY-EXIT OPTIONS AND VALIDATE INPUTS.
    CHECK_RESULT(Support::HandleArgsPreParse(args));

    // PARSE ARCFILE AND PREPARE THE SEMANTIC ENVIRONMENT.
    CHECK_RESULT(Parse(args));

    // HANDLE POST PARSE EARLY-EXIT OPTIONS.
    CHECK_RESULT(Support::HandleArgsPostParse(args, env));

    // LOAD CACHE AND APPLY PROFILE-RELATED CACHE RULES.
    Cache::Manager::Instance().LoadCache();
    Cache::Manager::Instance().HandleProfileChange(env.GetProfile().selected);

    // GENERATE JOBLIST AND EXECUTE.
    return Execute(args);
}