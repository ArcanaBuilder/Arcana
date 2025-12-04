#include "Core.h"

#include <mutex>

USE_MODULE(Arcana);





static Core::InstructionResult run_instruction(const std::string& interpreter, const std::string& command, 
                                               const Core::RunOptions& opt) noexcept
{
    Core::InstructionResult  res {};
    res.command = command;

    std::string full_cmd = interpreter + " -c \"" + command + "\"";
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
        // esecuzione sequenziale delle istruzioni
        for (const auto &cmd : job.instructions)
        {
            auto r = run_instruction(job.interpreter, cmd, opt);
            result.results.push_back(r);

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
        // esecuzione parallela delle istruzioni del job
        std::vector<std::thread> threads;
        std::mutex               mutex;

        result.results.resize(job.instructions.size());

        auto worker = [&](std::size_t idx)
        {
            const auto &cmd = job.instructions[idx];
            auto r          = run_instruction(job.interpreter, cmd, opt);

            std::lock_guard<std::mutex> lock(mutex);
            result.results[idx] = r;
        };

        for (std::size_t i = 0; i < job.instructions.size(); ++i)
        {
            threads.emplace_back(worker, i);

            // opzionale: limita il numero di thread contemporanei
            if (threads.size() >= opt.max_parallelism)
            {
                for (auto &t : threads)
                {
                    t.join();
                }

                threads.clear();
            }
        }

        for (auto &t : threads)
        {
            t.join();
        }

        // valuta il risultato complessivo
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





std::vector<Core::Result> Core::run_jobs(const Jobs::List& jobs, const Core::RunOptions& opt) noexcept
{
    std::vector<Core::Result> results;

    results.reserve(jobs.All().size());

    for (const auto &job : jobs.All())
    {
        DBG("Running job: " << job.name);
        auto r = run_job(job, opt);
        results.push_back(r);

        if (!r.ok && opt.stop_on_error)
        {
            DBG("Job failed, stopping: " << job.name);
            break;
        }
    }

    return results;
}
