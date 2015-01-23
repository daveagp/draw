#ifndef _H_DRAW
#define _H_DRAW

// user-facing API

namespace draw {
// drawing shapes. cx, cy means center
   void point(double x, double y);
   void line(double x0, double y0, double x1, double y1);
   void square(double cx, double cy, double side_length);
   void rectangle(double x0, double y0, double x1, double y1);
   void polygon(int num_points, double x[], double y[]);
   void circle(double cx, double cy, double r);
   void ellipse(double cx, double cy, double rx, double ry);
// drawing filled shapes
   void filled_square(double cx, double cy, double side_length);
   void filled_rectangle(double x0, double y0, double x1, double y1);
   void filled_polygon(int num_points, double x[], double y[]);
   void filled_circle(double cx, double cy, double r);
   void filled_ellipse(double cx, double cy, double rx, double ry);
// draw image or text centered at a given position
   void image(char filename[], double, double);
   void text(char text[], double, double);
// set coordinates for boundaries of screen. default is from 0 to 1
   void setxrange(double xmin, double ymax);
   void setyrange(double ymin, double ymax);
   void setrange(double min, double max); // sets both ranges
// set color. default is black
   void setcolor(int r, int g, int b);
   void setcolor(int color[3]); // to work with predefined colors
// other settings
   void setpenwidth(double w); // default: 1
   void settransparency(double t); // 1 transparent, 0 opaque. default: 0
   void setfontsize(int s); // default: 12 pt
   void setwindowsize(int width, int height); // default: 512x512 pixels
// show current frame, pause this many milliseconds, 
// & turn on animation mode: nothing will display until next call to show
   void show(int milliseconds);
// misc
   void clear(); // fill with white
   bool save(char filename[]); // save image to file. true = ok, false = error
   void play(char filename[]); // play a sound file

   // some pre-defined colors
   const int RED[3] = {255, 0, 0};
   const int BLUE[3] = {0, 0, 255};
   const int LIME[3] = {0, 255, 0};
   const int GREEN[3] = {0, 127, 0};
   const int YELLOW[3] = {255, 255, 0};
   const int CYAN[3] = {0, 255, 255};
   const int MAGENTA[3] = {255, 0, 255};
   const int PINK[3] = {255, 127, 255};
   const int WHITE[3] = {255, 255, 255};
   const int BLACK[3] = {0, 0, 0};
   const int GRAY[3] = {127, 127, 127};
   const int ORANGE[3] = {255, 127, 0};
   const int PURPLE[3] = {127, 0, 127};
   const int TEAL[3] = {0, 127, 127};
   const int OLIVE[3] = {127, 127, 0};
   const int MAROON[3] = {127, 0, 0};
   const int NAVY[3] = {0, 0, 127};
   const int MINT[3] = {127, 255, 127};
   const int CORAL[3] = {255, 127, 127};
   const int ROSE[3] = {255, 0, 127};
   const int CHARTREUSE[3] = {127, 255, 0};
   const int VIOLET[3] = {127, 0, 255};
   const int AZURE[3] = {0, 127, 255};

   // const-friendly versions
   void setcolor(const int color[3]); 
   void image(const char filename[], double, double);
   void text(const char text[], double, double);
   bool save(const char filename[]);
   void play(const char filename[]);

}

#endif






/* implementation details follow, not really intended for the user.

   the remainder of the file ensures that the window stays open after
   the end of the user's main(). 
   
   to disable it, add #define _DRAW_NO_XFORM_MAIN to your own code before 
   including this file. Call draw::_done() at end to wait for window close.
*/

#ifndef _DRAW_NO_XFORM_MAIN
#define _DRAW_NO_XFORM_MAIN

// warning: here be dragons
namespace draw {int actual_main(int, char**);}
int main(int x, char** y) {return draw::actual_main(x, y);}

// transform main() or main(int, char**) to _main(int, char**)
#define main(...) vamain(__VA_ARGS__)
#define vamain(...) vamainhelp(,##__VA_ARGS__, int, char**)
#define vamainhelp(blank, first, second, ...) _main(first, second)

#endif 
