#include "draw.h"
#include <cmath>
#include <iostream>
int main()
{
   double px = 0;
   double py = 0;
   double vx = 0.03;
   double vy = 0.02;
   double radius = 0.05;

   draw::setrange(-1, 1);
   while (true) {
      //draw::clear(); // try commenting this out
      if (fabs(px+vx) > 1-radius)
         vx *= -1;
      if (fabs(py+vy) > 1-radius)
         vy *= -1;
      px += vx;
      py += vy;
      draw::settransparency(.45);
      draw::setcolor((int)(127*(1+px)), (int)(127*(1+py)), (int)(63*(2-px-py)));
      draw::filled_circle(px, py, radius);
      draw::setcolor(0, 0, 0);
      draw::circle(px, py, radius);
      draw::show(25); // 25 ms, 40 fps
   }
   
}
