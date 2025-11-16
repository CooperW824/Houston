#ifndef LOAD_ENV_HPP
#define LOAD_ENV_HPP

#include <fstream>
#include <string>
#include <cstdlib>
#include <sstream>
#include <algorithm>
#include <cctype>
#include <iostream>

namespace env {

// Trim helper functions
inline std::string ltrim(const std::string &s) {
    size_t start = s.find_first_not_of(" \t\r\n");
    return (start == std::string::npos) ? "" : s.substr(start);
}

inline std::string rtrim(const std::string &s) {
    size_t end = s.find_last_not_of(" \t\r\n");
    return (end == std::string::npos) ? "" : s.substr(0, end + 1);
}

inline std::string trim(const std::string &s) {
    return rtrim(ltrim(s));
}

// Load environment variables from a .env file
inline void load_env_file(const std::string& path = ".env") {
    std::ifstream file(path);
    if (!file.is_open()) {
        std::cerr << "Warning: Could not open .env file at " << path << "\n";
        return;
    }

    std::string line;
    while (std::getline(file, line)) {
        line = trim(line);
        if (line.empty() || line[0] == '#') continue; // skip comments and empty lines

        size_t eq = line.find('=');
        if (eq == std::string::npos) continue;

        std::string key = trim(line.substr(0, eq));
        std::string val = trim(line.substr(eq + 1));

        // Remove surrounding quotes if present
        if (!val.empty() && val.front() == '"' && val.back() == '"')
            val = val.substr(1, val.size() - 2);

        // Set the environment variable
        setenv(key.c_str(), val.c_str(), 1);
    }
}

} // namespace env

#endif // LOAD_ENV_HPP
