#include "Core.h"
#include "Cache.h"
#include "Semantic.h"

#include <mutex>

USE_MODULE(Arcana);



using SymbolMap = Support::AbstractKeywordMap<std::string>;



static SymbolMap builtin_symbols = 
{
    { "__main__"       , "None"                                              },
    { "__root__"       , std::filesystem::current_path().string()            },
    { "__version__"    , __ARCANA__VERSION__                                 },
    { "__profile__"    , "None"                                              },
    { "__threads__"    , "None"                                              },
    { "__max_threads__", std::to_string(std::thread::hardware_concurrency()) },

#if defined(_WIN32)
    { "__os__"         , "windows"  },
#elif defined(__APPLE__) && defined(__MACH__)
    { "__os__"         , "macos"    },
#elif defined(__linux__)
    { "__os__"         , "linux"    },
#elif defined(__FreeBSD__)
    { "__os__"         , "freeBSD"  },
#elif defined(__unix__)
    { "__os__"         , "unix"     },
#else
    { "__os__"         , "unknown"  },
#endif

#if defined(__x86_64__) || defined(_M_X64)
    { "__arch__"       , "x86_64"   },
#elif defined(__i386__) || defined(_M_IX86)
    { "__arch__"       , "x86"      },
#elif defined(__aarch64__) || defined(_M_ARM64)
    { "__arch__"       , "aarch64"  },
#elif defined(__arm__) || defined(_M_ARM)
    { "__arch__"       , "arm"      },
#elif defined(__riscv) || defined(__riscv__)
    { "__arch__"       , "riscv"    },
#elif defined(__powerpc64__) || defined(__ppc64__)
    { "__arch__"       , "ppc64"    },
#elif defined(__powerpc__) || defined(__ppc__)
    { "__arch__"       , "ppc"      },
#else
    { "__arch__"       , "unknown"  },
#endif
};


static std::map<Core::SymbolType, std::string> Known_Symbols_By_Token = 
{
    { Core::SymbolType::MAIN       , "__main__"        },
    { Core::SymbolType::ROOT       , "__root__"        },
    { Core::SymbolType::VERSION    , "__version__"     },
    { Core::SymbolType::PROFILE    , "__profile__"     },
    { Core::SymbolType::THREADS    , "__threads__"     },
    { Core::SymbolType::MAX_THREADS, "__max_threads__" },
    { Core::SymbolType::OS         , "__os__"          },
    { Core::SymbolType::ARCH       , "__arch__"        },
};


static std::map<std::string, Core::SymbolType> Known_Symbols_By_String = 
{
    { "__main__"        , Core::SymbolType::MAIN        },
    { "__root__"        , Core::SymbolType::ROOT        },
    { "__version__"     , Core::SymbolType::VERSION     },
    { "__profile__"     , Core::SymbolType::PROFILE     },
    { "__threads__"     , Core::SymbolType::THREADS     },
    { "__max_threads__" , Core::SymbolType::MAX_THREADS },
    { "__os__"          , Core::SymbolType::OS          },
    { "__arch__"        , Core::SymbolType::ARCH        },
};



static std::vector<std::string> Known_OSs = 
{
    "windows",
    "macos"  ,
    "linux"  ,
    "freeBSD",
    "unix"   ,
};


static std::vector<std::string> Known_ARCHs = 
{
    "x86_64" ,
    "x86"    ,
    "aarch64",
    "arm"    ,
    "riscv"  ,
    "ppc64"  ,
    "ppc"    ,
};



//    ██████╗ ██████╗ ██╗██╗   ██╗ █████╗ ████████╗███████╗███████╗
//    ██╔══██╗██╔══██╗██║██║   ██║██╔══██╗╚══██╔══╝██╔════╝██╔════╝
//    ██████╔╝██████╔╝██║██║   ██║███████║   ██║   █████╗  ███████╗
//    ██╔═══╝ ██╔══██╗██║╚██╗ ██╔╝██╔══██║   ██║   ██╔══╝  ╚════██║
//    ██║     ██║  ██║██║ ╚████╔╝ ██║  ██║   ██║   ███████╗███████║
//    ╚═╝     ╚═╝  ╚═╝╚═╝  ╚═══╝  ╚═╝  ╚═╝   ╚═╝   ╚══════╝╚══════╝
//                                                                 

static Core::InstructionResult run_instruction(const std::string& jobname, 
                                               const std::size_t  idx,  
                                               const std::string& interpreter, 
                                               const std::string& command, 
                                               const bool         echo) noexcept
{
    Core::InstructionResult res { command, 0 };

    if (echo)
    {
        MSG(command);
    }

    Cache::Manager&         cache    = Cache::Manager::Instance();
    std::filesystem::path   script   = cache.WriteScript(jobname, idx, command);
    std::string             full_cmd = interpreter + " \"" + script.string() + "\"";
    
    int ret = std::system(full_cmd.c_str());

    if (ret == -1)
    {
        res.exit_code = 127;
    }
    else
    {
        res.exit_code = ret;
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
        std::size_t idx = 0;

        // LINEAR EXECUTION OF JOB: NO MT
        for (const auto &cmd : job.instructions)
        {
            auto r = run_instruction(job.name, idx, job.interpreter, cmd, job.echo);
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

            ++idx;
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
            auto        r   = run_instruction(job.name, idx, job.interpreter, cmd, job.echo);

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
    bool ok = true;
    std::vector<Core::Result> results;

    results.reserve(jobs.All().size());

    // RUN EACH JOB 
    for (auto &job : jobs.All())
    {
        if (!opt.silent)
        {
            ARC(ANSI_GRAY << "Running task: "  << job.name << ANSI_RESET);
        }

        auto r = run_job(job, opt);
        results.push_back(r);

        if (!r.ok && opt.stop_on_error)
        {
            ok = false;
            ERR(ANSI_GRAY << "Task failed: " << job.name << ANSI_RESET);
            break;
        }
    }

    if (ok && !opt.silent)
    {
        ARC("Action '" << jobs.main_job << "' done!");
    }

    return results;
}


std::string& Core::symbol(Core::SymbolType type) noexcept
{
    const std::string symbol = Known_Symbols_By_Token[type];

    return builtin_symbols[symbol];
}



Core::SymbolType Core::is_symbol(const std::string& symbol) noexcept
{
    Core::SymbolType type = Core::SymbolType::UNDEFINED;

    if (auto it = Known_Symbols_By_String.find(symbol); it != Known_Symbols_By_String.end())
    {
        type = it->second;
    }

    return type;
}



void Core::update_symbol(Core::SymbolType type, const std::string& val) noexcept
{
    const std::string symbol = Known_Symbols_By_Token[type];

    builtin_symbols[symbol] = val;

    return;
}


bool Core::is_os(const std::string& param) noexcept
{
    return (std::find(Known_OSs.begin(), Known_OSs.end(), param) != Known_OSs.end());
}



bool Core::is_arch(const std::string& param) noexcept
{
    return (std::find(Known_ARCHs.begin(), Known_ARCHs.end(), param) != Known_ARCHs.end());
}




