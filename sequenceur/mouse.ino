#include <Mouse.h>
#include <ps2.h>
#define PS2_DATA 10
#define PS2_CLK 9
byte mstat1;

byte mstat2;
byte mxy;
byte mx;
byte my;
byte mz;
uint16_t msval[3];
uint16_t repeatCnt;//PS2 moose(PS2_CLK, PS2_DATA);
PS2 mouse(PS2_CLK, PS2_DATA); //Pin 5 is the mouse data & pin 6 is the clock.void mouse_init(PS2 *mouse)

void mouse_init(PS2 *mouse){
  mouse->write(0xff);  // reset
  mouse->read();  // ack byte
  mouse->read();  // blank
  mouse->read();  // blank
  mouse->write(0xf0);  // remote mode
  mouse->read();  // ack
  delayMicroseconds(100);
}

struct xy { int x; int y; };

struct xy mouse_read(PS2 *mouse){
  static char mstat;
  static char mx;
  static char my;
  static int currentMillis = 0;
  static int prevMillis = 0;

  currentMillis = millis();
  if(currentMillis - prevMillis > 10){

   prevMillis = currentMillis;

   /* get a reading from the mouse */
   mouse->write(0xeb);  // give me data!
   mouse->read();      // ignore ack
   mstat = mouse->read();
  
   mx = mouse->read();
   my = mouse->read();
   /*Serial.print(mstat, BIN);
   Serial.print("\tX=");
   Serial.print(mx, DEC);
   Serial.print("\tY=");
   Serial.print(my, DEC);
   Serial.println();*/

  }
  xy coord = {.x = int(mx), .y = int(my)};
  return coord;
}

