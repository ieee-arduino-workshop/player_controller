/**
 * @brief EXAMPLE CODE TO USE GYRO SENSOR * 
 * 
 * Please read the onnection guide to connect modules to arduino boards Support
 * Arduino boad: Unp, Mega2560
 *Please add library in Libraries folder to Arduino libraries folder
 * Log:
 *  Create code 05/09/2019 by Long Tran:
 *    - library: https://github.com/davedarko/mpu6050
 * 
 */


#include <MPU6050.h>

///---------------------- ---------------------------------------------------------------------
///---------------------- variable for gyro library-----------------------------------------
///---------------------- ---------------------------------------------------------------------
#define THRESHOLD   1000 // sensitivity value (-32767 to 32768) for direction decision

int gyro_error_check;

accel_t_gyro_union accel_t_gyro; // init the struct which save value x,y,z,x_gyro,y_gyro,z_gyro
///---------------------- ---------------------------------------------------------------------
///---------------------- ---------------------------------------------------------------------

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

/* task readGyro with priority 1 */
// static void readGyro(void *pvParameters)
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
  if (accel_t_gyro.value.x_gyro < -THRESHOLD)
  {
    Serial.print(F("RIGHT \t"));
    player_package.right = 1;
    player_package.left = 0;
  }
  else if (accel_t_gyro.value.x_gyro > THRESHOLD)
  {
    Serial.print(F("LEFT  \t"));
    player_package.left = 1;
    player_package.right = 0;
  }
  else
  {
    Serial.print(F("      \t"));
    // player_package.packet_data = player_package.packet_data & 0xFFE7 ;//
    // 11111111 11100111;
    player_package.left = 0;
    player_package.right = 0;
  }
  // process y_gyro
  if (accel_t_gyro.value.y_gyro < -THRESHOLD)
  {
    Serial.print(F("UP   \t"));
    player_package.up = 1;
    player_package.down = 0;
  }
  else if (accel_t_gyro.value.y_gyro > THRESHOLD)
  {
    Serial.print(F("DOWN  \t"));
    player_package.down = 1;
    player_package.up = 0;
  }
  else
  {
    Serial.print(F("      \t"));
    player_package.down = 0;
    player_package.up = 0;
  }
}



/**
 * @brief Arduino setup function
 * Only run once.
 *
 */
void setup()
{

  // Initialize serial port for monitoring
  Serial.begin(115200);

  // call Gyro initialize function
  initGyro();
}

/**
 * @brief Program called in the loop() function will repeatly run.
 * 
 */
void loop()
{
  
  /// Read direction
  readGyro();   
  delay(300);
}
