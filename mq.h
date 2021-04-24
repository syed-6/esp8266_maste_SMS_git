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
void publisher(int8_t );
void del_response_publisher(int8_t );
void KeepALive ();
void FirmwareUpdates();
void update_publisher(t_httpUpdate_return );

WiFiClient espClient;
PubSubClient clients(MQTT_SERVER, 1883, espClient);
PubSubClientTools mqtt(clients);

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

      mqtt.subscribe("testing/add_node",  addition_topic_subscriber);
      mqtt.subscribe("testing/delete_node",    delete_topic_subscriber);
      mqtt.subscribe("testing/condition/#",        condition_topic_subscriber);
      mqtt.subscribe("testing/update/#", update_topic_subscriber);
      mqtt.subscribe("testing/fw/#", fw_topic_subscriber);
    } else {
      Sprintln("failed, rc=" + clients.state());
    }
  }
}

void FirmwareUpdates()
{
  WiFiClientSecure client;
  client.setTrustAnchors(&cert);
  if (!client.connect(host, httpsPort)) {
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
        update_publisher(ret);
        break;

      case HTTP_UPDATE_NO_UPDATES:
        Sprintln("HTTP_UPDATE_NO_UPDATES");
        update_publisher(ret);
        break;

      case HTTP_UPDATE_OK:
        Sprintln("HTTP_UPDATE_OK");
        update_publisher(ret);
        break;
    }
  }
}

void KeepALive() {
  if ((millis() - keepalive_timer) > KEEP_ALIVE_INTERVAL ) {
    keepalive_timer = millis();
    String message;
    StaticJsonDocument<200> docs;
    docs["mac_id"] = WiFi.macAddress();
    serializeJson(docs, message);
    if (!clients.connected()) {
      MQTT_BEG();
    }
    mqtt.publish("receive/Keepalive", message);
    Sprintln("KEEP alive done");
  }
}
void update_publisher(t_httpUpdate_return res) {
  StaticJsonDocument<200> docs;
  String mes;
  docs["mac_id"] = WiFi.macAddress();
  docs["update_status"] = res;
  serializeJson(docs, mes);
  mqtt.publish("receive/update_response", mes);
}
#endif
