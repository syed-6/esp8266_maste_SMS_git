#include "mq.h"
#include <time.h>
#include "var_pin.h"


void connect_wifi();
void setClock();
void print_wakeup_reason();

void connect_wifi()
{
  if (WiFi.status() != WL_CONNECTED) {
    WiFi.begin(ssid, password);
    wifi_timer = millis();
    while ((millis() - wifi_timer) < Ai_WIFI_INTERVAL)
    {
      if (WiFi.status() == WL_CONNECTED) break;
      delay(500);
      Sprint("O");
    }
    Sprintln("Connected to WiFi");
    Sprintln("IP address: ");
    Sprintln(WiFi.localIP());
  }
}

void setClock() {
  // Set time via NTP, as required for x.509 validation
  configTime(3 * 3600, 0, "pool.ntp.org", "time.nist.gov");
  Sprint("Waiting for NTP time sync: ");
  time_t now = time(nullptr);
  while (now < 8 * 3600 * 2) {
    delay(500);
    Sprint(".");
    now = time(nullptr);
  }

  Sprintln("");
  struct tm timeinfo;
  gmtime_r(&now, &timeinfo);
  Sprint("Current time: ");
  Sprint(asctime(&timeinfo));

}

void print_wakeup_reason() {

  switch (ESP.getResetInfoPtr()->reason) {

    case REASON_DEFAULT_RST:
      // do something at normal startup by power on
      strcpy_P(buff, PSTR("Power on"));
      boot_reason = 0;
      break;

    case REASON_WDT_RST:
      // do something at hardware watch dog reset
      strcpy_P(buff, PSTR("Hardware Watchdog"));
      boot_reason = -3;
      break;

    case REASON_EXCEPTION_RST:
      // do something at exception reset
      strcpy_P(buff, PSTR("Exception"));
      boot_reason = -2;
      break;

    case REASON_SOFT_WDT_RST:
      // do something at software watch dog reset
      strcpy_P(buff, PSTR("Software Watchdog"));
      boot_reason = -1;
      break;

    case REASON_SOFT_RESTART:
      // do something at software restart ,system_restart
      strcpy_P(buff, PSTR("Software/System restart"));
      boot_reason = 1;
      break;

    case REASON_DEEP_SLEEP_AWAKE:
      // do something at wake up from deep-sleep
      strcpy_P(buff, PSTR("Deep-Sleep Wake"));
      boot_reason = 2;
      break;

    case REASON_EXT_SYS_RST:
      // do something at external system reset (assertion of reset pin)
      strcpy_P(buff, PSTR("External System"));
      boot_reason = 3;
      break;

    default:
      // do something when reset occured for unknown reason
      strcpy_P(buff, PSTR("Unknown"));
      boot_reason = -4;
      break;
  }
  Sprint("the wake up reason is  ");
  Sprintln(buff);
  Sprint("and the boot reason is  ");
  Sprintln(boot_reason);
}
void print_app_type() {
  switch (App_Type) {
    case 0: {
        Sprintln("the device type is fan ");
        break;
      }
    case 1: {
        Sprintln("the device type is switch ");
        break;
      }
    case 2: {}


  }
}
