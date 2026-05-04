#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>
#include <winhttp.h>
#include <iostream>
#include <string>
#include <vector>
#include <cstring>
#include <fstream>
#include "api_exports.h"
#include "CLI.hpp"

#pragma comment(lib, "winhttp.lib")

// --- Helper for string conversion ---
std::wstring StringToWString(const std::string& str) {
  if (str.empty()) return L"";
  int size_needed = MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), NULL, 0);
  std::wstring wstrTo(size_needed, 0);
  MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), &wstrTo[0], size_needed);
  return wstrTo;
}

// --- UploadEngine Definition ---
class UploadEngine {
public:
    UploadEngine() {}
    ~UploadEngine() {}

    void SetServerConfig(const std::string& location, const std::string& url) {
        m_location = location;
        m_serverUrl = url;
        std::cout << "[UploadEngine] Server config set: " << location << " (" << url << ")" << std::endl;
    }

    bool UploadFile(const std::string& filePath) {
        if (m_serverUrl.empty() || m_location.empty()) {
            std::cerr << "[UploadEngine] Error: Server config not complete." << std::endl;
            return false;
        }

        std::cout << "[UploadEngine] Uploading file: " << filePath << " to " << m_location << m_serverUrl << "..." << std::endl;

        // Read file data
        std::ifstream file(filePath, std::ios::binary | std::ios::ate);
        if (!file.is_open()) {
            std::cerr << "[UploadEngine] Error: Could not open file " << filePath << std::endl;
            return false;
        }

        std::streamsize fileSize = file.tellg();
        file.seekg(0, std::ios::beg);

        std::vector<char> buffer((size_t)fileSize);
        if (!file.read(buffer.data(), fileSize)) {
            std::cerr << "[UploadEngine] Error: Could not read file " << filePath << std::endl;
            return false;
        }

        bool success = false;
        HINTERNET hSession = WinHttpOpen(L"IProvider/1.0", WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
        if (hSession) {
            std::wstring wLocation = StringToWString(m_location);
            HINTERNET hConnect = WinHttpConnect(hSession, wLocation.c_str(), INTERNET_DEFAULT_HTTPS_PORT, 0);
            if (!hConnect) {
                // Try HTTP if HTTPS fails or if that's what's intended. 
                // For now, we'll follow the pattern in ireporter.cpp which uses HTTPS by default.
                hConnect = WinHttpConnect(hSession, wLocation.c_str(), INTERNET_DEFAULT_HTTP_PORT, 0);
            }

            if (hConnect) {
                std::wstring wUrl = StringToWString(m_serverUrl);
                HINTERNET hRequest = WinHttpOpenRequest(hConnect, L"POST", wUrl.c_str(), NULL, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, WINHTTP_FLAG_SECURE);
                if (hRequest) {
                    // Ignore SSL errors for test purposes
                    DWORD dwFlags = SECURITY_FLAG_IGNORE_UNKNOWN_CA | 
                                    SECURITY_FLAG_IGNORE_CERT_WRONG_USAGE | 
                                    SECURITY_FLAG_IGNORE_CERT_CN_INVALID | 
                                    SECURITY_FLAG_IGNORE_CERT_DATE_INVALID;
                    WinHttpSetOption(hRequest, WINHTTP_OPTION_SECURITY_FLAGS, &dwFlags, sizeof(dwFlags));

                    LPCWSTR header = L"Content-Type: application/octet-stream\r\n";
                    if (filePath.size() >= 5 && filePath.substr(filePath.size() - 5) == ".json") {
                        header = L"Content-Type: application/json\r\n";
                    }
                    if (WinHttpSendRequest(hRequest, header, -1L, buffer.data(), (DWORD)buffer.size(), (DWORD)buffer.size(), 0)) {
                        if (WinHttpReceiveResponse(hRequest, NULL)) {
                            DWORD statusCode = 0;
                            DWORD dwSize = sizeof(statusCode);
                            WinHttpQueryHeaders(hRequest, WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER, WINHTTP_HEADER_NAME_BY_INDEX, &statusCode, &dwSize, WINHTTP_NO_HEADER_INDEX);
                            if (statusCode == 200) {
                                std::cout << "[UploadEngine] Upload successful (HTTP 200)." << std::endl;
                                success = true;
                            } else {
                                std::cerr << "[UploadEngine] Upload failed with HTTP status: " << statusCode << std::endl;
                            }
                        } else {
                            std::cerr << "[UploadEngine] WinHttpReceiveResponse failed: " << GetLastError() << std::endl;
                        }
                    } else {
                        std::cerr << "[UploadEngine] WinHttpSendRequest failed: " << GetLastError() << std::endl;
                    }
                    WinHttpCloseHandle(hRequest);
                } else {
                    std::cerr << "[UploadEngine] WinHttpOpenRequest failed: " << GetLastError() << std::endl;
                }
                WinHttpCloseHandle(hConnect);
            } else {
                std::cerr << "[UploadEngine] WinHttpConnect failed: " << GetLastError() << std::endl;
            }
            WinHttpCloseHandle(hSession);
        } else {
            std::cerr << "[UploadEngine] WinHttpOpen failed: " << GetLastError() << std::endl;
        }

        return success;
    }

private:
    std::string m_location;
    std::string m_serverUrl;
};

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
  
  CLI::App app{"CompressEngine Validator"};
  app.allow_extras();
  
  std::vector<std::string> inputs;
  std::vector<std::string> archives;
  std::string output = "archive.zip";

  app.add_option("--compress-input", inputs, "Input files");
  app.add_option("--compress-output", output, "Output file");
  app.add_option("--compress-archive", archives, "Archive names");

  try {
    app.parse(argc, argv);
  } catch (const CLI::ParseError &e) {
    return false;
  }

  outConfig->doCompress = true;
  strncpy(outConfig->outputFilePath, output.c_str(), sizeof(outConfig->outputFilePath) - 1);
  
  outConfig->inputFileCount = (int)inputs.size();
  if (outConfig->inputFileCount > 0) {
      outConfig->inputFilePaths = new char*[inputs.size()];
      for (size_t i = 0; i < inputs.size(); ++i) outConfig->inputFilePaths[i] = DuplicateString(inputs[i].c_str());
  } else {
      outConfig->inputFilePaths = nullptr;
  }

  outConfig->archiveNameCount = (int)archives.size();
  if (outConfig->archiveNameCount > 0) {
      outConfig->archiveNames = new char*[archives.size()];
      for (size_t i = 0; i < archives.size(); ++i) outConfig->archiveNames[i] = DuplicateString(archives[i].c_str());
  } else {
      outConfig->archiveNames = nullptr;
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

  CLI::App app{"SocwatchEngine Validator"};
  app.allow_extras();

  unsigned int duration = 30;
  std::string output = "socwatch.csv";

  app.add_option("--sw-duration", duration, "Duration");
  app.add_option("--sw-output", output, "Output file");

  try {
    app.parse(argc, argv);
  } catch (const CLI::ParseError &e) {
    return false;
  }

  outConfig->doSocwatch = true;
  outConfig->duration = duration;
  strncpy(outConfig->outputFileName, output.c_str(), sizeof(outConfig->outputFileName) - 1);

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

  CLI::App app{"PerfEngine Validator"};
  app.allow_extras();

  unsigned int duration = 30;
  std::string profile = "General";
  std::string level = "Verbose";
  std::string etl = "trace.etl";

  app.add_option("--perf-duration", duration);
  app.add_option("--profileName", profile);
  app.add_option("--profileLevel", level);
  app.add_option("--etlFile", etl);

  try {
    app.parse(argc, argv);
  } catch (const CLI::ParseError &e) {
    return false;
  }

  outConfig->perf = true;
  outConfig->duration = duration;
  strncpy(outConfig->profileName, profile.c_str(), sizeof(outConfig->profileName) - 1);
  strncpy(outConfig->profileLevel, level.c_str(), sizeof(outConfig->profileLevel) - 1);
  strncpy(outConfig->etlFile, etl.c_str(), sizeof(outConfig->etlFile) - 1);

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

  CLI::App app{"UploadEngine Validator"};
  app.allow_extras();

  std::string location = "C:\\Temp";
  std::string url = "http://localhost/upload";
  std::string file = "data.txt";

  app.add_option("--upload-location", location);
  app.add_option("--upload-url", url);
  app.add_option("--upload-file", file);

  try {
    app.parse(argc, argv);
  } catch (const CLI::ParseError &e) {
    return false;
  }

  outConfig->doUpload = true;
  memset(outConfig->serverLocation, 0, sizeof(outConfig->serverLocation));
  memset(outConfig->serverUrl, 0, sizeof(outConfig->serverUrl));
  memset(outConfig->uploadFile, 0, sizeof(outConfig->uploadFile));
  
  strncpy(outConfig->serverLocation, location.c_str(), sizeof(outConfig->serverLocation) - 1);
  strncpy(outConfig->serverUrl, url.c_str(), sizeof(outConfig->serverUrl) - 1);
  strncpy(outConfig->uploadFile, file.c_str(), sizeof(outConfig->uploadFile) - 1);

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
