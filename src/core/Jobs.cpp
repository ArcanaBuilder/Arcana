#include "Jobs.h"
#include "Cache.h"
#include "TableHelper.h"

#include <string_view>
#include <unordered_map>

USE_MODULE(Arcana::Jobs);




//    ████████╗██╗   ██╗██████╗ ███████╗███████╗
//    ╚══██╔══╝╚██╗ ██╔╝██╔══██╗██╔════╝██╔════╝
//       ██║    ╚████╔╝ ██████╔╝█████╗  ███████╗
//       ██║     ╚██╔╝  ██╔═══╝ ██╔══╝  ╚════██║
//       ██║      ██║   ██║     ███████╗███████║
//       ╚═╝      ╚═╝   ╚═╝     ╚══════╝╚══════╝
//                                              

/**
 * @brief DFS mark used for cycle detection and topo ordering.
 */
enum class VisitMark : uint8_t
{
    NONE,
    TEMP,
    PERM,
};



/**
 * @brief Expansion algorithm selector for `{arc:<mode>:<var>}` patterns.
 */
enum class VarExpansion : uint8_t
{
    LIST,
    INLINE,
    UNDEFINDED
};



/**
 * @brief Result container for topo traversal.
 */
struct TopoResult
{
    TopoResult() : ok(true) {}

    bool             ok;
    std::string      error_task;
    std::vector<Job> jobs;
};



/**
 * @brief Single match descriptor for per-instruction variable expansion.
 */
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


using ExpansionMap = Support::AbstractKeywordMap<VarExpansion>;
using Graph        = std::unordered_map<std::string, std::array<std::vector<std::string>, 2>>;



// ============================================================================
// EXPANSION MAP
// ============================================================================

/**
 * @brief Supported expansion modes.
 */
static const ExpansionMap Expansion_Map =
{
    { "list"   , VarExpansion::LIST   },
    { "inline" , VarExpansion::INLINE },
};






//    ██████╗ ██████╗ ██╗██╗   ██╗ █████╗ ████████╗███████╗███████╗
//    ██╔══██╗██╔══██╗██║██║   ██║██╔══██╗╚══██╔══╝██╔════╝██╔════╝
//    ██████╔╝██████╔╝██║██║   ██║███████║   ██║   █████╗  ███████╗
//    ██╔═══╝ ██╔══██╗██║╚██╗ ██╔╝██╔══██║   ██║   ██╔══╝  ╚════██║
//    ██║     ██║  ██║██║ ╚████╔╝ ██║  ██║   ██║   ███████╗███████║
//    ╚═╝     ╚═╝  ╚═╝╚═╝  ╚═══╝  ╚═╝  ╚═╝   ╚═╝   ╚══════╝╚══════╝
//                                                                 


/**
 * @brief Remove instructions that are not affected by any changed input file.
 *
 * The pruning is driven by task inputs (variables). For each input variable:
 * - if it has `glob_expansion`, each expanded file is checked;
 * - otherwise `var_value` is treated as a single file path.
 *
 * For each file, any instruction that contains the file substring is considered "dependent".
 * If the cache reports the file as unchanged, the instruction is pruned.
 *
 * @param job   Job to prune in-place.
 * @param task  Source semantic task (provides inputs).
 * @param vtable Variable table used to resolve input variables.
 */
static void PruneUnchangedInstructions(Jobs::Job& job,
                                       const Semantic::InstructionTask& task,
                                       const Semantic::VTable& vtable) noexcept
{
    // FAST-EXIT ON EMPTY INPUTS OR EMPTY INSTRUCTIONS
    if (task.task_inputs.empty() || job.instructions.empty())
    {
        return;
    }

    // TRACK WHICH INSTRUCTIONS MUST BE KEPT
    std::vector<bool> keep(job.instructions.size(), true);

    // MAP FILE -> INSTRUCTION DEPENDENCY AND PRUNE IF UNCHANGED
    auto process_file = [&] (const std::string& file)
    {
        if (file.empty())
        {
            return;
        }

        for (std::size_t i = 0; i < job.instructions.size(); ++i)
        {
            // SKIP ALREADY PRUNED INSTRUCTIONS
            if (!keep[i])
            {
                continue;
            }

            const std::string& instr = job.instructions[i];

            // SIMPLE DEPENDENCY HEURISTIC: INSTRUCTION CONTAINS FILE
            if (instr.find(file) != std::string::npos)
            {
                bool changed = Cache::Manager::Instance().HasFileChanged(file);

                // IF NOT CHANGED, PRUNE INSTRUCTION
                if (!changed)
                {
                    keep[i] = false;
                }
            }
        }
    };

    // LOOP OVER TASK INPUT VARIABLES
    for (const auto& input_name : task.task_inputs)
    {
        // LOOKUP VARIABLE (ASSUMED VALIDATED UPSTREAM)
        auto        it      = vtable.find(input_name);
        const auto& var_ref = it->second;

        // IF GLOB-EXPANDED, PROCESS EACH FILE
        if (!var_ref.glob_expansion.empty())
        {
            for (const auto& file : var_ref.glob_expansion)
            {
                process_file(file);
            }
        }
        else
        {
            // FALLBACK: PROCESS SINGLE VALUE
            if (!var_ref.var_value.empty())
            {
                for (auto& value : var_ref.var_value)
                {
                    process_file(value);
                }
            }
        }
    }

    // REBUILD INSTRUCTION LIST WITH KEPT ITEMS ONLY
    Semantic::Task::Instrs filtered;
    filtered.reserve(job.instructions.size());

    for (std::size_t i = 0; i < job.instructions.size(); ++i)
    {
        if (keep[i])
        {
            filtered.emplace_back(std::move(job.instructions[i]));
        }
    }

    // SWAP FILTERED INSTRUCTIONS BACK
    job.instructions.swap(filtered);
}



/**
 * @brief Expand `{arc:<mode>:<var>}` placeholders inside task instructions.
 *
 * Supported modes:
 * - `list`: generates N instructions by iterating glob-expanded vectors (N = expansion size)
 * - `inline`: joins glob-expanded vector into a single space-separated string
 *
 * If at least one `list` expansion is used, this function emits one instruction per list item.
 * Otherwise, all computed instructions are packed into a single script string separated by newlines.
 *
 * @param task Task providing the original instructions.
 * @param vtable Variable table used to resolve glob expansions.
 * @param out_instrs Output instruction list for the job.
 * @return ExpansionError with `ok=true` on success, otherwise `msg` is populated.
 */
static ExpansionError ExpandListInstuctions(const Semantic::InstructionTask& task,
                                           Semantic::VTable& vtable,
                                           Semantic::Task::Instrs& out_instrs) noexcept
{
    // ENCODE MULTIPLE LINES INTO A SINGLE SCRIPT STRING
    auto make_single_instruction = [] (const Semantic::Task::Instrs& instrs) noexcept -> std::string
    {
        std::stringstream ss;

        for (const auto& instr : instrs)
        {
            ss << instr << std::endl;
        }

        return ss.str();
    };

    // CHECK THAT ALL LIST VARS HAVE THE SAME GLOB EXPANSION SIZE
    auto glob_len_check = [] (const Semantic::VTable& vtable, const std::vector<std::string>& keys) noexcept -> std::size_t
    {
        if (keys.empty())
        {
            return 0;
        }

        auto it0 = vtable.find(keys[0]);
        if (it0 == vtable.end())
        {
            return 0;
        }

        const std::size_t expected = it0->second.glob_expansion.size();

        bool ok = std::none_of(keys.begin() + 1, keys.end(), [&] (const std::string& k)
        {
            auto it = vtable.find(k);
            return it == vtable.end() || it->second.glob_expansion.size() != expected;
        });

        return ok ? expected : 0;
    };

    // JOIN A VECTOR AS SPACE-SEPARATED STRING
    auto join_vec = [] (const std::vector<std::string>& v) noexcept -> std::string
    {
        std::string res;

        for (std::size_t i = 0; i < v.size(); ++i)
        {
            if (i)
            {
                res += ' ';
            }

            res += v[i];
        }

        return res;
    };

    bool                   never_matched = true;
    VarExpansion           expasion      = VarExpansion::UNDEFINDED;
    std::stringstream      err;
    ExpansionError         status;
    Semantic::Task::Instrs computed_instr;

    // MATCH PATTERN: {arc:<mode>:<var>}
    std::regex re(R"(\{arc:([a-z]+):([a-zA-Z][a-zA-Z0-9]*)\})");

    // PROCESS EACH ORIGINAL INSTRUCTION
    for (const auto& inst : task.task_instrs)
    {
        std::vector<JobExpandMatch> matches;
        std::vector<std::string>    list_vars;

        // FIND ALL MATCHES IN CURRENT INSTRUCTION
        for (std::sregex_iterator it(inst.begin(), inst.end(), re), end; it != end; ++it)
        {
            const auto& exp_type = ((*it)[1].str());
            const auto& variable = ((*it)[2].str());

            // VALIDATE VARIABLE EXISTS
            if (auto vit = vtable.find(variable); vit == vtable.end())
            {
                err << "Cannot locate and expand " << ANSI_BMAGENTA << variable << ANSI_RESET
                    << " in task " << ANSI_BOLD << task.task_name << ANSI_RESET;

                status.msg = err.str();
                return status;
            }

            // RESOLVE EXPANSION MODE
            VarExpansion exp;

            if (auto eit = Expansion_Map.find(exp_type); eit != Expansion_Map.end())
            {
                exp = eit->second;
            }
            else
            {
                err << "Invalid expansion algorithm " << ANSI_BMAGENTA << exp_type << ANSI_RESET
                    << " in task " << ANSI_BOLD << task.task_name << ANSI_RESET;

                status.msg = err.str();
                return status;
            }

            // COLLECT MATCH BOUNDS
            std::size_t start = it->position();
            std::size_t epos  = start + (*it)[0].length();

            matches.push_back(JobExpandMatch{ start, epos, variable, exp });

            // TRACK LIST VARS FOR CONSISTENT LENGTH CHECK
            if (exp == VarExpansion::LIST)
            {
                list_vars.push_back(variable);
            }
        }

        // IF NO MATCHES, KEEP INSTRUCTION AS-IS
        if (matches.empty())
        {
            computed_instr.push_back(inst);
            continue;
        }

        never_matched = false;

        // SORT MATCHES BY START POSITION
        std::sort(matches.begin(), matches.end(), [] (const JobExpandMatch& a, const JobExpandMatch& b)
        {
            return a.start < b.start;
        });

        // COMPUTE HOW MANY EXPANSIONS TO GENERATE
        std::size_t expand_count = 1;

        if (!list_vars.empty())
        {
            expand_count = glob_len_check(vtable, list_vars);

            if (!expand_count)
            {
                err << "Glob expansion size mismatch for vars: ";

                for (std::size_t i = 0; i < list_vars.size(); ++i)
                {
                    if (i)
                    {
                        err << ", ";
                    }

                    err << ANSI_BMAGENTA << list_vars[i] << ANSI_RESET;
                }

                err << " in task " << ANSI_BOLD << task.task_name << ANSI_RESET;

                status.msg = err.str();
                return status;
            }
        }

        // PRECOMPUTE INLINE EXPANSIONS
        std::unordered_map<std::string, std::string> inline_cache;

        for (const auto& m : matches)
        {
            if (m.var_exp == VarExpansion::INLINE)
            {
                const auto& vec = vtable[m.var_name].glob_expansion;
                inline_cache.emplace(m.var_name, join_vec(vec));
            }
        }

        // GENERATE EXPANDED INSTRUCTIONS
        for (std::size_t i = 0; i < expand_count; ++i)
        {
            std::size_t cursor = 0;
            std::string out;

            for (const auto& match : matches)
            {
                // APPEND PREFIX BEFORE MATCH
                out.append(inst, cursor, match.start - cursor);

                const auto& vec = vtable[match.var_name].glob_expansion;

                // EMIT EXPANSION
                if (match.var_exp == VarExpansion::LIST)
                {
                    expasion = match.var_exp;
                    out += vec[i];
                }
                else
                {
                    expasion = expasion == VarExpansion::LIST ? expasion : match.var_exp;
                    out += inline_cache[match.var_name];
                }

                cursor = match.end;
            }

            // APPEND TAIL AFTER LAST MATCH
            out.append(inst, cursor, std::string::npos);
            computed_instr.push_back(std::move(out));
        }
    }

    // IF NO LIST EXPANSION, PACK AS SINGLE SCRIPT INSTRUCTION
    if (never_matched || expasion != VarExpansion::LIST)
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



/**
 * @brief Build a runtime Job from a semantic InstructionTask.
 *
 * This function expands task instructions, prunes unchanged ones based on inputs,
 * and applies per-task execution flags (multithread, echo, flushcache).
 *
 * @param task Semantic task description.
 * @param vtable Variable table used for expansion and pruning.
 * @return Pair (ExpansionError, optional Job). If expansion fails, Job is nullopt.
 */
static std::pair<ExpansionError, std::optional<Job>>
FromInstruction(const Semantic::InstructionTask& task, Semantic::VTable& vtable, bool prunable = true) noexcept
{
    Job            new_job {};
    ExpansionError status;

    // INIT JOB HEADER
    new_job.name        = task.task_name;
    new_job.interpreter = task.interpreter;

    // EXPAND INSTRUCTIONS
    status = ExpandListInstuctions(task, vtable, new_job.instructions);

    if (!status.ok)
    {
        return std::make_pair(status, std::nullopt);
    }

    if (prunable)
    {
        // PRUNE UNCHANGED INSTRUCTIONS
        PruneUnchangedInstructions(new_job, task, vtable);
    }

    // IF NOTHING TO RUN, RETURN EMPTY JOB
    if (!new_job.instructions.size())
    {
        status.ok = true;
        return std::make_pair(status, std::nullopt);
    }

    // APPLY EXECUTION ATTRIBUTES
    new_job.parallelizable = task.hasAttribute(Semantic::Attr::Type::MULTITHREAD);
    new_job.echo           = task.hasAttribute(Semantic::Attr::Type::ECHO);

    // OPTIONALLY FLUSH CACHE
    if (task.hasAttribute(Semantic::Attr::Type::FLUSHCACHE))
    {
        Cache::Manager::Instance().ClearCache();
    }

    return std::make_pair(status, new_job);
}



/**
 * @brief Build a graph representation of task dependencies and successors.
 *
 * The graph node stores two adjacency lists:
 * - index 0: dependencies (REQUIRES)
 * - index 1: successors   (THEN)
 *
 * @param table Task table.
 * @return Graph keyed by task name.
 */
static Graph BuildGraph(const Semantic::FTable& table)
{
    Graph g;

    // BUILD GRAPH NODES
    for (const auto& [name, task] : table)
    {
        // INIT BOTH SIDES
        g[name][0] = {};
        g[name][1] = {};

        // DEPENDENCIES: dep -> task
        if (task.hasAttribute(Semantic::Attr::Type::REQUIRES))
        {
            auto& deps = task.getProperties(Semantic::Attr::Type::REQUIRES);

            for (const auto& dep_name : deps)
            {
                g[name][0].push_back(dep_name);
            }
        }

        // SUCCESSORS: task -> succ
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



/**
 * @brief DFS visit used to build an ordered job list starting from a root task.
 *
 * The visit:
 * - checks task existence,
 * - detects cycles via TEMP/PERM marks,
 * - visits dependencies first, then collects node, then visits successors.
 *
 * @param name Current task name.
 * @param table Task table.
 * @param vtable Variable table used by expansion.
 * @param graph Dependency/successor graph.
 * @param mark DFS marks.
 * @param out Output ordered jobs.
 * @param err Output expansion error.
 * @return True on success, false on error.
 */
static bool dfs_visit(const std::string&                name,
                      const Semantic::FTable&           table,
                      Semantic::VTable&                 vtable,
                      const Graph&                      graph,
                      std::map<std::string, VisitMark>& mark,
                      std::vector<Job>&                 out,
                      ExpansionError&                   err,
                      bool prunable = true) noexcept
{
    // COLLECT CURRENT NODE INTO OUT VECTOR
    auto collect_node = [&] (Arcana::Semantic::FTable::const_iterator& tit, bool pruning) noexcept -> bool
    {
        const Semantic::InstructionTask& t = tit->second;
        const auto& result = FromInstruction(t, vtable, pruning);

        if (!result.first.ok)
        {
            err = result.first;
            return false;
        }

        if (result.second)
        {
            out.push_back(*result.second);
        }

        return true;
    };

    std::stringstream ss;

    // CHECK TASK EXISTENCE
    auto tit = table.find(name);

    if (tit == table.end())
    {
        ss << "Unknown task '" << ANSI_BMAGENTA << name << ANSI_RESET << "'";
        err.msg = ss.str();
        err.ok  = false;
        return false;
    }

    // FETCH MARK
    VisitMark& m = mark[name];

    // ALREADY VISITED
    if (m == VisitMark::PERM)
    {
        return true;
    }

    // CYCLE DETECTED
    if (m == VisitMark::TEMP)
    {
        ss << "Cyclic dependency involving task '" << ANSI_BMAGENTA << name << ANSI_RESET << "'";
        err.msg = ss.str();
        err.ok  = false;
        return false;
    }

    // MARK AS IN-PROGRESS
    m = VisitMark::TEMP;

    // VISIT ADJACENCY LISTS
    auto git = graph.find(name);

    if (git != graph.end())
    {
        // VISIT DEPENDENCIES FIRST
        for (const auto& succ : git->second[0])
        {
            if (!dfs_visit(succ, table, vtable, graph, mark, out, err, prunable))
            {
                return false;
            }
        }

        // COLLECT CURRENT NODE
        if (!collect_node(tit, prunable))
        {
            return false;
        }

        // VISIT SUCCESSORS
        for (const auto& succ : git->second[1])
        {
            if (!dfs_visit(succ, table, vtable, graph, mark, out, err, prunable))
            {
                return false;
            }
        }
    }

    // MARK AS DONE
    m = VisitMark::PERM;

    // FINAL COLLECT (KEEP ORIGINAL BEHAVIOR)
    return collect_node(tit, prunable);
}



// ============================================================================
// JOBS LIST API
// ============================================================================

/**
 * @brief Insert a job if present and not already in the list.
 * @param j Optional job.
 */
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



/**
 * @brief Generate a job list from a semantic environment.
 *
 * The list is built by:
 * - building a graph from the task table,
 * - DFS visiting starting from the MAIN task (if present),
 * - inserting ALWAYS tasks afterwards.
 *
 * @param environment Semantic environment containing tables and expansions.
 * @param out Output job list.
 * @return ARCANA_RESULT__OK on success, otherwise a failure code.
 */
Arcana_Result List::FromEnv(Semantic::Enviroment& environment, List& out, std::vector<std::string>& recovery) noexcept
{
    // BUILD GRAPH FROM FTABLE
    Graph graph = BuildGraph(environment.ftable);

    for (const auto& task_name : recovery)
    {
        ExpansionError                   err;
        std::vector<Job>                 ordered;
        std::map<std::string, VisitMark> mark;

        // DFS VISIT ROOT
        if (!dfs_visit(task_name, environment.ftable, environment.vtable, graph, mark, ordered, err, false))
        {
            ERR(err.msg);
            return Arcana_Result::ARCANA_RESULT__NOK;
        }

        // INSERT ORDERED JOBS
        for (const Job& j : ordered)
        {
            out.Insert(j);
        }
    }

    // START FROM MAIN TASK
    if (auto main_task_opt = Table::GetValue(environment.ftable, Semantic::Attr::Type::MAIN))
    {
        const auto&       main_task = main_task_opt.value();
        const std::string main_name = main_task.get().task_name;

        ExpansionError                   err;
        std::vector<Job>                 ordered;
        std::map<std::string, VisitMark> mark;

        // DFS VISIT ROOT
        if (!dfs_visit(main_name, environment.ftable, environment.vtable, graph, mark, ordered, err))
        {
            ERR(err.msg);
            return Arcana_Result::ARCANA_RESULT__NOK;
        }

        // INSERT ORDERED JOBS
        for (const Job& j : ordered)
        {
            out.Insert(j);
        }

        out.main_job = main_name;
    }

    // COLLECT ALWAYS TASKS
    if (auto always_opt = Table::GetValues(environment.ftable, Semantic::Attr::Type::ALWAYS))
    {
        for (const auto& task : always_opt.value())
        {
            const auto& result = FromInstruction(task, environment.vtable);

            if (!result.first.ok)
            {
                ERR(result.first.msg);
                return Arcana_Result::ARCANA_RESULT__NOK;
            }
            else if (result.second)
            {
                out.Insert(*result.second);
            }
        }
    }

    return Arcana_Result::ARCANA_RESULT__OK;
}
 