#include "benchmark_memory.hpp"

#if defined(_WIN32)
#include <windows.h>
#include <psapi.h>
#elif defined(__linux__)
#include <fstream>
#include <sys/resource.h>
#include <unistd.h>
#elif defined(__APPLE__)
#include <mach/mach.h>
#include <sys/resource.h>
#endif

MemoryUsage getMemoryUsage() noexcept
{
    MemoryUsage result;

#if defined(_WIN32)
    PROCESS_MEMORY_COUNTERS_EX counters{};
    counters.cb = sizeof(counters);

    if (GetProcessMemoryInfo(
        GetCurrentProcess(),
        reinterpret_cast<PROCESS_MEMORY_COUNTERS*>(&counters),
        sizeof(counters)))
    {
        result.residentBytes = static_cast<std::size_t>(counters.WorkingSetSize);
        result.peakResidentBytes = static_cast<std::size_t>(counters.PeakWorkingSetSize);
        result.privateBytes = static_cast<std::size_t>(counters.PrivateUsage);
    }
#elif defined(__linux__)
    std::ifstream statm("/proc/self/statm");
    std::size_t totalPages = 0;
    std::size_t residentPages = 0;

    if (statm >> totalPages >> residentPages)
    {
        const long pageSize = sysconf(_SC_PAGESIZE);
        if (pageSize > 0)
        {
            result.residentBytes = residentPages * static_cast<std::size_t>(pageSize);
        }
    }

    rusage usage{};
    if (getrusage(RUSAGE_SELF, &usage) == 0)
    {
        result.peakResidentBytes =
            static_cast<std::size_t>(usage.ru_maxrss) * 1024U;
    }
#elif defined(__APPLE__)
    mach_task_basic_info info{};
    mach_msg_type_number_t count = MACH_TASK_BASIC_INFO_COUNT;

    if (task_info(
        mach_task_self(),
        MACH_TASK_BASIC_INFO,
        reinterpret_cast<task_info_t>(&info),
        &count) == KERN_SUCCESS)
    {
        result.residentBytes = static_cast<std::size_t>(info.resident_size);
    }

    rusage usage{};
    if (getrusage(RUSAGE_SELF, &usage) == 0)
    {
        result.peakResidentBytes = static_cast<std::size_t>(usage.ru_maxrss);
    }
#endif

    return result;
}
