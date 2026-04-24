#ifndef IREPORTER_H
#define IREPORTER_H

#include <cstdint>
#include <string>
#include <vector>

// Telemetry collection functions
std::string GetMachineName();
std::string GetMachineGuid();
uint64_t GetMachineUptime();

// Reporting functions
int ImportProviders();
bool ValidateCompressConfig(int argc, char** argv);
bool ValidateSocwatchConfig(int argc, char** argv);
bool ValidatePerfConfig(int argc, char** argv);
void RunMathEngine(int multiplier, int input);
void RunSocwatchEngine(unsigned int duration, const std::string& outputFileName);
void RunCompressEngine(const std::vector<std::string>& inputs,
                        const std::string& output,
                        const std::vector<std::string>& archives);
void RunPerfEngine(const std::string& profileName,

                   const std::string& profileLevel,
                   unsigned int duration,
                   const std::string& etlFileName);
void SendSystemData();

#endif  // IREPORTER_H
