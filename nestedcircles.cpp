#include "draw.h"
#include <cstdlib>

// draw a circle at ceter (x, y), radius r, with some embellishments
void fancyCircle(double x, double y, double r) {
   // use one of two color pairs at random
   bool randomBit = (random() % 2) == 0; 
   // this is the color that will fill the circle
   if (randomBit == 0)
      draw::setcolor(draw::RED);
   else 
      draw::setcolor(draw::YELLOW);
   draw::filled_circle(x, y, r);
   // this is the color for the circle's border
   if (randomBit == 0)
      draw::setcolor(draw::VIOLET);
   else
      draw::setcolor(draw::TEAL);
   draw::circle(x, y, r);
}

// draw an order-n nested circle, centred on (x, y) with radius r
void ncirc(int n, double x, double y, double r) {
   if (n==0) return;
   fancyCircle(x, y, r);

   double halfRadius = r/2;
   // recursively draw two nested circles of order n-1
   ncirc(n-1, x - halfRadius, y, halfRadius);
   ncirc(n-1, x + halfRadius, y, halfRadius);
}

// read in a command-line argument N and plot an order-N circle
int main(int argc, char* argv[]) {
   int n = 5;
   if (argc > 1) n = atoi(argv[1]);
   ncirc(n, .5, .5, .5);
}

