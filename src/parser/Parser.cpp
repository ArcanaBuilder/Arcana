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



/**
 * @brief Construct a parser over a lexer and a grammar engine.
 *
 * The parser drives:
 * - token stream consumption (lexer)
 * - grammar recognition (engine)
 * - semantic collection (instr_engine)
 *
 * @param l Lexer instance used to read tokens.
 * @param e Grammar engine used to match statements.
 */
Parser::Parser(Scan::Lexer& l, Grammar::Engine& e)
    :
    lexer(l),
    engine(e)
{}



/**
 * @brief Parse an Arcana script and build the semantic environment.
 *
 * The function:
 * - iterates tokens until EOF
 * - feeds the grammar engine incrementally
 * - dispatches complete matches to semantic handlers
 * - supports nested parsing via `import`
 *
 * @param env Output semantic environment (copy of the internal engine env).
 * @return ARCANA_RESULT__OK on success, otherwise a failure code.
 */
Arcana_Result Parser::Parse(Semantic::Enviroment& env)
{
    Scan::Token             token;
    Grammar::Match          match;
    Support::SemanticOutput astout;

    // ITERATE TOKENS UNTIL ERROR OR EOF
    do
    {
        // GET NEXT TOKEN
        token = lexer.next();

        // FEED GRAMMAR ENGINE
        engine.match(token, match);

        // DISPATCH ON COMPLETE STATEMENT MATCH
        if (match.isValid())
        {
            // HANDLE MATCHED RULE
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

            // HANDLE IMPORT BY SPAWNING A NEW PARSER INSTANCE
            if (match.type == Grammar::Rule::IMPORT)
            {
                // PARSE IMPORTED FILE INTO A TEMP ENV
                Semantic::Enviroment new_env;
                Arcana_Result        res = Handle_Import(match, new_env);

                // PROPAGATE ERROR
                if (res != ARCANA_RESULT__OK)
                {
                    return res;
                }
            }

            // REPORT SEMANTIC ERRORS VIA CALLBACK
            if (astout.result != Semantic_Result::AST_RESULT__OK)
            {
                return Analisys_Error(lexer.source(), astout, match);
            }
        }

        // REPORT GRAMMAR ERRORS VIA CALLBACK
        if (match.isError())
        {
            return Parsing_Error(lexer.source(), match);
        }
    }
    while (token.type != Scan::TokenType::ENDOFFILE);

    // EXPORT THE COLLECTED ENVIRONMENT
    env = instr_engine.GetEnvironment();

    return ARCANA_RESULT__OK;
}



/**
 * @brief Handle a variable assignment match and forward it to the semantic engine.
 *
 * Extracts:
 * - variable name token span
 * - value token span (ANY region)
 *
 * @param match Grammar match for VARIABLE_ASSIGN.
 * @return SemanticOutput from `Collect_Assignment`.
 */
Arcana::Support::SemanticOutput Parser::Handle_VarAssign(Grammar::Match& match)
{
    // EXTRACT MATCH POINTS
    Point p1 = match[_I(Grammar::VARIABLE_ASSIGN::VARNAME)];
    Point p2 = match[_I(Grammar::VARIABLE_ASSIGN::VALUE)];

    // SLICE RAW INPUT LINE
    Input  input = lexer[p1->token];
    Lexeme var   = input.substr(p1->start, p1->end - p1->start);
    Lexeme value = input.substr(p2->start, p2->end - p2->start);

    // COLLECT INTO SEMANTIC ENGINE
    return instr_engine.Collect_Assignment(var, value);
}



/**
 * @brief Handle an attribute match and forward it to the semantic engine.
 *
 * Extracts:
 * - attribute name
 * - raw attribute options payload (ANY region)
 *
 * @param match Grammar match for ATTRIBUTE.
 * @return SemanticOutput from `Collect_Attribute`.
 */
Arcana::Support::SemanticOutput Parser::Handle_Attribute(Grammar::Match& match)
{
    // EXTRACT MATCH POINTS
    Point p1 = match[_I(Grammar::ATTRIBUTE::ATTRNAME)];
    Point p2 = match[_I(Grammar::ATTRIBUTE::ATTROPTION)];

    // SLICE RAW INPUT LINE
    Input  input   = lexer[p1->token];
    Lexeme attr    = input.substr(p1->start, p1->end - p1->start);
    Lexeme attropt = input.substr(p2->start, p2->end - p2->start);

    // COLLECT INTO SEMANTIC ENGINE
    return instr_engine.Collect_Attribute(attr, attropt);
}



/**
 * @brief Handle a task declaration match and forward it to the semantic engine.
 *
 * The handler extracts:
 * - task name
 * - inputs list (raw)
 * - task body lines as a list of instruction strings
 *
 * Body extraction supports:
 * - `{ ... }` on the same line
 * - multiline bodies (first line after `{`, middle lines, last line before `}`)
 *
 * @param match Grammar match for TASK_DECL.
 * @return SemanticOutput from `Collect_Task`.
 */
Arcana::Support::SemanticOutput Parser::Handle_TaskDecl(Grammar::Match& match)
{
    // PREPARE BODY CONTAINER
    Statement body;

    // EXTRACT MATCH POINTS
    Point p1    = match[_I(Grammar::TASK_DECL::TASKNAME)];
    Point p2    = match[_I(Grammar::TASK_DECL::INPUTS)];
    Point bbody = match[_I(Grammar::TASK_DECL::CURLYLP)];
    Point ebody = match[_I(Grammar::TASK_DECL::CURLYRP)];

    // SLICE HEADER LINE
    Input  header_line = lexer[p1->token];
    Lexeme task        = header_line.substr(p1->start, p1->end - p1->start);
    Lexeme inputs      = header_line.substr(p2->start, p2->end - p2->start);

    // COMPUTE BODY LINE RANGE
    const std::size_t line_begin = bbody->token.line;
    const std::size_t line_end   = ebody->token.line;

    std::string instr;

    // HANDLE SINGLE-LINE BODY
    if (line_begin == line_end)
    {
        // SLICE SAME LINE BETWEEN '{' AND '}'
        Input line = lexer[bbody->token];

        std::size_t start = bbody->end;
        std::size_t end   = ebody->start;

        if (end > start)
        {
            instr = line.substr(start, end - start);

            // SKIP EMPTY/WHITESPACE-ONLY INSTRUCTIONS
            if (!Support::ltrim(instr).empty())
            {
                body.push_back(instr);
            }
        }
    }
    else
    {
        // FIRST BODY LINE: AFTER '{' TO END OF LINE
        {
            Input line = lexer[bbody->token];

            std::size_t start = bbody->end;
            if (start < line.size())
            {
                instr = line.substr(start);

                // SKIP EMPTY/WHITESPACE-ONLY INSTRUCTIONS
                if (!instr.empty() && !Support::ltrim(instr).empty())
                {
                    body.push_back(instr);
                }
            }
        }

        // MIDDLE BODY LINES: FULL LINES BETWEEN BEGIN AND END
        for (std::size_t line = line_begin + 1; line < line_end; ++line)
        {
            // MAP TOKEN LINE (1-BASED) TO STORED LINE INDEXING
            instr = lexer[line - 1];

            if (!instr.empty())
            {
                body.push_back(instr);
            }
        }

        // LAST BODY LINE: FROM LINE START TO BEFORE '}'
        {
            Input line = lexer[ebody->token];

            std::size_t end = ebody->start;
            if (end > 0)
            {
                instr = line.substr(0, end);

                // SKIP EMPTY/WHITESPACE-ONLY INSTRUCTIONS
                if (!instr.empty() && !Support::ltrim(instr).empty())
                {
                    body.push_back(instr);
                }
            }
        }
    }

    // COLLECT INTO SEMANTIC ENGINE
    return instr_engine.Collect_Task(task, inputs, body);
}



/**
 * @brief Handle an import match by recursively parsing the referenced script.
 *
 * The imported script is parsed into a temporary environment which is then merged
 * into this parser's semantic engine environment on success.
 *
 * @param match Grammar match for IMPORT.
 * @param new_env Output environment collected from the imported script.
 * @return ARCANA_RESULT__OK on success, otherwise a failure code.
 */
Arcana_Result Parser::Handle_Import(Grammar::Match& match, Semantic::Enviroment& new_env)
{
    // EXTRACT IMPORT PATH
    Point  p1     = match[_I(Grammar::IMPORT::SCRIPT)];

    Input  input  = lexer[p1->token];
    Lexeme script = input.substr(p1->start, p1->end - p1->start);

    // VALIDATE IMPORT PATH
    if (script.empty() || !Support::file_exists(script))
    {
        // BUILD ERROR MESSAGE (SAME STYLE AS OTHER DIAGNOSTICS)
        std::stringstream ss;

        ss << "[" << ANSI_BRED << "SEMANTIC ERROR" << ANSI_RESET << "] In file "
           << ANSI_BOLD << this->lexer.source() << ANSI_RESET
           << ", line " << ANSI_BOLD << match[0]->token.line << ": "
           << lexer[match[0]->token] << ANSI_RESET << std::endl;

        ss << "                 " << "Invalid import file" << std::endl;

        std::cerr << ss.str();

        return Arcana_Result::ARCANA_RESULT__NOK;
    }

    // SPAWN IMPORT PARSER
    Arcana_Result        result;
    Scan::Lexer          lexer(script);
    Grammar::Engine      engine;
    Parsing::Parser      parser(lexer, engine);

    // INSTALL ERROR HANDLERS FOR IMPORT CONTEXT
    parser.Set_ParsingError_Handler    (Support::ParserError   {lexer});
    parser.Set_AnalisysError_Handler   (Support::SemanticError {lexer});
    parser.Set_PostProcessError_Handler(Support::PostProcError {lexer});

    // PARSE IMPORT AND MERGE INTO CURRENT ENV ON SUCCESS
    result = parser.Parse(new_env);

    if (result == Arcana_Result::ARCANA_RESULT__OK)
    {
        Semantic::EnvMerge(instr_engine.EnvRef(), new_env);
    }

    return result;
}



/**
 * @brief Handle a using directive match and forward it to the semantic engine.
 *
 * Extracts:
 * - using keyword (WHAT)
 * - argument string (OPT)
 *
 * @param match Grammar match for USING.
 * @return SemanticOutput from `Collect_Using`.
 */
Arcana::Support::SemanticOutput Parser::Handle_Using(Grammar::Match& match)
{
    // EXTRACT MATCH POINTS
    Point p1 = match[_I(Grammar::USING::WHAT)];
    Point p2 = match[_I(Grammar::USING::OPT)];

    // SLICE RAW INPUT LINE
    Input  input = lexer[p1->token];
    Lexeme what  = input.substr(p1->start, p1->end - p1->start);
    Lexeme opt   = input.substr(p2->start, p2->end - p2->start);

    // COLLECT INTO SEMANTIC ENGINE
    return instr_engine.Collect_Using(what, opt);
}



/**
 * @brief Handle a mapping directive match and forward it to the semantic engine.
 *
 * Extracts:
 * - ITEM_1 (source symbol)
 * - ITEM_2 (destination symbol)
 *
 * @param match Grammar match for MAPPING.
 * @return SemanticOutput from `Collect_Mapping`.
 */
Arcana::Support::SemanticOutput Parser::Handle_Mapping(Grammar::Match& match)
{
    // EXTRACT MATCH POINTS
    Point p1 = match[_I(Grammar::MAPPING::ITEM_1)];
    Point p2 = match[_I(Grammar::MAPPING::ITEM_2)];

    // SLICE RAW INPUT LINE
    Input  input = lexer[p1->token];
    Lexeme item1 = input.substr(p1->start, p1->end - p1->start);
    Lexeme item2 = input.substr(p2->start, p2->end - p2->start);

    // COLLECT INTO SEMANTIC ENGINE
    return instr_engine.Collect_Mapping(item1, item2);
}



/**
 * @brief Handle an assert match and forward it to the semantic engine.
 *
 * Extracts:
 * - full statement substring (for diagnostics)
 * - lvalue, operator, rvalue, reason
 *
 * @param match Grammar match for ASSERT.
 * @return SemanticOutput from `Collect_Assert`.
 */
Arcana::Support::SemanticOutput Parser::Handle_Assert(Grammar::Match& match)
{
    // EXTRACT MATCH POINTS
    Point pStart = match[_I(Grammar::ASSERT::RESERVED2)];
    Point p1     = match[_I(Grammar::ASSERT::ITEM_1)];
    Point p2     = match[_I(Grammar::ASSERT::OP)];
    Point p3     = match[_I(Grammar::ASSERT::ITEM_2)];
    Point p4     = match[_I(Grammar::ASSERT::REASON)];
    Point pStop  = match[_I(Grammar::ASSERT::RESERVED5)];

    // SLICE RAW INPUT LINE
    Input  input  = lexer[p1->token];

    // EXTRACT SUBSTRINGS
    Lexeme stmt   = input.substr(pStart->start, pStop->end - pStart->start);
    Lexeme lvalue = input.substr(p1->start, p1->end - p1->start);
    Lexeme op     = input.substr(p2->start, p2->end - p2->start);
    Lexeme rvalue = input.substr(p3->start, p3->end - p3->start);
    Lexeme reason = input.substr(p4->start, p4->end - p4->start);

    // COLLECT INTO SEMANTIC ENGINE
    return instr_engine.Collect_Assert(p1->token.line, stmt, lvalue, op, rvalue, reason);
}
