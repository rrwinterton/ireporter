#include "ireporter.h"
#include <windows.h>
#include <winhttp.h>
#include <winreg.h>
#include <iostream>
#include <string>
#include <vector>
#include "api_exports.h"

// Link against necessary Windows libraries
#pragma comment(lib, "winhttp.lib")
#pragma comment(lib, "advapi32.lib")

// Global API Table Pointer
const IProviderAPI* g_pApi = nullptr;

int ImportProviders() {
  HMODULE hLib = LoadLibraryA("iprovider.dll");
  if (!hLib) {
    std::cerr << "Failed to load iprovider.dll" << std::endl;
    return -1;
  }

  typedef const IProviderAPI* (*GetIProviderAPIFunc)();
  GetIProviderAPIFunc pGetApi = (GetIProviderAPIFunc)GetProcAddress(hLib, "GetIProviderAPI");
  
  if (!pGetApi) {
    std::cerr << "Failed to locate GetIProviderAPI in iprovider.dll" << std::endl;
    return -1;
  }

  g_pApi = pGetApi();
  if (!g_pApi) {
    std::cerr << "Failed to retrieve API table from iprovider.dll" << std::endl;
    return -1;
  }

  std::cout << "All Engine APIs loaded successfully via IProviderAPI table." << std::endl;
  return 0;
}

bool ValidateCompressConfig(int argc, char** argv) {
  if (!g_pApi || !g_pApi->CompressEngine_ParseConfig) return false;
  CompressEngine_Config config;
  bool result = g_pApi->CompressEngine_ParseConfig(argc, argv, &config);
  if (result && g_pApi->CompressEngine_FreeConfig) {
    g_pApi->CompressEngine_FreeConfig(&config);
  }
  return result;
}

bool ValidateSocwatchConfig(int argc, char** argv) {
  if (!g_pApi || !g_pApi->SocwatchEngine_ParseConfig) return false;
  SocwatchEngine_Config config;
  memset(&config, 0, sizeof(config));
  bool result = g_pApi->SocwatchEngine_ParseConfig(argc, argv, &config);
  if (result) {
    std::cout << "[Reporter] Validated Socwatch Config: Duration="
              << config.duration << ", Output=" << config.outputFileName
              << std::endl;
  }
  return result;
}

bool ValidatePerfConfig(int argc, char** argv) {
  if (!g_pApi || !g_pApi->PerfEngine_ParseConfig) return false;
  PerfEngine_Config config;
  return g_pApi->PerfEngine_ParseConfig(argc, argv, &config);
}

void RunMathEngine(int multiplier, int input) {
  if (!g_pApi || !g_pApi->CreateMathEngine) return;
  EngineHandle handle = g_pApi->CreateMathEngine(multiplier);
  if (handle) {
    int result = g_pApi->MathEngine_Calculate(handle, input);
    std::cout << "MathEngine: " << input << " * " << multiplier << " = " << result << std::endl;
    g_pApi->DestroyMathEngine(handle);
  }
}

void RunSocwatchEngine(unsigned int duration, const std::string& outputFileName) {
  if (!g_pApi || !g_pApi->CreateSocwatchEngine) return;
  EngineHandle handle = g_pApi->CreateSocwatchEngine();
  if (handle) {
    std::cout << "Running SocwatchEngine (Duration: " << duration << "s, Output: " << outputFileName << ")..." << std::endl;
    const char* result = g_pApi->SocwatchEngine_Run(handle, duration, outputFileName.c_str());
    if (result) {
      std::cout << "SocwatchEngine result: " << result << std::endl;
    }
    g_pApi->DestroySocwatchEngine(handle);
  }
}

void RunPerfEngine(const std::string& profileName, const std::string& profileLevel, unsigned int duration, const std::string& etlFileName) {
  if (!g_pApi || !g_pApi->CreatePerfEngine) return;
  EngineHandle handle = g_pApi->CreatePerfEngine();
  if (handle) {
    wchar_t wProfile[256], wLevel[256], wEtlFile[260];
    MultiByteToWideChar(CP_ACP, 0, profileName.c_str(), -1, wProfile, 256);
    MultiByteToWideChar(CP_ACP, 0, profileLevel.c_str(), -1, wLevel, 256);
    MultiByteToWideChar(CP_ACP, 0, etlFileName.c_str(), -1, wEtlFile, 260);

    std::cout << "Starting PerfEngine trace (Profile: " << profileName << ", Level: " << profileLevel << ")..." << std::endl;
    if (g_pApi->PerfEngine_StartTrace(handle, wProfile, wLevel, duration, wEtlFile)) {
      std::cout << "Trace started successfully. Recording for " << duration << "s..." << std::endl;
    } else {
      std::cerr << "Failed to start PerfEngine trace." << std::endl;
    }
    g_pApi->DestroyPerfEngine(handle);
  }
}

std::wstring StringToWString(const std::string& str) {
  if (str.empty()) return std::wstring();
  int size_needed = MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), NULL, 0);
  std::wstring wstrTo(size_needed, 0);
  MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), &wstrTo[0], size_needed);
  return wstrTo;
}

void RunCompressEngine(const std::vector<std::string>& inputs, const std::string& output, const std::vector<std::string>& archives) {
  if (!g_pApi || !g_pApi->CreateCompressEngine) return;
  EngineHandle handle = g_pApi->CreateCompressEngine();
  if (handle) {
    std::cout << "Running CompressEngine (Output: " << output << ")..." << std::endl;
    std::vector<std::wstring> wInputs;
    for (const auto& s : inputs) wInputs.push_back(StringToWString(s));
    std::vector<const wchar_t*> pWInputs;
    for (const auto& s : wInputs) pWInputs.push_back(s.c_str());
    std::wstring wOutput = StringToWString(output);
    std::vector<const char*> pArchives;
    for (const auto& s : archives) pArchives.push_back(s.c_str());

    if (g_pApi->CompressEngine_CompressFileMapped(handle, pWInputs.data(), (int)pWInputs.size(), wOutput.data(), pArchives.data(), (int)pArchives.size())) {
      std::cout << "Compression successful." << std::endl;
    } else {
      std::cerr << "Compression failed." << std::endl;
    }
    g_pApi->DestroyCompressEngine(handle);
  }
}

bool ValidateUploadConfig(int argc, char** argv) {
  if (!g_pApi || !g_pApi->UploadEngine_ParseConfig) return false;
  UploadEngine_Config config;
  return g_pApi->UploadEngine_ParseConfig(argc, argv, &config);
}

void RunUploadEngine(const std::string& location, const std::string& url, const std::string& filePath) {
  if (!g_pApi || !g_pApi->CreateUploadEngine) return;
  EngineHandle handle = g_pApi->CreateUploadEngine();
  if (handle) {
    g_pApi->UploadEngine_SetServerConfig(handle, location.c_str(), url.c_str());
    g_pApi->UploadEngine_UploadFile(handle, filePath.c_str());
    g_pApi->DestroyUploadEngine(handle);
  }
}

std::string GetMachineName() {
  char buffer[MAX_COMPUTERNAME_LENGTH + 1];
  DWORD size = sizeof(buffer);
  if (GetComputerNameA(buffer, &size)) return std::string(buffer);
  return "Unknown-PC";
}

std::string GetMachineGuid() {
  HKEY hKey;
  char buffer[256];
  DWORD size = sizeof(buffer);
  if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Cryptography", 0, KEY_READ | KEY_WOW64_64KEY, &hKey) == ERROR_SUCCESS) {
    if (RegQueryValueExA(hKey, "MachineGuid", NULL, NULL, (LPBYTE)buffer, &size) == ERROR_SUCCESS) {
      RegCloseKey(hKey);
      return std::string(buffer);
    }
    RegCloseKey(hKey);
  }
  return "Unknown-GUID";
}

uint64_t GetMachineUptime() {
  return GetTickCount64() / 1000;
}

void SendSystemData() {
  HINTERNET hSession = WinHttpOpen(L"WinHeadlessClient/1.0", WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
  if (!hSession) return;
  HINTERNET hConnect = WinHttpConnect(hSession, L"rrwinterton.com", INTERNET_DEFAULT_HTTPS_PORT, 0);
  if (hConnect) {
    HINTERNET hRequest = WinHttpOpenRequest(hConnect, L"POST", L"/ireporter/api/data", NULL, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, WINHTTP_FLAG_SECURE);
    if (hRequest) {
      std::string machineName = GetMachineName();
      std::string machineGuid = GetMachineGuid();
      uint64_t uptime = GetMachineUptime();
      std::string jsonPayload = "{\"machine_id\": \"" + machineName + "\", \"machine_guid\": \"" + machineGuid + "\", \"status\": \"active\", \"uptime\": " + std::to_string(uptime) + "}";
      LPCWSTR header = L"Content-Type: application/json\r\n";
      BOOL bResults = WinHttpSendRequest(hRequest, header, -1L, (LPVOID)jsonPayload.c_str(), (DWORD)jsonPayload.length(), (DWORD)jsonPayload.length(), 0);
      if (bResults) {
        bResults = WinHttpReceiveResponse(hRequest, NULL);
        if (bResults) {
          std::cout << "JSON payload sent successfully.\n";
        }
      }
      WinHttpCloseHandle(hRequest);
    }
    WinHttpCloseHandle(hConnect);
  }
  WinHttpCloseHandle(hSession);
}
