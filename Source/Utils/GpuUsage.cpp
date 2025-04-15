#include <nvml.h>
#include <unistd.h> // for getpid()
#include <vector>
#include <iostream>

class GpuUsage {
public:
    nvmlDevice_t device;

    GpuUsage() {
        nvmlInit();
        nvmlDeviceGetHandleByIndex(0, &device); // GPU 0
    }

    ~GpuUsage() {
        nvmlShutdown();
    }

    unsigned int get_usage() {
        unsigned int infoCount = 100;
        std::vector<nvmlProcessInfo_t> infos(infoCount);

        nvmlReturn_t result = nvmlDeviceGetGraphicsRunningProcesses(device, &infoCount, infos.data());
        if (result == NVML_ERROR_INSUFFICIENT_SIZE) {
            infos.resize(infoCount);
            result = nvmlDeviceGetGraphicsRunningProcesses(device, &infoCount, infos.data());
        }

        if (result != NVML_SUCCESS) return 0;

        unsigned int pid = getpid();
        for (const auto& proc : infos) {
            if (proc.pid == pid) {
                return proc.usedGpuMemory / (1024 * 1024); // convert to MB
            }
        }

        return 0; // Not found
    }
};

// #include <nvml.h>
// class GpuUsage
// {
// public:
        
//     nvmlDevice_t device;
//     GpuUsage()
//     {
//         nvmlInit();
//         nvmlDeviceGetHandleByIndex(0, &device); // GPU 0
//     }

//     ~GpuUsage()
//     {
//         nvmlShutdown();
//     }

//     float get_usage()
//     {
//         nvmlUtilization_t utilization;
//         nvmlDeviceGetUtilizationRates(device, &utilization);
//         return (float)utilization.gpu; // In percentage
//     }
// };