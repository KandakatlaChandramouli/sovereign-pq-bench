#include "sovereign/metrics/system_info.hpp"
#include <chrono>
#include <ctime>
#include <fstream>
#include <sstream>
#include <string>

#ifdef __linux__
#include <sys/utsname.h>
#endif

namespace sovereign {

SystemInfo SystemInfo::collect() {
    SystemInfo info;

    info.timestamp = []() {
        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        std::ostringstream oss;
        oss << std::ctime(&time);
        std::string ts = oss.str();
        if (!ts.empty() && ts.back() == '\n') {
            ts.pop_back();
        }
        return ts;
    }();

#ifdef __linux__
    std::ifstream cpuinfo("/proc/cpuinfo");
    if (cpuinfo.is_open()) {
        std::string line;
        while (std::getline(cpuinfo, line)) {
            if (line.find("model name") != std::string::npos) {
                auto pos = line.find(": ");
                if (pos != std::string::npos) {
                    info.cpu_model = line.substr(pos + 2);
                }
                break;
            }
        }
    }

    std::ifstream meminfo("/proc/meminfo");
    if (meminfo.is_open()) {
        std::string line;
        while (std::getline(meminfo, line)) {
            if (line.find("MemTotal") != std::string::npos) {
                auto pos = line.find(": ");
                if (pos != std::string::npos) {
                    info.total_ram = line.substr(pos + 2);
                }
                break;
            }
        }
    }

    utsname sys_info{};
    if (uname(&sys_info) == 0) {
        info.os_name = sys_info.sysname;
        info.os_version = sys_info.release;
        info.kernel_version = sys_info.version;
    }
#endif

#ifdef __GNUC__
    info.compiler_name = "GCC";
    info.compiler_version = std::to_string(__GNUC__) + "." + 
                           std::to_string(__GNUC_MINOR__) + "." + 
                           std::to_string(__GNUC_PATCHLEVEL__);
#elif defined(__clang__)
    info.compiler_name = "Clang";
    info.compiler_version = std::to_string(__clang_major__) + "." + 
                           std::to_string(__clang_minor__) + "." + 
                           std::to_string(__clang_patchlevel__);
#endif

#ifdef NDEBUG
    info.build_type = "Release";
#else
    info.build_type = "Debug";
#endif

    info.git_commit = "unknown";

    return info;
}

std::string SystemInfo::to_string() const {
    std::ostringstream oss;
    oss << "CPU: " << cpu_model << "\n"
        << "OS: " << os_name << " " << os_version << "\n"
        << "Kernel: " << kernel_version << "\n"
        << "RAM: " << total_ram << "\n"
        << "Compiler: " << compiler_name << " " << compiler_version << "\n"
        << "Build: " << build_type << "\n"
        << "Timestamp: " << timestamp;
    return oss.str();
}

}
