#include "ireporter.h"
#include <windows.h>
#include <winreg.h>
#include <winhttp.h>
#include <iostream>
#include <string>
#include <vector>

// Link against necessary Windows libraries
#pragma comment(lib, "winhttp.lib")
#pragma comment(lib, "advapi32.lib")

// Function to retrieve the computer name
std::string GetMachineName() {
    char buffer[MAX_COMPUTERNAME_LENGTH + 1];
    DWORD size = sizeof(buffer);
    if (GetComputerNameA(buffer, &size)) {
        return std::string(buffer);
    }
    return "Unknown-PC";
}

// Function to retrieve the unique Machine GUID from the Windows Registry
std::string GetMachineGuid() {
    HKEY hKey;
    char buffer[256];
    DWORD size = sizeof(buffer);
    // Open the cryptography key where the MachineGuid is stored
    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Cryptography", 0, KEY_READ | KEY_WOW64_64KEY, &hKey) == ERROR_SUCCESS) {
        if (RegQueryValueExA(hKey, "MachineGuid", NULL, NULL, (LPBYTE)buffer, &size) == ERROR_SUCCESS) {
            RegCloseKey(hKey);
            return std::string(buffer);
        }
        RegCloseKey(hKey);
    }
    return "Unknown-GUID";
}

// Function to retrieve the system uptime in seconds
uint64_t GetMachineUptime() {
    // GetTickCount64 returns the number of milliseconds since the system was started
    return GetTickCount64() / 1000;
}

void SendSystemData() {
    // 1. Initialize the WinHTTP session
    HINTERNET hSession = WinHttpOpen(L"WinHeadlessClient/1.0",
                                     WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
                                     WINHTTP_NO_PROXY_NAME,
                                     WINHTTP_NO_PROXY_BYPASS, 0);

    if (!hSession) {
        std::cerr << "Failed to open WinHTTP session.\n";
        return;
    }

    // 2. Connect to the remote web server on port 443 (HTTPS)
    HINTERNET hConnect = WinHttpConnect(hSession, L"rrwinterton.com", INTERNET_DEFAULT_HTTPS_PORT, 0);
    if (hConnect) {
        // 3. Open a POST request to the specific API endpoint over HTTPS
        HINTERNET hRequest = WinHttpOpenRequest(hConnect, L"POST", L"/ireporter/api/data",
                                                NULL, WINHTTP_NO_REFERER,
                                                WINHTTP_DEFAULT_ACCEPT_TYPES,
                                                WINHTTP_FLAG_SECURE);
        if (hRequest) {
            // 4. Retrieve identity information and prepare the JSON payload
            std::string machineName = GetMachineName();
            std::string machineGuid = GetMachineGuid();
            uint64_t uptime = GetMachineUptime();
            
            // Construct the JSON payload with the real machine identity and uptime
            std::string jsonPayload = "{\"machine_id\": \"" + machineName + 
                                      "\", \"machine_guid\": \"" + machineGuid + 
                                      "\", \"status\": \"active\", \"uptime\": " + std::to_string(uptime) + "}";
            LPCWSTR header = L"Content-Type: application/json\r\n";

            // 5. Send the request
            BOOL bResults = WinHttpSendRequest(hRequest,
                                               header, -1L,
                                               (LPVOID)jsonPayload.c_str(),
                                               (DWORD)jsonPayload.length(),
                                               (DWORD)jsonPayload.length(), 0);

            // 6. Wait for the server's response
            if (bResults) {
                bResults = WinHttpReceiveResponse(hRequest, NULL);
                if (bResults) {
                    std::cout << "JSON payload sent successfully.\n";
                    std::cout << "Machine Identity: " << machineName << " | GUID: " << machineGuid << "\n";
                }
            } else {
                std::cerr << "Error sending request: " << GetLastError() << "\n";
            }
            WinHttpCloseHandle(hRequest);
        }
        WinHttpCloseHandle(hConnect);
    }
    WinHttpCloseHandle(hSession);
}

