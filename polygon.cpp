#include "draw.h"

int main()
{
   draw::setwindowsize(1000, 100);
   draw::setxrange(-1, 1);
   double x[] = {0.3, 0.4, 0.5};
   double y[] = {0.6, 0.9, 0.2};
   draw::polygon(3, x, y);
   draw::filled_square(0.5, 0.5, 0.2);
   draw::rectangle(0.1, 0.2, 0, 0.9);
   draw::setpenwidth(5);
   draw::point(0.4, 0.1);
   draw::line(0.2, 0.1, 0.1, 0.2);
   draw::setcolor(255, 0, 0);
   draw::text("hello!\nthis is a message.", 0.5, 0.5);
   draw::setfontsize(30);
   draw::setcolor(draw::BLUE);
   draw::text("wow", 0.5, 0.8);
   draw::save("polygon.png");
   return 0;
}
