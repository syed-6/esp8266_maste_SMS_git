#include <Wire.h>
#include<EEPROM.h>
#include "var_pin.h"


void get_EEprom_status();
void read_wifi_cre();
void wire( int );
void update_eeprom();
void update_status();
void i2c_eeprom_write_byte( int , unsigned int , byte  );
byte i2c_eeprom_read_byte( int , int  );
void i2c_slave_check();

void i2c_eeprom_write_byte( int deviceaddress, unsigned int eeaddress, byte data )
{
  int rdata = data;
  Wire.beginTransmission(deviceaddress);
  Wire.write((int)(eeaddress ));    // Address High Byte
  //  Wire.write((int)(eeaddress & 0xFF));  // Address Low Byte
  Sprint("the writting data to eeprom is:");
  Sprintln(rdata);
  Wire.write(rdata);
  if (Wire.endTransmission() == 0) {
    Sprintln("Device found");
  } else Sprintln("Device not found");
}

byte i2c_eeprom_read_byte( int deviceaddress, int eeaddress )
{
  byte rdata = 0;
  Wire.beginTransmission(deviceaddress);
  Wire.write((int)(eeaddress ));    // Address High Byte
  //  Wire.write((int)(eeaddress & 0xFF));  // Address Low Byte
  //  if(Wire.endTransmission()==0){Sprintln("Device found");}else Sprintln("Device not found");
  Wire.endTransmission();
  Wire.requestFrom(deviceaddress, 1);
  if (Wire.available()) rdata = Wire.read();
  //  Serial.write(char(rdata));
  return rdata;
}
void read_wifi_cre() {
  Sprint("Reading The ssid:");
  char ss[30], pp[30] = {'1'};
  // get the ssid from the external eeprom
  delay(25);
  eeprom_ssid_addr = 0;
  for (int i = 0; i == 0 || ss[i - 1] != '\0'; i++) { // loop for first 20 slots
    ss[i] = char(i2c_eeprom_read_byte(EEPROM_ADDR, eeprom_ssid_addr));
    delay(4);
    eeprom_ssid_addr++;
    //    Sprint(ss);
  }

  sside = ss;
  Sprint("Reading The password:");
  // get the password from the external eeprom
  eeprom_password_addr = 30;
  delay(25);
  for (int i = 0; i == 0 || pp[i - 1] != '\0'; i++) { // loop for first 20 slots
    pp[i] = (i2c_eeprom_read_byte(EEPROM_ADDR, eeprom_password_addr));
    delay(4);
    eeprom_password_addr++;
    //    Sprint(pp);
  }
  delay(25);
  passworde = pp;
}

void get_EEprom_status() {
  Sprintln("getting the state from eeprom......");
  for (int i = 0; i <= 10; i++) {
    Status[i] = EEPROM.read(i);
  }
  //set led pin and i2c pin data
  update_status();
}


void update_status() {
  Sprintln("reflecting the state from eeprom......");
  if (Status[0] == 0) //Ai_home.nodes[2];
  {
    digitalWrite(LED1_PIN, LOW);
    LED1_State = 0;
    digitalWrite(LED1_PIN, LOW);
    digitalWrite(main_led, LOW);
    Sprintln("LED1 LOW");
  }
  else
  {
    digitalWrite(LED1_PIN, HIGH);
    LED1_State = 1;
    digitalWrite(LED1_PIN, HIGH);
    digitalWrite(main_led, HIGH);
    Sprintln("LED1 HIGH");
  }
  if (count > 0) {
    Sprint("counts is "); Sprintln(count);

    for (int addr = 1; addr <= MAX_I2C_DEV + 1; addr++) { // have to change
      Sprint("address = "); Sprintln(addr);
      wire(addr);
    }
  }
  update_eeprom();
}

void update_eeprom() {
  Sprint("updating eeprom finally");
  for (int i = 0; i < 10; i++) {
    if (EEPROM.read(i) != Status[i]) {
      EEPROM.write(i, Status[i]);
      delay(1);
    }
  }
  EEPROM.commit();
}

void wire( int addr) {
  if (count > 0)
  { Wire.beginTransmission(addr);//+8);//slave_I2c_addr[addr];
    Wire.write(Status[addr]); //slave_I2c_state_new[slave_I2c_addr[addr]];
    Wire.endTransmission();
    Sprint("SENT TO Sub slave addr....0x0");
    Sprintln(addr);//slave_I2c_addr[addr];
    Wire.requestFrom(addr, 1); //slave_I2c_addr[addr];
    if (Wire.available())
    {
      Status[addr] = Wire.read(); //slave_I2c_state_new[slave_I2c_addr[addr]];
      Sprint("RECEIVED FROM Sub slave");
      Sprintln(Status[addr]); //slave_I2c_state_new[slave_I2c_addr[addr]];
    }
    else {
      Sprint("NOT RECEIVED FROM Sub slave status=");
      Status[addr] = 0; //slave_I2c_state_new[slave_I2c_addr[addr]];
      Sprintln(Status[addr]); //slave_I2c_state_new[slave_I2c_addr[addr]];
    }
  }
  delay(10);
}

void i2c_slave_check() {
  if ((millis() - i2c_check_time) > i2c_check_interval) {
    i2c_check_time = millis();
    if (count > 0) {
      for (int addr = 1; addr <= MAX_I2C_DEV + 1; addr++) {
        Sprint("address = "); Sprintln(addr);
        Wire.requestFrom(addr, 1); //slave_I2c_addr[addr];
        if (Wire.available())
        {
          slave_I2c_state_prev[addr] = Wire.read(); //slave_I2c_state_new[slave_I2c_addr[addr]];
          Sprint("RECEIVED FROM Sub slave");
          Sprintln(slave_I2c_state_prev[addr]); //slave_I2c_state_new[slave_I2c_addr[addr]];
        }
        else {
          Sprint("NOT RECEIVED FROM Sub slave status=");
          slave_I2c_state_prev[addr] = -1; //slave_I2c_state_new[slave_I2c_addr[addr]];
          Sprintln(slave_I2c_state_prev[addr]); //slave_I2c_state_new[slave_I2c_addr[addr]];
        }
      }
      for (int s = 1; s <= MAX_I2C_DEV + 1; s++) {
        if (slave_I2c_state_prev[s] != Status[s]) {
          updates = 1;
          Status[s] = slave_I2c_state_prev[s];
        }
      }
      if (updates) {
        updates = 0;
        update_eeprom();
      }
    }
  }
}
