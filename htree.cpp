/*
recursive program to draw an H-tree.
compile: make htree
run: htree 10 (or any other number)
*/

#include "draw.h"
#include <cstdlib>

// recursive function to draw a level-n H-tree centered at x, y
void htree(int n, double size, double x, double y) {
   // base case
   if (n == 0)
      return;

   // recursive case:

   // compute coordinates of corners of this H
   double x0 = x - size/2;
   double x1 = x + size/2;
   double y0 = y - size/2;
   double y1 = y + size/2;

   // draw the H
   draw::line(x0, y, x1, y);
   draw::line(x0, y0, x0, y1);
   draw::line(x1, y0, x1, y1);

   // draw 4 smaller H-trees
   htree(n-1, size/2, x0, y0);
   htree(n-1, size/2, x0, y1);
   htree(n-1, size/2, x1, y0);
   htree(n-1, size/2, x1, y1);
}

int main(int argc, char* argv[]) {
   int n = 5;
   if (argc > 1)
      n = atoi(argv[1]);
   
   // start the recursion
   htree(n, .5, .5, .5);
   
   return 0;
}
