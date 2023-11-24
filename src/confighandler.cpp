#include "confighandler.h"

struct_outgoing_message readDataToSend(fs::FS &fs, const uint8_t * mac_addr) {
    std::stringstream macAddressStream;
    macAddressStream << std::hex << (int)mac_addr[0] << "-" << (int)mac_addr[1] << "-" << (int)mac_addr[2] << "-" << (int)mac_addr[3] << "-" << (int)mac_addr[4] << "-" << (int)mac_addr[5];
    std::string configFilePath = "/sensordata/" + macAddressStream.str() + "/sensorConfig.json"; 
    struct_outgoing_message configToSend;
    bool isFileAvailable = fs.exists(configFilePath.c_str());
    //bool isFileAvailable = file.available();
    // if(!isFileAvailable)
    // {
    //     Serial.print("Brak pliku konfiguracyjnego dla ");
    //     Serial.print(macAddressStream.str().c_str());
    //     Serial.println("! Kopiowanie wartosci domyslnych...");
    //     copyFile(fs, "/defaults/sensorConfig.json", configFilePath.c_str());
    // }
    File configFile = fs.open(configFilePath.c_str(), FILE_READ);

    StaticJsonDocument<512> doc;

    DeserializationError error = deserializeJson(doc, configFile);
    configFile.close();
    

    if (error) {
        Serial.print("deserializeJson() failed: ");
        Serial.println(error.c_str());
        return configToSend;
    }

    double minimumMoistureLevel = doc["minimumMoistureLevel"];
    double maximumMoistureLevel = doc["maximumMoistureLevel"];
    Serial.print("minimumMoistureLevel: ");
    Serial.println(minimumMoistureLevel);
    Serial.print("maximumMoistureLevel: ");
    Serial.println(maximumMoistureLevel);

    configToSend.msgType = DATA;
    configToSend.minimumMoistureLevel = (float)minimumMoistureLevel;
    configToSend.maximumMoistureLevel = (float)maximumMoistureLevel;
    return configToSend;
}