#ifndef __ARCANA__DEBUG_H__
#define __ARCANA__DEBUG_H__


#include "Jobs.h"
#include "Defines.h"
#include "Support.h"
#include "Semantic.h"


BEGIN_MODULE(Debug)


inline void DebugJobsList(const Jobs::List& jobs)
{
    constexpr std::size_t WIDTH = 120;
    std::string border(WIDTH - 2, '-');

    auto print_center = [&] (const std::string& text)
    {
        std::size_t inner = WIDTH - 2;
        std::size_t pad_left  = (inner - text.size()) / 2;
        std::size_t pad_right = inner - text.size() - pad_left;

        DBG("|" << std::string(pad_left, ' ') << text << std::string(pad_right, ' ') << "|");
    };

    auto print_line = [&] (const std::string& text)
    {
        auto subprint = [&] (const std::string& text) 
        {
            // taglia se troppo lunga
            std::string t = text;
            if (t.size() > WIDTH - 4)
                t = t.substr(0, WIDTH - 4);

            std::size_t inner = WIDTH - 2;
            DBG("| " << t << std::string(inner - t.size() - 1, ' ') << "|");
        };

        std::size_t start = 0;
        while (true)
        {
            std::size_t pos = text.find('\n', start);

            if (pos == std::string::npos)
            {
                subprint(text.substr(start));
                break;
            }

            subprint(text.substr(start, pos - start));
            start = pos + 1;
        }
    };


    DBG("+" << border << "+");
    print_center("JOBS DEBUG");
    DBG("+" << border << "+");

    for (const auto& job : jobs.All())
    {
        print_line("NAME       : " + job.name);
        print_line("INTERPRETER: " + job.interpreter);
        print_line(std::string("MULTITHREAD: ") +(job.parallelizable ? "Yes" : "No"));
        print_line("INSTRUCTION: ");
        for (const auto& job_instr : job.instructions)
        {
            print_line(job_instr);
        }
        DBG("+" << border << "+");
    }
}



END_MODULE(Debug)



#endif /* __ARCANA__DEBUG_H__ */