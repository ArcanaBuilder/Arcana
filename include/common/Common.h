#ifndef __ARCANA_COMMON_H__
#define __ARCANA_COMMON_H__


#include <string>
#include <chrono>
#include <sstream>



class Stopwatch
{
public:
    using clock = std::chrono::steady_clock;

    void start()
    {
        running_ = true;
        t0_      = clock::now();
    }

    void stop()
    {
        if (running_)
        {
            t1_      = clock::now();
            running_ = false;
        }
    }

    template <typename Dur = std::chrono::milliseconds>
    long long elapsed() const
    {
        auto end = running_ ? clock::now() : t1_;
        return std::chrono::duration_cast<Dur>(end - t0_).count();
    }


    static std::string format(long long time)
    {
        std::stringstream ss;
        if (time < 1000)
        {   
            ss << time << " milliseconds";
            return ss.str();
        }
        else
        {
            double formatted = (double) time / 1000;
            ss << formatted << " seconds";
            return ss.str();
        }
    }

private:
    clock::time_point t0_{};
    clock::time_point t1_{};
    bool              running_{false};
};







#endif /* __ARCANA_COMMON_H__ */