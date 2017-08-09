#include <dht.h>          // We are using the DHT library found at http://playground.arduino.cc/Main/DHTLib
#include <LedMatrix.h>    // Custom animation library found at https://github.com/velvet-valdes/ledMatrix_lib.git

/*
   These are the humble beginnings of the "obvious detector".  A completely pointless device that does nothing of import yet something that you just can't live without in the 21st century.

   Notes:

   • Lower values indicate a brighter light source, higher values indicate dimmer light source.  If the state of the light sensor is LOW (read as 0) it detects light.  If the sensor state is HIGH (read as 1) there is no light detected.
*/

dht DHT;  // Here we are initializing the DHT object

// Initialize a matrix object and pass values for its banks
Matrix matrixMain(4, 5, 6, 7, 8, 9, 10, 11);

// Initialize global variables - we may need to change the circuit from a pull down config on the interrupt buttons to a pull up config and hardware debounce it with a capacitor
int bankState;	                  // initializing a value here to start the counter without resetting it throughout the loop - we should have a private variable for this.  I need to check
int timeFloat;                    // we're going to use this to store the value the digital light meter is reading and use it to set the 'throttle'
int timeFade;                     // using this to set a combination of humidity and temp to change the sleep fade duration
int cycleCount;                   // this is how many times through the animation we are going to run
int button01;                     // our left button read pin on the arduino
int button02;                     // our right button read pin on the arduino
int buttonState01;
int buttonState02;
int lightMeterIn = A0;            // this is our analog light intensity read pin (INPUT)
int lightMeterState = 13;         // this is our digital light state read pin which determines if the light is ON or OFF (INPUT)
int valLightMeter;                // we are storing the value returned from the light meter here
int valLightState;                // we are storing the value of the light state here.  0=LOW=ON 1=HIGH=OFF - we are going to use this for the "sleep" mode
int valTemp;                      // this is our temp reading in farenheit
int valHumidity;                  // this is our relative humidity value in percentage
const byte interruptPinUp = 2;    // Setting our first interrupt pin
const byte interruptPinDown = 3;  // Setting our second interrupt pin
volatile byte state = HIGH;       // We are setting the byte state to HIGH because we are in a PULLDOWN config with our resistor instead of the commonly found PULLUP config.  But it seems like im still having bounce issues with this config

int Red = A1;                     // Analog pin for Red "Im Hot!"
int Green = A2;                   // Analog pin for Green "This is fine.."
int Blue = A3;                    // Analog pin for Blue "Im Cold!"

// Set the pin we are going to read the data on for the temp humidity sensor
#define DHT11_PIN 12

// Here we are making a simple function to satisfy the ISR requriement of attachInterrupt and increment the bankState variable
void countUp() {
  bankState++;
  if (bankState > 6) {
    bankState = 6;
  }
}

// Here we are making a simple function to satisfy the ISR requriement of attachInterrupt and decrement the bankState variable
void countDown() {
  bankState--;
  if (bankState < 0) {
    bankState = 0;
  }
}

void setup() {

  Serial.begin(9600);
  // Set the read pins for the buttons
  button01 = 2;
  button02 = 3;
  // Increment and decrement buttons via the interrupt pins.  We are pulling down and have implemented the resistors on the breadboard.
  pinMode(interruptPinUp, INPUT);
  pinMode(interruptPinDown, INPUT);
  pinMode(Red, OUTPUT);
  pinMode(Green, OUTPUT);
  pinMode(Blue, OUTPUT);
  attachInterrupt(digitalPinToInterrupt(interruptPinUp), countUp, RISING);
  attachInterrupt(digitalPinToInterrupt(interruptPinDown), countDown, RISING);
  // Set the pins we are going to read data from the light intensity meter
  pinMode(lightMeterIn, INPUT);
  pinMode(lightMeterState, INPUT);
  //----------------------- Write Initial Values of 0 to outputs
  analogWrite(Red, 0);
  analogWrite(Green, 0);
  analogWrite(Blue, 0);

}

void loop() {
  // We are keeping the sensor from being polled too quickly and returning OOB results
  if (millis() % 15 == 0) {
    int chk = DHT.read11(DHT11_PIN);
  }
  // Temp Humidity and Light variables
  valTemp = ((DHT.temperature) * (1.8) + (32));
  valHumidity = DHT.humidity;
  valLightMeter = analogRead(lightMeterIn);
  valLightState = digitalRead(lightMeterState);
  // We have the digital light meter controlling the time values of the ON duration
  timeFloat = (1000 - valLightMeter) / 16;
  // We are taking the average of the temp and humidity and using that as the duration value.  The more hot and humid it is, the faster the fade cycle becomes.
  timeFade = (100 - (valHumidity + valTemp) / 2) / 10;
  // Changing the cycle count to a single count now that we have a button selecting states.  This way the default is a non-zero integer
  cycleCount = 1;

  // Serial Monitor Print Block:
  Serial.println(" ");
  Serial.print("Temperature = ");
  Serial.println(valTemp);
  Serial.print("Humidity = ");
  Serial.println(valHumidity);
  Serial.print("Light Meter Value = ");
  Serial.println(valLightMeter);
  Serial.print("Light Sensor State = ");
  Serial.println(valLightState);
  Serial.print("Bank State = ");
  Serial.println(bankState);
  Serial.println(" ");
  Serial.println("Time Fade (Duration): ");
  Serial.println(timeFade);

  // Temp Indicator
  if (valTemp < 60) {
    analogWrite(Red, 0);
    analogWrite(Green, 0);
    analogWrite(Blue, 255);
  }
  if ((valTemp >= 60) && (valTemp <= 80)) {
    analogWrite(Red, 0);
    analogWrite(Green, 255);
    analogWrite(Blue, 0);
  }
  if (valTemp > 80) {
    analogWrite(Red, 0);
    analogWrite(Green, 0);
    analogWrite(Blue, 255);
  }

  if (valLightState == 0) {
    switch (bankState) {
      case 0:
        // statements
        matrixMain.sweepRowUp(cycleCount, timeFloat, 1);
        matrixMain.sweepUpRight(cycleCount, timeFloat, 1);
        matrixMain.sweepUpLeft(cycleCount, timeFloat, 1);
        matrixMain.sweepUpRight(cycleCount, timeFloat, 1);
        matrixMain.sweepUpLeft(cycleCount, timeFloat, 1);
        matrixMain.sweepRowUp(cycleCount, timeFloat, 1);
        matrixMain.cycleForwardBravo(cycleCount, timeFloat, 1);
        matrixMain.cycleBackwardBravo(cycleCount, timeFloat, 1);
        break;
      case 1:
        // statements
        matrixMain.sweepRowDown(cycleCount, timeFloat, 1);
        matrixMain.sweepDownRight(cycleCount, timeFloat, 1);
        matrixMain.sweepDownLeft(cycleCount, timeFloat, 1);
        matrixMain.sweepDownRight(cycleCount, timeFloat, 1);
        matrixMain.sweepDownLeft(cycleCount, timeFloat, 1);
        matrixMain.sweepRowDown(cycleCount, timeFloat, 1);
        matrixMain.cycleBackwardAlpha(cycleCount, timeFloat, 1);
        matrixMain.cycleForwardAlpha(cycleCount, timeFloat, 1);
        break;
      case 2:
        // statements
        matrixMain.sweepWhiteDown(cycleCount, timeFloat, 1);
        matrixMain.sweepWhiteUp(cycleCount, timeFloat, 1);
        break;
      case 3:
        // statements
        matrixMain.sweepRedDown(cycleCount, timeFloat, 1);
        matrixMain.sweepRedUp(cycleCount, timeFloat, 1);
        break;
      case 4:
        // statements
        matrixMain.randomThree(cycleCount, timeFloat, 1);
        break;
      case 5:
        // statements
        matrixMain.blinkWhite(cycleCount, 500, 500);
        break;
      case 6:
        // statements
        matrixMain.blinkRed(cycleCount, 500, 500);
        break;
    }
  }

  else {
    if (valLightState == 1) {
      switch (bankState) {

        case 0:
          // statements
          matrixMain.fadeRed(1, timeFade, 1);
          break;

        case 1:
          // statements
          matrixMain.fadeRandom(1, timeFade, 1);
          break;
        case 2:
          // statements
          matrixMain.fadeRandom(2, timeFade, 1);
          matrixMain.rowUp(1, timeFade * 100, timeFade * 100);
          matrixMain.fadeRandom(2, timeFade, 1);
          break;
        case 3:
          // statements
          matrixMain.fadeRandom(2, timeFade, 1);
          matrixMain.rowDown(1, timeFade * 100, timeFade * 100);
          matrixMain.fadeRandom(2, timeFade, 1);
          break;
      }
    }
  }
}
