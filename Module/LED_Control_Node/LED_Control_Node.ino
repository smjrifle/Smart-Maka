/*
 * Copyright (c) 2013 by Felix Rusu <felix@lowpowerlab.com>
 *
 * This file is free software; you can redistribute it and/or modify
 * it under the terms of either the GNU General Public License version 2
 * or the GNU Lesser General Public License version 2.1, both as
 * published by the Free Software Foundation.
 */
 
// This is a NODE sketch (LEDControl_Node) for a Moteino that can act as
// a endpoint which blinks/pulses/stops the onboard LEDs per requests from Moteino(s)
// loaded with GATEWAY sketches (LEDControl_Gateway)
// Designed by Felix Rusu (felix@lowpowerlab.com), www.LowPowerLab.com

// The LEDs on the endpoint Moteinos can be controlled as follows. In the GATEWAY's serial terminal:
// Type a node ID in the rage [2-9] to set the target node
// Type a LED mode [b,p,o] - blink, pulse, off
// Type a speed for the LED action [s,m,f] - slow, medium, fast
// Hit [ENTER] to send request to target
// The target Moteino receives the message and applies the requested LED mode and speed

#include <RFM12B.h>
#include <avr\sleep.h>
#include <avr\delay.h>
#include <LowPower.h>

#define NETWORKID         212  //what network this node is on
#define NODEID              2  //this node's ID, should be unique among nodes on this NETWORKID
#define GATEWAYID           1  //central node to report data to
#define KEY "ABCDABCDABCDABCD" //(16 bytes of your choice - keep the same on all encrypted nodes)
#define LED                 3  //pin connected to onboard LED (digital 9, pulse width modulation capable for pulsing)
#define SERIAL_BAUD    115200

RFM12B radio;
char mode = 'w';      // mode : 'b' = blinking, 'p' = pulsing, 'o' = 'off'
char pinNumber = LED;
char pinValue = 0;  // speed : 's' = slow, 'm' = medium, 'f' = 'fast'
boolean LEDState;     // current LED state when blinking
long now=0, LED_lastStateChange=0;


void setup(void)
{
  radio.Initialize(NODEID, RF12_433MHZ, NETWORKID);
  radio.Encrypt((uint8_t*)KEY);
  Serial.begin(SERIAL_BAUD);
  pinMode(LED, OUTPUT);
  Serial.println("Listening for LED blink/pulse/stop requests...\n");
  Serial.println();
}

void loop()
{
  //handle blinking/fading/stopping

    digitalWrite(pinNumber,pinValue);
  
  //pass any received RF message to serial
  if (radio.ReceiveComplete())
  {
    if (radio.CRCPass())
    {
      Serial.print('[');Serial.print(radio.GetSender(), DEC);Serial.print("] ");
      for (byte i = 0; i < *radio.DataLen; i++)
        Serial.print((char)radio.Data[i]);

      Serial.print(" - ");
      //check for a LED blink/pulse/stop request
      if (*radio.DataLen == 3)
      pinValue =radio.Data[2];
      
      

      if (radio.ACKRequested())
      {
        radio.SendACK();
        Serial.print(" (ACK sent)");
      }
    }
    else Serial.print("BAD-CRC");
    
    Serial.println();
  }
}

 
