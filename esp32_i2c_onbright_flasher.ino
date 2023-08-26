// Jonathan Armstrong
// 8/25/2023
// attempt to behave like the MSM9066 flasher on inexpensive arduino platform (using ESP32 here)
// traces of the flasher are found at: https://github.com/mightymos/msm9066_capture


#include <Wire.h>

// this monitors status of 3.3V on target board
// has a pull down resistor to ground
// should probably also have some series resistance or other isolation
int vccMonitorPin = 19;
int sdaPin = 21;
int sclPin = 22;

// store past and current state of monitored pin
int oldValue = 0;
int value = 0;

// track how many times nacks occur as comparison to traces of original MSM9066 flasher
int errorCount = 0;

// track handshake status
enum { idle,
       powerup,
       retry,
       ackdetected };

// target flash memory addresses
#define BLOCK_SIZE 512

enum { block00 = 0x0000,
       block01 = 0x0200,
       block02 = 0x0400,
       block03 = 0x0600,
       block04 = 0x0800,
       block05 = 0x0A00,
       block06 = 0x0C00,
       block07 = 0x0E00,
       block08 = 0x1000,
       block09 = 0x1200,
       block10 = 0x1400,
       block11 = 0x1600,
       block12 = 0x1800,
       block13 = 0x1A00,
       block14 = 0x1C00,
       block15 = 0x1E00
};

unsigned char state = idle;

// FIXME: provide link to website that provided this
void checkError(const byte error) {
  switch (error) {
    case 0:
      Serial.println("Success");
      break;
    case 1:
      Serial.println("Data too long to fit in transmit buffer");
      break;
    case 2:
      Serial.println("NACK on transmit of address");
      break;
    case 3:
      Serial.println("NACK on transmit of data");
      break;
    case 4:
      Serial.println("Other error");
      break;
    case 5:
      Serial.println("Timeout");
      break;
  }
}

// should be 0x0A for OnBright OBS38S003 8051 based microcontroller
void readChipType(void) {
  byte data;

  // check chip type
  Wire.beginTransmission(0x7e);
  Wire.write(0x09);
  Wire.write(0x00);
  Wire.endTransmission();

  Wire.requestFrom(0x7f, 1);
  while (Wire.available()) {
    data = Wire.read();
    Serial.print("chip type: ");
    Serial.println(data, HEX);
  }
}

void eraseChip(void) {
  // erase chip
  Serial.println("Erasing chip...");

  Wire.beginTransmission(0x7e);
  Wire.write(0x03);
  Wire.endTransmission();
}

// TODO: not working across entire block size, yet
// read 512 bytes
// there are 16 blocks * 512 bytes = 8192 bytes total flash memory
void readBlock(unsigned int blockAddress) {
  byte data;

  unsigned char highByte = (blockAddress >> 8) & 0xff;
  unsigned char lowByte  = blockAddress & 0xff;

  // erase chip
  Wire.beginTransmission(0x7e);
  Wire.write(0x06);
  Wire.write(highByte);
  Wire.write(lowByte);
  Wire.endTransmission();

  // FIXME: we can only read 256 bytes with this function
  //        but chip can send 512 bytes at a time
  Wire.requestFrom(0x7f, 255);
  while (Wire.available()) {
    data = Wire.read();
    Serial.print(data, HEX);
    Serial.print(",");
  }
}

// TODO:
void writeBlock(void)
{

}

void setup() {
  // explicitly set pins on ESP32 because I do not know what is default
  // choose slower 100 kHz speed so that it is hopefully more reliable than higher speed
  Wire.begin(sdaPin, sclPin, 100000);

  Serial.begin(115200);

  //
  pinMode(vccMonitorPin, INPUT);

  // read initial value
  value = digitalRead(vccMonitorPin);

  // sanity check - should be low if power supply off, high with power supply on
  if (value) {
    Serial.println("vcc - high");
  } else {
    Serial.println("vcc - low");
  }
}


void loop() {
  byte error;
  byte address;
  byte data;

  // delay does not seem necessary for stable operation but could be inserted if needed
  //delay(10);

  // step through handshaking procedure once power supply is turned on
  // (or reset to idle if power supply is turned off)
  switch (state) {
    case idle:
      // check for power supply being turned on
      oldValue = value;
      value = digitalRead(vccMonitorPin);
      if (!oldValue && value) {
        Serial.println("low-to-high transition");
        state = powerup;
      }

      break;

    case powerup:
      // follow handshake procedure observed in traces
      Wire.beginTransmission(0x7e);
      error = Wire.endTransmission();
      checkError(error);

      errorCount = 0;
      state = retry;
      break;

    case retry:
      // keep retrying command until we observe an ACK
      Wire.beginTransmission(0x7c);
      error = Wire.endTransmission();
      checkError(error);

      if (error == 0) {
        Serial.println("ACK detected");
        state = ackdetected;
      } else {
        errorCount++;
      }

      // if power supply is turned off during retry, go back to idle state
      oldValue = value;
      value = digitalRead(vccMonitorPin);
      if (oldValue && !value) {
        Serial.println("high-to-low transition");
        Serial.println("reset state to idle");
        state = idle;
      }

      break;

    case ackdetected:
      // once we get ACK, continue handshake
      Serial.print("error (or nack) count: ");
      Serial.println(errorCount);

      // handshake
      Wire.beginTransmission(0x7d);
      error = Wire.endTransmission();
      checkError(error);

      Wire.beginTransmission(0x2d);
      error = Wire.endTransmission();
      checkError(error);

      // no actual read is performed apparently
      Wire.requestFrom(0x7e, 1);

      // issue various commands to target microcontroller
      readChipType();

      eraseChip();

      readChipType();

      readBlock(block00);

      state = idle;
      break;
  }
}