#include "draw.h"
#include <cstdlib>

int main()
{
   for (double r=0; r<=1; r+=0.006) {
      clear(); // try commenting this out
      circle(0, 0, r);
      circle(0, 0, 1-r);
      show(25); // 25 ms, 40 fps
   }
   
   done(); // wait for user to close the window
}
