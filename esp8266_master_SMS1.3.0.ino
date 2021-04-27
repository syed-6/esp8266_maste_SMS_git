#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266httpUpdate.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>
#include <PubSubClient.h>
#include <PubSubClientTools.h>
#include "cert.h"
#include <time.h>


#define MQTT_SERVER "broker.mqtt-dashboard.com"


WiFiClient espClient;
PubSubClient client(MQTT_SERVER, 1883, espClient);
PubSubClientTools mqtt(client);

void connect_wifi();
void firmwareUpdate();

const String FirmwareVer={"1.8"}; 
#define URL_fw_Version "/syed-6/ota/esp8266_maste_SMS_git/bin_version.txt.TXT"
#define URL_fw_Bin "https://raw.githubusercontent.com/syed-6/ota/main/esp_ota.ino.node32s.bin"


//#define URL_fw_Version "/programmer131/otaFiles/master/version.txt"
//#define URL_fw_Bin "https://raw.githubusercontent.com/programmer131/otaFiles/master/firmware.bin"
const char* host = "raw.githubusercontent.com";
const int httpsPort = 443;

unsigned long previousMillis_2 = 0;
unsigned long previousMillis = 0;        // will store last time LED was updated
const long interval = 60000;
const long mini_interval = 1000;

extern const unsigned char caCert[] PROGMEM;
extern const unsigned int caCertLen;

const char* ssid = "Circuit-7";
const char* password = "eee1guy007";

void setClock() {
   // Set time via NTP, as required for x.509 validation
  configTime(3 * 3600, 0, "pool.ntp.org", "time.nist.gov");
  Serial.print("Waiting for NTP time sync: ");
  time_t now = time(nullptr);
  while (now < 8 * 3600 * 2) {
    delay(500);
    Serial.print(".");
    now = time(nullptr);
  }
  
  Serial.println("");
  struct tm timeinfo;
  gmtime_r(&now, &timeinfo);
  Serial.print("Current time: ");
  Serial.print(asctime(&timeinfo));
  
}
  
void FirmwareUpdate()
{  
  WiFiClientSecure client;
  client.setTrustAnchors(&cert);
  if (!client.connect(host, httpsPort)) {
    Serial.println("Connection failed");
    return;
  }
  client.print(String("GET ") + URL_fw_Version + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" +
               "User-Agent: BuildFailureDetectorESP8266\r\n" +
               "Connection: close\r\n\r\n");
  while (client.connected()) {
    String line = client.readStringUntil('\n');
    if (line == "\r") {
      //Serial.println("Headers received");
      break;
    }
  }
  String payload = client.readStringUntil('\n');

  payload.trim();
  if(payload.equals(FirmwareVer) )
  {   
     Serial.println("Device already on latest firmware version"); 
  }
  else
  {
    Serial.println("New firmware detected");
    ESPhttpUpdate.setLedPin(LED_BUILTIN, LOW); 
    t_httpUpdate_return ret = ESPhttpUpdate.update(client, URL_fw_Bin);
        
    switch (ret) {
      case HTTP_UPDATE_FAILED:
        Serial.printf("HTTP_UPDATE_FAILD Error (%d): %s\n", ESPhttpUpdate.getLastError(), ESPhttpUpdate.getLastErrorString().c_str());
        break;

      case HTTP_UPDATE_NO_UPDATES:
        Serial.println("HTTP_UPDATE_NO_UPDATES");
        break;

      case HTTP_UPDATE_OK:
        Serial.println("HTTP_UPDATE_OK");
        break;
    } 
  }
 }  

 void repeatedCall(){
    unsigned long currentMillis = millis();
    if ((currentMillis - previousMillis) >= interval) 
    {
      // save the last time you blinked the LED
      previousMillis = currentMillis;
      setClock();
      FirmwareUpdate();
    }

    if ((currentMillis - previousMillis_2) >= mini_interval) {
      static int idle_counter=0;
      previousMillis_2 = currentMillis;    
      Serial.print(" Active fw version:");
      Serial.println(FirmwareVer);
      Serial.print("Idle Loop....");
      Serial.println(idle_counter++);
     if(idle_counter%2==0)
      digitalWrite(LED_BUILTIN, HIGH);
     else 
      digitalWrite(LED_BUILTIN, LOW);
     if(WiFi.status() == !WL_CONNECTED) 
          connect_wifi();
   }
 }

void MQTT_BEG(){
   if (!client.connected())
   {
    if (client.connect("ESP32Client")){
      Serial.println("MQTT connected");

//    mqtt.subscribe("testing/add_node",  topic1_subscriber);
//    mqtt.subscribe("testing/delete_node",    topic2_subscriber);
//    mqtt.subscribe("testing/condition/#",        topic3_subscriber);
    mqtt.subscribe("testing/update/#",topic4_subscriber);
    mqtt.subscribe("testing/fw/#",topic5_subscriber);
  } else {
    Serial.println("failed, rc=" + client.state());
  }
}
}

  
void setup()
{
  Serial.begin(115200);
  Serial.println("");
    String chipids;
  chipids = WiFi.macAddress();
  Serial.println("MAC id : " + chipids);
//  uint64_t chipid = ESP.getEfuseMac();
//  Serial.printf("ESP32 Chip ID = %04X", (uint16_t)(chipid >> 32)); //print High 2 bytes
//  Serial.printf("%08X\n", (uint32_t)chipid);
  Serial.println("Start");
  WiFi.mode(WIFI_STA);
  connect_wifi();  
  setClock();
   MQTT_BEG();
  pinMode(LED_BUILTIN, OUTPUT);
  
}
void loop()
{
  repeatedCall();    
  client.loop();
if(!client.connected()){
  MQTT_BEG();
  }
}

void connect_wifi()
{
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print("O");
  }                                   
  Serial.println("Connected to WiFi");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}



void topic4_subscriber(String topic, String message) {
  Serial.println("received");
  Serial.println("Message arrived in function 1 [" + topic + "] " + message);
  firmwareUpdate();
  //add_data_received(topic, message);
}
void topic5_subscriber(String topic, String message) {
  Serial.println("received");
  Serial.println("Message arrived in function 1 [" + topic + "] " + message);
  String mes;
  StaticJsonDocument<200> docs;

  docs["mac_id"] = WiFi.macAddress();
  docs["fw"] = FirmwareVer;

  serializeJson(docs, mes);
  mqtt.publish("receive/response", mes);
}
