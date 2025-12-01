///////////////////////////////////////////////////////////////////////////////
// INCLUDES
///////////////////////////////////////////////////////////////////////////////

#include "Jobs.h"
#include "Parser.h"
#include "Support.h"
#include "Defines.h"
#include "Semantic.h"
#include "TableHelper.h"

#include <variant>
#include <iostream>


///////////////////////////////////////////////////////////////////////////////
// USING
///////////////////////////////////////////////////////////////////////////////

USE_MODULE(Arcana);



///////////////////////////////////////////////////////////////////////////////
// PUBLIC PROTOS
///////////////////////////////////////////////////////////////////////////////




///////////////////////////////////////////////////////////////////////////////
// PRIVATE PROTOS
///////////////////////////////////////////////////////////////////////////////

static Arcana_Result run(const Support::Arguments& args);




///////////////////////////////////////////////////////////////////////////////
// MAIN
///////////////////////////////////////////////////////////////////////////////

int main(int argc, char** argv) 
{   
    Support::Arguments args;

    auto res = Support::ParseArgs(argc, argv);

    if (std::holds_alternative<std::string>(res)) 
    {
        ERR(std::get<std::string>(res));
        return ARCANA_RESULT__PARSING_ERROR;
    } 
    else 
    {
        args = std::get<Support::Arguments>(res);
    }

    return run(args);
}


///////////////////////////////////////////////////////////////////////////////
// PRIVATE FUNCTIONS
///////////////////////////////////////////////////////////////////////////////



static Arcana_Result run(const Support::Arguments& args)
{   
    Arcana_Result result = ARCANA_RESULT__OK;

    if (!Support::file_exists(args.arcfile))
    {
        ERR("Script arcfile not found!");
        return Arcana_Result::ARCANA_RESULT__INVALID_ARGS;
    }

    Scan::Lexer          lexer(args.arcfile);
    Grammar::Engine      engine;
    Parsing::Parser      parser(lexer, engine);
    Semantic::Enviroment env;
    Jobs::JobList        job_list;

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
        return Arcana_Result::ARCANA_RESULT__INVALID_ARGS;
    }
    
    env.Expand();

    job_list = Jobs::FromEnv(env);

    return result;
}