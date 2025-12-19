#include "Parser.h"
#include "Support.h"

#include <variant>


USE_MODULE(Arcana::Parsing);



//     ██████╗██╗      █████╗ ███████╗███████╗    ██╗███╗   ███╗██████╗ ██╗     
//    ██╔════╝██║     ██╔══██╗██╔════╝██╔════╝    ██║████╗ ████║██╔══██╗██║     
//    ██║     ██║     ███████║███████╗███████╗    ██║██╔████╔██║██████╔╝██║     
//    ██║     ██║     ██╔══██║╚════██║╚════██║    ██║██║╚██╔╝██║██╔═══╝ ██║     
//    ╚██████╗███████╗██║  ██║███████║███████║    ██║██║ ╚═╝ ██║██║     ███████╗
//     ╚═════╝╚══════╝╚═╝  ╚═╝╚══════╝╚══════╝    ╚═╝╚═╝     ╚═╝╚═╝     ╚══════╝
//                                                                                                                                                                


Parser::Parser(Scan::Lexer& l, Grammar::Engine& e) 
    :
    lexer(l),
    engine(e)
{}



Arcana_Result Parser::Parse(Semantic::Enviroment& env)
{
    Scan::Token             token;
    Grammar::Match          match;
    Support::SemanticOutput astout; 

    // ITERATE TOKENS UNTIL ERROR OR EOF
    do
    {   
        // GET THE TOKEN
        token = lexer.next();
        
        // COMPARE TOKEN AND GRAMMAR
        engine.match(token, match);
        
        // IF THERE IS A REGULAR MATCH
        if (match.isValid())
        {
            // HANDLE THE MATCH,
            // DEFAULT CASE HANDLES NEW LINES AND PARTIAL MATCH
            switch (match.type)
            {
                case Grammar::Rule::VARIABLE_ASSIGN:   astout = Handle_VarAssign(match);   match.valid = false;  break;
                case Grammar::Rule::ATTRIBUTE:         astout = Handle_Attribute(match);   match.valid = false;  break;
                case Grammar::Rule::TASK_DECL:         astout = Handle_TaskDecl(match);    match.valid = false;  break;
                case Grammar::Rule::USING:             astout = Handle_Using(match);       match.valid = false;  break;
                case Grammar::Rule::MAPPING:           astout = Handle_Mapping(match);     match.valid = false;  break;
                case Grammar::Rule::ASSERT:            astout = Handle_Assert(match);      match.valid = false;  break;
                default:                                                                   match.valid = false;  break;
            }

            // IF AN IMPORT IS MATCHED, JUST INVOKE THE PROPER HANDLER (ANOTHER PARSER)
            if (match.type == Grammar::Rule::IMPORT)
            {
                // A NEW ENV MUST BE PASSED
                Semantic::Enviroment new_env;
                Arcana_Result        res = Handle_Import(match, new_env);

                // CHECK FOR ERRORS
                if (res != ARCANA_RESULT__OK)
                {
                    return res;
                }
            }
            
            // IF THE HANDLERS RETURNS ERRORS, LOG THE MESSAGES WITH THE CALLBACK
            if (astout.result != Semantic_Result::AST_RESULT__OK)
            {
                return Analisys_Error(lexer.source(), astout, match);
            }
        }

        // IF THE GRAMMAR ENGINE RETURNS ERRORS, LOG THE MESSAGES WITH THE CALLBACK
        if (match.isError())
        {
            return Parsing_Error(lexer.source(), match);
        }
    } 
    while ( token.type != Scan::TokenType::ENDOFFILE );

    // OBTAIN THE GENERATED ENV FROM THE SEMANTIC ENGINE
    env = instr_engine.GetEnvironment();

    return ARCANA_RESULT__OK;
}



Arcana::Support::SemanticOutput Parser::Handle_VarAssign(Grammar::Match& match)
{
    // EXTRACT THE STRINGS FROM THE INPUT AND PASS THEM INTO THE SEMANTIC ENGINE
    Point  p1    = match[_I(Grammar::VARIABLE_ASSIGN::VARNAME)];
    Point  p2    = match[_I(Grammar::VARIABLE_ASSIGN::VALUE)];

    Input  input = lexer[p1->token];
    Lexeme var   = input.substr(p1->start, p1->end - p1->start);
    Lexeme value = input.substr(p2->start, p2->end - p2->start);
    
    return instr_engine.Collect_Assignment(var, value);    
}



Arcana::Support::SemanticOutput Parser::Handle_Attribute(Grammar::Match& match)
{      
    // EXTRACT THE STRINGS FROM THE INPUT AND PASS THEM INTO THE SEMANTIC ENGINE
    Point  p1      = match[_I(Grammar::ATTRIBUTE::ATTRNAME)];
    Point  p2      = match[_I(Grammar::ATTRIBUTE::ATTROPTION)];
    
    Input  input   = lexer[p1->token];
    Lexeme attr    = input.substr(p1->start, p1->end - p1->start);
    Lexeme attropt = input.substr(p2->start, p2->end - p2->start);

    return instr_engine.Collect_Attribute(attr, attropt);    
}



Arcana::Support::SemanticOutput Parser::Handle_TaskDecl(Grammar::Match& match)
{
    // EXTRACT THE STRINGS FROM THE INPUT AND PASS THEM INTO THE SEMANTIC ENGINE
    Statement body;

    Point  p1    = match[_I(Grammar::TASK_DECL::TASKNAME)];
    Point  p2    = match[_I(Grammar::TASK_DECL::INPUTS)];
    Point  bbody = match[_I(Grammar::TASK_DECL::CURLYLP)];
    Point  ebody = match[_I(Grammar::TASK_DECL::CURLYRP)];

    Input  header_line = lexer[p1->token];
    Lexeme task        = header_line.substr(p1->start, p1->end - p1->start);
    Lexeme inputs      = header_line.substr(p2->start, p2->end - p2->start);

    const std::size_t line_begin = bbody->token.line;
    const std::size_t line_end   = ebody->token.line;

    std::string instr;
    
    if (line_begin == line_end)
    {
        // { ... } on the same line
        Input line = lexer[bbody->token]; // line of the curly brace, not the task line

        std::size_t start = bbody->end;    // after  '{'
        std::size_t end   = ebody->start;  // before '}'

        if (end > start)
        {
            instr = line.substr(start, end - start);

            if (!Support::ltrim(instr).empty()) body.push_back(line.substr(start, end - start));
        }
    }
    else
    {
        // first line: after '{' until \n
        {
            Input line = lexer[bbody->token];
            std::size_t start = bbody->end;
            if (start < line.size())
            {
                instr = line.substr(start);
                
                if (!instr.empty() && !Support::ltrim(instr).empty()) body.push_back(instr);
            }
        }

        // task body lines
        for (std::size_t line = line_begin + 1; line < line_end; ++line)
        {
            instr = lexer[line - 1];

            if (!instr.empty()) body.push_back(instr);
        }

        // last line, before '}'
        {
            Input line = lexer[ebody->token];
            std::size_t end = ebody->start;
            if (end > 0)
            {
                instr = line.substr(0, end);

                if (!instr.empty() && !Support::ltrim(instr).empty()) body.push_back(instr);
            }
        }
    }

    return instr_engine.Collect_Task(task, inputs, body);
}



Arcana_Result Parser::Handle_Import(Grammar::Match& match, Semantic::Enviroment& new_env)
{
    // EXTRACT THE STRINGS FROM THE INPUT AND PASS THEM INTO THE IMPORT PARSER

    Point  p1     = match[_I(Grammar::IMPORT::SCRIPT)];
    
    Input  input  = lexer[p1->token];
    Lexeme script = input.substr(p1->start, p1->end - p1->start);

    if (script.empty() || !Support::file_exists(script))
    {
        std::stringstream ss;

        ss << "[" << ANSI_BRED << "SEMANTIC ERROR" << ANSI_RESET << "] In file " << ANSI_BOLD << this->lexer.source() << ANSI_RESET << ", line " 
           << ANSI_BOLD << match[0]->token.line << ": " << lexer[match[0]->token]  << ANSI_RESET << std::endl;
        ss << "                 " << "Invalid import file" << std::endl;

        std::cerr << ss.str();
        
        return Arcana_Result::ARCANA_RESULT__NOK;
    }

    Arcana_Result        result;
    Scan::Lexer          lexer(script);
    Grammar::Engine      engine;
    Parsing::Parser      parser(lexer, engine);

    parser.Set_ParsingError_Handler    (Support::ParserError   {lexer} );
    parser.Set_AnalisysError_Handler   (Support::SemanticError {lexer} );
    parser.Set_PostProcessError_Handler(Support::PostProcError {lexer} );
    
    result = parser.Parse(new_env);

    if (result == Arcana_Result::ARCANA_RESULT__OK)
    {
        Semantic::EnvMerge(instr_engine.EnvRef(), new_env);
    }

    return result;
}



Arcana::Support::SemanticOutput Parser::Handle_Using(Grammar::Match& match)
{
    // EXTRACT THE STRINGS FROM THE INPUT AND PASS THEM INTO THE SEMANTIC ENGINE
    Point  p1     = match[_I(Grammar::USING::WHAT)];
    Point  p2     = match[_I(Grammar::USING::OPT)];
    
    Input  input  = lexer[p1->token];

    Lexeme what   = input.substr(p1->start, p1->end - p1->start);
    Lexeme opt    = input.substr(p2->start, p2->end - p2->start);

    return instr_engine.Collect_Using(what, opt);
}



Arcana::Support::SemanticOutput Parser::Handle_Mapping(Grammar::Match& match)
{
    // EXTRACT THE STRINGS FROM THE INPUT AND PASS THEM INTO THE SEMANTIC ENGINE
    Point  p1     = match[_I(Grammar::MAPPING::ITEM_1)];
    Point  p2     = match[_I(Grammar::MAPPING::ITEM_2)];
    
    Input  input  = lexer[p1->token];

    Lexeme item1 = input.substr(p1->start, p1->end - p1->start);
    Lexeme item2 = input.substr(p2->start, p2->end - p2->start);

    return instr_engine.Collect_Mapping(item1, item2);
}



Arcana::Support::SemanticOutput Parser::Handle_Assert(Grammar::Match& match)
{
    // EXTRACT THE STRINGS FROM THE INPUT AND PASS THEM INTO THE SEMANTIC ENGINE
    Point  pStart = match[_I(Grammar::ASSERT::RESERVED2)];
    Point  p1     = match[_I(Grammar::ASSERT::ITEM_1)   ];
    Point  p2     = match[_I(Grammar::ASSERT::OP)       ];
    Point  p3     = match[_I(Grammar::ASSERT::ITEM_2)   ];
    Point  p4     = match[_I(Grammar::ASSERT::REASON)   ];
    Point  pStop  = match[_I(Grammar::ASSERT::RESERVED5)];

    
    Input  input  = lexer[p1->token];

    Lexeme stmt   = input.substr(pStart->start, pStop->end - pStart->start);
    Lexeme lvalue = input.substr(p1->start, p1->end - p1->start);
    Lexeme op     = input.substr(p2->start, p2->end - p2->start);
    Lexeme rvalue = input.substr(p3->start, p3->end - p3->start);
    Lexeme reason = input.substr(p4->start, p4->end - p4->start);

    return instr_engine.Collect_Assert(p1->token.line, stmt, lvalue, op, rvalue, reason);
}
