#include <nvml.h>
class GpuUsage
{
public:
        
    nvmlDevice_t device;
    GpuUsage()
    {
        nvmlInit();
        nvmlDeviceGetHandleByIndex(0, &device); // GPU 0
    }

    ~GpuUsage()
    {
        nvmlShutdown();
    }

    float get_usage()
    {
        nvmlUtilization_t utilization;
        nvmlDeviceGetUtilizationRates(device, &utilization);
        return (float)utilization.gpu; // In percentage
    }
};