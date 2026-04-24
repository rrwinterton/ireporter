#pragma once

// Handle Windows DLL export/import semantics
#ifdef _WIN32
#ifdef UNMANGLED_LIB_EXPORTS
#define API_EXPORT __declspec(dllexport)
#else
#define API_EXPORT __declspec(dllimport)
#endif
#else
// Fallback for Linux/macOS if you cross-compile later
#define API_EXPORT __attribute__((visibility("default")))
#endif

// Define an opaque type for our C++ class pointer
typedef void* EngineHandle;

// The extern "C" block disables C++ name mangling for these functions
extern "C" {
// rrw The Math Engine is for test purposes only and will be removed at some
// point
// 1. Function to create the class
API_EXPORT EngineHandle CreateMathEngine(int multiplier);

// 2. Function to use the class
API_EXPORT int MathEngine_Calculate(EngineHandle handle, int input);

// 3. Function to destroy the class (prevents memory leaks)
API_EXPORT void DestroyMathEngine(EngineHandle handle);

// --- CompressEngine Exports ---

// 4a. Configuration struct for CompressEngine
typedef struct {
    bool doCompress;
    char** inputFilePaths;
    int inputFileCount;
    char outputFilePath[260];
    char** archiveNames;
    int archiveNameCount;
} CompressEngine_Config;

// 4b. Function to parse command line arguments for CompressEngine
API_EXPORT bool CompressEngine_ParseConfig(int argc, char** argv, CompressEngine_Config* outConfig);

// 4c. Function to free memory allocated by CompressEngine_ParseConfig
API_EXPORT void CompressEngine_FreeConfig(CompressEngine_Config* config);

// 4. Function to create the CompressEngine
API_EXPORT EngineHandle CreateCompressEngine();

// 5. Function to use the CompressEngine
API_EXPORT bool CompressEngine_CompressFileMapped(EngineHandle handle,
                                                  const wchar_t** inputFilePaths,
                                                  int inputFileCount,
                                                  const wchar_t* outputFilePath,
                                                  const char** archiveNames,
                                                  int archiveNameCount);

// 6. Function to destroy the CompressEngine
API_EXPORT void DestroyCompressEngine(EngineHandle handle);

// --- SocwatchEngine Exports ---

// 7a. Configuration struct for SocwatchEngine
typedef struct {
    bool doSocwatch;
    unsigned int duration;
    char outputFileName[260];
} SocwatchEngine_Config;

// 7b. Function to parse command line arguments for SocwatchEngine
API_EXPORT bool SocwatchEngine_ParseConfig(int argc, char** argv, SocwatchEngine_Config* outConfig);

// 7. Function to create the SocwatchEngine
API_EXPORT EngineHandle CreateSocwatchEngine();

// 8. Function to use the SocwatchEngine
API_EXPORT const char* SocwatchEngine_Run(EngineHandle handle, unsigned int durationInSeconds, const char* outputFileName);

// 9. Function to destroy the SocwatchEngine
API_EXPORT void DestroySocwatchEngine(EngineHandle handle);

// --- PerfEngine Exports ---

// 10. Configuration struct for PerfEngine
typedef struct {
    bool perf;
    bool localOnly;
    bool isStartTrace;
    char profileName[256];
    char profileLevel[256];
    unsigned int duration;
    bool isStopTrace;
    char etlFile[260];
} PerfEngine_Config;

// 11. Function to parse command line arguments for PerfEngine
API_EXPORT bool PerfEngine_ParseConfig(int argc, char** argv, PerfEngine_Config* outConfig);

// 12. Function to create the PerfEngine
API_EXPORT EngineHandle CreatePerfEngine();

// 13. Function to start a trace
API_EXPORT bool PerfEngine_StartTrace(EngineHandle handle,
                                      const wchar_t* profileName,
                                      const wchar_t* profileLevel,
                                      unsigned int duration,
                                      const wchar_t* etlFileName);

// 14. Function to stop a trace and save to ETL
API_EXPORT bool PerfEngine_StopTrace(EngineHandle handle,
                                     const wchar_t* etlFileName);

// 15. Function to check if recording is active
API_EXPORT bool PerfEngine_IsRecording(EngineHandle handle);

// 16. Function to get the last result message
API_EXPORT const char* PerfEngine_GetLastResult(EngineHandle handle);

// 22. Function to destroy the PerfEngine
API_EXPORT void DestroyPerfEngine(EngineHandle handle);

// --- Dynamic Loading API ---

typedef struct {
    // Math Engine
    EngineHandle (*CreateMathEngine)(int multiplier);
    int (*MathEngine_Calculate)(EngineHandle handle, int input);
    void (*DestroyMathEngine)(EngineHandle handle);

    // Compress Engine
    bool (*CompressEngine_ParseConfig)(int argc, char** argv, CompressEngine_Config* outConfig);
    void (*CompressEngine_FreeConfig)(CompressEngine_Config* config);
    EngineHandle (*CreateCompressEngine)();
    bool (*CompressEngine_CompressFileMapped)(EngineHandle handle, const wchar_t** inputFilePaths, int inputFileCount, const wchar_t* outputFilePath, const char** archiveNames, int archiveNameCount);
    void (*DestroyCompressEngine)(EngineHandle handle);

    // Socwatch Engine
    bool (*SocwatchEngine_ParseConfig)(int argc, char** argv, SocwatchEngine_Config* outConfig);
    EngineHandle (*CreateSocwatchEngine)();
    const char* (*SocwatchEngine_Run)(EngineHandle handle, unsigned int durationInSeconds, const char* outputFileName);
    void (*DestroySocwatchEngine)(EngineHandle handle);

    // Perf Engine
    bool (*PerfEngine_ParseConfig)(int argc, char** argv, PerfEngine_Config* outConfig);
    EngineHandle (*CreatePerfEngine)();
    bool (*PerfEngine_StartTrace)(EngineHandle handle, const wchar_t* profileName, const wchar_t* profileLevel, unsigned int duration, const wchar_t* etlFileName);
    bool (*PerfEngine_StopTrace)(EngineHandle handle, const wchar_t* etlFileName);
    bool (*PerfEngine_IsRecording)(EngineHandle handle);
    const char* (*PerfEngine_GetLastResult)(EngineHandle handle);
    void (*DestroyPerfEngine)(EngineHandle handle);
} IProviderAPI;

// Function to retrieve the API structure
API_EXPORT const IProviderAPI* GetIProviderAPI();
}
