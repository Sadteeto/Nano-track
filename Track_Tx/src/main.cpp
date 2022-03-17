#include <Arduino.h>
#include <RadioLib.h>


//pin definitions from expressLRS repo
#define GPIO_PIN_NSS            15
#define GPIO_PIN_BUSY           5
#define GPIO_PIN_DIO1           4
#define GPIO_PIN_MOSI           13
#define GPIO_PIN_MISO           12
#define GPIO_PIN_SCK            14
#define GPIO_PIN_RST            2
#define GPIO_PIN_LED_RED        16

uint16_t i;
byte output_array [23];
SX1280 radio = new Module(GPIO_PIN_NSS, GPIO_PIN_DIO1, GPIO_PIN_RST, GPIO_PIN_BUSY);

void setup() {
  Serial.begin(115200);
  Serial.setTimeout(200);

  // initialize SX1280 with default settings
  // Serial.print(F("[SX1280] Initializing ... "));
  // carrier frequency:           2500.0 MHz
  // bandwidth:                   812.5 kHz
  // spreading factor:            11
  // coding rate:                 8/4
  // sync word:                   10
  // output power:                13 dBm
  // preamble length:             12 symbols
  // CRC:                         enabled

  int state = radio.begin(2500.0, 812.5, 11, 8, 10, 13, 12);

  if (state == RADIOLIB_ERR_NONE) {
    // Serial.println(F("success!"));
  } else {
    // Serial.print(F("failed, code "));
    // Serial.println(state);
    while (true);
  }
}

String getValue(String data, char separator, int index)
{
    int found = 0;
    int strIndex[] = { 0, -1 };
    int maxIndex = data.length() - 1;

    for (int i = 0; i <= maxIndex && found <= index; i++) {
        if (data.charAt(i) == separator || i == maxIndex) {
            found++;
            strIndex[0] = strIndex[1] + 1;
            strIndex[1] = (i == maxIndex) ? i+1 : i;
        }
    }
    return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
}

void loop() {
  String msg = Serial.readString();
  String partial_msg = msg;
  // delay(1000);
  if(msg != ""){
    // remove front part of unwanted string and only get stuff after GAA
    // GAA: Global positioning system fix data
    for(i = 0; i < msg.length(); i++){
      if (msg[i] == 'G'){
        if ((msg[i+1] =='G') && (msg[i+2] =='A')){
          partial_msg.remove(0,i+4);
          // SerialUSB.println(partial_msg);
          i = 0;
          break;
        }
      }

    }
    // remove later parts of unwanted string
    for (i = 0; i < partial_msg.length(); i++){
        if (partial_msg[i] == '\n'){
          // SerialUSB.println(i,DEC);
          partial_msg.remove(i,partial_msg.length()-i);
          i = 0;
          break;
        }
    }

    // get Longitude and Latitude
    String longitude_data = getValue(partial_msg, ',', 1);
    // if there is data then the gps is fixed
    if (longitude_data != ""){
      // get longitude
      uint32_t longitude1 = getValue(longitude_data, '.', 0).toInt();
      uint8_t longitude1_1 = (longitude1 >> 24);
      uint8_t longitude1_2 = (longitude1 >> 16);
      uint8_t longitude1_3 = (longitude1 >> 8);
      uint8_t longitude1_4 = longitude1;

      uint32_t longitude2 = getValue(longitude_data, '.', 1).toInt();
      uint8_t longitude2_1 = (longitude2 >> 24);
      uint8_t longitude2_2 = (longitude2 >> 16);
      uint8_t longitude2_3 = (longitude2 >> 8);
      uint8_t longitude2_4 = longitude2;

      String longitude_data_ns = getValue(partial_msg, ',', 2);
      
      // get Latitude
      String latitude_data = getValue(partial_msg, ',', 3);
      uint32_t latitude1 = getValue(latitude_data, '.', 0).toInt();
      uint8_t latitude1_1 = (latitude1 >> 24);
      uint8_t latitude1_2 = (latitude1 >> 16);
      uint8_t latitude1_3 = (latitude1 >> 8);
      uint8_t latitude1_4 = latitude1;
      uint32_t latitude2 = getValue(latitude_data, '.', 1).toInt();
      uint8_t latitude2_1 = (latitude2 >> 24);
      uint8_t latitude2_2 = (latitude2 >> 16);
      uint8_t latitude2_3 = (latitude2 >> 8);
      uint8_t latitude2_4 = latitude2;

      String latitude_data_we = getValue(partial_msg, ',', 4);
      
      //calculate altitude
      String altitude_data = getValue(partial_msg, ',', 8);
      int32_t altitude1 = getValue(altitude_data, '.', 0).toInt();
      uint8_t altitude1_1 = (altitude1 >> 24);
      uint8_t altitude1_2 = (altitude1 >> 16);
      uint8_t altitude1_3 = (altitude1 >> 8);
      uint8_t altitude1_4 = altitude1;

      uint8_t altitude2 = getValue(altitude_data, '.', 1).toInt();

      // construct byet array to send through serial
      // longitude followed by latitude followed by altitude
      byte output_array [23] = {longitude1_1, longitude1_2, longitude1_3, longitude1_4, longitude2_1, longitude2_2, longitude2_3, longitude2_4, latitude1_1, latitude1_2, latitude1_3, latitude1_4, latitude2_1, latitude2_2, latitude2_3, latitude2_4, altitude1_1, altitude1_2, altitude1_3, altitude1_4, altitude2, longitude_data_ns.charAt(0), latitude_data_we.charAt(0)};

      int state = radio.transmit(output_array, 23);
    }
    
    // else the gps is not fixed 
    // transmit with empty array
    else {
      uint8_t none = 0;
      byte empty_array [23] = {none, none, none, none, none, none, none, none, none, none, none, none, none, none, none, none, none, none, none, none, none, none};
      int state = radio.transmit(empty_array, 23);
    }
  }
}