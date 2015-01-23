#include "draw.h"                      // you have to include this

int main() {
   draw::setcolor(255, 0, 0);          // fully red, no green or blue
   draw::filled_circle(0.5, 0.5, 0.5); // center 0.5, 0.5 and radius 0.5
   draw::setcolor(255, 255, 255);      // white
   draw::filled_circle(0.5, 0.5, 0.3); // smaller circle
   draw::setcolor(draw::RED);          // predefined alias for red
   draw::filled_circle(0.5, 0.5, 0.1); // smaller circle
   return 0;
}
