#include "Jobs.h"
#include "TableHelper.h"

USE_MODULE(Arcana::Jobs);


Job::Job() 
    :
    name(""),
    parallelizable(false)
{}


GeneratedJobs Job::FromInstruction(const Semantic::InstructionTask& task) noexcept
{
    auto get_job = [] (const Semantic::InstructionTask& task) noexcept -> GeneratedJobs
    {
        Job new_job {};

        new_job.name         = task.task_name;
        new_job.interpreter  = task.interpreter;
        new_job.instructions = task.task_instrs;

        if (!new_job.instructions.size())
        {
            return std::nullopt;
        }

        return new_job;
    };

    return std::nullopt;
}






List::List()
{}


void List::Insert(Job& j)
{
    auto [it, inserted] = index.insert(j.name);
    if (inserted)
    {
        data.push_back(j);
    }
}


void List::Insert(std::vector<Job>& vj)
{
    for (const auto& j : vj)
    {
        Insert(j);
    }
}


void List::Insert(GeneratedJobs gj)
{
    if (!gj)
    {
        return;
    }

    std::visit
    (
        [&] (auto&& value) { Insert(value); }, 
        *gj
    );
}


List List::FromEnv(Semantic::Enviroment& environment) noexcept
{
    List jobs{};

    for (const auto& pre_task : environment.pretask)
    {
        jobs.Insert(Job::FromInstruction(pre_task));
    }

    auto main_task = Table::GetValue(environment.ftable, Semantic::Attr::Type::MAIN);

    if (main_task)
    {
        jobs.Insert(Job::FromInstruction(main_task.value()));
    }

    auto always = Table::GetValues(environment.ftable, Semantic::Attr::Type::ALWAYS);

    if (always)
    {
        for (const auto& task : always.value())
        {
            jobs.Insert(Job::FromInstruction(task));
        }
    }

    for (const auto& post_task : environment.posttask)
    {
        jobs.Insert(Job::FromInstruction(post_task));
    }

    return jobs;
}
