#ifndef __ARCANA_PARSER__H__
#define __ARCANA_PARSER__H__


///////////////////////////////////////////////////////////////////////////////
// INCLUDES
///////////////////////////////////////////////////////////////////////////////

#include "Lexer.h"
#include "Defines.h"
#include "Grammar.h"
#include "Semantic.h"

#include <functional>


///////////////////////////////////////////////////////////////////////////////
// MODULE NEMASPACEs
///////////////////////////////////////////////////////////////////////////////

BEGIN_MODULE(Parsing)


///////////////////////////////////////////////////////////////////////////////
// CALLBACKs
///////////////////////////////////////////////////////////////////////////////

using ParsingError  = std::function<Arcana_Result (const Grammar::Match&)>;
using AnalisysError = std::function<Arcana_Result (const Support::SemanticOutput&, const Grammar::Match&)>;


///////////////////////////////////////////////////////////////////////////////
// PUBLLIC CLASSES
///////////////////////////////////////////////////////////////////////////////

class Parser
{
public:
    Parser(Scan::Lexer& l, Grammar::Engine& e);

    Arcana_Result Parse(Semantic::Enviroment& env);

    void          Set_ParsingError_Handler (const ParsingError&  ecb) noexcept { Parsing_Error  = std::move(ecb); }
    void          Set_AnalisysError_Handler(const AnalisysError& ecb) noexcept { Analisys_Error = std::move(ecb); }

private:
    Scan::Lexer&     lexer;
    Grammar::Engine& engine;
    Semantic::Engine instr_engine;

    ParsingError     Parsing_Error;
    AnalisysError    Analisys_Error;

    Support::SemanticOutput Handle_VarAssign(Grammar::Match& match);
    Support::SemanticOutput Handle_Attribute(Grammar::Match& match);
    Support::SemanticOutput Handle_BuiltinTaskDecl(Grammar::Match& match);
    Support::SemanticOutput Handle_TaskDecl(Grammar::Match& match);
    Support::SemanticOutput Handle_TaskCall(Grammar::Match& match);
    Support::SemanticOutput Handle_Using(Grammar::Match& match);

};



END_MODULE(Parsing)



#endif /* __ARCANA_PARSER__H__ */