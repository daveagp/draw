#include "draw.h"

int main() {
   draw::text("(0, 0)", 0.05, 0.05);
   draw::text("(0, 1)", 0.05, 0.95);
   draw::text("(1, 0)", 0.95, 0.05);
   draw::text("(1, 1)", 0.95, 0.95);
   draw::text("x-axis", 0.5, 0.15);
   draw::text("y-axis", 0.05, 0.5);

   draw::line(0.1, 0.1, 0.1, 0.9);
   draw::line(0.0, 0.8, 0.1, 0.9); 
   draw::line(0.2, 0.8, 0.1, 0.9);

   draw::line(0.1, 0.1, 0.9, 0.1);
   draw::line(0.8, 0.0, 0.9, 0.1);
   draw::line(0.8, 0.2, 0.9, 0.1);
   
   draw::save("media/coords.png");
   return 0;
}
