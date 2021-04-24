#ifndef VAR_PIN_H

#define VAR_PIN_H

//#define debug true
#if defined(debug)
#define Sprintln(a) (Serial.println(a))
#define Sprint(b) (Serial.print(b))
#define Sbegin(c)  (Serial.begin(c))
#else
#define Sprintln(a)
#define Sprint(a)
#define Sbegin(c)
#endif

#define Ai_FAST_BLINK               200
#define Ai_MEDIUM_BLINK             500
#define Ai_SLOW_BLINK               1000

#define Ai_DELAY_SECONDS            1000
#define Ai_DELAY_MINUTES            Ai_DELAY_SECONDS*60
#define KEEP_ALIVE_INTERVAL         Ai_DELAY_MINUTES*1  // 1 min interval
#define MAX_I2C_DEV                 7
#define SWITCH_INTERRUPT_DELAY      500
#define INC_DEC_INTERRUPT_DELAY     200
#define Ai_WIFI_INTERVAL            Ai_DELAY_SECONDS*5

#define SDA_PIN                     A4 //change
#define SCL_PIN                     A5  //change

#define LED1_PIN                     0//16
#define SWITCH1_PIN                 14
#define INC_PIN                     12
#define DEC_PIN                     15
#define main_led                    16 //0
#define dec_led                      1
#define inc_led                      2
#define current_Sensor              A0
#define zero_crossing               13

#define EEPROM_ADDR                 0x51
#define MQTT_SERVER                 "3.128.32.110"
//#define MQTT_SERVER                 "3.128.32.110" // our mqtt ip
#define zerocross                   zero_crossing
#define URL_fw_Version              "/syed-6/esp8266_maste_SMS_git/main/bin_version.txt.TXT"
#define URL_fw_Bin                  "https://raw.githubusercontent.com/syed-6/esp8266_maste_SMS_git/main/esp8266_master_SMS.ino.nodemcu.bin"

const char* host = "raw.githubusercontent.com";
const int httpsPort = 443;

const char* ssid = "Circuit-7";
const char* password = "eee1guy007";

int eeprom_ssid_addr = 0;
int eeprom_password_addr = 30;
extern char* sside = "-------------------------------";
extern char* passworde = "--------------------------------";

int Status[10] = {0};
String chipids;
int updates = 0;
int state = 0;
String FirmwareVer = {
  "1.3.1"
};



volatile boolean LED1_State = false;

unsigned long previousMillis_2 = 0;
unsigned long previousMillis = 0;        // will store last time LED was updated
unsigned long i2c_check_time = 0;
unsigned long keepalive_timer = 0;
const long interval = 60000;
const long mini_interval = 1000;
const long i2c_check_interval = 10000;
unsigned long last_interrupted = 0;
unsigned long wifi_timer = 0;


int dim = 140;
int fan_speed = 0, set_dim = 0;

extern const unsigned char caCert[] PROGMEM;
extern const unsigned int caCertLen;

int slave_I2c_state_prev[10] = {0}; //byte
int count;
int prev_stat = 0;
int modes, slaves, rsp = 0;

void BLINK(int c, int y) {

  for (int i = 0; i < c; i++) {
    digitalWrite(inc_led, LOW);
    delay(y);
    digitalWrite(inc_led, HIGH);
    delay(y);
  }
}

int strcp(const char*x, String y) {
  for (int i = 0 ; y[i] != '\0'; i++) {
    if (x[i] != y[i]) {
      Sprintln("Mac id not matched");
      return 0;
    }
  }
  Sprintln("Mac id matched");
  return 1;
}


#endif
