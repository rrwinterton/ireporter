#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <string>
#include <vector>
#include <cstring>
#include "api_exports.h"
#include "upload_engine.h"

// --- Helper for string allocation in Config structs ---
char* DuplicateString(const char* src) {
  if (!src) return nullptr;
#ifdef _WIN32
  return _strdup(src);
#else
  return strdup(src);
#endif
}

// --- MathEngine Implementation ---
struct MathEngine {
  int multiplier;
};

extern "C" {
API_EXPORT EngineHandle CreateMathEngine(int multiplier) {
  return new MathEngine{multiplier};
}

API_EXPORT int MathEngine_Calculate(EngineHandle handle, int input) {
  if (!handle) return 0;
  return static_cast<MathEngine*>(handle)->multiplier * input;
}

API_EXPORT void DestroyMathEngine(EngineHandle handle) {
  delete static_cast<MathEngine*>(handle);
}

// --- CompressEngine Implementation ---
API_EXPORT bool CompressEngine_ParseConfig(int argc, char** argv, CompressEngine_Config* outConfig) {
  if (!outConfig) return false;
  
  // Initialize defaults
  outConfig->inputFilePaths = nullptr;
  outConfig->inputFileCount = 0;
  outConfig->archiveNames = nullptr;
  outConfig->archiveNameCount = 0;
  strncpy(outConfig->outputFilePath, "archive.zip", sizeof(outConfig->outputFilePath) - 1);

  std::vector<char*> inputs;
  std::vector<char*> archives;

  for (int i = 1; i < argc; ++i) {
    if (strcmp(argv[i], "--compress-input") == 0 && i + 1 < argc) {
      inputs.push_back(DuplicateString(argv[++i]));
    } else if (strcmp(argv[i], "--compress-output") == 0 && i + 1 < argc) {
      strncpy(outConfig->outputFilePath, argv[++i], sizeof(outConfig->outputFilePath) - 1);
    } else if (strcmp(argv[i], "--compress-archive") == 0 && i + 1 < argc) {
      archives.push_back(DuplicateString(argv[++i]));
    }
  }

  if (!inputs.empty()) {
    outConfig->inputFileCount = (int)inputs.size();
    outConfig->inputFilePaths = new char*[inputs.size()];
    for (size_t i = 0; i < inputs.size(); ++i) outConfig->inputFilePaths[i] = inputs[i];
  }

  if (!archives.empty()) {
    outConfig->archiveNameCount = (int)archives.size();
    outConfig->archiveNames = new char*[archives.size()];
    for (size_t i = 0; i < archives.size(); ++i) outConfig->archiveNames[i] = archives[i];
  }

  return true;
}

API_EXPORT void CompressEngine_FreeConfig(CompressEngine_Config* config) {
  if (!config) return;
  if (config->inputFilePaths) {
    for (int i = 0; i < config->inputFileCount; ++i) free(config->inputFilePaths[i]);
    delete[] config->inputFilePaths;
  }
  if (config->archiveNames) {
    for (int i = 0; i < config->archiveNameCount; ++i) free(config->archiveNames[i]);
    delete[] config->archiveNames;
  }
}

API_EXPORT EngineHandle CreateCompressEngine() {
  return (EngineHandle)0xCECE; // Dummy handle
}

API_EXPORT bool CompressEngine_CompressFileMapped(EngineHandle handle, const wchar_t** inputFilePaths, int inputFileCount, const wchar_t* outputFilePath, const char** archiveNames, int archiveNameCount) {
  std::wcout << L"[DLL] CompressEngine: Compressing " << inputFileCount << L" files into output file." << std::endl;
  return true;
}

API_EXPORT void DestroyCompressEngine(EngineHandle handle) {}

// --- SocwatchEngine Implementation ---
API_EXPORT bool SocwatchEngine_ParseConfig(int argc, char** argv, SocwatchEngine_Config* outConfig) {
  if (!outConfig) return false;
  std::cerr << "[DLL] SocwatchEngine_ParseConfig called with argc=" << argc << std::endl;
  for(int i=0; i<argc; ++i) std::cerr << "  argv[" << i << "]: " << argv[i] << std::endl;

  outConfig->duration = 30;
  strcpy(outConfig->outputFileName, "socwatch.csv");

  for (int i = 1; i < argc; ++i) {
    if (strcmp(argv[i], "--sw-duration") == 0 && i + 1 < argc) {
      outConfig->duration = (unsigned int)std::stoul(argv[++i]);
    } else if (strcmp(argv[i], "--sw-output") == 0 && i + 1 < argc) {
      strcpy(outConfig->outputFileName, argv[++i]);
    }
  }
  return true;
}

API_EXPORT EngineHandle CreateSocwatchEngine() {
  return (EngineHandle)0x505E; // Dummy handle
}

API_EXPORT const char* SocwatchEngine_Run(EngineHandle handle, unsigned int durationInSeconds, const char* outputFileName) {
  return "Socwatch run complete.";
}

API_EXPORT void DestroySocwatchEngine(EngineHandle handle) {}

// --- PerfEngine Implementation ---
API_EXPORT bool PerfEngine_ParseConfig(int argc, char** argv, PerfEngine_Config* outConfig) {
  if (!outConfig) return false;
  outConfig->isStartTrace = true;
  outConfig->isStopTrace = true;
  outConfig->duration = 30;
  strcpy(outConfig->profileName, "General");
  strcpy(outConfig->profileLevel, "Verbose");
  strcpy(outConfig->etlFile, "trace.etl");

  for (int i = 1; i < argc; ++i) {
    if (strcmp(argv[i], "--profileName") == 0 && i + 1 < argc) {
      strcpy(outConfig->profileName, argv[++i]);
    } else if (strcmp(argv[i], "--profileLevel") == 0 && i + 1 < argc) {
      strcpy(outConfig->profileLevel, argv[++i]);
    } else if (strcmp(argv[i], "--perf-duration") == 0 && i + 1 < argc) {
      outConfig->duration = (unsigned int)std::stoul(argv[++i]);
    } else if (strcmp(argv[i], "--etlFile") == 0 && i + 1 < argc) {
      strcpy(outConfig->etlFile, argv[++i]);
    }
  }
  return true;
}

API_EXPORT EngineHandle CreatePerfEngine() {
  return (EngineHandle)0xBEFE; // Dummy handle
}

API_EXPORT bool PerfEngine_StartTrace(EngineHandle handle, const wchar_t* profileName, const wchar_t* profileLevel, unsigned int duration, const wchar_t* etlFileName) {
  return true;
}

API_EXPORT bool PerfEngine_StopTrace(EngineHandle handle, const wchar_t* etlFileName) {
  return true;
}

API_EXPORT bool PerfEngine_IsRecording(EngineHandle handle) {
  return false;
}

API_EXPORT const char* PerfEngine_GetLastResult(EngineHandle handle) {
  return "PerfEngine result.";
}

API_EXPORT void DestroyPerfEngine(EngineHandle handle) {}

// --- UploadEngine Implementation ---
API_EXPORT bool UploadEngine_ParseConfig(int argc, char** argv, UploadEngine_Config* outConfig) {
  if (!outConfig) return false;
  strcpy(outConfig->location, "C:\\Temp");
  strcpy(outConfig->url, "http://localhost/upload");
  strcpy(outConfig->filePath, "data.txt");

  for (int i = 1; i < argc; ++i) {
    if (strcmp(argv[i], "--upload-location") == 0 && i + 1 < argc) {
      strcpy(outConfig->location, argv[++i]);
    } else if (strcmp(argv[i], "--upload-url") == 0 && i + 1 < argc) {
      strcpy(outConfig->url, argv[++i]);
    } else if (strcmp(argv[i], "--upload-file") == 0 && i + 1 < argc) {
      strcpy(outConfig->filePath, argv[++i]);
    }
  }
  return true;
}

API_EXPORT EngineHandle CreateUploadEngine() {
  return new UploadEngine();
}

API_EXPORT void UploadEngine_SetServerConfig(EngineHandle handle, const char* location, const char* url) {
  if (handle) {
    static_cast<UploadEngine*>(handle)->SetServerConfig(location, url);
  }
}

API_EXPORT bool UploadEngine_UploadFile(EngineHandle handle, const char* filePath) {
  if (handle) {
    return static_cast<UploadEngine*>(handle)->UploadFile(filePath);
  }
  return false;
}

API_EXPORT void DestroyUploadEngine(EngineHandle handle) {
  if (handle) {
    delete static_cast<UploadEngine*>(handle);
  }
}

// --- API Table Implementation ---
static const IProviderAPI globalApiTable = {
    CreateMathEngine,
    MathEngine_Calculate,
    DestroyMathEngine,
    CompressEngine_ParseConfig,
    CompressEngine_FreeConfig,
    CreateCompressEngine,
    CompressEngine_CompressFileMapped,
    DestroyCompressEngine,
    SocwatchEngine_ParseConfig,
    CreateSocwatchEngine,
    SocwatchEngine_Run,
    DestroySocwatchEngine,
    PerfEngine_ParseConfig,
    CreatePerfEngine,
    PerfEngine_StartTrace,
    PerfEngine_StopTrace,
    PerfEngine_IsRecording,
    PerfEngine_GetLastResult,
    DestroyPerfEngine,
    UploadEngine_ParseConfig,
    CreateUploadEngine,
    UploadEngine_SetServerConfig,
    UploadEngine_UploadFile,
    DestroyUploadEngine
};

API_EXPORT const IProviderAPI* GetIProviderAPI() {
    return &globalApiTable;
}
}
