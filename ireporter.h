#ifndef IREPORTER_H
#define IREPORTER_H

#include <string>
#include <cstdint>

// Telemetry collection functions
std::string GetMachineName();
std::string GetMachineGuid();
uint64_t GetMachineUptime();

// Reporting functions
void SendSystemData();

#endif // IREPORTER_H
