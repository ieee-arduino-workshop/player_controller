/*
	18/02/2019 by Long Tran:  create a test project to read x_gyro and y_gyro sensor. 
*/
#include "MPU6050.h"
#include "packet.h"    //call gyro sensor library
#define THRESHOLD 5000 //sensitivity value (-32767 to 32768) for direction decision

// Global variables
//  Constants variables
#define button_pin 2
#define led_pin 13
//  Dynamic variables
int button_state = 0;

//create packet data fromn struct
packet Packet;
int error;
accel_t_gyro_union accel_t_gyro;

void setup()
{
  // two variable below store the return data from MPU6050_read() function
  int error;
  uint8_t c;
  Packet.packet_data = 0xFFFF;
  // Initialize serial port for monitoring
  Serial.begin(115200);

  // Initialize the 'Wire' class for the I2C-bus.
  Wire.begin();

  // MPU6050 default at power-up:
  //    Gyro at 250 degrees second
  //    Acceleration at 2g
  //    Clock source at internal 8MHz
  //    The device is in sleep mode.
  //
  error = MPU6050_read(MPU6050_WHO_AM_I, &c, 1);
  if (error == 0) //no error
  {
    Serial.print(F("WHO_AM_I : "));
    Serial.print(c, HEX);
    Serial.print(F(", error = "));
    Serial.println(error, DEC);
  }
  else
  {
    Serial.println("Error in reading GYRO [-911]");
  }
  // According to the datasheet, the 'sleep' bit
  // should read a '1'.
  // That bit has to be cleared, since the sensor
  // is in sleep mode at power-up.
  error = MPU6050_read(MPU6050_PWR_MGMT_1, &c, 1);
  if (!error) //similar to error ==0
  {
    Serial.print(F("PWR_MGMT_1 : "));
    Serial.print(c, HEX);
    Serial.print(F(", error = "));
    Serial.println(error, DEC);
  }
  else
  {
    Serial.println("Error in reading GYRO [-912]");
  }
  // Clear the 'sleep' bit to start the sensor.
  error = MPU6050_write_reg(MPU6050_PWR_MGMT_1, 0);
  if (!error) //if write successfully
  {
    Serial.println("Clear the 'sleep' bit successfully [913]");
  }
  else
  {
    Serial.println("Fail to clear the 'sleep' bit [-914]");
  }

  // Setting the pins
  //  Initialising the LED as an output for testing purposes
  //  Initialsing the button as an input
  pinMode(led_pin, OUTPUT);
  pinMode(button_pin, INPUT);
}

// Swap all high and low bytes.
// After this, the registers values are swapped,
// Why has to swap?
// The AVR chip (on the Arduino board) has the Low Byte
// at the lower address.
// But the MPU-6050 has a different order: High Byte at
// lower address, so that has to be corrected.
// write function SWAP
uint8_t swap;
#define SWAP(x, y) \
  swap = x;        \
  x = y;           \
  y = swap

void loop()
{
  // Read the raw values.
  // Read 4 bytes at once,
  // containing x_gyro (2 bytes) and y_gyro (2 bytes).
  // With the default settings of the MPU-6050,
  // there is no filter enabled, and the values
  // are not very stable. But they are fine for left-right,up-down determination.
  error = MPU6050_read(MPU6050_ACCEL_XOUT_H, (uint8_t *)&accel_t_gyro, sizeof(accel_t_gyro));

  //uncomment code below for debugging purposes.
  /*  Serial.print(F("Read accel, temp and gyro, error = "));
	  Serial.println(error,DEC);
	*/

  //swap low and high byte of x_gyro and y_gyro
  SWAP(accel_t_gyro.reg.x_gyro_h, accel_t_gyro.reg.x_gyro_l);
  SWAP(accel_t_gyro.reg.y_gyro_h, accel_t_gyro.reg.y_gyro_l);

  // Determine direction
  // Compare x_gyro and y_gyro value to a THRESHOLD number - it can be calibrated for the sensitivity
  // process x_gyro
  if (accel_t_gyro.value.x_gyro < -THRESHOLD)
  {
    Serial.print(F("RIGHT \t"));
    Packet.right = 1;
  }
  else if (accel_t_gyro.value.x_gyro > THRESHOLD)
  {
    Serial.print(F("LEFT  \t"));
    Packet.left = 1;
  }
  else
  {
    Serial.print(F("      \t"));
    //Packet.packet_data = Packet.packet_data & 0xFFE7 ;// 11111111 11100111;
    Packet.left = 0;
    Packet.right = 0;
  }
  //process y_gyro
  if (accel_t_gyro.value.y_gyro > THRESHOLD)
  {
    Serial.print(F("UP   \n"));
    Packet.up = 1;
  }
  else if (accel_t_gyro.value.y_gyro < -THRESHOLD)
  {
    Serial.print(F("DOWN  \n"));
    Packet.down = 1;
  }
  else
  {
    Serial.print(F("      \n"));
    Packet.down = 0;
    Packet.up = 0;
  }
  Serial.println("---RLDUK|--ID--|");

  //a = itoa(Packet.packet_data, a, 2);
  Serial.println(Packet.packet_data, BIN);
  //Serial.println(a);

  // Read the state of the push button
  button_state = digitalRead(button_pin);

  // Check if the push button is pressed
  if (button_state == HIGH)
  {
    // Turn ON the LED
    digitalWrite(led_pin, HIGH);

    // no kick
    Packet.kick = 0;
  }
  else
  {
    // Turn OFF the LED
    digitalWrite(led_pin, LOW);

    //Kick trigger
    Packet.kick = 1;
  }

  delay(50);
}