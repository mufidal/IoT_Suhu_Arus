#include <Arduino.h>
#include <esp_now.h>
#include <esp_wifi.h>
#include "ESPAsyncWebServer.h"
#include <AsyncTCP.h>
#include <WiFi.h> //Library WiFi
#include <ESP32Ping.h> //Library PING
#include <HTTPClient.h> //Library untuk mendapatkan Client HTTP
//#include <Arduino_JSON.h>
#include <ArduinoJson.h> //Library untuk pengolahan data JSON

/* Variabel boolean untuk menandai apakah ada data baru yang diterima */
bool newDataReceived = false;

/*Fungsi untuk nyambung ke WiFi*/
String wifiSSID = "mlzzz"; 
String wifiPassword = "akuganteng";

const char* soft_ap_ssid = "ESP32_WS";  //--> access point name
const char* soft_ap_password = "akuganteng"; //--> access point password

IPAddress local_ip(192,168,2,2);
IPAddress gateway(192,168,2,2);
IPAddress subnet(255,255,255,0);

/*Data yang akan diterima dan dikirim*/
typedef struct struct_message {
  int id;
  float temp;
  float hum;
  float vrms;
  float arusRMS;
  float powerr;
  unsigned int readingId;
} struct_message;

struct_message incomingReadings;

String globaltoken;
StaticJsonDocument<1023>JsonDHT; //variabelnya buff
//Deklarasi Fungsi
void OnDataRecv(const uint8_t * mac_addr, const uint8_t *incomingData, int len);
void connectWifi();
void getHttp();

void setup() 
{
  Serial.begin(115200);

  // Set the device as a Station and Soft Access Point simultaneously
//  WiFi.mode(WIFI_STA);
connectWifi();  
  
  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  
//  connectWifi();  
}

void loop() {
  postHttp();
  esp_now_register_recv_cb(OnDataRecv);

  // Pemanggilan fungsi hanya jika ada data baru yang diterima
  if (newDataReceived) {

    postdata();
    newDataReceived = false; // Setelah pengiriman, reset flag
  }
}

void OnDataRecv(const uint8_t * mac_addr, const uint8_t *incomingData, int len) {
  
  char macStr[18];
  Serial.print("Packet received from: ");
  snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
           mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
  Serial.println(macStr);
  memcpy(&incomingReadings, incomingData, sizeof(incomingReadings));
  Serial.printf("Board ID %u: %u bytes\n", incomingReadings.id, len);
  
  /*Perlu Variabel static*/
  StaticJsonDocument<1023>posting; //variabelnya buff

  posting["deviceID"]=incomingReadings.id;
  posting["devEUI"]=incomingReadings.readingId;
//  posting["type"]="temp";
  posting["voltage"]=incomingReadings.vrms;
  posting["current"]=incomingReadings.arusRMS;
  posting["temperature"]=incomingReadings.temp;
  posting["ambient"]=incomingReadings.hum;
  
//  Serial.printf("BOARD ID value: %d \n", incomingReadings.id);
//  Serial.printf("temp value: %d \n", incomingReadings.temp);
//  Serial.printf("hum value: %d \n", incomingReadings.hum);
//  Serial.printf("vrms value: %d \n", incomingReadings.vrms);
//  Serial.printf("arusRMS value: %d \n", incomingReadings.arusRMS);
//  Serial.printf("powerr value: %d \n", incomingReadings.powerr);
//  Serial.printf("readingId value: %d \n", incomingReadings.readingId);
//  Serial.println();

  /*Simpan Ke Variabel Global*/
  JsonDHT = posting;

  // Set flag newDataReceived menjadi true
  newDataReceived = true;
}

/*Fungsi buat dapetin token*/
void postHttp(){
  Serial.println("Update Token...");
  String url = "Link API Kalyann yak";
  HTTPClient http ;
  String responsetoken;

  /*Perlu Variabel static*/
  StaticJsonDocument<1023>buff; //variabelnya buff
  String jsonParams; // tipe string

  /*Dokumennya Buff*/
  buff["username"]="...";
  buff["password"]="...";

  serializeJson(buff,jsonParams);//dokumen buff masuk ke jsonParams
  Serial.println(jsonParams);

  /*Posting, minta response*/
  http.begin(url);
  http.addHeader("Content-Type", "application/json");
  int statusCode = http.POST(jsonParams);
  responsetoken = http.getString();
//  Serial.println(response);
    
  /*Perlu Variabel static*/
  StaticJsonDocument<1024>doc; //variabelnya doc

  /*kita akan pecah data biar bisa tertata di arduino*/
  deserializeJson(doc,responsetoken); //Simpan ke doc sumber responsetoken
  
  /*Merubah Json doc ke Json Object*/
  JsonObject obj = doc.as<JsonObject>(); 
//  Serial.println(responsetoken);

  /*Mengambil Nilai token dari string Token*/
  String localtoken = obj["data"]["token"].as<String>();
  globaltoken = localtoken;
  Serial.println(localtoken);

  //  Serial.println(statusCode);
  if (statusCode==200){
    Serial.println("Get Token Succes!");
  }
  else{
    Serial.println("Get Token Failed!");
  }
}

//Fungsi Buat ngecek Token sudah ganti belum
void postdata(){
  Serial.println("--------------------");
  Serial.println(globaltoken);
  String url2 = "Link API Kalyann yak";
  HTTPClient http ;
  String responsekirim;

  /*Perlu Variabel static*/
  String jsonParamspost; // tipe string

  /*Ambil Dari JsonDHT (global variabel)*/
  serializeJson(JsonDHT,jsonParamspost);//dokumen buff masuk ke jsonParams
  Serial.println(jsonParamspost);

  /*Posting, minta response*/
  http.begin(url2);
  http.addHeader("Content-Type", "application/json");
  http.addHeader("Authorization", globaltoken);
  int statusCode = http.POST(jsonParamspost);
  responsekirim = http.getString();
    
  /*Perlu Variabel static*/
  StaticJsonDocument<1024>doc2; //variabelnya doc

  /*kita akan pecah data biar bisa tertata di arduino*/
  deserializeJson(doc2,responsekirim); //Simpan ke doc sumber responsetoken

  Serial.println(responsekirim);
}

void connectWifi(){
  // Set the device as a Station and Soft Access Point simultaneously
  WiFi.mode(WIFI_AP_STA);
  //Setting up ESP32 to be an Access Point.
  WiFi.softAP(soft_ap_ssid, soft_ap_password);
  //Setting up ESP32 softAPConfig.
  WiFi.softAPConfig(local_ip, gateway, subnet);
  
  Serial.println("Connecting To WiFi");
  WiFi.begin(wifiSSID.c_str(),wifiPassword.c_str()); // harus ke dalam bentuk char
  while(WiFi.status()!= WL_CONNECTED)
  {
    Serial.print(".");
    delay(500);
  }
  Serial.println("");
  Serial.println("Wifi Connected");

}
