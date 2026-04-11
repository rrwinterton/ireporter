#include <windows.h>
#include <winhttp.h>
#include <iostream>
#include <string>

// Link against the WinHTTP library
#pragma comment(lib, "winhttp.lib")

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

    // 2. Connect to the local web server on port 80 (Standard IIS/HTTP port)
    HINTERNET hConnect = WinHttpConnect(hSession, L"localhost", 80, 0);
    if (hConnect) {
        // 3. Open a POST request to the specific API endpoint
        HINTERNET hRequest = WinHttpOpenRequest(hConnect, L"POST", L"/ireporter/api/data",
                                                NULL, WINHTTP_NO_REFERER,
                                                WINHTTP_DEFAULT_ACCEPT_TYPES,
                                                0);
        if (hRequest) {
            // 4. Prepare the JSON payload and headers
            std::string jsonPayload = R"({"machine_id": "WS-01", "status": "active", "uptime": 3600})";
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
                    
                    // (Optional) Read the server's response JSON here using WinHttpReadData
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

// Note: To make this truly "headless" (invisible), you would compile this 
// as a Windows Subsystem application using WinMain instead of main().
int main() {
    SendSystemData();
    return 0;
}
