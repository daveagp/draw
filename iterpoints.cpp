#include "draw.h"
#include <cmath>
#include <cstdlib>

int main() {
   double cx[3] = {0, 1, 0.5};
   double cy[3] = {0, 0, sqrt(3)/2};
   Color color[3] = {draw::RED, draw::BLUE, draw::GREEN};

   double x = 0.0, y = 0.0; 
   while (true) { 
      int r = random()%3;
      draw::setcolor(color[r]);
      x = (x + cx[r]) / 2.0; 
      y = (y + cy[r]) / 2.0; 
      draw::point(x, y);
   }
}
