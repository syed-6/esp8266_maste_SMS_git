#include "mq.h"
#include <time.h>
#include "var_pin.h"


void connect_wifi();
void setClock();

void connect_wifi()
{
  if (WiFi.status() != WL_CONNECTED) {
    WiFi.begin(ssid, password);
    wifi_timer = millis();
    while ((millis() - wifi_timer) < Ai_WIFI_INTERVAL)
    {
      if(WiFi.status() == WL_CONNECTED) break;
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
