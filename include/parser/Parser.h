#ifndef __ARCANA_PARSER__H__
#define __ARCANA_PARSER__H__


///////////////////////////////////////////////////////////////////////////////
// INCLUDES
///////////////////////////////////////////////////////////////////////////////

#include "Lexer.h"
#include "Defines.h"
#include "Grammar.h"

#include "Instruction.h"

#include <functional>


///////////////////////////////////////////////////////////////////////////////
// MODULE NEMASPACEs
///////////////////////////////////////////////////////////////////////////////

BEGIN_MODULE(Parsing)


///////////////////////////////////////////////////////////////////////////////
// CALLBACKs
///////////////////////////////////////////////////////////////////////////////

using ParsingError  = std::function<Arcana_Result (const Grammar::Match&)>;
using SemanticError = std::function<Arcana_Result (const Support::AstOutput&)>;


///////////////////////////////////////////////////////////////////////////////
// PUBLLIC CLASSES
///////////////////////////////////////////////////////////////////////////////

class Parser
{
public:
    Parser(Scan::Lexer& l, Grammar::Engine& e);

    Arcana_Result Parse(Ast::Enviroment& env);

    void          Set_ParsingError_Handler (const ParsingError&  ecb) noexcept { Parsing_Error  = ecb; }
    void          Set_SemanticError_Handler(const SemanticError& ecb) noexcept { Semantic_Error = ecb; }

private:
    Scan::Lexer&           lexer;
    Grammar::Engine&       engine;
    Ast::InstructionEngine instr_engine;

    ParsingError           Parsing_Error;
    SemanticError          Semantic_Error;

    Support::AstOutput Handle_VarAssign(Grammar::Match& match);
    Support::AstOutput Handle_Attribute(Grammar::Match& match);
    Support::AstOutput Handle_BuiltinTaskDecl(Grammar::Match& match);
    Support::AstOutput Handle_TaskDecl(Grammar::Match& match);
    Support::AstOutput Handle_TaskCall(Grammar::Match& match);
    Support::AstOutput Handle_Using(Grammar::Match& match);

};



END_MODULE(Parsing)



#endif /* __ARCANA_PARSER__H__ */