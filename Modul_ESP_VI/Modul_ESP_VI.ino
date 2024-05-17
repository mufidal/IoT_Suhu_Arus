#include <SoftwareSerial.h>

#include <esp_now.h>
#include <esp_wifi.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <Adafruit_Sensor.h>
SoftwareSerial  TES(35,34); //RX TX
SoftwareSerial  TES2(32,33); //RX TX

/*Pertahankan*/
// Set your Board ID (ESP32 Sender #1 = BOARD_ID 1, ESP32 Sender #2 = BOARD_ID 2, etc)
#define BOARD_ID 1
/*Pertahankan*/

// Digital pin connected to the DHT sensor

// Variabel untuk pembacaan tegangan dan arus
//const int Sensor_Arus = 35; // Pin Analog ESP untuk mengukur arus
//int PeakVoltage;
//int PeakVoltageSebelum;
//int Vmax;
float Vrms;
int nilai_adc = 0;
int arus = 0;
float Arus_peak = 0;
float PF = 0.8;
float nilai_tertinggi = 0;
float Akar2 = 1.414;
float IRMS;
float daya = 0;

unsigned long waktuMulaiInterval = 0;
unsigned long intervalpeak = 1000;
//unsigned long counter;
//unsigned long countersebelum;

// Insert your SSID (The SSID of the "ESP32 Master" access point.).
constexpr char WIFI_SSID[] = "ESP32_WS";

//MAC Address of the receiver 
uint8_t broadcastAddress[] = {0xB0, 0xA7, 0x32, 0x34, 0xxx, 0xxx};

/*Pertahankan*/
//Structure example to send data
//Must match the receiver structure
typedef struct struct_message {
    int id;
    float temp;
    float hum;
    float vrms;
    float arusRMS;
    float powerr;
    int readingId;
} struct_message;

//Create a struct_message called myData
struct_message myData;
/*Pertahankan*/

//Register peer
  esp_now_peer_info_t peerInfo;

unsigned long previousMillis = 0;   // Stores last time temperature was published
unsigned long interval = 6000;        // Interval at which to publish sensor readings 10000

/*Pertahankan*/
unsigned int readingId = 0;
/*Pertahankan*/

/*Pertahankan*/

int32_t getWiFiChannel(const char *ssid) {
  if (int32_t n = WiFi.scanNetworks()) {
      for (uint8_t i=0; i<n; i++) {
          if (!strcmp(ssid, WiFi.SSID(i).c_str())) {
              return WiFi.channel(i);
          }
      }
  }
  return 0;
}
/*Pertahankan*/

/*Pertahankan*/
// callback when data is sent
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
//  ArusArduino();
}
/*Pertahankan*/

float ArusArduino(){
  String data_arduino = ""; // kosongin aja
  if (TES.available() > 0){
    data_arduino += char(TES.read());
  }
  else{
    data_arduino = char ('0');
  }
  data_arduino.trim();

  // Debugging: Cetak data yang diterima
//  Serial.print("Data Diterima: ");
//  Serial.println(data_arduino);

  // Konversi data dari Arduino ke float
  float IRMS = data_arduino.toFloat();

  // Debugging: Cetak nilai yang dikonversi
//  Serial.print("Nilai IRMS setelah konversi: ");
//  Serial.println(IRMS);
  return IRMS;
}

float TeganganArduino(){
    String data_arduino2 = ""; // kosongin aja
  if (TES2.available() > 0){
    data_arduino2 += char(TES2.read());
  }
  else{
    data_arduino2 = char ('0');
  } // Try, delete if failed
  data_arduino2.trim();
  float Vrms = data_arduino2.toFloat();
  return Vrms;
}


//float ukurVRMS() {
  //Minimal 513
  //Maximal belum
//   counter = millis();
//   PeakVoltage = analogRead(36);
  /*Perhitungan (y-y1)/(y2-y1)=(x-x1)/(x2-x1),
  dimana y1=0, y2=220V, x1=Pembacaan Minimal Analog, x2, pembacaan maximal Analog*/
  /*Perhitungan = y= m(x-x1), dimasukkan ke Vrms, dimana y adalah Vrms,
  dan m adalah pembacaan maximal Analog*/
//  Serial.println(PeakVoltage);
//  if (PeakVoltage > PeakVoltageSebelum) {
//    PeakVoltageSebelum = PeakVoltage;
//    if (PeakVoltage >= 2892) { // Kalibrasi
//      Vmax = PeakVoltage;
//    }
//  }
//  if (PeakVoltage < PeakVoltageSebelum) {
//    PeakVoltageSebelum = PeakVoltage;
//    Vrms = ((0.187 * Vmax) - 541); // Kalibrasi
//    if (counter - countersebelum >= 500) {
//      countersebelum = counter;
//      Serial.print("Vrms = ");
//      Serial.println(Vrms);
//    }
//  }
//  if (Vrms <=25){
//    Vrms = 0;
//    Serial.print("Vrms = ");
//    Serial.println(Vrms);
//  }
//   return Vrms;  
//}

float ukurdaya(){
  IRMS = ArusArduino(); // Memperbarui variabel global IRMS
  daya = IRMS * (Vrms / Akar2);
  Serial.print("Daya: ");
  return daya;
}

 
void setup() {
  //Init Serial Monitor
  Serial.begin(115200);
  TES.begin(115200);
  
/*Pertahankan*/ 
  // Set device as a Wi-Fi Station and set channel
  WiFi.mode(WIFI_STA);

  int32_t channel = getWiFiChannel(WIFI_SSID);

  WiFi.printDiag(Serial); // Uncomment to verify channel number before
  esp_wifi_set_promiscuous(true);
  esp_wifi_set_channel(channel, WIFI_SECOND_CHAN_NONE);
  esp_wifi_set_promiscuous(false);
  WiFi.printDiag(Serial); // Uncomment to verify channel change after

  //Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // Once ESPNow is successfully Init, we will register for Send CB to
  // get the status of Trasnmitted packet
  esp_now_register_send_cb(OnDataSent);
  
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.encrypt = false;
  
  //Add peer        
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer");
    return;
  }
}
/*Pertahankan*/

void loop() {
  /*Pertahankan*/
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    // Save the last time a new reading was published
    previousMillis = currentMillis;
  /*Pertahankan*/
  
    //Set values to send
    myData.id = BOARD_ID;
    myData.vrms = TeganganArduino();
    myData.arusRMS = ArusArduino();
    myData.powerr = ukurdaya();
    myData.readingId = readingId++;

    /*Pertahankan*/
    //Send message via ESP-NOW
    esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData));
    if (result == ESP_OK) {
      Serial.println("Sent with success");
    }
    else {
      Serial.println("Error sending the data");
    }
  }
}
/*Pertahankan*/

/* jelaskan apakah IRMS yang digunakan pada float ukurdaya merupakan
 * update nilai dari float ukur arus? ataukah mendapatkan nilai 0
 * dari variabel global?
 */
