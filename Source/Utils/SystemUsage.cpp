#include <nvml.h>
#include <unistd.h>
#include <sys/resource.h>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
#include <string>
#include <thread>
#include <atomic>
#include <chrono>

class SystemUsage {
public:
    SystemUsage() {
        pid = getpid();
        if (nvmlInit() != NVML_SUCCESS) {
            std::cerr << "Failed to initialize NVML\n";
            nvmlAvailable = false;
        } else {
            if (nvmlDeviceGetHandleByIndex(0, &device) != NVML_SUCCESS) {
                std::cerr << "Failed to get NVML device handle\n";
                nvmlShutdown();
                nvmlAvailable = false;
            } else {
                nvmlAvailable = true;
            }
        }
    }

    ~SystemUsage() {
        if (nvmlAvailable) {
            nvmlShutdown();
        }
    }

    // GPU memory usage in MB
    unsigned int getGpuMemoryUsageMB() {
        if (!nvmlAvailable) return 0;

        unsigned int infoCount = 0;
        nvmlReturn_t result = nvmlDeviceGetGraphicsRunningProcesses(device, &infoCount, nullptr);
        if (result == NVML_ERROR_INSUFFICIENT_SIZE) {
            std::vector<nvmlProcessInfo_t> infos(infoCount);
            result = nvmlDeviceGetGraphicsRunningProcesses(device, &infoCount, infos.data());

            if (result == NVML_SUCCESS) {
                for (const auto& proc : infos) {
                    if (proc.pid == pid) {
                        return proc.usedGpuMemory / (1024 * 1024); // convert to MB
                    }
                }
            }
        }

        return 0;
    }

    // RAM usage in MB
    long getRamUsageMB() {
        struct rusage usage;
        if (getrusage(RUSAGE_SELF, &usage) == 0) {
            return usage.ru_maxrss / 1024; // ru_maxrss is in kilobytes
        }
        return 0;
    }

    // CPU usage percentage (approximate, over a short delay)
    double getCpuUsagePercent() {
        std::ifstream statFile("/proc/stat");
        std::ifstream selfStatFile("/proc/" + std::to_string(pid) + "/stat");

        unsigned long long totalCpu1 = getTotalCpuTime(statFile);
        unsigned long long selfCpu1 = getProcessCpuTime(selfStatFile);

        usleep(100000); // sleep for 100ms

        statFile.clear(); statFile.seekg(0);
        selfStatFile.clear(); selfStatFile.seekg(0);

        unsigned long long totalCpu2 = getTotalCpuTime(statFile);
        unsigned long long selfCpu2 = getProcessCpuTime(selfStatFile);

        if (totalCpu2 == totalCpu1) return 0.0;

        double cpuUsage = 100.0 * (selfCpu2 - selfCpu1) / static_cast<double>(totalCpu2 - totalCpu1);
        return cpuUsage;
    }

private:
    nvmlDevice_t device;
    bool nvmlAvailable = false;
    pid_t pid;

    unsigned long long getTotalCpuTime(std::ifstream& file) {
        std::string line;
        std::getline(file, line);
        std::istringstream iss(line);
        std::string cpu;
        unsigned long long user, nice, system, idle, iowait, irq, softirq, steal;
        iss >> cpu >> user >> nice >> system >> idle >> iowait >> irq >> softirq >> steal;
        return user + nice + system + idle + iowait + irq + softirq + steal;
    }

    unsigned long long getProcessCpuTime(std::ifstream& file) {
        std::string dummy;
        unsigned long utime = 0, stime = 0;
        for (int i = 1; i <= 17; ++i) file >> dummy;
        file >> utime >> stime;
        return utime + stime;
    }
};



class SystemUsageAsync {
public:
    SystemUsageAsync() {
        numCores = std::thread::hardware_concurrency();
        clkTck = sysconf(_SC_CLK_TCK); // clock ticks per second
        running = true;
        usageThread = std::thread(&SystemUsageAsync::updateCpuUsageLoop, this);
    }

    ~SystemUsageAsync() {
        running = false;
        if (usageThread.joinable())
            usageThread.join();
    }

    double getCpuUsagePercent() const {
        return lastCpuUsage.load();
    }

private:
    std::atomic<double> lastCpuUsage{0.0};
    std::atomic<bool> running{false};
    std::thread usageThread;
    unsigned int numCores;
    long clkTck;

    unsigned long long getTotalCpuTime() {
        std::ifstream file("/proc/stat");
        std::string line;
        std::getline(file, line);
        std::istringstream iss(line);
        std::string cpu;
        unsigned long long times[10] = {0};
        iss >> cpu;
        for (int i = 0; i < 10; ++i) iss >> times[i];
        unsigned long long total = 0;
        for (int i = 0; i < 10; ++i) total += times[i];
        return total;
    }

    unsigned long long getProcessCpuTime(pid_t pid) {
        std::ifstream file("/proc/" + std::to_string(pid) + "/stat");
        std::string line;
        std::getline(file, line);

        // Find location of closing ')' because process name may contain spaces
        size_t pos = line.find(')');
        std::istringstream iss(line.substr(pos + 2));

        std::vector<std::string> fields;
        std::string temp;
        while (iss >> temp)
            fields.push_back(temp);

        unsigned long utime = std::stoul(fields[11]);
        unsigned long stime = std::stoul(fields[12]);

        return utime + stime;
    }

    void updateCpuUsageLoop() {
        pid_t pid = getpid();

        while (running) {
            auto total1 = getTotalCpuTime();
            auto proc1 = getProcessCpuTime(pid);

            std::this_thread::sleep_for(std::chrono::milliseconds(500));

            auto total2 = getTotalCpuTime();
            auto proc2 = getProcessCpuTime(pid);

            unsigned long long deltaProc = proc2 - proc1;
            unsigned long long deltaTotal = total2 - total1;

            if (deltaTotal > 0) {
                // Normalize to percentage
                double usage = 100.0 * (double)deltaProc / (double)deltaTotal;
                usage /= numCores;
                lastCpuUsage.store(usage);
            } else {
                lastCpuUsage.store(0.0);
            }
        }
    }
};
