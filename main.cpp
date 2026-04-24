#include <iostream>
#include "CLI.hpp"
#include "ireporter.h"

int main(int argc, char* argv[]) {
  CLI::App app{"Telemetry Reporter"};

  bool localOnly = false;
  app.add_flag("--localOnly", localOnly, "keep data on local machine (true)");

  bool runMath = false;
  int multiplier = 1;
  int mathInput = 0;
  app.add_flag("--math", runMath, "Run the MathEngine test");
  app.add_option("--multiplier", multiplier, "Multiplier for MathEngine")
      ->default_val(1);
  app.add_option("--math-input", mathInput, "Input value for MathEngine")
      ->default_val(0);

  bool runSocwatch = false;
  unsigned int swDuration = 30;
  std::string swOutput = "socwatch.csv";
  app.add_flag("--socwatch", runSocwatch, "Run the SocwatchEngine");
  app.add_option("--sw-duration", swDuration,
                 "Duration for SocwatchEngine (seconds)")
      ->default_val(30);
  app.add_option("--sw-output", swOutput,
                 "Output filename for SocwatchEngine")
      ->default_val("socwatch.csv");

  bool runPerf = false;
  std::string profileName = "General";
  std::string profileLevel = "Verbose";
  unsigned int perfDuration = 30;
  std::string etlFileName = "trace.etl";
  app.add_flag("--perf", runPerf, "Run the PerfEngine");
  app.add_option("--profileName", profileName, "Profile name for PerfEngine")
      ->default_val("General");
  app.add_option("--profileLevel", profileLevel, "Profile level for PerfEngine")
      ->default_val("Verbose");
  app.add_option("--perf-duration", perfDuration,
                 "Duration for PerfEngine (seconds)")
      ->default_val(30);
  app.add_option("--etlFile", etlFileName, "ETL filename for PerfEngine")
      ->default_val("trace.etl");

      bool runCompress = false;
  std::vector<std::string> compressInputs;
  std::string compressOutput = "archive.zip";
  std::vector<std::string> compressArchives;
  app.add_flag("--compress", runCompress, "Run the CompressEngine");
  app.add_option("--compress-input", compressInputs,
                 "Input file paths for CompressEngine");
  app.add_option("--compress-output", compressOutput,
                 "Output file path for CompressEngine")
      ->default_val("archive.zip");
  app.add_option("--compress-archive", compressArchives,
                 "Archive names for CompressEngine");

  CLI11_PARSE(app, argc, argv);

  ImportProviders();

  if (runSocwatch && !ValidateSocwatchConfig(argc, argv)) {
    std::cerr << "Invalid SocwatchEngine configuration." << std::endl;
    return 1;
  }
  if (runPerf && !ValidatePerfConfig(argc, argv)) {
    std::cerr << "Invalid PerfEngine configuration." << std::endl;
    return 1;
  }
  if (runCompress && !ValidateCompressConfig(argc, argv)) {
    std::cerr << "Invalid CompressEngine configuration." << std::endl;
    return 1;
  }
  if (runMath) {
    RunMathEngine(multiplier, mathInput);
  }
  if (runSocwatch) {
    RunSocwatchEngine(swDuration, swOutput);
  }
  if (runPerf) {
    RunPerfEngine(profileName, profileLevel, perfDuration, etlFileName);
  }
  if (runCompress) {
    RunCompressEngine(compressInputs, compressOutput, compressArchives);
  }

  std::cout << "Starting Telemetry Reporter..." << std::endl;

  // Call independent sections
  std::string machineName = GetMachineName();
  std::string machineGuid = GetMachineGuid();
  uint64_t uptime = GetMachineUptime();

  std::cout << "Machine Name: " << machineName << std::endl;
  std::cout << "Machine GUID: " << machineGuid << std::endl;
  std::cout << "System Uptime: " << uptime << " seconds" << std::endl;

  if (localOnly == false) {
    std::cout << "Sending system data..." << std::endl;
    SendSystemData();
  }
  return 0;
}
