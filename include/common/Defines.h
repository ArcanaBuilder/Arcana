#ifndef __ARCANA_COMMON_DEFINES__H__
#define __ARCANA_COMMON_DEFINES__H__



//    ███    ██  █████  ███    ███ ███████ ███████ ██████   █████   ██████ ███████     ██    ██ ████████ ██ ██      ███████ 
//    ████   ██ ██   ██ ████  ████ ██      ██      ██   ██ ██   ██ ██      ██          ██    ██    ██    ██ ██      ██      
//    ██ ██  ██ ███████ ██ ████ ██ █████   ███████ ██████  ███████ ██      █████       ██    ██    ██    ██ ██      ███████ 
//    ██  ██ ██ ██   ██ ██  ██  ██ ██           ██ ██      ██   ██ ██      ██          ██    ██    ██    ██ ██           ██ 
//    ██   ████ ██   ██ ██      ██ ███████ ███████ ██      ██   ██  ██████ ███████      ██████     ██    ██ ███████ ███████ 
//                                                                                                                          
//                                                                                                                          

#define BEGIN_NAMESPACE(ns)                     namespace ns {                 
#define END_NAMESPACE(ns)                       }      


#define MAIN_MODULE                             Arcana
#define BEGIN_MODULE(mod)                       BEGIN_NAMESPACE(MAIN_MODULE :: mod)
#define END_MODULE(mod)                         END_NAMESPACE(mod)
#define USE_MODULE(mod)                         using namespace mod



//    ██    ██ ████████ ██ ██      ██ ████████ ██    ██     ███    ███  █████   ██████ ██████   ██████  ███████ 
//    ██    ██    ██    ██ ██      ██    ██     ██  ██      ████  ████ ██   ██ ██      ██   ██ ██    ██ ██      
//    ██    ██    ██    ██ ██      ██    ██      ████       ██ ████ ██ ███████ ██      ██████  ██    ██ ███████ 
//    ██    ██    ██    ██ ██      ██    ██       ██        ██  ██  ██ ██   ██ ██      ██   ██ ██    ██      ██ 
//     ██████     ██    ██ ███████ ██    ██       ██        ██      ██ ██   ██  ██████ ██   ██  ██████  ███████ 
//                                                                                                              
//                                                                                                              

#define UNUSED(x)                               ((void) x)
#define _I(enum)                                ((size_t) enum) 





//     █████  ███    ██ ███████ ██     ███████ ███████  ██████  █████  ██████  ███████ ███████ 
//    ██   ██ ████   ██ ██      ██     ██      ██      ██      ██   ██ ██   ██ ██      ██      
//    ███████ ██ ██  ██ ███████ ██     █████   ███████ ██      ███████ ██████  █████   ███████ 
//    ██   ██ ██  ██ ██      ██ ██     ██           ██ ██      ██   ██ ██      ██           ██ 
//    ██   ██ ██   ████ ███████ ██     ███████ ███████  ██████ ██   ██ ██      ███████ ███████ 
//                                                                                             
//                                                                                             

#define ANSI_RESET                              "\x1b[0m"

// ==== Foreground (testo) ====
#define ANSI_BLACK                              "\x1b[30m"
#define ANSI_RED                                "\x1b[31m"
#define ANSI_GREEN                              "\x1b[32m"
#define ANSI_YELLOW                             "\x1b[33m"
#define ANSI_BLUE                               "\x1b[34m"
#define ANSI_MAGENTA                            "\x1b[35m"
#define ANSI_CYAN                               "\x1b[36m"
#define ANSI_WHITE                              "\x1b[37m"
#define ANSI_GRAY                               "\x1b[37;2m"

// ==== Foreground bright =====
#define ANSI_BBLACK                             "\x1b[90m"
#define ANSI_BRED                               "\x1b[91m"
#define ANSI_BGREEN                             "\x1b[92m"
#define ANSI_BYELLOW                            "\x1b[93m"
#define ANSI_BBLUE                              "\x1b[94m"
#define ANSI_BMAGENTA                           "\x1b[95m"
#define ANSI_BCYAN                              "\x1b[96m"
#define ANSI_BWHITE                             "\x1b[97m"

// ==== Background ====
#define ANSI_BG_BLACK                           "\x1b[40m"
#define ANSI_BG_RED                             "\x1b[41m"
#define ANSI_BG_GREEN                           "\x1b[42m"
#define ANSI_BG_YELLOW                          "\x1b[43m"
#define ANSI_BG_BLUE                            "\x1b[44m"
#define ANSI_BG_MAGENTA                         "\x1b[45m"
#define ANSI_BG_CYAN                            "\x1b[46m"
#define ANSI_BG_WHITE                           "\x1b[47m"

// ==== Background bright ====
#define ANSI_BG_BBLACK                          "\x1b[100m"
#define ANSI_BG_BRED                            "\x1b[101m"
#define ANSI_BG_BGREEN                          "\x1b[102m"
#define ANSI_BG_BYELLOW                         "\x1b[103m"
#define ANSI_BG_BBLUE                           "\x1b[104m"
#define ANSI_BG_BMAGENTA                        "\x1b[105m"
#define ANSI_BG_BCYAN                           "\x1b[106m"
#define ANSI_BG_BWHITE                          "\x1b[107m"

// ==== Styles ====
#define ANSI_BOLD                               "\x1b[1m"
#define ANSI_DIM                                "\x1b[2m"
#define ANSI_UNDERLINE                          "\x1b[4m"
#define ANSI_BLINK                              "\x1b[5m"
#define ANSI_REVERSE                            "\x1b[7m"
#define ANSI_HIDDEN                             "\x1b[8m"





//    ██████  ██████  ██ ███    ██ ████████ ███████ ██████      ███    ███  █████   ██████ ██████   ██████  ███████ 
//    ██   ██ ██   ██ ██ ████   ██    ██    ██      ██   ██     ████  ████ ██   ██ ██      ██   ██ ██    ██ ██      
//    ██████  ██████  ██ ██ ██  ██    ██    █████   ██████      ██ ████ ██ ███████ ██      ██████  ██    ██ ███████ 
//    ██      ██   ██ ██ ██  ██ ██    ██    ██      ██   ██     ██  ██  ██ ██   ██ ██      ██   ██ ██    ██      ██ 
//    ██      ██   ██ ██ ██   ████    ██    ███████ ██   ██     ██      ██ ██   ██  ██████ ██   ██  ██████  ███████ 
//                                                                                                                  
//                                                                                                                  

#include <iostream>

#ifdef DEBUG
  #define DBG(msg)                              std::cout << "[" << ANSI_BYELLOW << ANSI_DIM << "DEBUG" << ANSI_RESET "] " << ANSI_GRAY << msg << ANSI_RESET << std::endl
#else
  #define DBG(msg)
#endif

#define MSG(msg)                                std::cout << msg << std::endl
#define ERR(msg)                                std::cerr << "[" << ANSI_BRED << "ERROR" << ANSI_RESET << "] " << msg << std::endl
#define HINT(msg)                               std::cout << "[" << ANSI_BGREEN << "HINT" << ANSI_RESET << "]  " << msg << std::endl




//    ██████  ██    ██ ██████  ██      ██  ██████     ███████ ████████  █████  ████████ ██    ██ ███████     
//    ██   ██ ██    ██ ██   ██ ██      ██ ██          ██         ██    ██   ██    ██    ██    ██ ██          
//    ██████  ██    ██ ██████  ██      ██ ██          ███████    ██    ███████    ██    ██    ██ ███████     
//    ██      ██    ██ ██   ██ ██      ██ ██               ██    ██    ██   ██    ██    ██    ██      ██     
//    ██       ██████  ██████  ███████ ██  ██████     ███████    ██    ██   ██    ██     ██████  ███████     
//                                                                                                           
//                                                                                                           
//    ████████ ██    ██ ██████  ███████ ███████                                                              
//       ██     ██  ██  ██   ██ ██      ██                                                                   
//       ██      ████   ██████  █████   ███████                                                              
//       ██       ██    ██      ██           ██                                                              
//       ██       ██    ██      ███████ ███████                                                              
//                                                                                                           
//                                                                                                           


#include <stdint.h>


enum Arcana_Result : int32_t
{
    ARCANA_RESULT__OK               =  0,
    ARCANA_RESULT__INVALID_ARGS     = -1,
    ARCANA_RESULT__PARSING_ERROR    = -2,
};

enum class Semantic_Result : int32_t
{
    AST_RESULT__OK                   =  0,
     
    AST_RESULT__INVALID_ATTR         = -1,
    AST_RESULT__INVALID_ATTR_PROP    = -2,
    AST_RESULT__INVALID_ATTR_TARGET  = -3,
    AST_RESULT__ATTR_NOT_ALLOWED     = -4,
    AST_RESULT__INVALID_BUILTIN_TASK = -5,
    AST_RESULT__INVALID_ARGUMENTS    = -6,
    AST_RESULT__INVALID_TASK_CALL    = -7,
};



#endif /* __ARCANA_COMMON_DEFINES__H__ */