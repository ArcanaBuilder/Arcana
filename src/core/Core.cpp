#include "Core.h"
#include "Cache.h"
#include "Semantic.h"

#include <mutex>

USE_MODULE(Arcana);



//    ██████╗ ██████╗ ██╗██╗   ██╗ █████╗ ████████╗███████╗███████╗
//    ██╔══██╗██╔══██╗██║██║   ██║██╔══██╗╚══██╔══╝██╔════╝██╔════╝
//    ██████╔╝██████╔╝██║██║   ██║███████║   ██║   █████╗  ███████╗
//    ██╔═══╝ ██╔══██╗██║╚██╗ ██╔╝██╔══██║   ██║   ██╔══╝  ╚════██║
//    ██║     ██║  ██║██║ ╚████╔╝ ██║  ██║   ██║   ███████╗███████║
//    ╚═╝     ╚═╝  ╚═╝╚═╝  ╚═══╝  ╚═╝  ╚═╝   ╚═╝   ╚══════╝╚══════╝
//                                                                 

static Core::InstructionResult run_instruction(const std::string& interpreter, const std::string& command, const bool echo) noexcept
{
    Core::InstructionResult res { command, 0 };

    if (echo)
    {
        MSG(command);
    }

    Cache::Manager&         cache    = Cache::Manager::Instance();
    std::filesystem::path   script   = cache.WriteScript(command);
    std::string             full_cmd = interpreter + " \"" + script.string() + "\"";
    
    int ret = std::system(full_cmd.c_str());

    if (ret == -1)
    {
        res.exit_code = 127;
    }
    else
    {
        res.exit_code = WEXITSTATUS(ret);
    }

    return res;
}


static Core::Result run_job(const Jobs::Job& job, const Core::RunOptions& opt) noexcept
{
    Core::Result result {};

    result.name        = job.name;
    result.ok          = true;
    result.first_error = 0;

    if (!job.parallelizable)
    {
        // LINEAR EXECUTION OF JOB: NO MT
        for (const auto &cmd : job.instructions)
        {
            auto r = run_instruction(job.interpreter, cmd, job.echo);
            result.results.push_back(r);

            // ON ERROR JUST QUIT
            if (r.exit_code != 0)
            {
                result.ok          = false;
                result.first_error = r.exit_code;

                if (opt.stop_on_error)
                {
                    break;
                }
            }
        }
    }
    else
    {
        // PARALLEL JOB EXEUTION: MT
        std::vector<std::thread> threads;
        std::mutex               mutex;

        result.results.resize(job.instructions.size());

        // LABDA USED AS WORKER THREAD BODY
        auto worker = [&] (std::size_t idx)
        {
            const auto& cmd = job.instructions[idx];
            auto        r   = run_instruction(job.interpreter, cmd, job.echo);

            // BE SURE TO USE A MUTEX TO WRITE IN THE RESULT ARRAY
            std::lock_guard<std::mutex> lock(mutex);
            result.results[idx] = r;
        };

        // GENRATE A VECTOR OF EACH INSTRCTUION
        for (std::size_t i = 0; i < job.instructions.size(); ++i)
        {
            threads.emplace_back(worker, i);

            // IF THE MAX THRAD IS REACHED, JUST JOIN ALL THREAD IN THE VECTOR,
            // THEN CONTINUE 
            if (threads.size() >= opt.max_parallelism)
            {
                for (auto &t : threads)
                {
                    t.join();
                }

                threads.clear();
            }
        }

        // IF THE VECTOR IS NOT EMPTY, JUST JOIN THEM
        for (auto &t : threads)
        {
            t.join();
        }

        // CHECK ALL RESULTS IN ORDER TO DISCOVER ERRORS
        for (const auto &r : result.results)
        {
            if (r.exit_code != 0)
            {
                result.ok = false;

                if (result.first_error == 0)
                {
                    result.first_error = r.exit_code;
                }
            }
        }
    }

    return result;
}





//    ██████╗ ██╗   ██╗██████╗ ██╗     ██╗ ██████╗███████╗
//    ██╔══██╗██║   ██║██╔══██╗██║     ██║██╔════╝██╔════╝
//    ██████╔╝██║   ██║██████╔╝██║     ██║██║     ███████╗
//    ██╔═══╝ ██║   ██║██╔══██╗██║     ██║██║     ╚════██║
//    ██║     ╚██████╔╝██████╔╝███████╗██║╚██████╗███████║
//    ╚═╝      ╚═════╝ ╚═════╝ ╚══════╝╚═╝ ╚═════╝╚══════╝
//                                                        


std::vector<Core::Result> Core::run_jobs(const Jobs::List& jobs, const Core::RunOptions& opt) noexcept
{
    std::vector<Core::Result> results;

    results.reserve(jobs.All().size());

    // RUN EACH JOB 
    for (auto &job : jobs.All())
    {
        ARC(ANSI_GRAY << "Running task: "  << job.name << ANSI_RESET);

        auto r = run_job(job, opt);
        results.push_back(r);

        if (!r.ok && opt.stop_on_error)
        {
            ERR(ANSI_GRAY << "Task failed: " << job.name << ANSI_RESET);
            break;
        }
    }

    return results;
}
