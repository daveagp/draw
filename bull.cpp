#include "draw.h"

int main() {
   draw::setcolor(255, 0, 0);
   draw::filled_circle(0.5, 0.5, 0.5);
   draw::setcolor(255, 255, 255);
   draw::filled_circle(0.5, 0.5, 0.3);
   draw::setcolor(draw::RED);
   draw::filled_circle(0.5, 0.5, 0.1);   
   draw::save("bull.png");
}
