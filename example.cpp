#include "draw.h"
#include <cstdlib>

int main()
{
   draw::setrange(-1, 1);
   while (true) for (double r=0; r<=1; r+=0.006) {
      draw::clear(); // try commenting this out
      draw::circle(0, 0, r);
      draw::circle(0, 0, 1-r);
      draw::show(25); // 25 ms, 40 fps
   }
   
}
