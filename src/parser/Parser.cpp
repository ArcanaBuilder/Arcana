#include "Parser.h"



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
    Token  token;
    Tokens tokens;

    do
    {   
        token       = lexer.next();
        Match match = semantic.match(token.type);

        tokens.push_back(token);

        if (match.isValid())
        {
            switch (match.type)
            {
                case SemanticType::VARIABLE_ASSIGN:   Handle_VarAssign(tokens);       tokens.clear(); break;
                case SemanticType::EMPTY_LINE:        /* just ignore this */          tokens.clear(); break;
                case SemanticType::ATTRIBUTE:         Handle_Attribute(tokens);       tokens.clear(); break;
                case SemanticType::BUILTIN_TASK_DECL: Handle_BuiltinTaskDecl(tokens); tokens.clear(); break;
                case SemanticType::TASK_DECL:         Handle_TaskDecl(tokens);        tokens.clear(); break;

                /* how can we reach this case? */
                case SemanticType::UNDEFINED:                                         tokens.clear(); break;
            }
        }

        if (match.isError())
        {
            match.Error.current = token;
            errorcb(match);
            break;
        }
    } 
    while ( token.type != TokenType::ENDOFFILE );
}


void Parser::Handle_VarAssign(Tokens& tokens)
{
    DMSG( "(VARASSIGN)         " << lexer[tokens[0].line - 1]);
}


void Parser::Handle_Attribute(Tokens& tokens)
{      
    DMSG( "(ATTRIBUTE)         " << lexer[tokens[0].line - 1]);
}


void Parser::Handle_BuiltinTaskDecl(Tokens& tokens)
{
    DMSG( "(BUILTIN TASK DECL) " << lexer[tokens[0].line - 1]);
}


void Parser::Handle_TaskDecl(Tokens& tokens)
{
    DMSG( "(TASK DECL)         " << lexer[tokens[0].line - 1]);
}