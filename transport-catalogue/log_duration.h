#pragma once

#include <chrono>
#include <iostream>

#define PROFILE_CONCAT_INTERNAL(X, Y) X##Y
#define PROFILE_CONCAT(X, Y) PROFILE_CONCAT_INTERNAL(X, Y)
#define UNIQUE_VAR_NAME_PROFILE PROFILE_CONCAT(duration_counter, __LINE__)
#define LOG_DURATION(x) LogDuration UNIQUE_VAR_NAME_PROFILE(x)

#define OUT_STREAM_NAME(stream_name) stream_name
#define WITH_NAME(x, s) LogDuration UNIQUE_VAR_NAME_PROFILE(x, OUT_STREAM_NAME(s))
#define WITHOUT_NAME(s) LogDuration UNIQUE_VAR_NAME_PROFILE(OUT_STREAM_NAME(s))
#define VARIABLE_ARGS(_1, _2, NAME, ...) NAME
#define LOG_DURATION_STREAM(...) VARIABLE_ARGS(__VA_ARGS__, WITH_NAME, WITHOUT_NAME)(__VA_ARGS__)


template <typename T>
class LogDuration
{
public:
    using Clock = std::chrono::steady_clock;

    explicit LogDuration(const T&id, std::ostream &out = std::cerr)
        : id_(id), out_(out)
    {
        using namespace std;
    }
    explicit LogDuration(std::ostream &out)
        : id_("Operation time"), out_(out)
    {
        using namespace std;
    }

    ~LogDuration()
    {
        using namespace std::chrono;
        using namespace std::literals;

        const auto end_time = Clock::now();
        const auto dur = end_time - start_time_;
        out_ << id_ << ": "s << duration_cast<milliseconds>(dur).count() << " ms"s << std::endl;
    }

private:
    const std::string id_;
    std::ostream &out_;
    const Clock::time_point start_time_ = Clock::now();
};
