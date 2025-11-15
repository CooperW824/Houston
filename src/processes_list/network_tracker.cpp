#include "network_tracker.hpp"
#include <fstream>
#include <sstream>
#include <dirent.h>
#include <unistd.h>
#include <cstring>
#include <sys/stat.h>

NetworkTracker& NetworkTracker::getInstance() {
    static NetworkTracker instance;
    return instance;
}

unsigned long NetworkTracker::getSocketBytesForPid(pid_t pid) {
    unsigned long total_bytes = 0;

    std::stringstream fd_path;
    fd_path << "/proc/" << pid << "/fd";

    DIR* dir = opendir(fd_path.str().c_str());
    if (!dir) {
        return 0;
    }

    struct dirent* entry;
    while ((entry = readdir(dir)) != nullptr) {
        if (entry->d_name[0] == '.') continue;

        std::stringstream link_path;
        link_path << fd_path.str() << "/" << entry->d_name;

        char link_target[256];
        ssize_t len = readlink(link_path.str().c_str(), link_target, sizeof(link_target) - 1);
        if (len > 0) {
            link_target[len] = '\0';
            if (strstr(link_target, "socket:") != nullptr) {
                total_bytes++;
            }
        }
    }

    closedir(dir);

    std::stringstream io_path;
    io_path << "/proc/" << pid << "/io";
    std::ifstream io_file(io_path.str());

    if (io_file.is_open()) {
        std::string line;
        unsigned long read_bytes = 0, write_bytes = 0;

        while (std::getline(io_file, line)) {
            if (line.find("read_bytes:") == 0) {
                sscanf(line.c_str(), "read_bytes: %lu", &read_bytes);
            } else if (line.find("write_bytes:") == 0) {
                sscanf(line.c_str(), "write_bytes: %lu", &write_bytes);
            }
        }

        if (total_bytes > 0) {
            total_bytes = (read_bytes + write_bytes) / 1024;
        } else {
            total_bytes = 0;
        }
    }

    return total_bytes;
}

unsigned long NetworkTracker::getProcessNetworkUsage(pid_t pid) {
    std::lock_guard<std::mutex> lock(tracker_mutex);

    unsigned long current_bytes = getSocketBytesForPid(pid);
    unsigned long last = last_bytes[pid];

    unsigned long delta = 0;
    if (current_bytes >= last) {
        delta = current_bytes - last;
    } else {
        delta = current_bytes;
    }

    last_bytes[pid] = current_bytes;
    process_bytes[pid] = delta;

    return delta;
}

