#ifndef __ARCANA_PARSER__H__
#define __ARCANA_PARSER__H__


#include <functional>

#include "Util.h"
#include "Lexer.h"
#include "Semantic.h"



BEGIN_MODULE(Parser)


using ErrorCallback = std::function<void (const Match&)>;



class Parser
{
public:
    Parser(Lexer& l, Semantic& s);

    void parse();
    void AddErrorCallback(const ErrorCallback& ecb);

private:
    Lexer&         lexer;
    Semantic&      semantic;
    ErrorCallback  errorcb;

    void Handle_VarAssign(Match& match);
    void Handle_Attribute(Match& match);
    void Handle_BuiltinTaskDecl(Match& match);
    void Handle_TaskDecl(Match& match);
    void Handle_TaskCall(Match& match);

};



END_MODULE(Parser)



#endif /* __ARCANA_PARSER__H__ */