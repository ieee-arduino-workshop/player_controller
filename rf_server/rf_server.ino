/*
* Arduino Wireless Communication Tutorial
*       Example 1 - Receiver Code
*                
* by Dejan Nedelkovski, www.HowToMechatronics.com
* 
* Library: TMRh20/RF24, https://github.com/tmrh20/RF24/
*/
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include "packet.h"

#define CE 5
#define CSN 10

RF24 radio(CE, CSN); // CE, CSN
const byte address[6] = "00001";
packet player1;
void setup()
{
  Serial.begin(115200);
  radio.begin();
  radio.openReadingPipe(0, address);
  radio.setPALevel(RF24_PA_MIN);
  radio.startListening();
}

void print_data(packet player)
{
  Serial.print("-----------------------------------------------\n");
  Serial.print("Player ID: ");
  Serial.println(player1.player_id, BIN);

  if (player.left == 1)
  {
    Serial.print(F("LEFT  \t"));
  }
  else if (player.right == 1)
  {
    Serial.print(F("RIGHT  \t"));
  }

  if (player.up == 1)
  {
    Serial.print(F("UP  \t"));
  }
  else if (player.down == 1)
  {
    Serial.print(F("DOWN  \t"));
  }

  if (player.kick == 1)
  {
    Serial.print(F("SHOOT  \n"));
  }
  else
  {
    Serial.print(F("\n-------------------------------------------\n"));
  }
}

uint16_t prev_value = 0;
void loop()
{
  if (radio.available())
  {
    //uint16_t text;
    // radio.read(&text, sizeof(text));
    //radio.read(&text, sizeof(text));
    //Serial.println(text,BIN);
    radio.read(&player1.packet_data, sizeof(player1.packet_data));
    //Serial.println(player1.packet_data, BIN);

    if (prev_value != player1.packet_data)
    {
      print_data(player1);
    }
    prev_value = player1.packet_data;
  }
}