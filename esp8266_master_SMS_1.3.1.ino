
#include "eeprom_r_w.h"
#include "mq.h"
#include "ai_wifi.h"
#include "var_pin.h"
#include "dim.h"

//#define URL_fw_Version "/programmer131/otaFiles/master/version.txt"
//#define URL_fw_Bin "https://raw.githubusercontent.com/programmer131/otaFiles/master/firmware.bin"


void repeatedCall() {
  unsigned long currentMillis = millis();
  if ((currentMillis - previousMillis) >= interval)
  {
    // save the last time you blinked the LED
    previousMillis = currentMillis;
    setClock();
    FirmwareUpdates();
  }

  if ((currentMillis - previousMillis_2) >= mini_interval) {
    static int idle_counter = 0;
    previousMillis_2 = currentMillis;
    Sprint(" Active fw version:");
    Sprintln(FirmwareVer);
    Sprint("Idle Loop....");
    Sprintln(idle_counter++);
    if (idle_counter % 2 == 0)
      digitalWrite(LED_BUILTIN, HIGH);
    else
      digitalWrite(LED_BUILTIN, LOW);
    if (WiFi.status() == !WL_CONNECTED)
      connect_wifi();
  }
}

void setup()
{
  //  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(SWITCH1_PIN, INPUT_PULLUP);
  pinMode(INC_PIN, INPUT);
  pinMode(DEC_PIN, INPUT_PULLUP);
  BLINK(2, Ai_MEDIUM_BLINK);
  pinMode(current_Sensor, INPUT);
  //  pinMode(LED1_PIN, OUTPUT);
  pinMode(main_led, OUTPUT);
  pinMode(inc_led, OUTPUT);
  pinMode(dec_led, OUTPUT);
  digitalWrite(dec_led, LOW);
  digitalWrite(main_led, HIGH);
  delay(10);
  Sbegin(115200);
  Sprintln("");
  Sprintln("Start");
  Wire.begin();
  EEPROM.begin(200);
  for (byte i = 1; i <= 10; i++)
  {
    Wire.beginTransmission (i);
    if (Wire.endTransmission () == 0)
    {
      Sprint ("i2c scanning address Found address: ");
      Sprint( (i, DEC));
      Sprint (" (0x");
      Sprintln( (i, HEX));
      Sprintln (")");
      count++;
      delay (1);  // maybe unneeded?
    } // end of good response
  }
  Sprint ("i2c device Found ");
  Sprintln (count);
  get_EEprom_status();

  WiFi.mode(WIFI_STA);
  connect_wifi();
  chipids = WiFi.macAddress();
  Sprintln("MAC id : " + chipids);
  //  setClock();
  MQTT_BEG();

  BLINK(4, Ai_FAST_BLINK);
  if (!LED1_State)  digitalWrite(main_led, LOW);
  digitalWrite(inc_led, HIGH);
  digitalWrite(dec_led, HIGH);
  attachInterrupt(digitalPinToInterrupt(SWITCH1_PIN), TRIAC_CTRL, RISING);
  attachInterrupt(digitalPinToInterrupt(INC_PIN), INC_CTRL, RISING);
  attachInterrupt(digitalPinToInterrupt(DEC_PIN), DEC_CTRL, RISING);
  dimmer.begin(NORMAL_MODE, OFF);
  dimmer.setPower(dim);
  //  Wire.beginTransmission(LED);
  //  Wire.write(4);
  //  Wire.endTransmission(); // check i2c
}


void data_receiveds(String topics, String messages) {
  Sprint("received data is");
  Sprintln(messages);
  StaticJsonDocument<200> doc;
  deserializeJson(doc, messages);
  const char* macid = doc["mac_id"];
  if (strcp(macid, chipids)) {
    modes = doc["m"];
    int switch_name = doc["sw"];
    const char* condition = doc["stat"];
    slaves = doc["slave"];
    if (strcp(condition, "on")) state = 1;
    else state = 0;
    Sprint("switch_name = ");
    Sprintln(switch_name);
    Sprint("condition  = ");
    Sprintln(condition);
    Sprint("state  = ");
    Sprintln(state);
    Sprint("slaves = ");
    Sprintln(slaves);

    if (state != Status[slaves]) {
      Status[slaves] = state;
      Sprintln("inside  control");
      if (slaves == 0) {
        //      led condition
        Sprintln("inside master control");
        if (Status[slaves] == 0) //Ai_home.nodes[2];
        {
          dimmer.begin(NORMAL_MODE, OFF);
          digitalWrite(LED1_PIN, LOW);
          digitalWrite(main_led, LOW);
          LED1_State = 0;
          Sprintln("LED1 LOW");
          fan_speed = 0;
        }
        else
        {
          dimmer.begin(NORMAL_MODE, ON);
          digitalWrite(LED1_PIN, HIGH);
          digitalWrite(main_led, HIGH);
          LED1_State = 1;
          Sprintln("LED1 HIGH");
          fan_speed = 4;
        }
        setspeed();
      }
      else {
        //        i2c condition
        Sprintln("I2C loop");
        wire(slaves);
      }
      update_eeprom();
    }
    publisher(1);
  }
}

void del_data_received(String topics, String messages) {
  StaticJsonDocument<200> doc;
  deserializeJson(doc, messages);
  Sprint("received data is");
  Sprintln(messages);
  const char* macid = doc["mac_id"];
  if (strcp(macid, chipids)) {
    //  del_slave = doc["slave"];
    modes = doc["m"];
    del_response_publisher(1);
  }
}

void add_data_received(String topics, String messages) {
  StaticJsonDocument<200> doc;
  deserializeJson(doc, messages);
  Sprint("received data is");
  Sprintln(messages);
  const char* macid = doc["mac_id"];
  if (strcp(macid, chipids)) {
    //  add_slave = doc["slave"];
    modes = doc["m"];
    add_response_publisher(1);
  }
}

void publisher(int8_t rsp) {
  String message;
  StaticJsonDocument<200> docs;
  docs["mac_id"] = WiFi.macAddress();
  docs["node"] = slaves;
  //  docs["sw"] = sub_slave;
  docs["stat"] = Status[slaves];
  docs["rsp"] = rsp;
  serializeJson(docs, message);
  mqtt.publish("receive/response", message);
}

void add_response_publisher(int8_t rsp) {
  String message;
  StaticJsonDocument<200> docs;
  docs["mac_id"] = WiFi.macAddress();
  docs["m"] = modes;
  docs["node"] = slaves;
  docs["num_of_node"] = 8; //slave count
  docs["rsp"] = rsp;
  // json formatt {"mac_id":"ff:gg:hh:ii:kk","m":"2","node":"noename","":"","":"",}
  serializeJson(docs, message);
  mqtt.publish("receive/response", message);
}
void del_response_publisher(int8_t rsp) {
  String message;
  StaticJsonDocument<200> docs;

  docs["mac_id"] = WiFi.macAddress();
  docs["m"] = modes;
  docs["node"] = slaves;// slaves
  docs["rsp"] = rsp;

  serializeJson(docs, message);
  mqtt.publish("receive/response", message);
}

void loop()
{
  //  repeatedCall();
  connect_wifi();
  clients.loop();
  if (!clients.connected()) {
    MQTT_BEG();
  }
  i2c_slave_check();
  KeepALive();
}
