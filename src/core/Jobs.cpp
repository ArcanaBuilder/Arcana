#include "Jobs.h"
#include "TableHelper.h"

#include <string_view>
#include <unordered_map>

USE_MODULE(Arcana::Jobs);


enum class VisitMark : uint8_t
{
    NONE,
    TEMP,
    PERM,
};


enum class VarExpansion : uint8_t
{
    LIST,
    INLINE,
};


struct TopoResult
{
    TopoResult() : ok(true) {}

    bool                ok;
    std::string         error_task;
    std::vector<Job>    jobs;
};

struct JobExpandMatch
{
    std::size_t  start;
    std::size_t  end;
    std::string  var_name; 
    VarExpansion var_exp;
    
    JobExpandMatch(std::size_t a, std::size_t b, std::string vn, VarExpansion ve) noexcept
    : start(a), end(b), var_name(vn), var_exp(ve)
    {}
};



template < typename T >
using AbstractKeywordMap = std::unordered_map<
    std::string_view,
    T,
    Arcana::Support::StringViewHash,
    Arcana::Support::StringViewEq
>;


using ExpansionMap = AbstractKeywordMap<VarExpansion>;


static const ExpansionMap Expansion_Map = 
{
    { "list"     , VarExpansion::LIST   },          
    { "inline"   , VarExpansion::INLINE },        
};



static bool HasFileChanged(const std::string& path) noexcept
{
    UNUSED(path);
    return true;
}



void PruneUnchangedInstructions(Job& job,
                                const Semantic::InstructionTask& task,
                                const Semantic::VTable& vtable) noexcept
{
    if (task.task_inputs.empty() || job.instructions.empty())
    {
        return;
    }

    // vettore di flag per tenere/trimmare le istruzioni
    std::vector<bool> keep(job.instructions.size(), true);

    // helper locale: dato un file, marca le istruzioni che lo usano
    auto process_file = [&] (const std::string & file)
    {
        if (file.empty())
        {
            return;
        }

        for (std::size_t i = 0; i < job.instructions.size(); ++i)
        {
            if (!keep[i])
            {
                continue; // già scartata
            }

            const std::string & instr = job.instructions[i];

            // match banale: il nome file compare nella stringa dell'istruzione
            if (instr.find(file) != std::string::npos)
            {
                // stub: in futuro qui controlli timestamp/hash del file
                bool changed = HasFileChanged(file);

                if (!changed)
                {
                    keep[i] = false; // istruzione da togliere
                }
            }
        }
    };

    // iteri gli input dichiarati sul task
    for (const auto & input_name : task.task_inputs)
    {
        auto it = vtable.find(input_name);
        if (it == vtable.end())
        {
            // input non trovato in vtable: per ora ignoro
            continue;
        }

        const auto & var_ref = it->second;

        if (!var_ref.glob_expansion.empty())
        {
            // caso glob: più file
            for (const auto & file : var_ref.glob_expansion)
            {
                process_file(file);
            }
        }
        else
        {
            // fallback a valore singolo (es. una singola path)
            if (!var_ref.var_value.empty())
            {
                process_file(var_ref.var_value);
            }
        }
    }

    // compattiamo le istruzioni mantenendo l'ordine
    Semantic::Task::Instrs filtered;
    filtered.reserve(job.instructions.size());

    for (std::size_t i = 0; i < job.instructions.size(); ++i)
    {
        if (keep[i])
        {
            filtered.emplace_back(std::move(job.instructions[i]));
        }
    }

    job.instructions.swap(filtered);
}






static ExpansionError ExpandListInstuctions(const Semantic::InstructionTask&  task, 
                                                  Semantic::VTable&           vtable,
                                                  Semantic::Task::Instrs&     out_instrs) noexcept
{
    // INSTRUCTION ENCODER LAMDA
    auto make_single_instruction = [] (const Semantic::Task::Instrs& instrs) noexcept -> std::string 
    {
        std::stringstream ss;
        for (const auto& instr : instrs)
        {
            ss << instr << std::endl;
        }

        return ss.str();
    };


    // VARIABLE'S GLOB_EXPANSION LEGNTH CHECKER
    auto glob_len_check = [] (const Semantic::VTable& vtable, const std::vector<std::string>& keys) noexcept -> std::size_t
    {
        if (keys.empty())
            return 0;

        auto it0 = vtable.find(keys[0]);
        if (it0 == vtable.end())
            return 0;

        const std::size_t expected = it0->second.glob_expansion.size();

        // Controllo da key[1] in poi
        bool ok = std::none_of(keys.begin() + 1, keys.end(), [&](const std::string& k)
        {
            auto it = vtable.find(k);
            return it == vtable.end() || it->second.glob_expansion.size() != expected;
        });

        return ok ? expected : 0;
    };

    // VARIABLE'S GLOB_EXPANSION CONCATTER
    auto join_vec = [] (const std::vector<std::string>& v) noexcept -> std::string
    {
        std::string res;
        for (std::size_t i = 0; i < v.size(); ++i)
        {
            if (i)
                res += ' ';
            res += v[i];
        }
        return res;
    };

    bool                   never_matched = true;
    std::stringstream      err;
    ExpansionError         status;
    Semantic::Task::Instrs computed_instr;

    // MULTI INSTRUCTION MATCH PATTER
    std::regex re(R"(\{arc:([a-z]+):([a-zA-Z][a-zA-Z0-9]*)\})");

    // FOR EACH INSTRUCTION IN THE PASSED TASK
    for (const auto& inst : task.task_instrs)
    {
        // VECTOR OF SINGLE INSTRUCTION MATCH
        std::vector<JobExpandMatch> matches;
        std::vector<std::string>    list_vars;

        // FOR EACH MATCHED PATTERN
        for (std::sregex_iterator it(inst.begin(), inst.end(), re), end; it != end; ++it)
        {
            const auto& exp_type = ((*it)[1].str());
            const auto& variable = ((*it)[2].str());

            if (auto vit = vtable.find(variable); vit == vtable.end())
            {
                err << "Cannot locate and expand " << ANSI_BMAGENTA << variable << ANSI_RESET << " in task " << ANSI_BOLD << task.task_name << ANSI_RESET;
                status.msg = err.str();
                return status;
            }

            VarExpansion exp;
            if (auto eit = Expansion_Map.find(exp_type); eit != Expansion_Map.end())
            {
                exp = eit->second;
            }
            else
            {
                err << "Invalid expansion algorithm " << ANSI_BMAGENTA << exp_type << ANSI_RESET << " in task " << ANSI_BOLD << task.task_name << ANSI_RESET;
                status.msg = err.str();
                return status;
            }

            std::size_t  start = it->position();
            std::size_t  epos  = start + (*it)[0].length();

            matches.push_back( JobExpandMatch{start, epos, variable, exp} );
            
            if (exp == VarExpansion::LIST)
            {
                list_vars.push_back(variable);
            }
        }

        // IF THERE IS NO MATCH, JUST PUSH THE INSTRUCTION AS IT IS AND CONTINUE
        if (matches.empty())
        {
            computed_instr.push_back(inst);
            continue;
        }

        never_matched = false;

        // SORT THE MATCHED BY REVERSED START POSITION 
        std::sort(matches.begin(), matches.end(), [] (const JobExpandMatch& a, const JobExpandMatch& b) 
        {
            return a.start < b.start;
        });


        std::size_t expand_count = 1;

        if (!list_vars.empty())
        {
            expand_count = glob_len_check(vtable, list_vars);
            if (!expand_count)
            {
                err << "Glob expansion size mismatch for vars: ";

                for (std::size_t i = 0; i < list_vars.size(); ++i)
                {
                    if (i) err << ", ";
                    err  << ANSI_BMAGENTA << list_vars[i] << ANSI_RESET;
                }

                err << " in task " << ANSI_BOLD << task.task_name << ANSI_RESET;

                status.msg = err.str();
                return status;
            }
        }

        // INLINE VRS CACHE: pre-computed join
        std::unordered_map<std::string, std::string> inline_cache;

        for (const auto& m : matches)
        {
            if (m.var_exp == VarExpansion::INLINE)
            {
                const auto& vec = vtable[m.var_name].glob_expansion;
                inline_cache.emplace(m.var_name, join_vec(vec));
            }
        }

        // EXPANSION
        for (std::size_t i = 0; i < expand_count; ++i)
        {
            std::size_t cursor = 0;
            std::string out;

            for (const auto& match : matches)
            {
                out.append(inst, cursor, match.start - cursor);

                const auto& vec = vtable[match.var_name].glob_expansion;

                if (match.var_exp == VarExpansion::LIST)
                {
                    out += vec[i];
                }
                else // INLINE
                {
                    out += inline_cache[match.var_name];
                }

                cursor = match.end;
            }

            out.append(inst, cursor, std::string::npos);
            computed_instr.push_back(std::move(out));
        }
    }

    if (never_matched)
    {
        if (task.task_instrs.size())
        {
            out_instrs.push_back(make_single_instruction(computed_instr));
        }
    }
    else
    {  
        out_instrs = std::move(computed_instr);
    }

    status.ok = true;

    return status;
}


static std::pair<ExpansionError, std::optional<Job>> FromInstruction(const Semantic::InstructionTask& task, Semantic::VTable& vtable) noexcept
{
    Job            new_job {};
    ExpansionError status;

    new_job.name         = task.task_name;
    new_job.interpreter  = task.interpreter;

    status = ExpandListInstuctions(task, vtable, new_job.instructions);

    if (!status.ok)
    {
        return std::make_pair(status, std::nullopt);
    }

    PruneUnchangedInstructions(new_job, task, vtable);

    if (!new_job.instructions.size())
    {
        status.ok = true;
        return std::make_pair(status, std::nullopt);
    }

    if (task.hasAttribute(Semantic::Attr::Type::MULTITHREAD))
    {
        new_job.parallelizable = true;
    } 

    return std::make_pair(status, new_job);
}



using Graph = std::unordered_map<std::string, std::array<std::vector<std::string>, 2>>;

static Graph BuildGraph(const Semantic::FTable& table)
{
    Graph g;

    for (const auto& [name, task] : table)
    {
        // assicura che ogni task abbia almeno un vettore vuoto
        g[name][0] = {};
        g[name][1] = {}; 

        // AFTER: dep -> task
        if (task.hasAttribute(Semantic::Attr::Type::AFTER))
        {
            auto& deps = task.getProperties(Semantic::Attr::Type::AFTER);
            for (const auto& dep_name : deps)
            {
                g[name][0].push_back(dep_name);
            }
        }

        // THEN: task -> succ
        if (task.hasAttribute(Semantic::Attr::Type::THEN))
        {
            auto& succs = task.getProperties(Semantic::Attr::Type::THEN);
            for (const auto& succ_name : succs)
            {
                g[name][1].push_back(succ_name);
            }
        }
    }

    return g;
}

static bool dfs_visit(const std::string&                name,
                      const Semantic::FTable&           table,
                            Semantic::VTable&           vtable,
                      const Graph&                      graph,
                      std::map<std::string, VisitMark>& mark,
                      std::vector<Job>&                 out,
                      ExpansionError&                   err) noexcept
{
    std::stringstream ss;

    auto tit = table.find(name);
    if (tit == table.end())
    {
        ss << "Unknown task '" << ANSI_BMAGENTA << name << ANSI_RESET << "'";
        err.msg = ss.str();
        err.ok  = false;
        return false;
    }

    VisitMark& m = mark[name];

    if (m == VisitMark::PERM)
        return true;

    if (m == VisitMark::TEMP)
    {
        ss << "Cyclic dependency involving task '" << ANSI_BMAGENTA << name << ANSI_RESET << "'";
        err.msg = ss.str();
        err.ok  = false;
        return false;
    }

    m = VisitMark::TEMP;

    // visita tutti i successori nel grafo (AFTER + THEN già “fusi”)
    auto git = graph.find(name);
    if (git != graph.end())
    {
        for (const auto& succ : git->second[0])
        {
            if (!dfs_visit(succ, table, vtable, graph, mark, out, err))
            {
                return false;
            }
        }

        // genera il Job associato a questo task
        const Semantic::InstructionTask& t = tit->second;
        const auto& result = FromInstruction(t, vtable);

        if (!result.first.ok)
        {
            err = result.first;
            return false;
        }

        if (result.second)
            out.push_back(*result.second);

        for (const auto& succ : git->second[1])
        {
            if (!dfs_visit(succ, table, vtable, graph, mark, out, err))
            {
                return false;
            }
        }
    }

    m = VisitMark::PERM;

    // genera il Job associato a questo task
    const Semantic::InstructionTask& t = tit->second;
    const auto& result = FromInstruction(t, vtable);

    if (!result.first.ok)
    {
        err = result.first;
        return false;
    }

    if (result.second)
        out.push_back(*result.second);

    return true;
}






void List::Insert(const std::optional<Job>& j)
{
    if (j)
    {
        auto [it, inserted] = index.insert(j.value().name);
        if (inserted)
        {
            data.push_back(j.value());
        }
    }
}


ExpansionError List::FromEnv(Semantic::Enviroment& environment, List& out) noexcept
{
    ExpansionError status;
    status.ok = true;

    // build graph da ftable (AFTER + THEN)
    Graph graph = BuildGraph(environment.ftable);

    // 2) MAIN
    if (auto main_task_opt = Table::GetValue(environment.ftable, Semantic::Attr::Type::MAIN))
    {
        const auto&       main_task  = main_task_opt.value();
        const std::string main_name  = main_task.get().task_name;
    
        ExpansionError                   err;   
        std::vector<Job>                 ordered;
        std::map<std::string, VisitMark> mark;

        if (!dfs_visit(main_name, environment.ftable, environment.vtable, graph, mark, ordered, err))
        {
            return err.ok ? status : err;
        }

        // se dfs fa push_back dopo i figli, qui puoi fare reverse
        //std::reverse(ordered.begin(), ordered.end());

        for (const Job& j : ordered)
        {
            out.Insert(j);
        }
    }

    // 4) ALWAYS invariato
    if (auto always_opt = Table::GetValues(environment.ftable, Semantic::Attr::Type::ALWAYS))
    {
        for (const auto& task : always_opt.value())
        {
            const auto& result = FromInstruction(task, environment.vtable);
            if (!result.first.ok)
            {
                return result.first;
            }
            else if (result.second)
            {
                out.Insert(*result.second);
            }
        }
    }

    return status;
}
