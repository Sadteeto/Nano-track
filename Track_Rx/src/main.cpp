#include <Arduino.h>
#include <RadioLib.h>


//pin definitions from expressLRS repo
#define GPIO_PIN_NSS            5
#define GPIO_PIN_BUSY           21
#define GPIO_PIN_DIO1           4
#define GPIO_PIN_MOSI           23
#define GPIO_PIN_MISO           19
#define GPIO_PIN_SCK            18
#define GPIO_PIN_RST            14
#define GPIO_PIN_RX_ENABLE      27
#define GPIO_PIN_TX_ENABLE      26
#define GPIO_PIN_RCSIGNAL_RX    13
#define GPIO_PIN_RCSIGNAL_TX    13
#define GPIO_PIN_LED_WS2812     15
#define GPIO_PIN_FAN_EN         17



// flag to indicate that a packet was received
volatile bool receivedFlag = false;

// disable interrupt when it's not needed
volatile bool enableInterrupt = true;

// this function is called when a complete packet
// is received by the module
// IMPORTANT: this function MUST be 'void' type
//            and MUST NOT have any arguments!
void setFlag(void) {
  // check if the interrupt is enabled
  if(!enableInterrupt) {
    return;
  }

  // we got a packet, set the flag
  receivedFlag = true;
}
SX1280 radio = new Module(GPIO_PIN_NSS, GPIO_PIN_DIO1, GPIO_PIN_RST, GPIO_PIN_BUSY);


void setup() {
  Serial.begin(9600);
  Serial.print(F("[SX1280] Initializing ... "));
  int state = radio.begin(2500.0, 812.5, 11, 8, 10, -18, 12);
  // radio.setSyncWord(syncword,2);
  // radio.explicitHeader();
  // radio.setCRC(3);


  if (state == RADIOLIB_ERR_NONE) {
    Serial.println(F("success!"));
  } else {
    Serial.print(F("failed, code "));
    Serial.println(state);
    while (true);
  }

  // set the function that will be called
  // when new packet is received
  radio.setDio1Action(setFlag);

  // start listening for LoRa packets
  Serial.print(F("[SX1280] Starting to listen ... "));
  state = radio.startReceive();
  if (state == RADIOLIB_ERR_NONE) {
    Serial.println(F("success!"));
  } else {
    Serial.print(F("failed, code "));
    Serial.println(state);
    while (true);
  }
}


void loop() {
  // check if the flag is set
  if(receivedFlag) {
    // disable the interrupt service routine while
    // processing the data
    enableInterrupt = false;

    // reset flag
    receivedFlag = false;
  
    byte byteArr[23];
    int state = radio.readData(byteArr, 23);
    uint8_t longitude1_1 = {byteArr[0]};
    uint8_t longitude1_2 = {byteArr[1]};
    uint8_t longitude1_3 = {byteArr[2]};
    uint8_t longitude1_4 = {byteArr[3]};
    uint8_t longitude2_1 = {byteArr[4]};
    uint8_t longitude2_2 = {byteArr[5]};
    uint8_t longitude2_3 = {byteArr[6]};
    uint8_t longitude2_4 = {byteArr[7]};
    uint8_t latitude1_1 = {byteArr[8]};
    uint8_t latitude1_2 = {byteArr[9]};
    uint8_t latitude1_3 = {byteArr[10]};
    uint8_t latitude1_4 = {byteArr[11]};
    uint8_t latitude2_1 = {byteArr[12]};
    uint8_t latitude2_2 = {byteArr[13]};
    uint8_t latitude2_3 = {byteArr[14]};
    uint8_t latitude2_4 = {byteArr[15]};
    uint8_t altitude1_1 = {byteArr[16]};
    uint8_t altitude1_2 = {byteArr[17]};
    uint8_t altitude1_3 = {byteArr[18]};
    uint8_t altitude1_4 = {byteArr[19]};
    uint8_t altitude2 = {byteArr[20]};
    char longitude_data_ns = {byteArr[21]};
    char latitude_data_we = {byteArr[22]};
    

    if (state == RADIOLIB_ERR_NONE) {
      // print data of the packet
      uint32_t longitude1 = (longitude1_1 << 24) | (longitude1_2 << 16) | (longitude1_3 << 8) | (longitude1_4);
      uint32_t longitude2 = (longitude2_1 << 24) | (longitude2_2 << 16) | (longitude2_3 << 8) | (longitude2_4);
      uint32_t latitude1 = (latitude1_1 << 24) | (latitude1_2 << 16) | (latitude1_3 << 8) | (latitude1_4);
      uint32_t latitude2 = (latitude2_1 << 24) | (latitude2_2 << 16) | (latitude2_3 << 8) | (latitude2_4);
      int32_t altitude1 = (altitude1_1 << 24) | (altitude1_2 << 16) | (altitude1_3 << 8) | (altitude1_4);

      Serial.print(longitude1,DEC);
      Serial.print(",");
      Serial.print(longitude2,DEC);
      Serial.print(",");
      Serial.print(longitude_data_ns);
      Serial.print(",");
      Serial.print(latitude1,DEC);
      Serial.print(",");
      Serial.print(latitude2,DEC);
      Serial.print(",");
      Serial.print(latitude_data_we);
      Serial.print(",");
      Serial.print(altitude1,DEC);
      Serial.print(",");
      Serial.print(altitude2,DEC);
      Serial.print(",");
      Serial.print(radio.getRSSI());
      Serial.print(",");
      Serial.print(radio.getSNR());
      Serial.println();

    } else if (state == RADIOLIB_ERR_CRC_MISMATCH) {
      // packet was received, but is malformed
      Serial.println(F("CRC error!"));

    } else {
      // some other error occurred
      Serial.print(F("failed, code "));
      Serial.println(state);

    }

    // put module back to listen mode
    radio.startReceive();

    // we're ready to receive more packets,
    // enable interrupt service routine
    enableInterrupt = true;
  }

}
