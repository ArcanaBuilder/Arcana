#ifndef __ARCANA_GLOB_H__
#define __ARCANA_GLOB_H__


#include "Defines.h"

#include <string>
#include <vector>
#include <cstdint>
#include <filesystem>
#include <string_view>



BEGIN_MODULE(Glob)



namespace fs = std::filesystem;




//    ███████╗████████╗██████╗ ██╗   ██╗ ██████╗████████╗███████╗
//    ██╔════╝╚══██╔══╝██╔══██╗██║   ██║██╔════╝╚══██╔══╝██╔════╝
//    ███████╗   ██║   ██████╔╝██║   ██║██║        ██║   ███████╗
//    ╚════██║   ██║   ██╔══██╗██║   ██║██║        ██║   ╚════██║
//    ███████║   ██║   ██║  ██║╚██████╔╝╚██████╗   ██║   ███████║
//    ╚══════╝   ╚═╝   ╚═╝  ╚═╝ ╚═════╝  ╚═════╝   ╚═╝   ╚══════╝
//                                                               


struct ParseError
{
    enum class Code : std::uint8_t
    {
        NONE,
        EMPTY_PATTERN,
        INVALID_ESCAPE,
        UNCLOSED_CHARCLASS,
        EMPTY_CHARCLASS,
        INVALID_RANGE,
        INVALID_DOUBLESTAR
    };

    Code        code   = Code::NONE;
    std::size_t offset = 0;
};



struct CharRange
{
    char first = '\0';
    char last  = '\0';
};



struct CharClass
{
    bool                   negated = false;  // [^...]
    std::vector<char>      singles;          // explicit chars
    std::vector<CharRange> ranges;           // a-z, 0-9
};



struct Atom
{
    enum class Kind : std::uint8_t
    {
        LITERAL,
        STAR,          // *
        QMARK,         // ?
        CHARCLASS,     // [...]
        DOUBLESTAR     // ** (segment-only)
    };

    Kind kind = Kind::LITERAL;

    // payload
    std::string literal;
    CharClass   cls;

    static Atom MakeLiteral(std::string s)
    {
        Atom a{};
        a.kind    = Kind::LITERAL;
        a.literal = std::move(s);
        return a;
    }


    static Atom MakeStar()
    {
        Atom a{};
        a.kind = Kind::STAR;
        return a;
    }


    static Atom MakeQMark()
    {
        Atom a{};
        a.kind = Kind::QMARK;
        return a;
    }


    static Atom MakeDoubleStar()
    {
        Atom a{};
        a.kind = Kind::DOUBLESTAR;
        return a;
    }


    static Atom MakeCharClass(CharClass c)
    {
        Atom a{};
        a.kind = Kind::CHARCLASS;
        a.cls  = std::move(c);
        return a;
    }
};



struct Segment
{
    std::vector<Atom> atoms;

    bool IsDoubleStarOnly() const
    {
        if (atoms.size() != 1)
        {
            return false;
        }
        return atoms[0].kind == Atom::Kind::DOUBLESTAR;
    }
};



struct Pattern
{
    bool                  absolute = false;   // pattern inizia con '/'
    std::vector<Segment>  segments;
    std::string           normalized;         // debug/trace: pattern normalizzato
};



struct Options
{
    char separator               = '/';
    bool backslash_escape        = true;
    bool doublestar_segment_only = true;
};




struct ExpandOptions
{
    bool follow_symlinks    = false;
    bool include_dotfiles   = false;
};





struct Capture
{
    enum class Kind : std::uint8_t
    {
        PATH,    // from **
        SEGMENT, // from *
        CHAR     // from ? or []
    };

    Kind        kind  = Kind::SEGMENT;
    std::string value;
};




//    ██████╗ ██╗   ██╗██████╗     ███████╗██╗   ██╗███╗   ██╗ ██████╗███████╗
//    ██╔══██╗██║   ██║██╔══██╗    ██╔════╝██║   ██║████╗  ██║██╔════╝██╔════╝
//    ██████╔╝██║   ██║██████╔╝    █████╗  ██║   ██║██╔██╗ ██║██║     ███████╗
//    ██╔═══╝ ██║   ██║██╔══██╗    ██╔══╝  ██║   ██║██║╚██╗██║██║     ╚════██║
//    ██║     ╚██████╔╝██████╔╝    ██║     ╚██████╔╝██║ ╚████║╚██████╗███████║
//    ╚═╝      ╚═════╝ ╚═════╝     ╚═╝      ╚═════╝ ╚═╝  ╚═══╝ ╚═════╝╚══════╝
//                                                                                                                                                                                     



bool Parse(std::string_view input, Pattern& out, ParseError& err, const Options& opt = Options{}) noexcept;



bool Expand(const Pattern& pattern, const fs::path& base_dir, std::vector<std::string>& out,
            const ExpandOptions& opt = ExpandOptions{}) noexcept;


bool MapGlobToGlob(std::string_view          from_glob,
                    std::string_view          to_glob,
                    const std::vector<std::string>& src_list,
                    std::vector<std::string>&       out_list,
                    ParseError&       err_from,
                    ParseError&       err_to) noexcept;


END_MODULE(Glob)




#endif /* __ARCANA_GLOB_H__ */