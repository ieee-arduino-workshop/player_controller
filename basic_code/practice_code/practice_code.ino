
/*
* Getting Started example sketch for nRF24L01+ radios
* This is a very basic example of how to send data from one node to another
* Updated: Dec 2014 by TMRh20
*/

#include <SPI.h>
#include "RF24.h"
#include <MPU6050_tockn.h>
#include <Wire.h>

///call object mpu6050
MPU6050 mpu6050(Wire);

/****************** User Config ***************************/
/***      Set this radio as radio number 0 or 1         ***/
bool radioNumber = 0;

/* Hardware configuration: Set up nRF24L01 radio on SPI bus plus pins 7 & 8 */
RF24 radio(5, 10);
/**********************************************************/

byte addresses[][6] = {"2Node", "1Node"};

// Used to control whether this node is sending or receiving
bool role = 0;

//
typedef struct data
{
  uint16_t gyro_x;
  uint16_t gyro_y;
};

data receive_data;

void setup()
{
  Serial.begin(115200);

  /// setup for GYRO sensor
  Wire.begin();
  mpu6050.begin();
  mpu6050.calcGyroOffsets(true);

  Serial.println(F("Send X,Y gyro via RF24 program"));
  Serial.println(F("*** PRESS 'T' to begin transmitting to the other node"));

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
  //keep updating gyrodata
  mpu6050.update();
  data gyro_data_send;
  gyro_data_send.gyro_x = mpu6050.getGyroX();
  gyro_data_send.gyro_y = mpu6050.getGyroY();
  /****************** Ping Out Role ***************************/
  if (role == 1)
  {

    radio.stopListening(); // First, stop listening so we can talk.

    Serial.println(F("Now sending"));

    unsigned long start_time = micros(); // Take the time, and send it.  This will block until complete

    ///send GyroX via RF24
    if (!radio.write(&gyro_data_send, sizeof(gyro_data_send)))
    {
      Serial.println(F("failed"));
    }
   
    radio.startListening(); // Now, continue listening

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
    else
    {
      unsigned long got_time; // Grab the response, compare, and send to debugging spew
      radio.read(&got_time, sizeof(unsigned long));
      unsigned long end_time = micros();

      // Spew it
      Serial.print(F("Sent "));
      Serial.print(start_time);
      Serial.print(F(", Got response "));
      Serial.print(got_time);
      Serial.print(F(", Round-trip delay "));
      Serial.print(end_time - start_time);
      Serial.println(F(" microseconds"));
    }

    // Try again 200ms later
    delay(500);
  }

  /****************** Pong Back Role ***************************/

  if (role == 0)
  {
    // unsigned long got_time;

    if (radio.available())
    {
      // Variable for the received timestamp
      while (radio.available())
      {                                               // While there is data ready
        radio.read(&receive_data, sizeof(receive_data)); // Get the payload
      }

      radio.stopListening();                         // First, stop listening so we can talk
      radio.write(&receive_data, sizeof(receive_data)); // Send the final one back.
      radio.startListening();                        // Now, resume listening so we catch the next packets.
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
