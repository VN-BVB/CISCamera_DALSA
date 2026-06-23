#ifndef JSON_SAVER_H
#define JSON_SAVER_H

#include <string>

/**
 * @brief JSON文件保存器
 */
class JsonSaver
{
public:
    explicit JsonSaver(const std::string& baseDirectory = "./data/seamEndpointInfos/");

    bool saveJsonToFile(const std::string& jsonString,
                        int batchNumber,
                        const std::string& fileName = "");

    void setBaseDirectory(const std::string& baseDirectory);
    std::string getBaseDirectory() const;
    std::string generateFileName(int batchNumber) const;

private:
    std::string m_baseDirectory;
    bool ensureDirectoryExists(const std::string& directoryPath);
};

#endif // JSON_SAVER_H