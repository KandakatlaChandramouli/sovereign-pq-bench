#pragma once

#include <string>

namespace sovereign {

struct SystemInfo {
    std::string cpu_model;
    std::string cpu_cores;
    std::string cpu_frequency;
    std::string total_ram;
    std::string os_name;
    std::string os_version;
    std::string kernel_version;
    std::string compiler_name;
    std::string compiler_version;
    std::string build_type;
    std::string git_commit;
    std::string timestamp;

    [[nodiscard]] static SystemInfo collect();
    [[nodiscard]] std::string to_string() const;
};

}
