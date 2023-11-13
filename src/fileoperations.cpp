#include "fileoperations.h"

void createDir(fs::FS &fs, const char * path){
    if(!fs.mkdir(path)){
        Serial.printf("mkdir failed: %s\n", path);
    }
}

void appendFile(fs::FS &fs, const char * path, const char * message){
    Serial.printf("Appending to file: %s\r\n", path);

    File file = fs.open(path, FILE_APPEND);
    if(!file){
        Serial.printf("Failed to open file for appending: %s\r\n", path);
        return;
    }
    if(!file.print(message)){
        Serial.printf("Append failed: %s\r\n", path);
    }
    file.close();
}

void deleteFile(fs::FS &fs, const char * path){
    Serial.printf("Deleting file: %s\r\n", path);
    if(fs.remove(path)){
        Serial.println("- file deleted");
    } else {
        Serial.println("- delete failed");
    }
}