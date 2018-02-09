//
//  esp-serial-lcd.ino
//
//  Requirements:
//    - Akiduki ESP-WROOM-02 development board
//      - http://akizukidenshi.com/catalog/g/gK-12236/
//    - Akiduki AE-AQM0802 LCD module
//      - http://akizukidenshi.com/catalog/g/gK-06795///
//    - EspSoftwareSerial
//      - https://github.com/plerup/espsoftwareserial/
//  
//  Copyright and license:
//    Copyright(c) 2018 yoggy<yoggy0@gmail.com>
//    Released under the MIT license
//    http://opensource.org/licenses/mit-license.php;
//
#include <Wire.h>
#include <SoftwareSerial.h> // https://github.com/plerup/espsoftwareserial/

#define READ_BUF_SIZE 32
char read_buf[READ_BUF_SIZE];
int read_buf_idx = 0;

char message_buf[64];

SoftwareSerial software_serial(13, 15, false, 256);  // RX pin, TX pin, inverse_logic, bufsize

uint32_t st_m = 0;
uint32_t epoch = 0;
uint32_t st_epoch = 0;
uint32_t last_received_epoch = 0;

void log(char *msg) {
  Serial.println(msg);
  aqm0802_clear();
  aqm0802_print(msg);
}

void setup() {
  clear_read_buffer();

  Serial.begin(115200);
  software_serial.begin(19200);
  Wire.begin(14, 12); //SDA, SCL
  aqm0802_init();

  log("serial-lcd");
  delay(1000);
}

void loop() {
  process_serial();
  process_idle();
}

void process_idle() {
  char buf[64];

  uint32_t diff_m = millis() - st_m;
  if (diff_m > 1000) {
    epoch ++;

    // display idle sceen...
    if (epoch - last_received_epoch >= 10) {
      aqm0802_clear();
      switch (epoch % 2) {
        case 0:
          aqm0802_print("+");
          break;
        case 1:
          aqm0802_print(" ");
          break;
      }
    }

    st_m = millis();
  }
}

void process_serial() {
  // read loop...
  if (software_serial.available() > 0) {
    while (software_serial.available() > 0) {
      char c = software_serial.read();
      int n = c;
      Serial.println(n, HEX);

      // CR:0x0D, LF:0x0A,
      if (c == 0x0D) {
        // noting to do...
      }
      else if (c == 0x0A) {
        process_buffer();
      }
      else {
        read_buf[read_buf_idx] = c;
        read_buf_idx ++;
        if (read_buf_idx == READ_BUF_SIZE - 1) {
          process_buffer();
        }
      }
    }
  }
}

void clear_read_buffer() {
  memset(read_buf, 0, READ_BUF_SIZE);
  read_buf_idx = 0;
}

void process_buffer() {

  // serial console...
  Serial.print("receive=");
  Serial.println(read_buf);

  // display message
  char buf[64];
  memset(buf, 0, 64);
  sprintf(buf, "recv:%s", read_buf);
  aqm0802_clear();
  aqm0802_print(buf);

  clear_read_buffer();

  last_received_epoch = epoch;
}

