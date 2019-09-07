
/*
* Getting Started example sketch for nRF24L01+ radios
* This is a very basic example of how to send data from one node to another
* Updated: Dec 2014 by TMRh20
*/

#include <SPI.h>
#include "RF24.h"
#include <MPU6050.h>

///create struct gyro to save data from gyro sensor
accel_t_gyro_union gyro;

/****************** User Config ***************************/
/***      Set this radio as radio number 0 or 1         ***/
bool radioNumber = 1;

/* Hardware configuration: Set up nRF24L01 radio on SPI bus plus pins 5 & 10 */
RF24 radio(5, 10);
/**********************************************************/

byte addresses[][6] = {"1Node", "2Node"};

// Used to control whether this node is sending or receiving
bool role = 0;

typedef union {
  uint32_t gyro_data;
  struct
  {
    uint16_t gyro_x : 16;
    uint16_t gyro_y : 16;
  };
} data;

data receive_data;
data gyro_data_send;

///********************************************************************************************************************
///*****************************GYRO INIT CODE - DO NOT TOUCH**********************************************************
///********************************************************************************************************************
int gyro_error_check;
void initGyro()
{
  // the variable below store the return data from MPU6050_read() function
  uint8_t return_data;

  // Initialize the 'Wire' class for the I2C-bus.
  Wire.begin();

  // MPU6050 default at power-up:
  //    Gyro at 250 degrees second
  //    Acceleration at 2g
  //    Clock source at internal 8MHz
  //    The device is in sleep mode.
  //

  gyro_error_check = MPU6050_read(MPU6050_WHO_AM_I, &return_data, 1);

  // no gyro_error_check
  if (gyro_error_check == 0)
  {
    Serial.print(F("WHO_AM_I : "));
    Serial.print(return_data, HEX);
    Serial.print(F(", gyro_error_check = "));
    Serial.println(gyro_error_check, DEC);
  }
  else
  {
    Serial.println("Error in reading GYRO [-911]");
  }

  // According to the datasheet, the 'sleep' bit
  // should read a '1'.
  // That bit has to be cleared, since the sensor
  // is in sleep mode at power-up.

  gyro_error_check = MPU6050_read(MPU6050_PWR_MGMT_1, &return_data, 1);
  if (!gyro_error_check) // if no error
  {
    Serial.print(F("PWR_MGMT_1 : "));
    Serial.print(return_data, HEX);
    Serial.print(F(", gyro_error_check = "));
    Serial.println(gyro_error_check, DEC);
  }
  else
  {
    Serial.println("Error in reading GYRO [-912]");
  }
  // Clear the 'sleep' bit to start the sensor.
  gyro_error_check = MPU6050_write_reg(MPU6050_PWR_MGMT_1, 0);
  // if write successfully
  if (!gyro_error_check)
  {
    Serial.println("Clear the 'sleep' bit successfully [913]");
  }
  else
  {
    Serial.println("Fail to clear the 'sleep' bit [-914]");
  }
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
static void swap(uint8_t *x, uint8_t *y)
{
  uint8_t temp;
  temp = *x;
  *x = *y;
  *y = temp;
}

static void readGyro()
{
  // Read the raw values.
  // Read 4 bytes at once,
  // containing x_gyro (2 bytes) and y_gyro (2 bytes).
  // With the default settings of the MPU-6050,
  // there is no filter enabled, and the values
  // are not very stable. But they are fine for left-right,up-down
  // determination.
  gyro_error_check = MPU6050_read(
      MPU6050_ACCEL_XOUT_H, (uint8_t *)&gyro, sizeof(gyro));

  // uncomment code below for debugging purposes.
  /*  Serial.print(F("Read accel, temp and gyro, gyro_error_check = "));
Serial.println(gyro_error_check,DEC);
*/

  // swap low and high byte of x_gyro and y_gyro
  swap(&gyro.reg.x_gyro_h, &gyro.reg.x_gyro_l);
  swap(&gyro.reg.y_gyro_h, &gyro.reg.y_gyro_l);

  gyro_data_send.gyro_x = gyro.value.x_gyro;
  gyro_data_send.gyro_y = gyro.value.y_gyro;

  // ///print RAW value
  // Serial.print("x_gyro = ");
  // Serial.print(gyro.value.x_gyro);
  // Serial.print("\ty_gyro = ");
  // Serial.print(gyro.value.y_gyro);
  // Serial.print("\n");
}
///********************************************************************************************************************
///*****************************END GYRO INIT CODE***************************************************************************************
///********************************************************************************************************************

void setup()
{
  //init serial port for monitoring purpose
  Serial.begin(115200);

  /// init for GYRO sensor
  initGyro();

  Serial.println(F("Send X,Y gyro via RF24 program"));
  Serial.println(F("*** PRESS 'T' to begin transmitting to the other node"));

  //init RF24 transceiver
  radio.begin();

  // Set the PA Level low to prevent power supply related issues since this is a
  // getting_started sketch, and the likelihood of close proximity of the devices. RF24_PA_MAX is default.
  radio.setPALevel(RF24_PA_LOW);

  // Open a writing and reading pipe on each radio, with opposite addresses
  if (radioNumber)
  {
    radio.openWritingPipe(addresses[1]);
    radio.openReadingPipe(1, addresses[0]);
  }
  else
  {
    radio.openWritingPipe(addresses[0]);
    radio.openReadingPipe(1, addresses[1]);
  }

  // Start the radio listening for data
  radio.startListening();
}

void loop()
{
  //keep updating gyrodata of gyro_x and gyro_y by calling readGyro() function
  readGyro();
  /****************** Ping Out Role ***************************/
  if (role == 1)
  {

    radio.stopListening(); // First, stop listening so we can talk.

    Serial.println(F("Now sending"));

    //print out value of x,y for debugging purpose
    Serial.print("gyro_data_send.gyro_x: =");
    Serial.print(gyro_data_send.gyro_x, DEC);
    Serial.print("\tgyro_data_send.gyro_y: =");
    Serial.println(gyro_data_send.gyro_y, DEC);

    //print out binary data for debugging purpose - comment it when not using
    // Serial.println(gyro_data_send.gyro_x, BIN);
    // Serial.println(gyro_data_send.gyro_y, BIN);
    // Serial.println(gyro_data_send.gyro_data, BIN);
    // Serial.println("");

    ///send Gyro data (x and y) via RF24
    radio.write(&gyro_data_send.gyro_data, sizeof(gyro_data_send));

    ///check time out code. If receiver does not response within 200ms
    radio.startListening();                      // Now, continue listening
    unsigned long started_waiting_at = micros(); // Set up a timeout period, get the current microseconds
    boolean timeout = false;                     // Set up a variable to indicate if a response was received or not

    while (!radio.available())
    { // While nothing is received
      if (micros() - started_waiting_at > 200000)
      { // If waited longer than 200ms, indicate timeout and exit while loop
        timeout = true;
        break;
      }
    }

    if (timeout)
    { // Describe the results
      Serial.println(F("Failed, response timed out."));
    }

    // Try again 1s later
    delay(1000);
  }

  /****************** Pong Back Role ***************************/

  if (role == 0)
  {
    // unsigned long got_time;

    if (radio.available())
    {
      // Variable for the received timestamp
      while (radio.available())
      {                                                            // While there is data ready
        radio.read(&receive_data.gyro_data, sizeof(receive_data)); // Get the payload
      }

      radio.stopListening();

      /// Print binary value for debug purpose - comment when not using
      // Serial.println(receive_data.gyro_x, BIN);
      // Serial.println(receive_data.gyro_y, BIN);
      // Serial.println(receive_data.gyro_data, BIN);
      // Serial.println("");

      // First, stop listening so we can talk
      radio.write(&receive_data.gyro_data, sizeof(receive_data)); // Send the final one back.
      radio.startListening();                                     // Now, resume listening so we catch the next packets.
      Serial.print(F("received: gyro_x =  "));
      Serial.print(receive_data.gyro_x);
      Serial.print(F("\treceived: gyro_y =  "));
      Serial.println(receive_data.gyro_y);
    }
  }

  /****************** Change Roles via Serial Commands ***************************/

  if (Serial.available())
  {
    char c = toupper(Serial.read());
    if (c == 'T' && role == 0)
    {
      Serial.println(F("*** CHANGING TO TRANSMIT ROLE -- PRESS 'R' TO SWITCH BACK"));
      role = 1; // Become the primary transmitter (ping out)
    }
    else if (c == 'R' && role == 1)
    {
      Serial.println(F("*** CHANGING TO RECEIVE ROLE -- PRESS 'T' TO SWITCH BACK"));
      role = 0; // Become the primary receiver (pong back)
      radio.startListening();
    }
  }

} // Loop
