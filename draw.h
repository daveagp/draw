#ifndef _H_DRAW
#define _H_DRAW

// user-facing API

namespace draw {
   void ellipse(double, double, double, double);
   void circle(double, double, double);
   void setxscale(double, double);
   void setyscale(double, double);
   void setscale(double, double);
   void show(int);
   void clear();
}

#endif






/* implementation details follow, not really intended for the user.

   the remainder of the file ensures that done() is automatically called
   at the end of the user's main(). 
   
   to disable it, add #define _DRAW_NO_XFORM_MAIN to your own code before 
   including this file. 
*/

#ifndef _DRAW_NO_XFORM_MAIN
#define _DRAW_NO_XFORM_MAIN

// warning: here be dragons
namespace draw {void _done(int);}
int _main(int, char**); // user's main will be transformed to this
int main(int x, char** y) {int r = _main(x, y); draw::_done(r); return r;}

// transform main() or main(int, char**) to _main(int, char**)
#define main(...) vamain(__VA_ARGS__)
#define vamain(...) vamainhelp(,##__VA_ARGS__, int, char**)
#define vamainhelp(blank, first, second, ...) _main(first, second)

#endif 
