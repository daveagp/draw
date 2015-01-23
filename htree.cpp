#include "draw.h"
#include <cstdlib>

void htree(int n, double size, double x, double y) {
   if (n == 0) return;
   double x0 = x - size/2;
   double x1 = x + size/2;
   double y0 = y - size/2;
   double y1 = y + size/2;

   draw::line(x0, y, x1, y);
   draw::line(x0, y0, x0, y1);
   draw::line(x1, y0, x1, y1);
        
   htree(n-1, size/2, x0, y0);
   htree(n-1, size/2, x0, y1);
   htree(n-1, size/2, x1, y0);
   htree(n-1, size/2, x1, y1);
}

int main(int argc, char* argv[]) {
   int n = 5;
   if (argc > 1) n = atoi(argv[1]);
   htree(n, .5, .5, .5);
   return 0;
}
