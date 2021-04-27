#ifndef MQ_H

#define MQ_H
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266httpUpdate.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>
#include <PubSubClient.h>
#include <PubSubClientTools.h>
#include "cert.h"
#include "var_pin.h"



void MQTT_BEG();
void addition_topic_subscriber(String , String );
void delete_topic_subscriber(String , String );
void condition_topic_subscriber(String , String );
void update_topic_subscriber(String , String );
void fw_topic_subscriber(String , String );
void del_data_received(String , String );
void data_receiveds(String , String );
void add_data_received(String , String );
void publisher();
void del_response_publisher();
void add_response_publisher();
void condition_publish();
void del_response_publisher(int8_t );
void KeepALive ();
void FirmwareUpdates();
void update_publisher(String );
void wakeup_publish();
String construct_packet(int, int);

WiFiClient espClient;
PubSubClient clients(MQTT_SERVER, 1883, espClient);
PubSubClientTools mqtt(clients);
struct mqttpacket {
  String mac_id;
  char master_name[6];
  char node_name[6];
  int modes;
  int Switch;
  bool Status;
  int No_of_slave;
};
//function for topic 1 callback
void addition_topic_subscriber(String topic, String message) {
  Sprintln("received");
  //  Sprintln("Message arrived in function 1 [" + topic + "] " + message);
  add_data_received(topic, message);
}

//function for topic 2 callback
void delete_topic_subscriber(String topic, String message) {
  Sprintln("received");
  //  Sprintln("Message arrived in function 2 [" + topic + "] " + message);
  del_data_received(topic, message);
}

//function for topic 3 callback
void condition_topic_subscriber(String topic, String message) {
  //  Sprintln("Message arrived in function 3 [" + topic + "] " + message);
  data_receiveds(topic, message);
}

//function for topic 4 callback
void update_topic_subscriber(String topic, String message) {
  //  Sprintln("Message arrived in function 1 [" + topic + "] " + message);
  Sprint("received data is");
  Sprintln(message);
  StaticJsonDocument<200> doc;
  deserializeJson(doc, message);
  const char* macid = doc["mac_id"];
  if (strcp(macid, chipids)) {
    FirmwareUpdates();
  }
  //add_data_received(topic, message);
}

//function for topic 5 callback
void fw_topic_subscriber(String topic, String message) {
  Sprintln("received");
  Sprintln("Message arrived in function 1 [" + topic + "] " + message);
  String mes;
  StaticJsonDocument<200> docs;
  docs["mac_id"] = WiFi.macAddress();
  docs["fw"] = FirmwareVer;
  serializeJson(docs, mes);
  mqtt.publish("receive/response", mes);
}

void MQTT_BEG() {
  if (!clients.connected())
  {
    String client_name = "ESP8266_";
    client_name += chipids;
    if (clients.connect((char*) client_name.c_str())) {
      Sprintln("MQTT connected");

      mqtt.subscribe("from_web/add_node",  addition_topic_subscriber);
      mqtt.subscribe("from_web/delete_node",    delete_topic_subscriber);
      mqtt.subscribe("from_web/condition/#",        condition_topic_subscriber);
      mqtt.subscribe("from_web/update/#", update_topic_subscriber);
      mqtt.subscribe("from_web/fw/#", fw_topic_subscriber);
    } else {
      Sprintln("failed, rc=" + clients.state());
    }
  }
}

void FirmwareUpdates()
{
  WiFiClientSecure client;
  client.setTrustAnchors(&cert);
  if (!client.connect(hosts, httpsPort)) {
    Sprintln("Connection failed");
    return;
  }
  client.print(String("GET ") + URL_fw_Version + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" +
               "User-Agent: BuildFailureDetectorESP8266\r\n" +
               "Connection: close\r\n\r\n");
  while (client.connected()) {
    String line = client.readStringUntil('\n');
    if (line == "\r") {
      //Sprintln("Headers received");
      break;
    }
  }
  String payload = client.readStringUntil('\n');

  payload.trim();
  if (payload.equals(FirmwareVer) )
  {
    Sprintln("Device already on latest firmware version");
  }
  else
  {
    Sprintln("New firmware detected");
    ESPhttpUpdate.setLedPin(LED_BUILTIN, LOW);
    t_httpUpdate_return ret = ESPhttpUpdate.update(client, URL_fw_Bin);

    switch (ret) {
      case HTTP_UPDATE_FAILED:
        Serial.printf("HTTP_UPDATE_FAILD Error (%d): %s\n", ESPhttpUpdate.getLastError(), ESPhttpUpdate.getLastErrorString().c_str());
        update_publisher("HTTP_UPDATE_FAILD");
        break;

      case HTTP_UPDATE_NO_UPDATES:
        Sprintln("HTTP_UPDATE_NO_UPDATES");
        update_publisher("HTTP_UPDATE_NO_UPDATES");
        break;

      case HTTP_UPDATE_OK:
        Sprintln("HTTP_UPDATE_OK");
        update_publisher("HTTP_UPDATE_OK");
        break;
    }
  }
}

void KeepALive() {
  if ((millis() - keepalive_timer) > KEEP_ALIVE_INTERVAL ) {
    keepalive_timer = millis();
    String packet = construct_packet(KEEPAlive, PUBLISH_PACKET);
    if (!clients.connected()) {
      MQTT_BEG();
    }
    mqtt.publish("from_esp/publish/Keepalive", packet);
    Sprintln("KEEP alive done");
  }
}
void update_publisher(String res) {
  StaticJsonDocument<500> PACKET_MQTT;
  const size_t CAPACITY = JSON_ARRAY_SIZE(10);
  JsonArray Slave_data = PACKET_MQTT.to<JsonArray>();
  for (int i = 0; i < 10; i++) {
    Slave_data.add(Status[i]);
  }
  String mss;
  PACKET_MQTT["pub_type"] = "update_res";
  PACKET_MQTT["seq_No"] = seq_No;
  PACKET_MQTT["Device_Type"] = Device_Type;
  PACKET_MQTT["App_Type"] = App_Type;
  PACKET_MQTT["FW_Version"] = FirmwareVer;
  PACKET_MQTT["boot_reason"] = boot_reason;
  PACKET_MQTT["error_code"] = error_code;
  PACKET_MQTT["Mac_Id"] = Mac_Id;
  PACKET_MQTT["Master_Name"] = Master_Name;
  PACKET_MQTT["Node_Name"] = Node_Name;
  PACKET_MQTT["Status"] = LED1_State;
  PACKET_MQTT["No_of_Slaves"] = No_of_Slaves;
  PACKET_MQTT["Slave_data"] = Slave_data;
  PACKET_MQTT["pairing_status"] = device_condition;
  PACKET_MQTT["Response"] = res;
  serializeJson(PACKET_MQTT, mss);
  mqtt.publish("from_esp/response/update", mss);
}

void wakeup_publish() {
  String packet = construct_packet(WAKEup, PUBLISH_PACKET);
  mqtt.publish("from_esp/publish/wakeup", packet);
  seq_No++;
}
void condition_publish() {
  String packet = construct_packet(CONDition, PUBLISH_PACKET);
  mqtt.publish("from_esp/publish/condition", packet);
  seq_No++;
}
void publisher() {
//  Response=true;
  String packet = construct_packet(CONDition, RESPONSE_PACKET);
  mqtt.publish("receive/response", packet);
}

void add_response_publisher() {
//  Response=true;
  String packet = construct_packet(ADDition, RESPONSE_PACKET);
  mqtt.publish("receive/response", packet);
  
}
void del_response_publisher() {
  
//  Response=true;
  String packet = construct_packet(DELEtion, RESPONSE_PACKET);
  mqtt.publish("receive/response", packet);
}
String construct_packet(int type, int reason) {
  StaticJsonDocument<200> PACKET_MQTT;
  const size_t CAPACITY = JSON_ARRAY_SIZE(10);
  JsonArray Slave_data = PACKET_MQTT.to<JsonArray>();
  for (int i = 0; i < 10; i++) {
    Slave_data.add(Status[i]);
  }
  String mss;
  if (reason) {
    switch (type) {
      case WAKEup: {
          PACKET_MQTT["pub_type"] = "wakeup";
          PACKET_MQTT["seq_No"] = seq_No;
          PACKET_MQTT["Device_Type"] = Device_Type;
          PACKET_MQTT["App_Type"] = App_Type;
          PACKET_MQTT["FW_Version"] = FirmwareVer;
          PACKET_MQTT["boot_reason"] = boot_reason;
          PACKET_MQTT["error_code"] = error_code;
          PACKET_MQTT["Mac_Id"] = Mac_Id;
          PACKET_MQTT["Master_Name"] = Master_Name;
          PACKET_MQTT["Node_Name"] = Node_Name;
          PACKET_MQTT["Status"] = LED1_State;
          PACKET_MQTT["No_of_Slaves"] = No_of_Slaves;
          PACKET_MQTT["Slave_data"] = Slave_data; // = Status;
          PACKET_MQTT["pairing_status"] = device_condition;
          break;
        }

      case CONDition: {
          PACKET_MQTT["pub_type"] = "condition_pub";
          PACKET_MQTT["seq_No"] = seq_No;
          PACKET_MQTT["Device_Type"] = Device_Type;
          PACKET_MQTT["App_Type"] = App_Type;
          PACKET_MQTT["boot_reason"] = boot_reason;
          PACKET_MQTT["error_code"] = error_code;
          PACKET_MQTT["Mac_Id"] = Mac_Id;
          PACKET_MQTT["Master_Name"] = Master_Name;
          PACKET_MQTT["Node_Name"] = Node_Name;
          PACKET_MQTT["Status"] = LED1_State;
          PACKET_MQTT["speed"] = fan_speed;
          PACKET_MQTT["No_of_Slaves"] = No_of_Slaves;
          PACKET_MQTT["Slave_data"] = Slave_data; // = Status;
          PACKET_MQTT["pairing_status"] = device_condition;
          break;
        }
      case KEEPAlive: {
          PACKET_MQTT["pub_type"] = "KA_pub";
          PACKET_MQTT["Device_Type"] = Device_Type;
          PACKET_MQTT["App_Type"] = App_Type;
          PACKET_MQTT["boot_reason"] = boot_reason;
          PACKET_MQTT["error_code"] = error_code;
          PACKET_MQTT["Mac_Id"] = Mac_Id;
          PACKET_MQTT["Status"] = LED1_State;
          PACKET_MQTT["Slave_data"] = Slave_data; // = Status;
          PACKET_MQTT["pairing_status"] = device_condition;
          break;
        }
    }
  }
  else {
    switch (type) {
      case ADDition: {
          PACKET_MQTT["pub_type"] = "addition_pub";
          PACKET_MQTT["Device_Type"] = Device_Type;
          PACKET_MQTT["App_Type"] = App_Type;
          PACKET_MQTT["FW_Version"] = FirmwareVer;
          PACKET_MQTT["Mac_Id"] = Mac_Id;
          PACKET_MQTT["Master_Name"] = Master_Name;
          PACKET_MQTT["Node_Name"] = Node_Name;
          PACKET_MQTT["Status"] = LED1_State;
          PACKET_MQTT["No_of_Slaves"] = No_of_Slaves;
          PACKET_MQTT["Slave_data"] = Slave_data; // = Status;
          PACKET_MQTT["pairing_status"] = device_condition;
          PACKET_MQTT["Response"] = Response;
          break;
        }
      case DELEtion: {
          PACKET_MQTT["pub_type"] = "deletion_pub";
          PACKET_MQTT["Device_Type"] = Device_Type;
          PACKET_MQTT["App_Type"] = App_Type;
          PACKET_MQTT["FW_Version"] = FirmwareVer;
          PACKET_MQTT["boot_reason"] = boot_reason;
          PACKET_MQTT["error_code"] = error_code;
          PACKET_MQTT["Mac_Id"] = Mac_Id;
          PACKET_MQTT["Master_Name"] = Master_Name;
          PACKET_MQTT["Node_Name"] = Node_Name;
          PACKET_MQTT["Status"] = LED1_State;
          PACKET_MQTT["No_of_Slaves"] = No_of_Slaves;
          PACKET_MQTT["Slave_data"] = Slave_data; // = Status;
          PACKET_MQTT["pairing_status"] = device_condition;
          PACKET_MQTT["Response"] = Response;
          break;
        }
      case CONDition: {
          PACKET_MQTT["pub_type"] = "condition_pub";
          PACKET_MQTT["seq_No"] = seq_No;
          PACKET_MQTT["Device_Type"] = Device_Type;
          PACKET_MQTT["App_Type"] = App_Type;
          PACKET_MQTT["boot_reason"] = boot_reason;
          PACKET_MQTT["error_code"] = error_code;
          PACKET_MQTT["Mac_Id"] = Mac_Id;
          PACKET_MQTT["Master_Name"] = Master_Name;
          PACKET_MQTT["Node_Name"] = Node_Name;
          PACKET_MQTT["Status"] = LED1_State;
          PACKET_MQTT["speed"] = fan_speed;
          PACKET_MQTT["No_of_Slaves"] = No_of_Slaves;
          PACKET_MQTT["Slave_data"] = Slave_data; // = Status;
          PACKET_MQTT["pairing_status"] = device_condition;
          PACKET_MQTT["Response"] = Response;
          break;
        }
    }
  }
  serializeJson(PACKET_MQTT, mss);
  return mss;
}
#endif
