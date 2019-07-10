/*
  18/02/2019 by Long Tran:  create a test project to read x_gyro and y_gyro
  sensor.
*/

//#include "Arduino_FreeRTOS.h"

//#include "timers.h"
// #include <Arduino_FreeRTOS.h>
#include <RF24.h>
#include <SPI.h>
#include <nRF24L01.h>
// #include <semphr.h>
#include "MPU6050.h"
#include "packet.h" //call gyro sensor library

#define CE 5        // pin CE on NRF24L01
#define CSN 10      // pin CSN on NRF24L01
#define PLAYER_NO 9 // choose from 1 - 12

/**
 * @brief For arduino uno:  
 * 
 * MOSI: pin 11
 * MISO: pin 12
 * SCK: pin 13
 *
 */

#define THRESHOLD 1000 // sensitivity value (-32767 to 32768) for direction decision

// Global variables
//  Constants variables
#define button_pin 2
#define led_pin 13
#define DEBUG 0
//  Dynamic variables
int button_state = 0;

// create packet data fromn struct
packet player_package;
int gyro_error_check;
accel_t_gyro_union accel_t_gyro;

// create semaphore to protect data read, write.
// SemaphoreHandle_t binSemaphore_A = NULL;

// TaskHandle_t gyro_TaskHandle;
// TaskHandle_t kick_TaskHandle;
// TaskHandle_t print_TaskHandle;
// TaskHandle_t radio_TaskHandle;

/// radio object for NRF24L01
RF24 radio(CE, CSN); // CE, CSN
// const byte address[6] = "00001";
const uint64_t address[] = {0x7878787878LL, 0xB3B4B5B6F1LL, 0xB3B4B5B6CDLL,
                            0xB3B4B5B6A3LL, 0xB3B4B5B60FLL, 0xB3B4B5B605LL};

void initGyro()
{
  // two variable below store the return data from MPU6050_read() function
  // int gyro_error_check;
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
  if (gyro_error_check == 0)
  {
    Serial.print(F("WHO_AM_I : "));
    Serial.print(c, HEX);
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
  gyro_error_check = MPU6050_read(MPU6050_PWR_MGMT_1, &c, 1);
  if (!gyro_error_check) // similar to gyro_error_check ==0
  {
    Serial.print(F("PWR_MGMT_1 : "));
    Serial.print(c, HEX);
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

void initNRF24()
{
  /// set up NRF24L01
  radio.begin();

  radio.openWritingPipe(address[(PLAYER_NO - 1) % 6]);
  // radio.openWritingPipe(address[0]);
  // radio.setPALevel(RF24_PA_MIN);
  radio.setPALevel(RF24_PA_LOW);
  radio.stopListening();
}

void setup()
{
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

  /* Create binary semaphore */
  // // vSemaphoreCreateBinary(binSemaphore_A);
  // if (!binSemaphore_A) {
  //   Serial.println(F("Creating sem successfully [13]"));
  // } else {
  //   Serial.println(F("Failed to create Semaphore [-11]"));
  // }

  /* Use INT0(pin2) falling edge interrupt for detect button tasks */
  attachInterrupt(digitalPinToInterrupt(button_pin), kickISR, RISING);

  /// create 3 task in FreeRTOS
  // xTaskCreate(readGyro, "Read Gyro", 100, NULL, 3, &gyro_TaskHandle);
  // xTaskCreate(ck_kick_bt, "ck_kick_bt", 100, NULL, 0, &kick_TaskHandle);
  // //xTaskCreate(printStatus, "printStatus", 100, NULL, 2,
  // &print_TaskHandle);
  // xTaskCreate(sendRF, "sendRF", 100, NULL, 0,&radio_TaskHandle);
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
  gyro_error_check = MPU6050_read(MPU6050_ACCEL_XOUT_H, (uint8_t *)&accel_t_gyro,
                                  sizeof(accel_t_gyro));

  // uncomment code below for debugging purposes.
  /*  Serial.print(F("Read accel, temp and gyro, gyro_error_check = "));
Serial.println(gyro_error_check,DEC);
*/

  // swap low and high byte of x_gyro and y_gyro
  swap(&accel_t_gyro.reg.x_gyro_h, &accel_t_gyro.reg.x_gyro_l);
  swap(&accel_t_gyro.reg.y_gyro_h, &accel_t_gyro.reg.y_gyro_l);

  // trigger sem to protect writing packet
  // if (DEBUG) Serial.println(F("readGyro::Acquiring semaphore [123]"));
  // xSemaphoreTake(binSemaphore_A, portMAX_DELAY);

  // Determine direction
  // Compare x_gyro and y_gyro value to a THRESHOLD number - it can be
  // calibrated for the sensitivity process x_gyro
  if (accel_t_gyro.value.x_gyro < -THRESHOLD)
  {
    Serial.print(F("RIGHT \t"));
    player_package.right = 1;
  }
  else if (accel_t_gyro.value.x_gyro > THRESHOLD)
  {
    Serial.print(F("LEFT  \t"));
    player_package.left = 1;
  }
  else
  {
    Serial.print(F("      \t"));
    // player_package.packet_data = player_package.packet_data & 0xFFE7 ;// 11111111 11100111;
    player_package.left = 0;
    player_package.right = 0;
  }
  // process y_gyro
  if (accel_t_gyro.value.y_gyro < -THRESHOLD)
  {
    Serial.print(F("UP   \t"));
    player_package.up = 1;
  }
  else if (accel_t_gyro.value.y_gyro > THRESHOLD)
  {
    Serial.print(F("DOWN  \t"));
    player_package.down = 1;
  }
  else
  {
    Serial.print(F("      \t"));
    player_package.down = 0;
    player_package.up = 0;
  }

  // print shoot status
  if (player_package.kick == 1)
  {
    Serial.print(F("\t SHOOT \n"));
  }
  else
  {
    Serial.print(F("\n"));
  }

  // release sem
  // if (DEBUG) Serial.println(F("readGyro::Releasing semaphore [123]"));
  // xSemaphoreGive(binSemaphore_A);
  // // Serial.println(F("Task1"));
  // vTaskDelay(100 / portTICK_PERIOD_MS);
  // }
}

// /* Task2 with priority 2 */
// static void ck_kick_bt(void *pvParameters) {
//   // vTaskDelete(NULL);
//   while (1) {
//     // Read the state of the push button
//     button_state = digitalRead(button_pin);
//     if (DEBUG) Serial.println(F("ck_kick_bt::Acquiring semaphore [120]"));
//     xSemaphoreTake(binSemaphore_A, portMAX_DELAY);

//     // Check if the push button is pressed
//     // if (button_state == HIGH)
//     // {
//     // Turn ON the LED
//     digitalWrite(led_pin, HIGH);

//     // kick
//     player_package.kick = 1;
//     // }
//     // else
//     // {
//     //   // Turn OFF the LED
//     //   digitalWrite(led_pin, LOW);

//     //   //Kick trigger
//     //   player_package.kick = 1;
//     // }
//     if (DEBUG) Serial.println(F("ck_kick_bt::Release semaphore [121]"));
//     xSemaphoreGive(binSemaphore_A);

//     // Serial.println(F("Task2"));
//     // vTaskDelay(150 / portTICK_PERIOD_MS);
//     vTaskSuspend(NULL);
//   }
// }

/* Idle Task with priority Zero */
static void printStatus()
{
  // vTaskSuspend(kick_TaskHandle);
  // while (1) {
  Serial.println("---RLDUK|--ID--|");
  if (DEBUG)
    Serial.println(F("printStatus::Acquiring semaphore [119]"));
  // xSemaphoreTake(binSemaphore_A, portMAX_DELAY);

  Serial.println(player_package.packet_data, BIN);
  if (DEBUG)
    Serial.println(F("printStatus::Release semaphore [120]"));

  // // reset kick
  // player_package.kick = 0;
  // digitalWrite(led_pin, LOW);
  // xSemaphoreGive(binSemaphore_A);

  // vTaskDelay(150 / portTICK_PERIOD_MS);
  // Serial.println(F("Idle state"));
  // delay(50);
  // }
}

/**
 * @brief send RF function
 * 
 */
static void sendRF()
{
  // vTaskSuspend(kick_TaskHandle);
  // while (1) {
  // Serial.println(player_package.packet_data, BIN);

  // char *text = (char *)malloc(50);
  // strcpy(text,"hello");
  // sprintf2(text, "hello");
  // radio.write(text, strlen(text) + 1);

  // radio.write("hello", strlen(text)+1);
  // xSemaphoreTake(binSemaphore_A, portMAX_DELAY);
  // snprintf(text,50, "%d", player_package.packet_data);

  // itoa(player_package.packet_data, text, 2);
  // radio.write(&text, sizeof(text));
  // Serial.println("sendRF:: ref [48]");
  radio.write(&player_package.packet_data, 2);
  // vTaskDelay(150 / portTICK_PERIOD_MS);
  // radio.write(&player_package.second_byte_data, 1);
  // Serial.println(F("sendRF::print text need to be sent [62]"));
  // Serial.println((text));
  // Serial.println((text));

  Serial.println(player_package.packet_data, BIN);
  Serial.println(player_package.packet_data, DEC);
  Serial.println(player_package.packet_data, HEX);
  // Serial.println(player_package.first_byte_data, BIN);
  // Serial.println(player_package.second_byte_data, BIN);

  // reset kick
  if (player_package.kick == 1)
  {
    player_package.kick = 0;
    digitalWrite(led_pin, LOW);
  }
  // xSemaphoreGive(binSemaphore_A);
  // Serial.println(text);

  // free(text);

  // vTaskDelay(150 / portTICK_PERIOD_MS);
  // Serial.println(F("Idle state"));
  // delay(50);
  // }
}

/**
 * @brief Kick interrupt function
 * 
 */
static void kickISR()
{
  // Turn ON the LED
  digitalWrite(led_pin, HIGH);

  // kick
  player_package.kick = 1;
}

void loop()
{
  // Serial.println("loop::before call readGyro()");
  readGyro();
  // Serial.println("loop::before call printStatus()");
  printStatus();
  // Serial.println("loop::before call sendRF()");
  sendRF();
  // vTaskDelay(200 / portTICK_PERIOD_MS);
  delay(200);
}
