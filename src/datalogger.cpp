#include "datalogger.h"

void saveReading(fs::FS &fs, const uint8_t *mac_addr, struct_message &incomingReadings)
{
    Serial.println("Saving reading!");
    time_t now;
    struct tm timeinfo;
    getLocalTime(&timeinfo);
    time(&now);

    if(now < 1000000000)
    {
        Serial.println("Nie ustawiono czasu!");
        return;
    }

    std::string currentPath = "/sensordata/";
    std::stringstream helperStream;
    createDir(fs, "/sensordata");

    helperStream << std::hex << (int)mac_addr[0] << "-" << (int)mac_addr[1] << "-" << (int)mac_addr[2] << "-" << (int)mac_addr[3] << "-" << (int)mac_addr[4] << "-" << (int)mac_addr[5];
    //helperStream << "test-mac-address";
    currentPath += helperStream.str();
    createDir(fs, currentPath.c_str());
    currentPath += "/data";
    createDir(fs, currentPath.c_str());
    char year[5], month[3], day[3];
    strftime(year,5, "%Y", &timeinfo);
    strftime(month,3, "%m", &timeinfo);
    strftime(day,3, "%d", &timeinfo);
    helperStream.str(std::string()); // Czyszczenie helperStream
    helperStream << "/" << year << "-" << month << "-" << day << ".txt";
    currentPath += helperStream.str();
    helperStream.str(std::string()); // Czyszczenie helperStream
    helperStream << now << " " << incomingReadings.humidity << std::endl;
    appendFile(fs, currentPath.c_str(), helperStream.str().c_str());
}