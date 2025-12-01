#include "Jobs.h"
#include "TableHelper.h"

USE_MODULE(Arcana::Jobs);


Job::Job() 
{}


Job Job::FromInstruction(Semantic::Enviroment& environment, const Semantic::Instruction& instruction) noexcept
{
    Job new_job {};

    if (auto task = dynamic_cast<const Semantic::InstructionTask*>(&instruction); task)
    {
        new_job.name         = task->task_name;
        new_job.interpreter  = environment.GetInterpreter();
        new_job.parameters   = task->task_params;
        new_job.instructions = task->task_instrs;
        
        if (task->hasAttribute(Semantic::Attr::Type::INTERPRETER))
        {
            new_job.interpreter = task->getProperties(Semantic::Attr::Type::INTERPRETER).value().get()[0];
        }
    }
    else if (auto call = dynamic_cast<const Semantic::InstructionCall*>(&instruction); call)
    {
        DBG("CALL");
    }

    return new_job;
}



JobList Jobs::FromEnv(Semantic::Enviroment& environment) noexcept
{
    JobList jobs {};

    auto main_task = Table::GetValue(environment.ftable, Semantic::Attr::Type::MAIN);

    if (main_task)
    {
        jobs.push_back(Job::FromInstruction(environment, main_task.value()));
    }

    return jobs;
}
