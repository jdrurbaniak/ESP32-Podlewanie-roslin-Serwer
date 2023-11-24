#include <esp_now.h>
#include <WiFi.h>
#include "time.h"
#include <ESPAsyncWebServer.h>
#include <LittleFS.h>
#include "AsyncTCP.h"
#include <ArduinoJson.h>
#include <ESPmDNS.h>
#include "datastructures.h"
#include "datalogger.h"
#include "confighandler.h"
#include "AsyncJson.h"
#include "oneparamrewrite.h"

#include "wifi_pass.h"

const char* ntpServer = "pool.ntp.org";
const std::string hostname = "ESP32 Serwer systemu monitorowania roślin";
const std::string domainName = "podlewanieroslin";

esp_now_peer_info_t slave;
int chan; 

MessageType messageType;

int counter = 0;

struct_message incomingReadings;
struct_outgoing_message configToSend;
struct_pairing pairingData;

AsyncWebServer server(80);
AsyncEventSource events("/events");

void printMAC(const uint8_t * mac_addr){
  char macStr[18];
  snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
           mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
  Serial.print(macStr);
}

bool addPeer(const uint8_t *peer_addr) {
  memset(&slave, 0, sizeof(slave));
  const esp_now_peer_info_t *peer = &slave;
  memcpy(slave.peer_addr, peer_addr, 6);
  
  slave.channel = chan;
  slave.encrypt = 0;

  bool exists = esp_now_is_peer_exist(slave.peer_addr);
  if (exists) {
    // Slave already paired.
    Serial.println("Already Paired");
    return true;
  }
  else {
    esp_err_t addStatus = esp_now_add_peer(peer);
    if (addStatus == ESP_OK) {
      Serial.println("Pair success");
      return true;
    }
    else 
    {
      Serial.println("Pair failed");
      return false;
    }
  }
} 

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("Last Packet Send Status: ");
  Serial.print(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success to " : "Delivery Fail to ");
  printMAC(mac_addr);
  Serial.println();
}

void OnDataRecv(const uint8_t * mac_addr, const uint8_t *incomingData, int len) { 
  Serial.print(len);
  Serial.print(" bytes of data received from : ");
  printMAC(mac_addr);
  Serial.println();
  StaticJsonDocument<1000> root;
  String payload;
  uint8_t type = incomingData[0];
  switch (type) {
  case DATA : 
    memcpy(&incomingReadings, incomingData, sizeof(incomingReadings));
    saveReading(LittleFS, mac_addr, incomingReadings);
    struct_outgoing_message dataToSend;
    dataToSend = readDataToSend(LittleFS, mac_addr);
    if(dataToSend.msgType == DATA)
    {
      esp_now_send(NULL, (uint8_t *) &dataToSend, sizeof(dataToSend));
    }
    root["id"] = incomingReadings.id;
    root["humidity"] = incomingReadings.humidity;
    root["readingId"] = String(incomingReadings.readingId);
    //root["time"] = getTime();
    serializeJson(root, payload);
    Serial.print("event send :");
    serializeJson(root, Serial);
    events.send(payload.c_str(), "new_readings", millis());
    Serial.println();
    break;
  
  case PAIRING: 
    memcpy(&pairingData, incomingData, sizeof(pairingData));
    Serial.println(pairingData.msgType);
    Serial.println(pairingData.id);
    Serial.print("Pairing request from: ");
    printMAC(mac_addr);
    Serial.println();
    Serial.println(pairingData.channel);
    if (pairingData.id > 0) { 
      if (pairingData.msgType == PAIRING) { 
        pairingData.id = 0;       // 0 is server
        WiFi.softAPmacAddress(pairingData.macAddr);   
        pairingData.channel = chan;
        Serial.println("send response");
        esp_err_t result = esp_now_send(mac_addr, (uint8_t *) &pairingData, sizeof(pairingData));
        addPeer(mac_addr);
      }  
    }  
    break; 
  }
}

void initESP_NOW(){
    if (esp_now_init() != ESP_OK) {
      Serial.println("Error initializing ESP-NOW");
      return;
    }
    esp_now_register_send_cb(OnDataSent);
    esp_now_register_recv_cb(OnDataRecv);
} 

std::string listFiles(fs::FS &fs, const char * dirname, bool isDir=true){
    std::string result;
    File root = fs.open(dirname);
    if(!root){
        Serial.println("- failed to open directory");
        return "";
    }
    if(!root.isDirectory()){
        Serial.println(" - not a directory");
        return "";
    }

    File file = root.openNextFile();
    while(file){
        if((file.isDirectory() && isDir == true) || (!file.isDirectory() && isDir == false)){
          result += file.name();
          result += '\n';
        }
        file = root.openNextFile();
    }
    return result;
}

void setup() {
  Serial.begin(115200);

  if(!LittleFS.begin(true)){
    Serial.println("An Error has occurred while mounting SPIFS");
    return;
  }

  WiFi.mode(WIFI_AP_STA);
  WiFi.config(INADDR_NONE, INADDR_NONE, INADDR_NONE, INADDR_NONE);
  WiFi.setHostname(hostname.c_str());
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Setting as a Wi-Fi Station..");
  }

  MDNS.begin(domainName.c_str());
  MDNS.addService("http", "tcp", 80);
  configTime(0, 0, ntpServer);

  chan = WiFi.channel();
  Serial.print("Station IP Address: ");
  Serial.println(WiFi.localIP());
  Serial.print("Wi-Fi Channel: ");
  Serial.println(WiFi.channel());

  initESP_NOW();

  server.addRewrite( new OneParamRewrite("/managed-sensors/{dev}", "/managed-sensors?device={dev}") );

  server.on("/managed-sensors", HTTP_GET, [](AsyncWebServerRequest *request){
    std::string responseData;
    if(request->hasParam("device"))
    {
      std::stringstream responsePath;
      auto paramDevice = request->getParam("device");
      if(request->hasParam("date"))
      {
        auto paramDate = request->getParam("date");
        responsePath << "/sensordata/" << paramDevice->value().c_str() << "/data/" << paramDate->value().c_str() << ".txt";
        request->send(LittleFS, responsePath.str().c_str());
      }
      else
      {
        responsePath << "/sensordata/" << paramDevice->value().c_str() << "/data";
        responseData = listFiles(LittleFS, responsePath.str().c_str(), false);
      }
    }
    else
    {
      responseData = listFiles(LittleFS, "/sensordata");
    }
    if(!request->hasParam("date")) request->send(200, "text/plain", responseData.c_str());
  });

  server.on("/get-device-settings", HTTP_GET, [](AsyncWebServerRequest *request){
    std::string testString = listFiles(LittleFS, "/sensordata/40-91-51-fc-c0-d8");
    Serial.println(testString.c_str());
    Serial.println("/get-device-settings");
    if(request->hasParam("device"))
    {
      std::stringstream responsePath;
      auto paramDevice = request->getParam("device");
      responsePath << "/sensordata/" << paramDevice->value().c_str() << "/sensorConfig.json";
      Serial.println(responsePath.str().c_str());
      Serial.println(LittleFS.exists(responsePath.str().c_str()));
      //File file = LittleFS.open(responsePath.str().c_str());
      bool isFileAvailable = LittleFS.exists(responsePath.str().c_str());
      //file.close();
      if(!isFileAvailable)
      {
          Serial.print("Brak pliku konfiguracyjnego dla ");
          Serial.print(paramDevice->value().c_str());
          Serial.println("! Kopiowanie wartosci domyslnych...");
          copyFile(LittleFS, "/defaults/sensorConfig.json", responsePath.str().c_str());

      }
      request->send(LittleFS, responsePath.str().c_str());
    }
  });

  // server.on("/update-device-settings", HTTP_POST, [](AsyncWebServerRequest *request)
  // {
  //   Serial.print("/update-device-settings request: ");
  //   Serial.println(request->getParam("deviceName")->value().c_str());
  //   int params = request->params();
  //   for(int i=0;i<params;i++){
  //     AsyncWebParameter* p = request->getParam(i);
  //     if(p->isFile()){ //p->isPost() is also true
  //       Serial.printf("FILE[%s]: %s, size: %u\n", p->name().c_str(), p->value().c_str(), p->size());
  //     } else if(p->isPost()){
  //       Serial.printf("POST[%s]: %s\n", p->name().c_str(), p->value().c_str());
  //     } else {
  //       Serial.printf("GET[%s]: %s\n", p->name().c_str(), p->value().c_str());
  //     }
  //   }
  // });

  AsyncCallbackJsonWebHandler* updateDeviceSettingsHandler = new AsyncCallbackJsonWebHandler("/update-device-settings", [](AsyncWebServerRequest *request, JsonVariant &json) {
    StaticJsonDocument<512> configData;
    if (json.is<JsonObject>())
    {
      configData = json.as<JsonObject>();
    }
    std::string macAddressString = configData["macAddress"];
    std::string configPath = "/sensordata/" + macAddressString + "/sensorConfig.json"; 
    // bool isFileAvailable = configFile.available();
    bool isFileAvailable = LittleFS.exists(configPath.c_str());
    Serial.println(isFileAvailable);
    // configFile.close();
    if (isFileAvailable == true)
    {
        LittleFS.remove(configPath.c_str());
    }
    File configFile2 = LittleFS.open(configPath.c_str(), FILE_WRITE);
    serializeJson(json, configFile2);
    configFile2.close();
  });
  server.addHandler(updateDeviceSettingsHandler);

  server.serveStatic("/", LittleFS, "/www/").setDefaultFile("index.html");

  server.addHandler(&events);
  server.begin();

}

void loop() {
}