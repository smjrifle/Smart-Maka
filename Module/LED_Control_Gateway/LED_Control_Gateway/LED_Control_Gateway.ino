/*
 * Copyright (c) 2013 by Felix Rusu <felix@lowpowerlab.com>
 *
 * This file is free software; you can redistribute it and/or modify
 * it under the terms of either the GNU General Public License version 2
 * or the GNU Lesser General Public License version 2.1, both as
 * published by the Free Software Foundation.
 */
 
// This is a GATEWAY sketch (LEDControl_Gateway) for a Moteino that can act as
// a coordinator to control the LEDs on other Moteinos loaded
// with the corresponding NODE sketches (LEDControl_Node)
// Designed by Felix Rusu (felix@lowpowerlab.com), www.LowPowerLab.com

// The LEDs on the endpoint Moteinos can be controlled as follows. In the GATEWAY's serial terminal:
// Type a node ID in the rage [2-9] to set the target node
// Type a LED mode [b,p,o] - blink, pulse, off
// Type a speed for the LED action [s,m,f] - slow, medium, fast
// Hit [ENTER] to send request to target
// The target Moteino receives the message and applies the requested LED mode and speed

#include <RFM12B.h>
#include <SoftwareSerial.h>

SoftwareSerial mySerial(5, 6); // RX, TX

#define NETWORKID       212
#define NODEID            1        // network ID used for this unit
#define KEY "ABCDABCDABCDABCD"     //(16 bytes of your choice - keep the same on all encrypted nodes)
#define REQUESTACK     true        // whether to request ACKs for sent messages
#define ACK_TIME         50        // # of ms to wait for an ack
#define SERIAL_BAUD  9600
#define LED               9

uint8_t input;
RFM12B radio;
char mode = 'w';      //mode : 'b' = blinking, 'p' = pulsing, 'o' = 'off'
char pinState = '0';
byte nodeId = 2;      //node to send message to
char * sendBuf="x:y"; //format is MODE:SPEED

void displayMenu(boolean includeHeader=true)
{
  if (includeHeader)
  {
  Serial.println("******************************************************************");
  Serial.println("*                  Node blinking/pulsing program                 *");
  }
  Serial.println("******************************************************************");
  Serial.println("* [2 (default) to 9]                   - change target node ID   *");
  Serial.println("* [b=BLINK (default), p=PULSE, o=OFF]  - change target LED mode  *");
  Serial.println("* [s=SLOW, m=MEDIUM (default), f=FAST] - change target LED speed *");
  Serial.println("* [ENTER]                              - send request to target  *");
  Serial.println("******************************************************************\n");
}

void setup()
{
  pinMode(LED, OUTPUT);
  radio.Initialize(NODEID, RF12_433MHZ, NETWORKID);
  radio.Encrypt((uint8_t*)KEY);
  Serial.begin(SERIAL_BAUD);
  mySerial.begin(SERIAL_BAUD);
  //displayMenu();
}

void loop()
{
  //check for serial input (from 2-9, to correspond to node IDs [2,9])
  if (mySerial.available() > 0) {
    input = mySerial.read();
    Serial.write(input);

    if (input == 'W' || input == 'w') 
    {
      Serial.print("Pin Mode : Write ");
      mode = 'w';
    }
    else if (input == 'r'||input == 'R' ){
       Serial.print("Pin Mode : Read");
      mode = 'r';
    }
    
    else if(input == '1' || input == '0')
     pinState = input;
     
    else if (input >= '2' && input <= '9') //[2 to 9]
    {
      nodeId = input-48;
      Serial.print("Switched to node [" );
      Serial.print(nodeId);
      Serial.println("]...\n");
    }
    else if (input == '\n') //ENTER key pressed, send message with existing data (mode, nodeid)
      sendMessage();
    else
    {
      Serial.println("Invalid input, try again: \n"+input);
      //displayMenu(false);
    }
  }
  
  //pass any received RF message to serial
  if (radio.ReceiveComplete())
  {
    if (radio.CRCPass())
    {
      digitalWrite(LED,1);
      Serial.print('[');Serial.print(radio.GetSender(), DEC);Serial.print("] ");
      for (byte i = 0; i < *radio.DataLen; i++)
        Serial.print((char)radio.Data[i]);

      if (radio.ACKRequested())
      {
        radio.SendACK();
        Serial.print(" - ACK sent");
      }
      delay(5);
      digitalWrite(LED,0);
    }
    else Serial.print("BAD-CRC");
    
    Serial.println();
  }
}

void sendMessage()
{
  sprintf(sendBuf, "%c:%c", mode, pinState);
  radio.Send(nodeId, sendBuf, 3, REQUESTACK);
  Serial.print("Request sent ");
  Serial.print(nodeId);
  Serial.println(waitForAck() ? " (ACK OK)" : "(no ACK reply)");
  Blink(LED, 5);
}

// wait up to ACK_TIME for proper ACK, return true if received
static bool waitForAck() {
  long now = millis();
  while (millis() - now <= ACK_TIME)
    if (radio.ACKReceived(nodeId))
      return true;
  return false;
}

void Blink(byte PIN, byte DELAY_MS)
{
  pinMode(PIN, OUTPUT);
  digitalWrite(PIN,HIGH);
  delay(DELAY_MS);
  digitalWrite(PIN,LOW);
}
