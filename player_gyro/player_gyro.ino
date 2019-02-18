/*
	18/02/2019 by Long Tran:  create a test project to read x_gyro and y_gyro sensor. 
*/
#include"MPU6050.h" //call gyro sensor library
#define THRESHOLD 5000 //sensitivity value (0 - 65535) for direction decision

void setup()
{      
	// two variable below store the return data from MPU6050_read() function
  int error;	
  uint8_t c; 

	// Initialize serial port for monitoring
  Serial.begin(9600);
  
  // Initialize the 'Wire' class for the I2C-bus.
  Wire.begin();


  // MPU6050 default at power-up:
  //    Gyro at 250 degrees second
  //    Acceleration at 2g
  //    Clock source at internal 8MHz
  //    The device is in sleep mode.
  //
  error = MPU6050_read (MPU6050_WHO_AM_I, &c, 1);
  Serial.print(F("WHO_AM_I : "));
  Serial.print(c,HEX);
  Serial.print(F(", error = "));
  Serial.println(error,DEC);

  // According to the datasheet, the 'sleep' bit
  // should read a '1'.
  // That bit has to be cleared, since the sensor
  // is in sleep mode at power-up. 
	error = MPU6050_read (MPU6050_PWR_MGMT_1, &c, 1);
  Serial.print(F("PWR_MGMT_1 : "));
  Serial.print(c,HEX);
  Serial.print(F(", error = "));
  Serial.println(error,DEC);

  // Clear the 'sleep' bit to start the sensor.
  MPU6050_write_reg (MPU6050_PWR_MGMT_1, 0);
}

void loop()
{
  int error;
  accel_t_gyro_union accel_t_gyro;

  // Read the raw values.
  // Read 4 bytes at once, 
  // containing x_gyro (2 bytes) and y_gyro (2 bytes).
  // With the default settings of the MPU-6050,
  // there is no filter enabled, and the values
  // are not very stable. But they are fine for left-right,up-down determination.
  error = MPU6050_read (MPU6050_ACCEL_XOUT_H, (uint8_t *) &accel_t_gyro, sizeof(accel_t_gyro));
	
	//uncomment code below for debugging purposes.
	/*  Serial.print(F("Read accel, temp and gyro, error = "));
	  Serial.println(error,DEC);
	*/

  // Swap all high and low bytes.
  // After this, the registers values are swapped, 
  // Why has to swap?
	// The AVR chip (on the Arduino board) has the Low Byte 
	// at the lower address.
	// But the MPU-6050 has a different order: High Byte at
	// lower address, so that has to be corrected.
	// write function SWAP
	uint8_t swap;
  #define SWAP(x,y) swap = x; x = y; y = swap

	//swap low and high byte of x_gyro and y_gyro
  SWAP (accel_t_gyro.reg.x_gyro_h, accel_t_gyro.reg.x_gyro_l);
  SWAP (accel_t_gyro.reg.y_gyro_h, accel_t_gyro.reg.y_gyro_l);

	// Determine direction
	// Compare x_gyro and y_gyro value to a THRESHOLD number - it can be calibrated for the sensitivity 
		// process x_gyro
	if(accel_t_gyro.value.x_gyro<-THRESHOLD)
		Serial.print(F("RIGHT \t"));  
  else
    if(accel_t_gyro.value.x_gyro>THRESHOLD) 
			Serial.print(F("LEFT  \t"));  
    else
			Serial.print(F("      \t"));  
							
		//process y_gyro
  if(accel_t_gyro.value.y_gyro>THRESHOLD)
		Serial.print(F("UP   \n"));  
  else
    if(accel_t_gyro.value.y_gyro<-THRESHOLD)
			Serial.print(F("DOWN  \n"));  
    else
			Serial.print(F("      \n"));          
  delay(100);
}
