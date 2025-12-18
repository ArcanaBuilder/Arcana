#include "Glob.h"

#include <algorithm>

USE_MODULE(Arcana::Glob);


                                                                                                     
//    ██████╗ ██████╗ ██╗██╗   ██╗ █████╗ ████████╗███████╗    ███████╗██╗   ██╗███╗   ██╗ ██████╗███████╗
//    ██╔══██╗██╔══██╗██║██║   ██║██╔══██╗╚══██╔══╝██╔════╝    ██╔════╝██║   ██║████╗  ██║██╔════╝██╔════╝
//    ██████╔╝██████╔╝██║██║   ██║███████║   ██║   █████╗      █████╗  ██║   ██║██╔██╗ ██║██║     ███████╗
//    ██╔═══╝ ██╔══██╗██║╚██╗ ██╔╝██╔══██║   ██║   ██╔══╝      ██╔══╝  ██║   ██║██║╚██╗██║██║     ╚════██║
//    ██║     ██║  ██║██║ ╚████╔╝ ██║  ██║   ██║   ███████╗    ██║     ╚██████╔╝██║ ╚████║╚██████╗███████║
//    ╚═╝     ╚═╝  ╚═╝╚═╝  ╚═══╝  ╚═╝  ╚═╝   ╚═╝   ╚══════╝    ╚═╝      ╚═════╝ ╚═╝  ╚═══╝ ╚═════╝╚══════╝
//                                                                                                                                                                                                              



//    ██████╗  █████╗ ██████╗ ███████╗███████╗██████╗                                                     
//    ██╔══██╗██╔══██╗██╔══██╗██╔════╝██╔════╝██╔══██╗                                                    
//    ██████╔╝███████║██████╔╝███████╗█████╗  ██████╔╝                                                    
//    ██╔═══╝ ██╔══██║██╔══██╗╚════██║██╔══╝  ██╔══██╗                                                    
//    ██║     ██║  ██║██║  ██║███████║███████╗██║  ██║                                                    
//    ╚═╝     ╚═╝  ╚═╝╚═╝  ╚═╝╚══════╝╚══════╝╚═╝  ╚═╝                                                    
//   



static bool IsMeta(char c) noexcept
{
    switch (c)
    {
        case '*': 
        case '?': 
        case '[': 
        case ']': 
        case '\\': 
            return true;

        default:
            return false;
    }
}



static std::string Normalize(std::string_view input, const Options& opt) noexcept
{
    std::string s;
    s.reserve(input.size());

    for (char c : input)
    {
        if (c == '\\' && opt.separator == '/')
        {
            s.push_back('/');
        }
        else
        {
            s.push_back(c);
        }
    }

    return s;
}



static bool EmitLiteralIfAny(std::string& lit, Segment& out) noexcept
{
    if (lit.empty())
    {
        return true;
    }

    out.atoms.push_back(Atom::MakeLiteral(std::move(lit)));
    lit.clear();
    return true;
}



static bool ParseCharClass(std::string_view seg, std::size_t& i, CharClass& out, std::size_t base_offset, ParseError& err, const Options& opt) noexcept
{
    out = CharClass{};

    std::size_t start = i;

    i += 1;
    if (i >= seg.size())
    {
        err.code   = ParseError::Code::UNCLOSED_CHARCLASS;
        err.offset = base_offset + start;
        return false;
    }

    if (seg[i] == '^')
    {
        out.negated = true;
        i += 1;
    }

    if (i >= seg.size())
    {
        err.code   = ParseError::Code::UNCLOSED_CHARCLASS;
        err.offset = base_offset + start;
        return false;
    }

    bool any = false;

    auto read_char = [&] (char& ch) noexcept -> bool
    {
        if (i >= seg.size())
        {
            return false;
        }

        char c = seg[i];

        if (opt.backslash_escape && c == '\\')
        {
            if (i + 1 >= seg.size())
            {
                return false;
            }
            char n = seg[i + 1];
            ch = n;
            i += 2;
            return true;
        }

        ch = c;
        i += 1;
        return true;
    };

    // parsing fino a ']'
    while (i < seg.size())
    {
        if (seg[i] == ']')
        {
            if (!any)
            {
                err.code   = ParseError::Code::EMPTY_CHARCLASS;
                err.offset = base_offset + start;
                return false;
            }

            // posiziona i sul ']'
            return true;
        }

        char first = '\0';
        if (!read_char(first))
        {
            err.code   = ParseError::Code::UNCLOSED_CHARCLASS;
            err.offset = base_offset + start;
            return false;
        }

        if (i < seg.size() && seg[i] == '-' && (i + 1) < seg.size() && seg[i + 1] != ']')
        {
            // range: first-last
            i += 1; // consuma '-'

            char last = '\0';
            if (!read_char(last))
            {
                err.code   = ParseError::Code::INVALID_RANGE;
                err.offset = base_offset + i;
                return false;
            }

            if (static_cast<unsigned char>(first) > static_cast<unsigned char>(last))
            {
                err.code   = ParseError::Code::INVALID_RANGE;
                err.offset = base_offset + (i - 1);
                return false;
            }

            out.ranges.push_back(CharRange{first, last});
            any = true;
            continue;
        }

        out.singles.push_back(first);
        any = true;
    }

    err.code   = ParseError::Code::UNCLOSED_CHARCLASS;
    err.offset = base_offset + start;

    return false;
}



static bool ParseSegment(std::string_view seg, Segment& out, std::size_t base_offset, ParseError& err, const Options& opt) noexcept
{
    out = Segment{};

    if (seg.empty())
    {
        return true;
    }

    if (opt.doublestar_segment_only && seg == "**")
    {
        out.atoms.push_back(Atom::MakeDoubleStar());
        return true;
    }

    std::string literal;
    literal.reserve(seg.size());

    for (std::size_t i = 0; i < seg.size(); ++i)
    {
        char c = seg[i];

        if (opt.backslash_escape && c == '\\')
        {
            if (i + 1 >= seg.size())
            {
                err.code   = ParseError::Code::INVALID_ESCAPE;
                err.offset = base_offset + i;
                return false;
            }

            char n = seg[i + 1];

            if (!IsMeta(n))
            {
                err.code   = ParseError::Code::INVALID_ESCAPE;
                err.offset = base_offset + i;
                return false;
            }

            literal.push_back(n);
            i += 1;
            continue;
        }

        if (c == '*')
        {
            if (!EmitLiteralIfAny(literal, out))
            {
                return false;
            }

            out.atoms.push_back(Atom::MakeStar());
            continue;
        }

        if (c == '?')
        {
            if (!EmitLiteralIfAny(literal, out))
            {
                return false;
            }

            out.atoms.push_back(Atom::MakeQMark());
            continue;
        }

        if (c == '[')
        {
            if (!EmitLiteralIfAny(literal, out))
            {
                return false;
            }

            CharClass cls{};
            if (!ParseCharClass(seg, i, cls, base_offset, err, opt))
            {
                return false;
            }

            out.atoms.push_back(Atom::MakeCharClass(std::move(cls)));
            continue;
        }

        literal.push_back(c);
    }

    if (!EmitLiteralIfAny(literal, out))
    {
        return false;
    }

    if (opt.doublestar_segment_only)
    {
        for (const auto& a : out.atoms)
        {
            if (a.kind == Atom::Kind::STAR)
            {
                // niente qui: STAR è ok
            }
            if (a.kind == Atom::Kind::LITERAL && a.literal.find("**") != std::string::npos)
            {
                err.code   = ParseError::Code::INVALID_DOUBLESTAR;
                err.offset = base_offset;
                return false;
            }
        }
    }

    return true;
}



static bool SplitSegments(const std::string& norm, Pattern& out, ParseError& err, const Options& opt) noexcept
{
    std::size_t i = 0;

    if (!norm.empty() && norm[0] == opt.separator)
    {
        out.absolute = true;
        i = 1;
    }

    Segment cur{};
    std::size_t seg_start = i;

    auto flush_segment = [&] () noexcept -> bool
    {
        std::string_view seg(&norm[seg_start], i - seg_start);

        Segment parsed{};
        if (!ParseSegment(seg, parsed, seg_start, err, opt))
        {
            return false;
        }

        out.segments.push_back(std::move(parsed));
        return true;
    };

    for (; i <= norm.size(); ++i)
    {
        if (i == norm.size() || norm[i] == opt.separator)
        {
            if (!flush_segment())
            {
                return false;
            }

            seg_start = i + 1;
        }
    }

    return true;
}




//    ███████╗██╗  ██╗██████╗  █████╗ ███╗   ██╗██████╗ ███████╗██████╗                                   
//    ██╔════╝╚██╗██╔╝██╔══██╗██╔══██╗████╗  ██║██╔══██╗██╔════╝██╔══██╗                                  
//    █████╗   ╚███╔╝ ██████╔╝███████║██╔██╗ ██║██║  ██║█████╗  ██████╔╝                                  
//    ██╔══╝   ██╔██╗ ██╔═══╝ ██╔══██║██║╚██╗██║██║  ██║██╔══╝  ██╔══██╗                                  
//    ███████╗██╔╝ ██╗██║     ██║  ██║██║ ╚████║██████╔╝███████╗██║  ██║                                  
//    ╚══════╝╚═╝  ╚═╝╚═╝     ╚═╝  ╚═╝╚═╝  ╚═══╝╚═════╝ ╚══════╝╚═╝  ╚═╝                                  
//    


static bool StartsWithDot(const std::string& name) noexcept
{
    return !name.empty() && name[0] == '.';
}



static bool SegmentAllowsDotfiles(const Segment& seg) noexcept
{
    // Se il primo atom è literal che inizia con '.' => permette dotfiles
    // Se il primo atom è CHARCLASS e include '.' come primo match... non lo sappiamo qui.
    // Regola semplice e stabile: "pattern-segment inizia con '.'" => dotfiles ok.
    if (seg.atoms.empty())
    {
        return true;
    }

    const Atom& a0 = seg.atoms[0];
    if (a0.kind == Atom::Kind::LITERAL)
    {
        if (!a0.literal.empty() && a0.literal[0] == '.')
        {
            return true;
        }
    }

    return false;
}



static bool CharClassMatch(const CharClass& cc, char ch) noexcept
{
    bool hit = false;

    for (char c : cc.singles)
    {
        if (c == ch)
        {
            hit = true;
            break;
        }
    }

    if (!hit)
    {
        for (const auto& r : cc.ranges)
        {
            unsigned char a = static_cast<unsigned char>(r.first);
            unsigned char b = static_cast<unsigned char>(r.last);
            unsigned char x = static_cast<unsigned char>(ch);

            if (x >= a && x <= b)
            {
                hit = true;
                break;
            }
        }
    }

    if (cc.negated)
    {
        return !hit;
    }

    return hit;
}



static bool MatchSegmentAtoms(const Segment& seg, const std::string& name) noexcept
{
    // DP su posizione atom e posizione char
    // dp[i][j] = match fino atom i e char j
    const std::size_t A = seg.atoms.size();
    const std::size_t N = name.size();

    std::vector<std::vector<bool>> dp(A + 1, std::vector<bool>(N + 1, false));
    dp[0][0] = true;

    for (std::size_t i = 0; i < A; ++i)
    {
        const Atom& a = seg.atoms[i];

        if (a.kind == Atom::Kind::LITERAL)
        {
            const std::string& lit = a.literal;
            for (std::size_t j = 0; j <= N; ++j)
            {
                if (!dp[i][j])
                {
                    continue;
                }

                if (j + lit.size() <= N && name.compare(j, lit.size(), lit) == 0)
                {
                    dp[i + 1][j + lit.size()] = true;
                }
            }
        }
        else if (a.kind == Atom::Kind::QMARK)
        {
            for (std::size_t j = 0; j < N; ++j)
            {
                if (dp[i][j])
                {
                    dp[i + 1][j + 1] = true;
                }
            }
        }
        else if (a.kind == Atom::Kind::STAR)
        {
            // STAR: 0..N chars
            for (std::size_t j = 0; j <= N; ++j)
            {
                if (!dp[i][j])
                {
                    continue;
                }

                // match zero
                dp[i + 1][j] = true;

                // match >=1
                for (std::size_t k = j; k < N; ++k)
                {
                    dp[i + 1][k + 1] = true;
                }
            }
        }
        else if (a.kind == Atom::Kind::CHARCLASS)
        {
            for (std::size_t j = 0; j < N; ++j)
            {
                if (!dp[i][j])
                {
                    continue;
                }

                if (CharClassMatch(a.cls, name[j]))
                {
                    dp[i + 1][j + 1] = true;
                }
            }
        }
        else
        {
            // DOUBLESTAR non deve apparire dentro un segment normale (segment-only).
            // Se ci arriva, non matcha.
            return false;
        }
    }

    return dp[A][N];
}



static void ListDir(const fs::path& dir, std::vector<fs::directory_entry>& entries) noexcept
{
    entries.clear();

    std::error_code ec;
    fs::directory_iterator it(dir, ec);
    if (ec)
    {
        return;
    }

    for (const auto& de : it)
    {
        entries.push_back(de);
    }

    // ordine deterministico: sort per filename (generic)
    std::sort(entries.begin(), entries.end(),
                [] (const fs::directory_entry& a, const fs::directory_entry& b) {
                    return a.path().filename().generic_string() < b.path().filename().generic_string();
                });
}



static bool IsDir(const fs::directory_entry& de, bool follow_symlinks) noexcept
{
    std::error_code ec;

    if (follow_symlinks)
    {
        return de.is_directory(ec);
    }

    // Non seguire symlink: se è symlink a dir, qui torna false (scelta conservativa)
    if (de.is_symlink(ec))
    {
        return false;
    }

    return de.is_directory(ec);
}


/*
static bool IsReg(const fs::directory_entry& de, bool follow_symlinks) noexcept
{
    std::error_code ec;

    if (follow_symlinks)
    {
        return de.is_regular_file(ec);
    }

    if (de.is_symlink(ec))
    {
        return false;
    }

    return de.is_regular_file(ec);
}
*/


static void ExpandRec(const Pattern& pattern, const ExpandOptions& opt, const fs::path& cur_dir, std::size_t seg_index, std::vector<std::string>& out) noexcept
{
    if (seg_index >= pattern.segments.size())
    {
        // pattern consumato: "cur_dir" è match finale (ma può essere file o dir a seconda del pattern)
        // Qui aggiungiamo sempre il path corrente.
        out.push_back(cur_dir.lexically_normal().generic_string());
        return;
    }

    const Segment& seg = pattern.segments[seg_index];

    // ** : 0..N directory
    if (seg.IsDoubleStarOnly())
    {
        // 0 directory: avanza pattern senza scendere
        ExpandRec(pattern, opt, cur_dir, seg_index + 1, out);

        // N directory: scendi in ogni subdir e riprova lo stesso seg_index
        std::vector<fs::directory_entry> entries;
        ListDir(cur_dir, entries);

        for (const auto& de : entries)
        {
            std::string name = de.path().filename().generic_string();

            if (!opt.include_dotfiles && StartsWithDot(name))
            {
                continue;
            }

            if (!IsDir(de, opt.follow_symlinks))
            {
                continue;
            }

            ExpandRec(pattern, opt, de.path(), seg_index, out);
        }

        return;
    }

    // segmento normale: matcha entries in questa dir, poi avanza
    std::vector<fs::directory_entry> entries;
    ListDir(cur_dir, entries);

    bool allow_dot = opt.include_dotfiles || SegmentAllowsDotfiles(seg);

    for (const auto& de : entries)
    {
        std::string name = de.path().filename().generic_string();

        if (!allow_dot && StartsWithDot(name))
        {
            continue;
        }

        if (!MatchSegmentAtoms(seg, name))
        {
            continue;
        }

        // Se non è l’ultimo segment, deve essere directory
        if (seg_index + 1 < pattern.segments.size())
        {
            if (!IsDir(de, opt.follow_symlinks))
            {
                continue;
            }
        }

        ExpandRec(pattern, opt, de.path(), seg_index + 1, out);
    }
}





//     ██████╗ ██╗      ██████╗ ██████╗     ██████╗      ██████╗ ██╗      ██████╗ ██████╗ 
//    ██╔════╝ ██║     ██╔═══██╗██╔══██╗    ╚════██╗    ██╔════╝ ██║     ██╔═══██╗██╔══██╗
//    ██║  ███╗██║     ██║   ██║██████╔╝     █████╔╝    ██║  ███╗██║     ██║   ██║██████╔╝
//    ██║   ██║██║     ██║   ██║██╔══██╗    ██╔═══╝     ██║   ██║██║     ██║   ██║██╔══██╗
//    ╚██████╔╝███████╗╚██████╔╝██████╔╝    ███████╗    ╚██████╔╝███████╗╚██████╔╝██████╔╝
//     ╚═════╝ ╚══════╝ ╚═════╝ ╚═════╝     ╚══════╝     ╚═════╝ ╚══════╝ ╚═════╝ ╚═════╝ 
//                                                                                        


static void SplitPathSegments(std::string_view s, std::vector<std::string_view>& segs) noexcept
{
    segs.clear();

    std::size_t start = 0;
    for (std::size_t i = 0; i <= s.size(); ++i)
    {
        if (i == s.size() || s[i] == '/')
        {
            if (i > start)
            {
                segs.push_back(s.substr(start, i - start));
            }
            start = i + 1;
        }
    }
}



static std::string JoinPathSegments(const std::vector<std::string_view>& segs,
                                    std::size_t                         from,
                                    std::size_t                         to) noexcept
{
    std::string out;

    for (std::size_t i = from; i < to; ++i)
    {
        if (i != from)
        {
            out.push_back('/');
        }
        out.append(segs[i].data(), segs[i].size());
    }

    return out;
}


// ---- Segment matcher con catture (solo dentro segmento, no '/')
struct PrevCell
{
    bool        has_prev = false;
    std::size_t pi       = 0;
    std::size_t pj       = 0;

    bool        capture  = false;
    Capture::Kind kind   = Capture::Kind::SEGMENT;
    std::size_t  cs      = 0; // capture start in name
    std::size_t  ce      = 0; // capture end in name
};



static bool MatchSegmentCapture(const Segment&           seg,
                                std::string_view         name,
                                std::vector<Capture>&    out_caps) noexcept
{
    out_caps.clear();

    const std::size_t A = seg.atoms.size();
    const std::size_t N = name.size();

    std::vector<std::vector<bool>>     dp(A + 1, std::vector<bool>(N + 1, false));
    std::vector<std::vector<PrevCell>> prev(A + 1, std::vector<PrevCell>(N + 1));

    dp[0][0] = true;
    prev[0][0].has_prev = true;
    prev[0][0].pi = 0;
    prev[0][0].pj = 0;

    for (std::size_t i = 0; i < A; ++i)
    {
        const Atom& a = seg.atoms[i];

        if (a.kind == Atom::Kind::DOUBLESTAR)
        {
            return false;
        }

        if (a.kind == Atom::Kind::LITERAL)
        {
            const std::string& lit = a.literal;

            for (std::size_t j = 0; j <= N; ++j)
            {
                if (!dp[i][j])
                {
                    continue;
                }

                if (j + lit.size() <= N && name.substr(j, lit.size()) == lit)
                {
                    if (!dp[i + 1][j + lit.size()])
                    {
                        dp[i + 1][j + lit.size()] = true;

                        prev[i + 1][j + lit.size()].has_prev = true;
                        prev[i + 1][j + lit.size()].pi = i;
                        prev[i + 1][j + lit.size()].pj = j;
                    }
                }
            }
        }
        else if (a.kind == Atom::Kind::QMARK)
        {
            for (std::size_t j = 0; j < N; ++j)
            {
                if (!dp[i][j])
                {
                    continue;
                }

                if (!dp[i + 1][j + 1])
                {
                    dp[i + 1][j + 1] = true;

                    PrevCell& p = prev[i + 1][j + 1];
                    p.has_prev = true;
                    p.pi = i;
                    p.pj = j;

                    p.capture = true;
                    p.kind = Capture::Kind::CHAR;
                    p.cs = j;
                    p.ce = j + 1;
                }
            }
        }
        else if (a.kind == Atom::Kind::CHARCLASS)
        {
            for (std::size_t j = 0; j < N; ++j)
            {
                if (!dp[i][j])
                {
                    continue;
                }

                if (!CharClassMatch(a.cls, name[j]))
                {
                    continue;
                }

                if (!dp[i + 1][j + 1])
                {
                    dp[i + 1][j + 1] = true;

                    PrevCell& p = prev[i + 1][j + 1];
                    p.has_prev = true;
                    p.pi = i;
                    p.pj = j;

                    p.capture = true;
                    p.kind = Capture::Kind::CHAR;
                    p.cs = j;
                    p.ce = j + 1;
                }
            }
        }
        else if (a.kind == Atom::Kind::STAR)
        {
            for (std::size_t j = 0; j <= N; ++j)
            {
                if (!dp[i][j])
                {
                    continue;
                }

                // deterministico: prova k da j..N, il primo che raggiunge uno stato non settato vince
                for (std::size_t k = j; k <= N; ++k)
                {
                    if (!dp[i + 1][k])
                    {
                        dp[i + 1][k] = true;

                        PrevCell& p = prev[i + 1][k];
                        p.has_prev = true;
                        p.pi = i;
                        p.pj = j;

                        p.capture = true;
                        p.kind = Capture::Kind::SEGMENT;
                        p.cs = j;
                        p.ce = k;
                    }
                }
            }
        }
    }

    if (!dp[A][N])
    {
        return false;
    }

    // ricostruzione catture (reverse)
    std::vector<Capture> rev;

    std::size_t ci = A;
    std::size_t cj = N;

    while (!(ci == 0 && cj == 0))
    {
        const PrevCell& p = prev[ci][cj];
        if (!p.has_prev)
        {
            return false;
        }

        if (p.capture)
        {
            Capture cap{};
            cap.kind = p.kind;
            cap.value.assign(name.substr(p.cs, p.ce - p.cs));
            rev.push_back(std::move(cap));
        }

        ci = p.pi;
        cj = p.pj;
    }

    std::reverse(rev.begin(), rev.end());
    out_caps = std::move(rev);
    return true;
}



// ---- Pattern matcher con catture (supporta più **)
static bool MatchCaptureRec(const Pattern&                        from_pat,
                            const std::vector<std::string_view>&   src_segs,
                            std::size_t                            pi,
                            std::size_t                            si,
                            std::vector<Capture>&                  caps,
                            std::vector<std::vector<std::int8_t>>& memo_fail) noexcept
{
    if (pi == from_pat.segments.size())
    {
        return si == src_segs.size();
    }

    if (si > src_segs.size())
    {
        return false;
    }

    if (memo_fail[pi][si] == 0)
    {
        return false;
    }

    const Segment& seg = from_pat.segments[pi];

    if (seg.IsDoubleStarOnly())
    {
        // ** cattura 0..N segmenti (PATH), deterministico: prova prima la match più corta
        for (std::size_t t = si; t <= src_segs.size(); ++t)
        {
            Capture c{};
            c.kind = Capture::Kind::PATH;
            c.value = JoinPathSegments(src_segs, si, t);

            caps.push_back(std::move(c));

            if (MatchCaptureRec(from_pat, src_segs, pi + 1, t, caps, memo_fail))
            {
                return true;
            }

            caps.pop_back();
        }

        memo_fail[pi][si] = 0;
        return false;
    }

    if (si >= src_segs.size())
    {
        memo_fail[pi][si] = 0;
        return false;
    }

    std::vector<Capture> local;

    if (!MatchSegmentCapture(seg, src_segs[si], local))
    {
        memo_fail[pi][si] = 0;
        return false;
    }

    // append local, recurse, rollback se fail
    const std::size_t old_size = caps.size();
    for (auto& c : local)
    {
        caps.push_back(std::move(c));
    }

    if (MatchCaptureRec(from_pat, src_segs, pi + 1, si + 1, caps, memo_fail))
    {
        return true;
    }

    caps.resize(old_size);
    memo_fail[pi][si] = 0;
    return false;
}


static bool MatchCapture(const Pattern&            from_pat,
                    std::string_view          src_generic,
                    std::vector<Capture>&     caps) noexcept
{
    caps.clear();

    std::vector<std::string_view> src_segs;
    SplitPathSegments(src_generic, src_segs);

    // memo_fail[pi][si] = 0 => stato fallito già visto
    std::vector<std::vector<std::int8_t>> memo_fail(
        from_pat.segments.size() + 1,
        std::vector<std::int8_t>(src_segs.size() + 1, -1)
    );

    return MatchCaptureRec(from_pat, src_segs, 0, 0, caps, memo_fail);
}



static bool Consume(const Capture& c, Capture::Kind k) noexcept
{
    return c.kind == k;
}



static bool Instantiate(const Pattern&              to_pat,
                    const std::vector<Capture>& caps,
                    std::string&                out_generic) noexcept
{
    out_generic.clear();

    std::size_t cap_i = 0;
    std::vector<std::string> out_segs;

    for (const auto& seg : to_pat.segments)
    {
        if (seg.IsDoubleStarOnly())
        {
            if (cap_i >= caps.size())
            {
                return false;
            }
            if (!Consume(caps[cap_i], Capture::Kind::PATH))
            {
                return false;
            }

            std::vector<std::string_view> mid;
            SplitPathSegments(caps[cap_i].value, mid);

            for (const auto& s : mid)
            {
                out_segs.emplace_back(std::string(s));
            }

            ++cap_i;
            continue;
        }

        std::string built;

        for (const auto& a : seg.atoms)
        {
            if (a.kind == Atom::Kind::LITERAL)
            {
                built += a.literal;
                continue;
            }

            if (cap_i >= caps.size())
            {
                return false;
            }

            if (a.kind == Atom::Kind::STAR)
            {
                if (!Consume(caps[cap_i], Capture::Kind::SEGMENT))
                {
                    return false;
                }
                built += caps[cap_i].value;
                ++cap_i;
                continue;
            }

            if (a.kind == Atom::Kind::QMARK || a.kind == Atom::Kind::CHARCLASS)
            {
                if (!Consume(caps[cap_i], Capture::Kind::CHAR))
                {
                    return false;
                }
                if (caps[cap_i].value.size() != 1)
                {
                    return false;
                }
                built += caps[cap_i].value;
                ++cap_i;
                continue;
            }

            return false;
        }

        out_segs.push_back(std::move(built));
    }

    // Se restano catture non consumate => incompatibilità pattern
    if (cap_i != caps.size())
    {
        return false;
    }

    // join
    for (std::size_t i = 0; i < out_segs.size(); ++i)
    {
        if (i != 0)
        {
            out_generic.push_back('/');
        }
        out_generic += out_segs[i];
    }

    return true;
}





//    ███╗   ███╗ ██████╗ ██████╗ ██╗   ██╗██╗     ███████╗    ██╗███╗   ███╗██████╗ ██╗     
//    ████╗ ████║██╔═══██╗██╔══██╗██║   ██║██║     ██╔════╝    ██║████╗ ████║██╔══██╗██║     
//    ██╔████╔██║██║   ██║██║  ██║██║   ██║██║     █████╗      ██║██╔████╔██║██████╔╝██║     
//    ██║╚██╔╝██║██║   ██║██║  ██║██║   ██║██║     ██╔══╝      ██║██║╚██╔╝██║██╔═══╝ ██║     
//    ██║ ╚═╝ ██║╚██████╔╝██████╔╝╚██████╔╝███████╗███████╗    ██║██║ ╚═╝ ██║██║     ███████╗
//    ╚═╝     ╚═╝ ╚═════╝ ╚═════╝  ╚═════╝ ╚══════╝╚══════╝    ╚═╝╚═╝     ╚═╝╚═╝     ╚══════╝
//                                                                                                                                                                           


bool Arcana::Glob::Parse(std::string_view input, Pattern& out, ParseError& err, const Options& opt) noexcept
{
    out = Pattern{};
    err = ParseError{};

    if (input.size() == 0)
    {
        err.code   = ParseError::Code::EMPTY_PATTERN;
        err.offset = 0;
        return false;
    }

    out.normalized = Normalize(input, opt);

    if (!SplitSegments(out.normalized, out, err, opt))
    {
        return false;
    }

    return true;
}
                                                                                                  


bool Arcana::Glob::Expand(const Pattern& pattern, const fs::path& base_dir, std::vector<std::string>& out, const ExpandOptions& opt) noexcept
{
    out.clear();

    std::error_code ec;

    fs::path start = base_dir;
    if (pattern.absolute)
    {
        // Pattern assoluto: ignora base_dir e parte da root del sistema.
        // Portable: su Windows root dipende dal path (drive). Qui prendiamo root_path() di base_dir.
        // Se base_dir è relativo, root_path può essere vuoto -> fallback a "/".
        fs::path root = base_dir.root_path();
        if (root.empty())
        {
            root = fs::path("/");
        }
        start = root;
    }

    if (!fs::exists(start, ec))
    {
        return false;
    }

    // Avvio espansione
    ExpandRec(pattern, opt, start, 0, out);

    // Determinismo: sort + dedup
    std::sort(out.begin(), out.end());
    out.erase(std::unique(out.begin(), out.end()), out.end());

    return true;
}



bool Arcana::Glob:: MapGlobToGlob(std::string_view  from_glob,
                    std::string_view  to_glob,
                    const std::vector<std::string>& src_list,
                    std::vector<std::string>&       out_list,
                    ParseError&       err_from,
                    ParseError&       err_to) noexcept
{
    Pattern from_pat;
    Pattern to_pat;

    if (!Glob::Parse(from_glob, from_pat, err_from))
    {
        return false;
    }

    if (!Glob::Parse(to_glob, to_pat, err_to))
    {
        return false;
    }

    out_list.clear();

    for (const auto& src : src_list)
    {
        std::vector<Arcana::Glob::Capture> caps;
        if (!MatchCapture(from_pat, src, caps))
        {
            return false;
        }

        std::string out;
        if (!Instantiate(to_pat, caps, out))
        {
            return false;
        }

        out_list.emplace_back(std::move(out));
    }

    return true;
}