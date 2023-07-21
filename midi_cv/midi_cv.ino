/*
 *    MIDI2CV
 *    Copyright (C) 2017  Larry McGovern
 *
 *    This program is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License <http://www.gnu.org/licenses/> for more details.
 */

#include <MIDI.h>
#include <SPI.h>

// Note priority is set by pins A0 and A2
// Highest note priority: A0 and A2 high (open)
// Lowest note priority:  A0 low (ground), A2 high (open)
// Last note priority:    A2 low (ground)

#define NP_SEL1 A0 // Note priority is set by pins A0 and A2
#define NP_SEL2 A2

#define GATE1 2
#define GATE2 3
#define CLOCK 4
#define DAC1 8
#define DAC2 9

#define MIDI_CH1 7
#define MIDI_CH2 8

MIDI_CREATE_DEFAULT_INSTANCE();

void setup()
{
  pinMode(NP_SEL1, INPUT_PULLUP);
  pinMode(NP_SEL2, INPUT_PULLUP);

  pinMode(CLOCK, OUTPUT);
  pinMode(GATE1, OUTPUT);
  pinMode(GATE2, OUTPUT);
  pinMode(DAC1, OUTPUT);
  pinMode(DAC2, OUTPUT);

  digitalWrite(CLOCK, LOW);
  digitalWrite(GATE1, LOW);
  digitalWrite(GATE2, LOW);
  digitalWrite(DAC1, HIGH);
  digitalWrite(DAC2, HIGH);

  SPI.begin();
  MIDI.begin(MIDI_CHANNEL_OMNI);
}

bool notes[2][88] = {0};
int8_t noteOrder[2][20] = {0};
int8_t orderIndex[2] = {0};

unsigned long clock_timer = 0, clock_timeout = 0;
unsigned int clock_count = 0;

void loop()
{
  int type, noteMsg, channel, channelNum, d1, d2;

  if ((clock_timer > 0) && (millis() - clock_timer > 20))
  {
    digitalWrite(CLOCK, LOW); // Set clock pulse low after 20 msec
    clock_timer = 0;
  }

  if (MIDI.read())
  {
    byte type = MIDI.getType();
    
    switch (type)
    {
    case midi::NoteOn:
    case midi::NoteOff:
      channel = MIDI.getChannel();
      if (MIDI_CH1 != channel && MIDI_CH2 != channel)
      {
        break; // Only 2 channels support
      }

      channelNum = MIDI_CH1 == channel ? 0 : 1;

      noteMsg = MIDI.getData1() - 21; // A0 = 21, Top Note = 108
      if ((noteMsg < 0) || (noteMsg > 87)) 
      {
        break; // Only 88 notes of keyboard are supported
      }

      notes[channelNum][noteMsg] = commandVelocity(type, channelNum);

      handleNote(noteMsg, channelNum);
      
      break;

    case midi::Clock:
      handleClock();
      
      break;


    default:
      break;
    }
  }
}

void handleClock()
{
    if (millis() > clock_timeout + 300)
    {
      clock_count = 0; // Prevents Clock from starting in between quarter notes after clock is restarted!
    }

    clock_timeout = millis();

    if (clock_count == 0) {
      digitalWrite(CLOCK,HIGH); // Start clock pulse
      clock_timer = millis();
    }

    clock_count++;
    if (clock_count == 24) {  // MIDI timing clock sends 24 pulses per quarter note. Sent pulse only once every 24 pulses
      clock_count = 0;
    }
}

bool commandVelocity(int type, int channelNum)
{
  int velocity = type == midi::NoteOn ? MIDI.getData2() : 0;
  
  if (velocity == 0)
  {
    return false;
  }

  // velocity range from 0 to 4095 mV  Left shift d2 by 5 to scale from 0 to 4095,
  // and choose gain = 2X
  setVoltage(channelNum == 0 ? DAC1 : DAC2, 1, 1, velocity << 5); // DAC1 or DAC2, channel 1, gain = 2X

  return true;
}

void handleNote(int noteMsg, int channelNum)
{
  bool S1, S2;
  
  // Pins NP_SEL1 and NP_SEL2 indictate note priority
  S1 = digitalRead(NP_SEL1);
  S2 = digitalRead(NP_SEL2);

  if (S1 && S2)
  { // Highest note priority
    commandTopNote(channelNum);
  }
  else if (!S1 && S2)
  { // Lowest note priority
    commandBottomNote(channelNum);
  }
  else
  { // Last note priority
    if (notes[channelNum][noteMsg])
    { // If note is on and using last note priority, add to ordered list
      orderIndex[channelNum] = (orderIndex[channelNum] + 1) % 20;
      noteOrder[channelNum][orderIndex[channelNum]] = noteMsg;
    }
    
    commandLastNote(channelNum);
  }
}

void commandTopNote(int channelNum)
{
  int topNote = 0;
  bool noteActive = false;

  for (int i = 0; i < 88; i++)
  {
    if (notes[channelNum][i])
    {
      topNote = i;
      noteActive = true;
    }
  }

  if (noteActive)
  {
    commandNote(topNote, channelNum);
  }
  else // All notes are off, turn off gate
  {
    digitalWrite(channelNum == 0 ? GATE1 : GATE2, LOW);
  }
}

void commandBottomNote(int channelNum)
{
  int bottomNote = 0;
  bool noteActive = false;

  for (int i = 87; i >= 0; i--)
  {
    if (notes[channelNum][i])
    {
      bottomNote = i;
      noteActive = true;
    }
  }

  if (noteActive)
  {
    commandNote(bottomNote, channelNum);
  }
  else // All notes are off, turn off gate
  {
    digitalWrite(channelNum == 0 ? GATE1 : GATE2, LOW);
  }
}

void commandLastNote(int channelNum)
{

  int8_t noteIndx;

  for (int i = 0; i < 20; i++)
  {
    noteIndx = noteOrder[channelNum][mod(orderIndex[channelNum] - i, 20)];
    if (notes[channelNum][noteIndx])
    {
      commandNote(noteIndx, channelNum);
      return;
    }
  }
  
  digitalWrite(channelNum == 0 ? GATE1 : GATE2, LOW); // All notes are off
}

// Rescale 88 notes to 4096 mV:
//    noteMsg = 0 -> 0 mV
//    noteMsg = 87 -> 4096 mV
// DAC output will be (4095/87) = 47.069 mV per note, and 564.9655 mV per octive
// Note that DAC output will need to be amplified by 1.77X for the standard 1V/octave

#define NOTE_SF 47.069f // This value can be tuned if CV output isn't exactly 1V/octave

void commandNote(int noteMsg, int channelNum)
{
  unsigned int mV = (unsigned int)((float)noteMsg * NOTE_SF + 0.5);
  setVoltage(channelNum == 0 ? DAC1 : DAC2, 0, 1, mV); // DAC1 or DAC2, channel 0, gain = 2X

  digitalWrite(channelNum == 0 ? GATE1 : GATE2, HIGH);
}

// setVoltage -- Set DAC voltage output
// dacpin: chip select pin for DAC.  Note and velocity on DAC1, pitch bend and CC on DAC2
// channel: 0 (A) or 1 (B).  Note and pitch bend on 0, velocity and CC on 2.
// gain: 0 = 1X, 1 = 2X.
// mV: integer 0 to 4095.  If gain is 1X, mV is in units of half mV (i.e., 0 to 2048 mV).
// If gain is 2X, mV is in units of mV

void setVoltage(int dacpin, bool channel, bool gain, unsigned int mV)
{
  unsigned int command = channel ? 0x9000 : 0x1000;

  command |= gain ? 0x0000 : 0x2000;
  command |= (mV & 0x0FFF);

  SPI.beginTransaction(SPISettings(8000000, MSBFIRST, SPI_MODE0));
  digitalWrite(dacpin, LOW);
  SPI.transfer(command >> 8);
  SPI.transfer(command & 0xFF);
  digitalWrite(dacpin, HIGH);
  SPI.endTransaction();
}

int mod(int a, int b)
{
  int r = a % b;
  return r < 0 ? r + b : r;
}
