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

void copyFile(fs::FS &fs, const char * inputPath, const char * outputPath)
{
    File file2 = fs.open(outputPath, FILE_WRITE);
    if (file2.available())
    {
        fs.remove(outputPath);
    }
    file2.close();

    File file1 = fs.open(inputPath, FILE_READ);
    if (!file1)
    {
        Serial.print("Blad kopiowania, nie mozna otworzyc ");
        Serial.println(inputPath);
    }
    file2 = fs.open(outputPath, FILE_WRITE);
    if (!file2)
    {
        Serial.print("Blad kopiowania, nie mozna otworzyc ");
        Serial.println(outputPath);
    }
    
    while( file1.available() ) {
        file2.write(file1.read());
    }
    
    file2.close();
    file1.close();
}