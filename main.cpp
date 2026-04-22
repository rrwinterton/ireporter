#include "ireporter.h"
#include <iostream>

int main() {
    std::cout << "Starting Telemetry Reporter..." << std::endl;

    // Call independent sections
    std::string machineName = GetMachineName();
    std::string machineGuid = GetMachineGuid();
    uint64_t uptime = GetMachineUptime();

    std::cout << "Machine Name: " << machineName << std::endl;
    std::cout << "Machine GUID: " << machineGuid << std::endl;
    std::cout << "System Uptime: " << uptime << " seconds" << std::endl;

    std::cout << "Sending system data..." << std::endl;
    SendSystemData();

    return 0;
}
