/*
 * This example sketch shows how you can manage the nRF5 pin mapping as part of your code.
 * You can use the method for any nRF51822 or nRF52832 board or module.
 * 
 * Most components, like UART, SPI, Wire Bus, of the nRF5 series chips don't
 * have a fixed pin mapping. There are some pins with restrictions like analog
 * inputs, NFC or pins near the radio module. Please refer the latest
 * documentation about pin restrictions at http://infocenter.nordicsemi.com 
 * 
 * To use the custom pin mapping you have to do following steps:
 * 
 * 1. Install "arduino-nrf5" like described at
 *    https://github.com/sandeepmistry/arduino-nRF5/
 * 2. Install the "My Sensors nRF5 Boards" with the board manager like
 *    explained at https://github.com/mysensors/ArduinoBoards
 * 3. Copy the files "MyBoardNRF5.cpp" and "MyBoardNRF5.h" from
 *    "MyBoardNRF5" example into your sketch.
 * 4. Modify pin mappings in "MyBoardNRF5.cpp" and "MyBoardNRF5.h" to fit
 *    your requirements.
 * 5. Select "MyBoardNRF5 nrf52832" or "MyBoardNRF5 nrf52822" as your board.
 *    Choose the correct parameters and programmer in the Tools menu.
 */

// Enable debug prints to serial monitor
//#define MY_DEBUG

// Enable passive mode
#define MY_PASSIVE_NODE

// Passive mode requires static node ID
#define MY_NODE_ID 100

// Enable and select radio type attached
//#define MY_RADIO_NRF24
#define MY_RADIO_NRF5_ESB
//#define MY_RADIO_RFM69
//#define MY_RADIO_RFM95

#include <Wire.h>
#include <SI7021.h>
#include <MySensors.h>

#define SHORT_WAIT 50
#define LONG_WAIT 6000

SI7021 sensor;

#define CHILD_ID_TEMP 0
#define CHILD_ID_HUM 1

// Initialize general message
MyMessage msgTemp(CHILD_ID_TEMP, V_TEMP);
MyMessage msgHum(CHILD_ID_HUM, V_HUM);

void blink(uint8_t repetitions) {
  for(int i = 0; i < repetitions; i++) {
    digitalWrite(LED_BUILTIN,HIGH);
    wait(20);
    digitalWrite(LED_BUILTIN,LOW);
    if(i < (repetitions - 1)) {
      wait(500);
    }
  }
}

void setup() {
#ifdef MY_DEBUG
  Serial.begin(115200);
#endif
  
  hwPinMode(LED_BUILTIN, OUTPUT_D0H1);
  blink(1);
  
  if (!sensor.begin()) {
    while(1) {
#ifdef MY_DEBUG
      Serial.println("Sensor error");
#endif
      blink(3);
      sleep(LONG_WAIT);
    }
  }
  
  NRF_CLOCK->INTENSET=B11;  //enable interrupts for EVENTS_HFCLKSTARTED and  EVENTS_LFCLKSTARTED
  NRF_CLOCK->TASKS_HFCLKSTART=1;  //start the high frequency crystal oscillator clock
  while(!(NRF_CLOCK->EVENTS_HFCLKSTARTED)) {} //wait until high frequency crystal oscillator clock is up to speed and working
}

void presentation()
{
  // Send the sketch version information to the gateway and controller
  sendSketchInfo("nRF5-Si7021 node", "1.0");

  // Register all sensors to gw (they will be created as child devices)
  present(CHILD_ID_TEMP, S_TEMP);
  present(CHILD_ID_HUM, S_HUM);
}

void loop() {
  static si7021_env data;
  static int batteryPcnt = 0;
  static int oldBatteryPcnt = 0;

  sleep(LONG_WAIT);

  // get humidity and temperature in one shot, saves power because sensor takes temperature when doing humidity anyway
  data = sensor.getHumidityAndTemperature();

  sleep(LONG_WAIT);
  
  send(msgTemp.set((float)data.celsiusHundredths / 100.0, 2));
  sleep(LONG_WAIT);
  
  send(msgHum.set(data.humidityBasisPoints / 100, 0));
  
  //take voltage measurement after transmission to hopefully measure lowest voltage that occurs.
  batteryPcnt = pow((((float)hwCPUVoltage() / 1000.0) - 2), 4) * 100;  
  if(batteryPcnt > 100) batteryPcnt = 100;
  
  sleep(LONG_WAIT);

  if(oldBatteryPcnt != batteryPcnt) {
      sendBatteryLevel(batteryPcnt);
      oldBatteryPcnt = batteryPcnt;
  }
}

