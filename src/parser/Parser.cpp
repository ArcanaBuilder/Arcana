#include "Parser.h"
#include "Support.h"


USE_MODULE(Arcana::Parsing);



Parser::Parser(Scan::Lexer& l, Grammar::Engine& e) 
    :
    lexer(l),
    engine(e)
{}

Arcana_Result Parser::Parse(Ast::Enviroment& env)
{
    Scan::Token        token;
    Grammar::Match     match;
    Support::AstOutput astout; 

    do
    {   
        token = lexer.next();
        
        engine.match(token, match);
        
        if (match.isValid())
        {
            switch (match.type)
            {
                case Grammar::Rule::VARIABLE_ASSIGN:   astout = Handle_VarAssign(match);       break;
                case Grammar::Rule::EMPTY_LINE:        /* just ignore this */                  break;
                case Grammar::Rule::ATTRIBUTE:         astout = Handle_Attribute(match);       break;
                case Grammar::Rule::BUILTIN_TASK_DECL: astout = Handle_BuiltinTaskDecl(match); break;
                case Grammar::Rule::TASK_DECL:         astout = Handle_TaskDecl(match);        break;
                case Grammar::Rule::TASK_CALL:         astout = Handle_TaskCall(match);        break;
                case Grammar::Rule::USING:             astout = Handle_Using(match);           break;

                /* how can we reach this case? */
                case Grammar::Rule::UNDEFINED:                                                 break;
            }
            
            if (astout.result != Ast_Result::AST_RESULT__OK)
            {
                return Semantic_Error(astout);
            }
        }

        if (match.isError())
        {
            return Parsing_Error(match);
        }
    } 
    while ( token.type != Scan::TokenType::ENDOFFILE );

    env = instr_engine.Generate_Enviroment();

    return ARCANA_RESULT__OK;
}


Arcana::Support::AstOutput Parser::Handle_VarAssign(Grammar::Match& match)
{
    Point  p1    = match[_I(Grammar::VARIABLE_ASSIGN::VARNAME)];
    Point  p2    = match[_I(Grammar::VARIABLE_ASSIGN::VALUE)];

    Input  input = lexer[p1->token];
    Lexeme var   = input.substr(p1->start, p1->end - p1->start);
    Lexeme value = input.substr(p2->start, p2->end - p2->start);
    
    DBG( "(VARASSIGN)         " << "Var:    " << var);
    DBG( "                    " << "Val:    " << value);

    DBG("------------------------------------------------------------------------");

    return Support::AstOutput{};
}


Arcana::Support::AstOutput Parser::Handle_Attribute(Grammar::Match& match)
{      
    Point  p1      = match[_I(Grammar::ATTRIBUTE::ATTRNAME)];
    Point  p2      = match[_I(Grammar::ATTRIBUTE::ATTROPTION)];
    
    Input  input   = lexer[p1->token];
    Lexeme attr    = input.substr(p1->start, p1->end - p1->start);
    Lexeme attropt = input.substr(p2->start, p2->end - p2->start);

    DBG( "(ATTRIBUTE)         " << "Attr:   " << attr);
    DBG( "                    " << "Option: " << attropt);
    DBG("------------------------------------------------------------------------");

    return instr_engine.Collect_Attribute(attr, attropt);    
}


Arcana::Support::AstOutput Parser::Handle_BuiltinTaskDecl(Grammar::Match& match)
{
    Point  p1    = match[_I(Grammar::BUILTIN_TASK_DECL::TASKNAME)];
    Point  p2    = match[_I(Grammar::BUILTIN_TASK_DECL::PARAMS)];

    Input  input = lexer[p1->token];
    Lexeme task  = input.substr(p1->start, p1->end - p1->start);
    Lexeme param = input.substr(p2->start, p2->end - p2->start);

    DBG( "(BUILTIN TASK DECL) " << "Name:   " << task);
    DBG( "                    " << "Params: " << param);
    DBG("------------------------------------------------------------------------");

    Ast::Task::Params params = Support::split(param, ' ');

    return instr_engine.Collect_Task(task, params, {});
}


Arcana::Support::AstOutput Parser::Handle_TaskDecl(Grammar::Match& match)
{
    Statement body;
    
    Point  p1    = match[_I(Grammar::TASK_DECL::TASKNAME)];
    Point  p2    = match[_I(Grammar::TASK_DECL::PARAMS)];
    Point  bbody = match[_I(Grammar::TASK_DECL::CURLYLP)];
    Point  ebody = match[_I(Grammar::TASK_DECL::CURLYRP)];
    Point  any   = match[_I(Grammar::TASK_DECL::INSTRUCTIONS)];

    Input  input = lexer[p1->token];
    Lexeme task  = input.substr(p1->start, p1->end - p1->start);
    Lexeme param = input.substr(p2->start, p2->end - p2->start);

    if (bbody->token.line == ebody->token.line)
    {
        body.push_back(Arcana::Support::ltrim(input.substr(any->start, any->end - any->start)));
    }
    else
    {
        for (size_t i = bbody->token.line; i < ebody->token.line - 1; ++ i)
        {
            body.push_back(Arcana::Support::ltrim(lexer[i]));
        }
    }
    
    DBG( "(TASK DECL)         " << "Name:   " << task);
    DBG( "                    " << "Params: " << param);
    DBG( "                    " << "Body:");

    for (size_t i = 0; i < body.size(); ++ i)
    {
        DBG( "                            " << i + 1 << ": " << body[i]);
    }
    DBG("------------------------------------------------------------------------");

    Ast::Task::Params params = Support::split(param, ' ');

    return instr_engine.Collect_Task(task, params, body);
}

Arcana::Support::AstOutput Parser::Handle_TaskCall(Grammar::Match& match)
{
    Point  p1    = match[_I(Grammar::TASK_CALL::TASKNAME)];
    Point  p2    = match[_I(Grammar::TASK_CALL::PARAMS)];
    
    Input  input = lexer[p1->token];
    Lexeme task  = input.substr(p1->start, p1->end - p1->start);
    Lexeme param = input.substr(p2->start, p2->end - p2->start);
    
    DBG( "(TASK CALL)         " << "Name:   " << task);
    DBG( "                    " << "Params: " << param);
    DBG("------------------------------------------------------------------------");

    Ast::Task::Params params = Support::split(param, ' ');

    return instr_engine.Collect_TaskCall(task, params);
}


Arcana::Support::AstOutput Parser::Handle_Using(Grammar::Match& match)
{
    Point  p1     = match[_I(Grammar::USING::SCRIPT)];
    
    Input  input  = lexer[p1->token];
    Lexeme script = input.substr(p1->start, p1->end - p1->start);
    
    DBG( "(USING)             " << "Script: " << script);
    DBG("------------------------------------------------------------------------");

    return instr_engine.Collect_Using(script);
}