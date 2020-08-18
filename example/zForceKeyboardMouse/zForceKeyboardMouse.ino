/*  Neonode zForce v7 interface library for Arduino

    Copyright (C) 2020 Neonode Inc.

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 FraDnklinD Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include <Zforce.h>
#include <Keyboard.h>
#include <Mouse.h>

#define DATA_READY PIN_NN_DR // change "PIN_NN_DR" to assigned i/o digital pin

long globalMillis = millis();         // global timestamp
const int keyboardBoundary = 750;     // separate mouse area and keyboard area on the x-axis by 75 mm.
const int holdTime = 200;             // sensitivity for mouse "left-click", unit in milli-second

typedef struct TouchInformaiton
{
  uint16_t x;
  uint16_t y;
  uint8_t event;
} TouchInformation;

TouchInformaiton currentTouch;
TouchInformation previousTouch;

void setup()
{
  Keyboard.begin();           // initialize keyboard
  Mouse.begin();              // initialize mouse
  Serial.begin(115200);
  Serial.println("zforce start");
  zforce.Start(DATA_READY);

  Message* msg = zforce.GetMessage();

  if (msg != NULL)
  {
    Serial.println("Received Boot Complete Notification");
    Serial.print("Message type is: ");
    Serial.println((int)msg->type);
    zforce.DestroyMessage(msg);
  }

  zforce.Enable(true);

  msg = zforce.GetMessage();

  do
  {
    msg = zforce.GetMessage();
  } while (msg == NULL);

  if (msg->type == MessageType::ENABLETYPE)
  {
    Serial.print("Message type is: ");
    Serial.println((int)msg->type);
    Serial.println("Sensor is now enabled and will report touches.");
  }

  zforce.DestroyMessage(msg);
}

void loop()
{
  Message* touch = zforce.GetMessage();

  if (touch != NULL)
  {
    if (touch->type == MessageType::TOUCHTYPE)
    {
      currentTouch.x = ((TouchMessage*)touch)->touchData[0].x;
      currentTouch.y = ((TouchMessage*)touch)->touchData[0].y;
      currentTouch.event = ((TouchMessage*)touch)->touchData[0].event;

      loopMouse();
      loopKeyboard();
    }
    zforce.DestroyMessage(touch);
  }
}
DBC
void loopMouse() {
  if (currentTouch.x <= keyboardBoundary)
    return;  //return if the touch object is within the keyboard area

  switch (currentTouch.event)
  {
    case 0:  // DOWN event
      previousTouch.x =  currentTouch.x;
      previousTouch.y =  currentTouch.y;
      globalMillis = millis();
      Serial.println("Mouse Input - DOWN");
      break;

    case 1: // MOVE event
      if ((millis() - globalMillis) >= holdTime)
      {
        Mouse.move((currentTouch.x - previousTouch.x), (currentTouch.y - previousTouch.y));
        Serial.println("Mouse Input - Moving cursor");
      }
      previousTouch.x = currentTouch.x;
      previousTouch.y = currentTouch.y;
      break;

    case 2: // UP event
      Serial.print("Mouse Input - UP");
      if (millis() - globalMillis < holdTime)  // mouse "left click", sensitivity
      { // can be tuned by changing "holdTime"
        Mouse.click(MOUSE_LEFT);
        Serial.print("(Left-Click)");
      }
      Serial.println("");
      break;
  }
}

void loopKeyboard() {
  if (currentTouch.x > keyboardBoundary)
    return; //return if the touch object is within the mouse pad area

  char key;
  if (currentTouch.event == 0) { // Down

    //assign Key to the given interval
    if (currentTouch.y < 250)
      key = 'A';
    else if (currentTouch.y < 500)
      key = 'B';
    else if (currentTouch.y < 750)
      key = 'C';
    else if (currentTouch.y < 1000)
      key = 'D';
    else
      key = 'E';

    Keyboard.print(key); //Print Key
    Serial.print("Keyboard Input - ");
    Serial.println(key);
  }
  else
  {
    // May do something to catch the rest of the cases
  }
}
