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


#include "wifi_pass.h"

const char* ntpServer = "pool.ntp.org";
const std::string hostname = "ESP32 Serwer systemu monitorowania roślin";
const std::string domainName = "podlewanieroslin";

esp_now_peer_info_t slave;
int chan; 

MessageType messageType;

int counter = 0;

struct_message incomingReadings;
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

  server.serveStatic("/", LittleFS, "/www/").setDefaultFile("index.html");

  server.addHandler(&events);
  server.begin();

}

void loop() {
}