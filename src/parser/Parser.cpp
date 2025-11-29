#include "Parser.h"
#include "Support.h"

#include <fstream>


USE_MODULE(Arcana::Parsing);



Parser::Parser(Scan::Lexer& l, Grammar::Engine& e) 
    :
    lexer(l),
    engine(e)
{}

Arcana_Result Parser::Parse(Semantic::Enviroment& env)
{
    Scan::Token        token;
    Grammar::Match     match;
    Support::SemanticOutput astout; 

    DBG("");
    DBG("########################################################################");
    DBG("## PARSER");
    DBG("########################################################################");
    DBG("");

    do
    {   
        token = lexer.next();
        
        engine.match(token, match);
        
        if (match.isValid())
        {
            switch (match.type)
            {
                case Grammar::Rule::VARIABLE_ASSIGN:   astout = Handle_VarAssign(match);       break;
                case Grammar::Rule::ATTRIBUTE:         astout = Handle_Attribute(match);       break;
                case Grammar::Rule::TASK_DECL:         astout = Handle_TaskDecl(match);        break;
                case Grammar::Rule::TASK_CALL:         astout = Handle_TaskCall(match);        break;
                case Grammar::Rule::USING:             astout = Handle_Using(match);           break;

                default:                                                                       break;
            }

            if (match.type == Grammar::Rule::IMPORT)
            {
                Semantic::Enviroment new_env;
                Arcana_Result        res = Handle_Import(match, new_env);
                if (res != ARCANA_RESULT__OK)
                {
                    return res;
                }
            }
            
            if (astout.result != Semantic_Result::AST_RESULT__OK)
            {
                return Analisys_Error(astout, match);
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


Arcana::Support::SemanticOutput Parser::Handle_VarAssign(Grammar::Match& match)
{
    Point  p1    = match[_I(Grammar::VARIABLE_ASSIGN::VARNAME)];
    Point  p2    = match[_I(Grammar::VARIABLE_ASSIGN::VALUE)];

    Input  input = lexer[p1->token];
    Lexeme var   = input.substr(p1->start, p1->end - p1->start);
    Lexeme value = input.substr(p2->start, p2->end - p2->start);
    
    DBG( "(VARASSIGN)         " << "Var:    " << var);
    DBG( "                    " << "Val:    " << value);

    DBG("------------------------------------------------------------------------");

    return instr_engine.Collect_Assignment(var, value);    
}


Arcana::Support::SemanticOutput Parser::Handle_Attribute(Grammar::Match& match)
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


Arcana::Support::SemanticOutput Parser::Handle_TaskDecl(Grammar::Match& match)
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

    return instr_engine.Collect_Task(task, param, body);
}

Arcana::Support::SemanticOutput Parser::Handle_TaskCall(Grammar::Match& match)
{
    Point  p1    = match[_I(Grammar::TASK_CALL::TASKNAME)];
    Point  p2    = match[_I(Grammar::TASK_CALL::PARAMS)];
    
    Input  input = lexer[p1->token];
    Lexeme task  = input.substr(p1->start, p1->end - p1->start);
    Lexeme param = input.substr(p2->start, p2->end - p2->start);
    
    DBG( "(TASK CALL)         " << "Name:   " << task);
    DBG( "                    " << "Params: " << param);
    DBG("------------------------------------------------------------------------");

    return instr_engine.Collect_TaskCall(task, param);
}


Arcana_Result Parser::Handle_Import(Grammar::Match& match, Semantic::Enviroment& new_env)
{
    Point  p1     = match[_I(Grammar::IMPORT::SCRIPT)];
    
    Input  input  = lexer[p1->token];
    Lexeme script = input.substr(p1->start, p1->end - p1->start);
    
    DBG( "(IMPORT)            " << "Script: " << script);
    DBG("------------------------------------------------------------------------");

    Arcana_Result        result;
    std::ifstream        file(script);
    Scan::Lexer          lexer(file);
    Grammar::Engine      engine;
    Parsing::Parser      parser(lexer, engine);
    
    result = parser.Parse(new_env);

    if (result == Arcana_Result::ARCANA_RESULT__OK)
    {
        Semantic::EnvMerge(instr_engine.EnvRef(), new_env);
    }

    return result;
}


Arcana::Support::SemanticOutput Parser::Handle_Using(Grammar::Match& match)
{
    Point  p1     = match[_I(Grammar::USING::WHAT)];
    Point  p2     = match[_I(Grammar::USING::OPT)];
    
    Input  input  = lexer[p1->token];

    Lexeme what   = input.substr(p1->start, p1->end - p1->start);
    Lexeme opt    = input.substr(p2->start, p2->end - p2->start);
    
    DBG( "(USING    )         " << "What:   " << what);
    DBG( "                    " << "Option: " << opt);
    DBG("------------------------------------------------------------------------");

    return instr_engine.Collect_Using(what, opt);
}