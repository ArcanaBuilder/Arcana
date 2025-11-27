#ifndef __ARCANA_PARSER__H__
#define __ARCANA_PARSER__H__


///////////////////////////////////////////////////////////////////////////////
// INCLUDES
///////////////////////////////////////////////////////////////////////////////

#include "Lexer.h"
#include "Defines.h"
#include "Grammar.h"

#include <functional>


///////////////////////////////////////////////////////////////////////////////
// MODULE NEMASPACEs
///////////////////////////////////////////////////////////////////////////////

BEGIN_MODULE(Parsing)


///////////////////////////////////////////////////////////////////////////////
// CALLBACKs
///////////////////////////////////////////////////////////////////////////////

using ErrorCallback = std::function<void (const Grammar::Match&)>;


///////////////////////////////////////////////////////////////////////////////
// PUBLLIC CLASSES
///////////////////////////////////////////////////////////////////////////////

class Parser
{
public:
    Parser(Scan::Lexer& l, Grammar::Engine& e);

    void parse();
    void AddErrorCallback(const ErrorCallback& ecb);

private:
    Scan::Lexer&         lexer;
    Grammar::Engine&     engine;
    ErrorCallback        errorcb;

    void Handle_VarAssign(Grammar::Match& match);
    void Handle_Attribute(Grammar::Match& match);
    void Handle_BuiltinTaskDecl(Grammar::Match& match);
    void Handle_TaskDecl(Grammar::Match& match);
    void Handle_TaskCall(Grammar::Match& match);
    void Handle_Using(Grammar::Match& match);

};



END_MODULE(Parsing)



#endif /* __ARCANA_PARSER__H__ */