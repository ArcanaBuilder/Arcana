#include "Parser.h"
#include "Instruction.h"


USE_MODULE(Arcana::Parser);


Parser::Parser(Lexer& l, Semantic& s) 
    :
    lexer(l),
    semantic(s)
{}


void Parser::AddErrorCallback(const ErrorCallback& ecb)
{
    errorcb = std::move(ecb);
}

void Parser::parse()
{
    Token token;
    Match match;

    do
    {   
        token = lexer.next();

        semantic.match(token, match);

        if (match.isValid())
        {
            switch (match.type)
            {
                case SemanticType::VARIABLE_ASSIGN:   Handle_VarAssign(match);       break;
                case SemanticType::EMPTY_LINE:        /* just ignore this */         break;
                case SemanticType::ATTRIBUTE:         Handle_Attribute(match);       break;
                case SemanticType::BUILTIN_TASK_DECL: Handle_BuiltinTaskDecl(match); break;
                case SemanticType::TASK_DECL:         Handle_TaskDecl(match);        break;
                case SemanticType::TASK_CALL:         Handle_TaskCall(match);        break;

                /* how can we reach this case? */
                case SemanticType::UNDEFINED:                                        break;
            }
        }

        if (match.isError())
        {
            errorcb(match);
            break;
        }
    } 
    while ( token.type != TokenType::ENDOFFILE );
}


void Parser::Handle_VarAssign(Match& match)
{
    const Index* p1            = match[(size_t) Grammar_VARIABLE_ASSIGN::VARNAME];
    const Index* p2            = match[(size_t) Grammar_VARIABLE_ASSIGN::VALUE];
    std::string& input         = lexer[p1->token];

    std::string  var           = input.substr(p1->start, p1->end - p1->start);
    std::string  value         = input.substr(p2->start, p2->end - p2->start);
    
    DMSG( "(VARASSIGN)         " << "Var:    " << var);
    DMSG( "                    " << "Val:    " << value);

    DMSG("------------------------------------------------------------------------");
}


void Parser::Handle_Attribute(Match& match)
{      
    const Index* p1            = match[(size_t) Grammar_ATTRIBUTE::ATTRNAME];
    const Index* p2            = match[(size_t) Grammar_ATTRIBUTE::ATTROPTION];
    std::string& input         = lexer[p1->token];

    std::string  attr          = input.substr(p1->start, p1->end - p1->start);
    std::string  attropt       = input.substr(p2->start, p2->end - p2->start);
    
    DMSG( "(ATTRIBUTE)         " << "Attr:   " << attr);
    DMSG( "                    " << "Option: " << attropt);
    DMSG("------------------------------------------------------------------------");
}


void Parser::Handle_BuiltinTaskDecl(Match& match)
{
    const Index* p1            = match[(size_t) Grammar_BUILTIN_TASK_DECL::TASKNAME];
    const Index* p2            = match[(size_t) Grammar_BUILTIN_TASK_DECL::PARAMS];
    std::string& input         = lexer[p1->token];

    std::string  task          = input.substr(p1->start, p1->end - p1->start);
    std::string  param         = input.substr(p2->start, p2->end - p2->start);
    
    DMSG( "(BUILTIN TASK DECL) " << "Name:   " << task);
    DMSG( "                    " << "Params: " << param);
    DMSG("------------------------------------------------------------------------");
}


void Parser::Handle_TaskDecl(Match& match)
{
    std::vector<std::string> body;
    
    const Index* p1            = match[(size_t) Grammar_TASK_DECL::TASKNAME];
    const Index* p2            = match[(size_t) Grammar_TASK_DECL::PARAMS];
    std::string  input         = lexer[p1->token];

    const Index* bbody         = match[(size_t) Grammar_TASK_DECL::CURLYLP];
    const Index* ebody         = match[(size_t) Grammar_TASK_DECL::CURLYRP];

    for (size_t i = bbody->token.line; i < ebody->token.line - 1; ++ i)
    {
        body.push_back(ltrim(lexer[i]));
    }
    
    std::string  task          = input.substr(p1->start, p1->end - p1->start);
    std::string  param         = input.substr(p2->start, p2->end - p2->start);
    
    DMSG( "(TASK DECL)         " << "Name:   " << task);
    DMSG( "                    " << "Params: " << param);
    DMSG( "                    " << "Body:");

    for (size_t i = 0; i < body.size(); ++ i)
    {
        DMSG( "                            " << i + 1 << ": " << body[i]);
    }
    DMSG("------------------------------------------------------------------------");
}

void Parser::Handle_TaskCall(Match& match)
{
    const Index* p1            = match[(size_t) Grammar_TASK_CALL::TASKNAME];
    const Index* p2            = match[(size_t) Grammar_TASK_CALL::PARAMS];
    std::string& input         = lexer[p1->token];

    std::string  task          = input.substr(p1->start, p1->end - p1->start);
    std::string  param         = input.substr(p2->start, p2->end - p2->start);
    
    DMSG( "(TASK CALL)         " << "Name:   " << task);
    DMSG( "                    " << "Params: " << param);
    DMSG("------------------------------------------------------------------------");
}