#pragma once
#include <string>

class UploadEngine {
public:
    UploadEngine();
    ~UploadEngine();

    void SetServerConfig(const std::string& location, const std::string& url);
    bool UploadFile(const std::string& filePath);

private:
    std::string m_serverLocation = "rrwinterton.com";
    std::string m_serverUrl = "/ireporter/ireporter.cgi/upload";
};
