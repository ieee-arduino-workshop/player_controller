/*
	18/02/2019 by Long Tran:  create a test project to read x_gyro and y_gyro sensor. 
*/

//#include "Arduino_FreeRTOS.h"

//#include "timers.h"
#include <Arduino_FreeRTOS.h>
#include <semphr.h>
#include "MPU6050.h"
#include "packet.h" //call gyro sensor library
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

#define CE 5        //pin CE on NRF24L01
#define CSN 10      //pin CSN on NRF24L01
#define PLAYER_NO 1 //choose from 1 - 12

// MOSI: pin 11
//  MISO: pin 12
//  SCK: pin 13

#define THRESHOLD 1000 //sensitivity value (-32767 to 32768) for direction decision

// Global variables
//  Constants variables
#define button_pin 2
#define led_pin 13
#define DEBUG 0
//  Dynamic variables
int button_state = 0;

//create packet data fromn struct
packet Packet;
int error;
accel_t_gyro_union accel_t_gyro;

//create semaphore to protect data read, write.
SemaphoreHandle_t binSemaphore_A = NULL;

TaskHandle_t gyro_TaskHandle;
TaskHandle_t kick_TaskHandle;
TaskHandle_t print_TaskHandle;
TaskHandle_t radio_TaskHandle;

///radio object for NRF24L01
RF24 radio(CE, CSN); // CE, CSN
// const byte address[6] = "00001";
const uint64_t address[] = {0x7878787878LL, 0xB3B4B5B6F1LL, 0xB3B4B5B6CDLL, 0xB3B4B5B6A3LL, 0xB3B4B5B60FLL, 0xB3B4B5B605LL};
void setup()
{
  // two variable below store the return data from MPU6050_read() function
  int error;
  uint8_t c;
  Packet.packet_data = 0xFF01 + PLAYER_NO;
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

  /* Create binary semaphore */
  vSemaphoreCreateBinary(binSemaphore_A);
  if (!binSemaphore_A)
  {
    Serial.println(F("Creating sem successfully [13]"));
  }
  else
  {
    Serial.println(F("Failed to create Semaphore [-11]"));
  }

  ///set up NRF24L01
  radio.begin();
  
  
  radio.openWritingPipe(address[PLAYER_NO/6]);
  // radio.setPALevel(RF24_PA_MIN);
  radio.setPALevel(RF24_PA_LOW);
  radio.stopListening();

  /* Use INT0(pin2) falling edge interrupt for detect button tasks */
  attachInterrupt(digitalPinToInterrupt(button_pin), Detect_kick_bt, FALLING);

  //create 3 task in FreeRTOS
  xTaskCreate(read_gyro, "Read Gyro", 100, NULL, 3, &gyro_TaskHandle);
  xTaskCreate(ck_kick_bt, "ck_kick_bt", 100, NULL, 0, &kick_TaskHandle);
  //xTaskCreate(print_status, "print_status", 100, NULL, 2, &print_TaskHandle);
  xTaskCreate(RF_send, "RF_send", 100, NULL, 0, &radio_TaskHandle);
}

static void Detect_kick_bt()
{
  BaseType_t taskYieldRequired = 0;
  if (DEBUG)
    Serial.println(F("ISR Resuming task ck_kick_bt"));
  taskYieldRequired = xTaskResumeFromISR(kick_TaskHandle);
  if (DEBUG)
    Serial.println(F("Leaving ISR"));
  if (taskYieldRequired == 1)
  {
    taskYIELD();
  }
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
}

/* task read_gyro with priority 1 */
static void read_gyro(void *pvParameters)
{
  vTaskSuspend(kick_TaskHandle);
  while (1)
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

    //trigger sem to protect writing packet
    if (DEBUG)
      Serial.println(F("read_gyro::Acquiring semaphore [123]"));
    xSemaphoreTake(binSemaphore_A, portMAX_DELAY);

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
    if (accel_t_gyro.value.y_gyro < -THRESHOLD)
    {
      Serial.print(F("UP   \t"));
      Packet.up = 1;
    }
    else if (accel_t_gyro.value.y_gyro > THRESHOLD)
    {
      Serial.print(F("DOWN  \t"));
      Packet.down = 1;
    }
    else
    {
      Serial.print(F("      \t"));
      Packet.down = 0;
      Packet.up = 0;
    }

    //print shoot status
    if (Packet.kick == 1)
    {
      Serial.print(F("\t SHOOT \n"));
    }
    else
    {
      Serial.print(F("\n"));
    }

    //release sem
    if (DEBUG)
      Serial.println(F("read_gyro::Releasing semaphore [123]"));
    xSemaphoreGive(binSemaphore_A);
    //Serial.println(F("Task1"));
    vTaskDelay(100 / portTICK_PERIOD_MS);
  }
}

/* Task2 with priority 2 */
static void ck_kick_bt(void *pvParameters)
{
  // vTaskDelete(NULL);
  while (1)
  {
    // Read the state of the push button
    button_state = digitalRead(button_pin);
    if (DEBUG)
      Serial.println(F("ck_kick_bt::Acquiring semaphore [120]"));
    xSemaphoreTake(binSemaphore_A, portMAX_DELAY);

    // Check if the push button is pressed
    // if (button_state == HIGH)
    // {
    // Turn ON the LED
    digitalWrite(led_pin, HIGH);

    //kick
    Packet.kick = 1;
    // }
    // else
    // {
    //   // Turn OFF the LED
    //   digitalWrite(led_pin, LOW);

    //   //Kick trigger
    //   Packet.kick = 1;
    // }
    if (DEBUG)
      Serial.println(F("ck_kick_bt::Release semaphore [121]"));
    xSemaphoreGive(binSemaphore_A);

    // Serial.println(F("Task2"));
    //vTaskDelay(150 / portTICK_PERIOD_MS);
    vTaskSuspend(NULL);
  }
}

/* Idle Task with priority Zero */
static void print_status(void *pvParameters)
{
  vTaskSuspend(kick_TaskHandle);
  while (1)
  {

    Serial.println("---RLDUK|--ID--|");
    if (DEBUG)
      Serial.println(F("print_status::Acquiring semaphore [119]"));
    xSemaphoreTake(binSemaphore_A, portMAX_DELAY);

    Serial.println(Packet.packet_data, BIN);
    if (DEBUG)
      Serial.println(F("print_status::Release semaphore [120]"));

    //reset kick
    Packet.kick = 1;
    digitalWrite(led_pin, LOW);
    xSemaphoreGive(binSemaphore_A);

    vTaskDelay(150 / portTICK_PERIOD_MS);
    //Serial.println(F("Idle state"));
    // delay(50);
  }
}

static void RF_send(void *pvParameters)
{
  vTaskSuspend(kick_TaskHandle);
  while (1)
  {

    // Serial.println(Packet.packet_data, BIN);

    //char *text = (char *)malloc(50);
    //strcpy(text,"hello");
    //sprintf2(text, "hello");
    //radio.write(text, strlen(text) + 1);

    // radio.write("hello", strlen(text)+1);
    xSemaphoreTake(binSemaphore_A, portMAX_DELAY);
    //snprintf(text,50, "%d", Packet.packet_data);

    //itoa(Packet.packet_data, text, 2);
    // radio.write(&text, sizeof(text));
    radio.write(&Packet.packet_data, 2);
    // vTaskDelay(150 / portTICK_PERIOD_MS);
    // radio.write(&Packet.second_byte_data, 1);
    //Serial.println(F("RF_send::print text need to be sent [62]"));
    //Serial.println((text));
    //Serial.println((text));

    Serial.println(Packet.packet_data, BIN);
    Serial.println(Packet.packet_data, DEC);
    Serial.println(Packet.packet_data, HEX);
    // Serial.println(Packet.first_byte_data, BIN);
    // Serial.println(Packet.second_byte_data, BIN);

    //reset kick
    Packet.kick = 0;
    digitalWrite(led_pin, LOW);
    xSemaphoreGive(binSemaphore_A);
    // Serial.println(text);

    // free(text);

    vTaskDelay(150 / portTICK_PERIOD_MS);
    //Serial.println(F("Idle state"));
    // delay(50);
  }
}
