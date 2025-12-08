#ifndef __ARCANA__DEBUG_H__
#define __ARCANA__DEBUG_H__


#include "Jobs.h"
#include "Defines.h"
#include "Support.h"
#include "Semantic.h"


BEGIN_MODULE(Debug)


constexpr std::size_t WIDTH = 120;


auto print_center = [] (const std::string& text)
{
    std::size_t inner = WIDTH - 2;
    std::size_t pad_left  = (inner - text.size()) / 2;
    std::size_t pad_right = inner - text.size() - pad_left;

    DBG("|" << std::string(pad_left, ' ') << text << std::string(pad_right, ' ') << "|");
};


auto print_line = [] (const std::string& text)
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


auto vector_inline = [] (const std::vector<std::string>& vec) noexcept -> std::string
{
    uint32_t i = 0;
    std::stringstream ss;

    for (const auto& item : vec)
    {
        if (i) ss << ", ";
        ss << item;
        i++;
    }

    return ss.str();
};



auto reference_inline = [] (const Semantic::FListCRef& tasks) noexcept -> std::string
{
    uint32_t i = 0;
    std::stringstream ss;

    for (const auto& item : tasks)
    {
        if (i) ss << ", ";
        ss << item.get().task_name;
        i++;
    }

    return ss.str();
};


auto print_attributes = [] (const Semantic::Attr::List & attrs) noexcept -> void
{
    std::stringstream ss;

    for (const auto& item : attrs)
    {
        ss << item.name;
        if (item.props.size())
        {
            ss << " -> " << vector_inline(item.props);
        }
        ss << std::endl;
    }

    print_line(ss.str());
};








inline void VTable(const Semantic::VTable& vtable)
{
    std::string border(WIDTH - 2, '-');

    DBG("+" << border << "+");
    print_center("VTABLE DEBUG");
    DBG("+" << border << "+");

    for (const auto& [key, v] : vtable)
    {
        print_line("NAME      : " + key);
        print_line("VALUE     : " + v.var_value);
        print_line("GLOB EXP  : ");

        for (const auto& exp : v.glob_expansion)
        {
            print_line(exp);
        }

        print_line("ATTRIBUTES: ");
        print_attributes(v.attributes);

        DBG("+" << border << "+");
    }
}




inline void FTable(const Semantic::FTable& vtable)
{
    std::string border(WIDTH - 2, '-');

    DBG("+" << border << "+");
    print_center("FTABLE DEBUG");
    DBG("+" << border << "+");

    for (const auto& [key, f] : vtable)
    {
        print_line("NAME        : " + key);
        print_line("INPUTS      : " + vector_inline(f.task_inputs));
        print_line("INTERPETER  : " + f.interpreter);
        print_line("ATTRIBUTES  : ");
        print_attributes(f.attributes);
        print_line("INSTRUCTIONS:");

        for (const auto& instr : f.task_instrs)
        {
            print_line(instr);
        }

        DBG("+" << border << "+");
    }
}





inline void JobsList(const Jobs::List& jobs)
{
    std::string border(WIDTH - 2, '-');

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