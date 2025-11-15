#ifndef __NETWORK_TRACKER_HPP
#define __NETWORK_TRACKER_HPP

#include <sys/types.h>
#include <map>
#include <mutex>

class NetworkTracker {
public:
    static NetworkTracker& getInstance();

    unsigned long getProcessNetworkUsage(pid_t pid);

private:
    NetworkTracker() = default;
    std::map<pid_t, unsigned long> process_bytes;
    std::map<pid_t, unsigned long> last_bytes;
    std::mutex tracker_mutex;

    unsigned long getSocketBytesForPid(pid_t pid);
};

#endif

