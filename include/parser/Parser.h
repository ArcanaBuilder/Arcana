#ifndef __ARCANA_PARSER__H__
#define __ARCANA_PARSER__H__

#include "Lexer.h"
#include "Defines.h"
#include "Grammar.h"
#include "Semantic.h"

#include <functional>



BEGIN_MODULE(Parsing)



//     ██████╗ █████╗ ██╗     ██╗     ██████╗  █████╗  ██████╗██╗  ██╗███████╗
//    ██╔════╝██╔══██╗██║     ██║     ██╔══██╗██╔══██╗██╔════╝██║ ██╔╝██╔════╝
//    ██║     ███████║██║     ██║     ██████╔╝███████║██║     █████╔╝ ███████╗
//    ██║     ██╔══██║██║     ██║     ██╔══██╗██╔══██║██║     ██╔═██╗ ╚════██║
//    ╚██████╗██║  ██║███████╗███████╗██████╔╝██║  ██║╚██████╗██║  ██╗███████║
//     ╚═════╝╚═╝  ╚═╝╚══════╝╚══════╝╚═════╝ ╚═╝  ╚═╝ ╚═════╝╚═╝  ╚═╝╚══════╝
//                                                                                                                                                       

using ParsingError  = std::function<Arcana_Result (const std::string& ctx, const Grammar::Match&)>;
using AnalisysError = std::function<Arcana_Result (const std::string& ctx, const Support::SemanticOutput&, const Grammar::Match&)>;
using PostProcError = std::function<Arcana_Result (const std::string& ctx, const std::string& err)>;




//     ██████╗██╗      █████╗ ███████╗███████╗███████╗███████╗
//    ██╔════╝██║     ██╔══██╗██╔════╝██╔════╝██╔════╝██╔════╝
//    ██║     ██║     ███████║███████╗███████╗█████╗  ███████╗
//    ██║     ██║     ██╔══██║╚════██║╚════██║██╔══╝  ╚════██║
//    ╚██████╗███████╗██║  ██║███████║███████║███████╗███████║
//     ╚═════╝╚══════╝╚═╝  ╚═╝╚══════╝╚══════╝╚══════╝╚══════╝
//                                                                                                                                                                  

class Parser
{
public:
    Parser(Scan::Lexer& l, Grammar::Engine& e);

    Arcana_Result Parse(Semantic::Enviroment& env);

    void          Set_ParsingError_Handler    (const ParsingError&  ecb) noexcept { Parsing_Error  = std::move(ecb); }
    void          Set_AnalisysError_Handler   (const AnalisysError& ecb) noexcept { Analisys_Error = std::move(ecb); }
    void          Set_PostProcessError_Handler(const PostProcError& ecb) noexcept { PostProc_Error = std::move(ecb); }


private:
    Scan::Lexer&     lexer;
    Grammar::Engine& engine;
    Semantic::Engine instr_engine;

    ParsingError     Parsing_Error;
    AnalisysError    Analisys_Error;
    PostProcError    PostProc_Error;

    Support::SemanticOutput Handle_VarAssign(Grammar::Match& match);
    Support::SemanticOutput Handle_Attribute(Grammar::Match& match);
    Support::SemanticOutput Handle_TaskDecl(Grammar::Match& match);
    Support::SemanticOutput Handle_Using(Grammar::Match& match);
    Support::SemanticOutput Handle_Mapping(Grammar::Match& match);
    
    Arcana_Result           Handle_Import(Grammar::Match& match, Semantic::Enviroment& new_env);
};



END_MODULE(Parsing)



#endif /* __ARCANA_PARSER__H__ */