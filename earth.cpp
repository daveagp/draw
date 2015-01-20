#include "draw.h"
#include <cmath>
#include <iostream>

using namespace std;

int main()
{
   double px = 0;
   double py = 0;
   double vx = 0.03;
   double vy = 0.02;
   double radius = 0.05; // depends on image size and scale

   draw::play("media/2001.mid");
   draw::setrange(-1, 1);
   while (true) {
      draw::image("media/starfield.jpg", 0, 0);
      if (fabs(px+vx) > 1-radius) {
         vx *= -1;
         draw::play("media/pop.wav");
      }
      if (fabs(py+vy) > 1-radius) {
         vy *= -1;
         draw::play("media/laser.wav");
      }
      px += vx;
      py += vy;
      draw::image("media/earth.gif", px, py);
      draw::show(25); // 25 ms, 40 fps
   }
   
}
