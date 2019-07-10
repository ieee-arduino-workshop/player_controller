/**
 * @brief player_gyro Arduino program to program for player.
 * This program read movement from gyro sensor, detect kick button and send
 * these data to the central controller via RF module NRF24
 *
 * Please read the onnection guide to connect modules to arduino boards Support
 * Arduino boad: Unp, Mega2560
 *
 * Please add library in Libraries folder to Arduino libraries folder
 * Log:
 *  update 10/07/2019: version 1.1m Clean up the code, add more comments.
 *
 *
 * 18/02/2019 by Long Tran:  create a test project to read x_gyro and y_gyro
 * sensor.
 */

#include <RF24.h>
#include <SPI.h>
#include <nRF24L01.h>
// #include <semphr.h>

#include <MPU6050.h>
#include <packet.h>  //call gyro sensor library

#define PLAYER_NO 9  // choose from 1 - 12

// Global variables
//  Constants variables
#define button_pin 2
#define led_pin 13
#define DEBUG 0

// create packet data from struct
packet player_package;

/// variable for gyro library
#define THRESHOLD \
  1000  // sensitivity value (-32767 to 32768) for direction decision
int gyro_error_check;
accel_t_gyro_union accel_t_gyro;

/**
 * @brief radio object for NRF24L01
 *
 * NRF24 connection For arduino uno:
 *          MOSI: pin 11
 *          MISO: pin 12
 *          SCK: pin 13
 *
 */

#define CE 5          // pin CE on NRF24L01
#define CSN 10        // pin CSN on NRF24L01
RF24 radio(CE, CSN);  // CE, CSN

/** one NRF24 can received maximum 6 different pipes.
 * address[0-5]
 * We send player_package to central controller via NRF24
 * each player attaches a unique ID inside package.
 * As a result, despite of limited 6 pipes, we can add as many players as we
 * want Currently, we take the remainer of "player_ID % 6" to decide address for
 * each player E.g: player_ID=5   --> uses address[5] as 5%6=5 player_ID=1   -->
 * uses address[1] as 1%6=1 player_ID=10  --> uses address[4] as 10%6=4 and so
 * on.
 */
const uint64_t address[] = {0x7878787878LL, 0xB3B4B5B6F1LL, 0xB3B4B5B6CDLL,
                            0xB3B4B5B6A3LL, 0xB3B4B5B60FLL, 0xB3B4B5B605LL};

void initGyro() {
  // the variable below store the return data from MPU6050_read() function
  uint8_t c;

  // Initialize the 'Wire' class for the I2C-bus.
  Wire.begin();

  // MPU6050 default at power-up:
  //    Gyro at 250 degrees second
  //    Acceleration at 2g
  //    Clock source at internal 8MHz
  //    The device is in sleep mode.
  //

  gyro_error_check = MPU6050_read(MPU6050_WHO_AM_I, &c, 1);

  // no gyro_error_check
  if (gyro_error_check == 0) {
    Serial.print(F("WHO_AM_I : "));
    Serial.print(c, HEX);
    Serial.print(F(", gyro_error_check = "));
    Serial.println(gyro_error_check, DEC);
  } else {
    Serial.println("Error in reading GYRO [-911]");
  }

  // According to the datasheet, the 'sleep' bit
  // should read a '1'.
  // That bit has to be cleared, since the sensor
  // is in sleep mode at power-up.

  gyro_error_check = MPU6050_read(MPU6050_PWR_MGMT_1, &c, 1);
  if (!gyro_error_check)  // similar to gyro_error_check ==0
  {
    Serial.print(F("PWR_MGMT_1 : "));
    Serial.print(c, HEX);
    Serial.print(F(", gyro_error_check = "));
    Serial.println(gyro_error_check, DEC);
  } else {
    Serial.println("Error in reading GYRO [-912]");
  }
  // Clear the 'sleep' bit to start the sensor.
  gyro_error_check = MPU6050_write_reg(MPU6050_PWR_MGMT_1, 0);
  // if write successfully
  if (!gyro_error_check) {
    Serial.println("Clear the 'sleep' bit successfully [913]");
  } else {
    Serial.println("Fail to clear the 'sleep' bit [-914]");
  }
}

void initNRF24() {
  /// set up NRF24L01
  radio.begin();

  radio.openWritingPipe(address[(PLAYER_NO - 1) % 6]);
  // radio.openWritingPipe(address[0]);
  // radio.setPALevel(RF24_PA_MIN);
  radio.setPALevel(RF24_PA_LOW);
  radio.stopListening();
}

/**
 * @brief write function SWAP
 *
 * Swap all high and low bytes.
 * After this, the registers values are swapped,
 * Why has to swap?
 * The AVR chip (on the Arduino board) has the Low Byte
 * at the lower address.
 * But the MPU-6050 has a different order: High Byte at
 * lower address, so that has to be corrected.
 *
 */
static void swap(uint8_t *x, uint8_t *y) {
  uint8_t temp;
  temp = *x;
  *x = *y;
  *y = temp;
}

/* task readGyro with priority 1 */
// static void readGyro(void *pvParameters)
static void readGyro() {
  // Read the raw values.
  // Read 4 bytes at once,
  // containing x_gyro (2 bytes) and y_gyro (2 bytes).
  // With the default settings of the MPU-6050,
  // there is no filter enabled, and the values
  // are not very stable. But they are fine for left-right,up-down
  // determination.
  gyro_error_check = MPU6050_read(
      MPU6050_ACCEL_XOUT_H, (uint8_t *)&accel_t_gyro, sizeof(accel_t_gyro));

  // uncomment code below for debugging purposes.
  /*  Serial.print(F("Read accel, temp and gyro, gyro_error_check = "));
Serial.println(gyro_error_check,DEC);
*/

  // swap low and high byte of x_gyro and y_gyro
  swap(&accel_t_gyro.reg.x_gyro_h, &accel_t_gyro.reg.x_gyro_l);
  swap(&accel_t_gyro.reg.y_gyro_h, &accel_t_gyro.reg.y_gyro_l);

  // Determine direction
  // Compare x_gyro and y_gyro value to a THRESHOLD number - it can be
  // calibrated for the sensitivity process x_gyro
  if (accel_t_gyro.value.x_gyro < -THRESHOLD) {
    Serial.print(F("RIGHT \t"));
    player_package.right = 1;
  } else if (accel_t_gyro.value.x_gyro > THRESHOLD) {
    Serial.print(F("LEFT  \t"));
    player_package.left = 1;
  } else {
    Serial.print(F("      \t"));
    // player_package.packet_data = player_package.packet_data & 0xFFE7 ;//
    // 11111111 11100111;
    player_package.left = 0;
    player_package.right = 0;
  }
  // process y_gyro
  if (accel_t_gyro.value.y_gyro < -THRESHOLD) {
    Serial.print(F("UP   \t"));
    player_package.up = 1;
  } else if (accel_t_gyro.value.y_gyro > THRESHOLD) {
    Serial.print(F("DOWN  \t"));
    player_package.down = 1;
  } else {
    Serial.print(F("      \t"));
    player_package.down = 0;
    player_package.up = 0;
  }

  // print shoot status
  if (player_package.kick == 1) {
    Serial.print(F("\t SHOOT \n"));
  } else {
    Serial.print(F("\n"));
  }
}

/**
 * @brief Print packet data
 * @param Data size: 2-byte
 * @param Data contain: 3-bit reserved
 * |Right-bit|Left-bit|Down-bit|Up-bit|Kick-bit|8-bit ID|
 */
static void printStatus() {
  Serial.println("---RLDUK|--ID--|");
  Serial.println(player_package.packet_data, BIN);
}

/**
 * @brief send RF function
 *
 */
static void sendRF() {
  radio.write(&player_package.packet_data, 2);

  /// Debugging package data
  Serial.println(player_package.packet_data, BIN);
  Serial.println(player_package.packet_data, DEC);
  Serial.println(player_package.packet_data, HEX);

  // reset kick
  if (player_package.kick == 1) {
    player_package.kick = 0;
    digitalWrite(led_pin, LOW);
  }
}

/**
 * @brief Kick interrupt function
 *
 */
static void kickISR() {
  // Turn ON the LED
  digitalWrite(led_pin, HIGH);

  // kick
  player_package.kick = 1;
}

/**
 * @brief Arduino setup function
 * Only run once.
 *
 */
void setup() {
  player_package.packet_data = 0xFF00 + PLAYER_NO;
  // Initialize serial port for monitoring
  Serial.begin(115200);
  initGyro();
  initNRF24();

  // Setting the pins
  //  Initialising the LED as an output for testing purposes
  //  Initialsing the button as an input
  pinMode(led_pin, OUTPUT);
  pinMode(button_pin, INPUT);

  /* Use INT0(pin2) falling edge interrupt for detect kick button */
  attachInterrupt(digitalPinToInterrupt(button_pin), kickISR, RISING);
}

void loop() {
  // Serial.println("loop::before call readGyro()");
  readGyro();
  // Serial.println("loop::before call printStatus()");
  printStatus();
  // Serial.println("loop::before call sendRF()");
  sendRF();
  delay(200);
}
