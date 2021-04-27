
#include "eeprom_r_w.h"
#include "mq.h"
#include "ai_wifi.h"
#include "var_pin.h"
#include "dim.h"

//#define URL_fw_Version "/programmer131/otaFiles/master/version.txt"
//#define URL_fw_Bin "https://raw.githubusercontent.com/programmer131/otaFiles/master/firmware.bin"

/*add app._type sevice status(either it is added or not) in eeprom */
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
  print_wakeup_reason();
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
  print_app_type();
  if (device_condition) {
    Sprintln ("device is paired with web ");
  }
  else Sprintln ("device is not paired with web ");
  WiFi.mode(WIFI_STA);
  connect_wifi();
  Mac_Id = WiFi.macAddress();
  Sprintln("MAC id : " + Mac_Id);
  //  setClock();
  MQTT_BEG();


  BLINK(4, Ai_FAST_BLINK);
  if (!LED1_State)  digitalWrite(main_led, LOW);
  States = LED1_State;
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
  No_of_Slaves = count;
  if (!clients.connected()) {
    wakeup_publish();

  }
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
      Response = true;
    }
    publisher();
  }
}

void del_data_received(String topics, String messages) {
  StaticJsonDocument<200> doc;
  deserializeJson(doc, messages);
  Sprint("received data is");
  Sprintln(messages);
  const char* macid = doc["mac_id"];
  if (strcp(macid, chipids)) {
    /*condition to be write*/
    App_Type = doc["App_Type"] ;
    if (remove_device(App_Type)) Response = true;
    else Response = false;
    del_response_publisher();
  }
}

void add_data_received(String topics, String messages) {
  StaticJsonDocument<200> doc;
  deserializeJson(doc, messages);
  Sprint("received data is");
  Sprintln(messages);
  const char* macid = doc["mac_id"];
  if (strcp(macid, chipids)) {
    App_Type = doc["App_Type"] ;
    if (add_device(App_Type)) Response = true;
    else Response = false;
    add_response_publisher();
  }
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
