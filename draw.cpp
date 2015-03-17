#include "draw.h"
#include <QPainter>
#include <QApplication>
#include <QPixmap>
#include <QWidget>
#include <QThread>
#include <QMetaMethod>
#include <QMetaObject>
#include <time.h>
#include <iostream>
#ifdef DRAW_UNMUTE
#include <phonon/phonon>
#endif
 
namespace draw {

using std::cout; using std::endl; using std::cerr;

QAtomicInt pending(0); // how many calls are queued?
QAtomicInt save_result(-1); // synchronous call. 1 ok, 0 error, -1 pending

/************************************* Part 1 ****************************
             Qt Widget that receives calls and does drawing              */

class DrawWidget : public QWidget {
Q_OBJECT  

private:      
   QPixmap* pm, *prepared_frame;
   double xmin, xmax, ymin, ymax;
   int width, height;
   int r, g, b, a;
   double penwidth;
   int fontsize;
   bool y_increases_up;
   bool animation_mode;
   
protected:
void paintEvent(QPaintEvent *) {
   QPainter(this).drawPixmap(0, 0, animation_mode ? (*prepared_frame) : (*pm));
}
 
public:
DrawWidget(QWidget *parent = 0) : QWidget(parent) { 
   this->width = this->height = 512;
   r = g = b = 0;
   a = 255;
   penwidth = 1;
   fontsize = 12;
   this->setFixedSize(width, height);
   this->move(100, 100);     // not crammed in corner 
   this->setWindowTitle("draw");
   this->QWidget::show();
   pending = 0;
   y_increases_up = true;
   animation_mode = false;
   
   pm = new QPixmap(width, height);
   (*pm).fill(); // clear   

   this->setxrange(0, 1);
   this->setyrange(0, 1);
}

double affx(double x0) {
   return (x0-xmin)/(xmax-xmin)*width;
}

double affy(double y0) {
   return (y0-ymin)/(ymax-ymin)*height;
}

double linx(double dx) {
   return dx*width/(xmax-xmin);
}

double liny(double dy) {
   return dy*height/(ymax-ymin);
}

QPainter& filler(QPainter& result) {
   result.setRenderHint(QPainter::Antialiasing, true);
   result.setPen(Qt::NoPen);
   result.setBrush(QBrush(QColor(r, g, b, a), Qt::SolidPattern));
   return result;
}

QPainter& liner(QPainter& result) {
   result.setRenderHint(QPainter::Antialiasing, true);
   result.setPen(QPen(QBrush(QColor(r, g, b, a)), penwidth, 
      Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
   return result;
}

QPainter& styler(bool filled, QPainter& result) {
   return filled ? filler(result) : liner(result);
}

void done_visible_call() {
   pending = pending - 1;
   if (!animation_mode) this->repaint();
}

void done_invisible_call() {
   pending = pending - 1;
}

Q_INVOKABLE void point(double x, double y) {
   QPainter painter(pm);
   liner(painter).drawPoint(QPointF(affx(x), affy(y)));
   done_visible_call();
}

Q_INVOKABLE void line(double x0, double y0, double x1, double y1) {
   QPainter painter(pm);
   liner(painter).drawLine(QPointF(affx(x0), affy(y0)), QPointF(affx(x1), affy(y1)));
   done_visible_call();
}

void either_ellipse(bool filled, double x, double y, double xr, double yr) {
   QPainter painter(pm);
   styler(filled, painter).drawEllipse(QPointF(affx(x), affy(y)), linx(xr), liny(yr));
   done_visible_call();
}

Q_INVOKABLE void ellipse(double x, double y, double xr, double yr) {
   either_ellipse(false, x, y, xr, yr);
}

Q_INVOKABLE void filled_ellipse(double x, double y, double xr, double yr) {
   either_ellipse(true, x, y, xr, yr);
}

void either_polygon(bool filled, QList<double> x, QList<double> y) {
   QPolygonF p;
   for (int i=0; i<x.size(); i++) p << QPointF(affx(x[i]), affy(y[i]));
   QPainter painter(pm);
   styler(filled, painter).drawPolygon(p);
   done_visible_call();
}

Q_INVOKABLE void polygon(QList<double> x, QList<double> y) {
   either_polygon(false, x, y);
}

Q_INVOKABLE void filled_polygon(QList<double> x, QList<double> y) {
   either_polygon(true, x, y);
}

Q_INVOKABLE void setfontsize(int w) {
   this->fontsize = w;
   done_invisible_call();
}

Q_INVOKABLE void setpenwidth(double w) {
   this->penwidth = w;
   done_invisible_call();
}

Q_INVOKABLE void setcolor(int rnew, int gnew, int bnew) {
   this->r = rnew;
   this->g = gnew;
   this->b = bnew;
   done_invisible_call();
}

Q_INVOKABLE void settransparency(double t) {
   this->a = (int)(255*(1-t));
   done_invisible_call();
}

Q_INVOKABLE void setxrange(double min, double max) {
   xmin = min;
   xmax = max;
   done_invisible_call();
}

Q_INVOKABLE void setyrange(double min, double max) {
   if (y_increases_up) {ymin = max; ymax = min;}
   else {ymax = min; ymin = max;}
   done_invisible_call();
}

Q_INVOKABLE void setwindowsize(int newwidth, int newheight) {
   this->width = newwidth;
   this->height = newheight;
   this->setFixedSize(width, height);
   delete pm;
   pm = new QPixmap(width, height);
   (*pm).fill(); // clear
   if (animation_mode) {
      delete prepared_frame;
      prepared_frame = new QPixmap(width, height);
      (*prepared_frame).fill();
   }
   done_visible_call();
}

Q_INVOKABLE void text(QString text, double x, double y) {
   QPainter painter(pm);
   QFont f = painter.font();
   f.setPointSize((int)fontsize);
   painter.setFont(f);
   QRectF rect(affx(x)-width/2, affy(y)-height/2, width, height);
   liner(painter).drawText(rect, Qt::AlignCenter, text);
   done_visible_call();
}

Q_INVOKABLE void image(QString filename, double x, double y) {
   QPainter painter(pm);
   QImage img(filename);
   painter.drawImage(QPointF(affx(x)-img.width()/2, affy(y)-img.height()/2), img);
   done_visible_call();
}

Q_INVOKABLE void save(QString filename) {
   save_result = pm->save(filename) ? 1 : 0;
   done_invisible_call();
}

Q_INVOKABLE void play(QString filename) {
#ifdef DRAW_UNMUTE
   freopen("/dev/null", "w", stderr); // hide phonon's many status messages

   using namespace Phonon;
   MediaObject *mediaObject = createPlayer(NoCategory, 
                                           MediaSource(filename));
   mediaObject->play();

/*   cout << QSound::isAvailable() << endl;
   QSound sound(filename);
   sound.play();*/
#endif
   done_invisible_call();
}

Q_INVOKABLE void clear() {
   this->pm->fill();
   done_visible_call();
}

Q_INVOKABLE void showframe() {  
   if (!animation_mode) {
      prepared_frame = new QPixmap(*pm);
      animation_mode = true;
   }
   else {
      QPainter p(prepared_frame);
      p.drawPixmap(0, 0, *pm);
      this->repaint();
   }
   done_visible_call();
}
}; // end of DrawWidget

/************************************* part 2 ****************************
                 linkage between Qt Widget and student code              */

QAtomicPointer<DrawWidget> drawwidget; // to invoke
QAtomicInt retcode(0); // return code from main

int drawmain(int argc, char** argv);

// escape namespace
} 

// forward declare ::_main(int, char**)
// (draw.h will transform student main to that signature instead)
int _main(int, char**);  

// this is the real entry point. don't let mangling in draw.h get to it
#undef main
int main(int argc, char** argv) {
   return draw::drawmain(argc, argv);
}

// re-enter namespace
namespace draw {

class StudentThread : public QThread {
   Q_OBJECT
   int argc;
   char** argv;
   public: StudentThread(int argc0, char** argv0) 
   { this->argc = argc0; this->argv = argv0; }
   protected: void run() 
   { retcode = ::_main(argc, argv); }
};

int drawmain(int argc, char** argv) {
   QApplication app(argc, argv);
   qRegisterMetaType<QList<double> >("QList<double>");
   app.setApplicationName("draw");
   drawwidget = new DrawWidget;
   StudentThread* st = new StudentThread(argc, argv);
   st->start(); // start student main() in its own thread
   app.exec(); // wait until student closes window
   return retcode; // pass result of student's main, if it finished
}

/************************************* part 3 ****************************
      declaration of student API, code that happens in their thread      */

const int MAX_QUEUE_SIZE = 100;

void start_call() {
   // a giant queue may crash your VM. prevent that!
   do QThread::yieldCurrentThread(); while (pending > MAX_QUEUE_SIZE);
   pending = pending + 1;
}

// CALL(func, type1, arg1, func2, arg2): pseudo-signal to DrawWidget
#define CALL(meth,...) \
 start_call(); \
 QByteArray normalizedSignature = \
 QMetaObject::normalizedSignature( #meth "(" ARGS(S, ##__VA_ARGS__) ")" );\
 const QMetaObject* rmo = (*drawwidget).metaObject(); \
 int methodIndex = rmo->indexOfMethod(normalizedSignature); \
 QMetaMethod method = rmo->method(methodIndex); \
 method.invoke(drawwidget, Qt::QueuedConnection ARGS(Q, ##__VA_ARGS__));

// e.g. ARGS(S, int, bar, double, x) => "int, double"
// e.g. ARGS(Q, int, bar, double, x) => , Q_ARG(int, bar), Q_ARG(double, x)
#define ARGS(...) ARGSN(__VA_ARGS__,4,3,3,2,2,1,1,0,0)
#define ARGSN(FMT,a,b,c,d,e,f,g,h,i,...) ARGS##i(FMT,a,b,c,d,e,f,g,h)
#define ARGS0(FMT,...) 
#define ARGS1(FMT,a,b,...) ARGHELP##FMT(a,b) 
#define ARGS2(FMT,a,b,...) ARGHELP##FMT(a,b) SEP##FMT ARGS1(FMT,__VA_ARGS__)
#define ARGS3(FMT,a,b,...) ARGHELP##FMT(a,b) SEP##FMT ARGS2(FMT,__VA_ARGS__)
#define ARGS4(FMT,a,b,...) ARGHELP##FMT(a,b) SEP##FMT ARGS3(FMT,__VA_ARGS__)
#define ARGHELPS(a,b) #a
#define ARGHELPQ(a,b) , Q_ARG(a, b)
#define SEPS ","
#define SEPQ
// end definition of CALL

// syntactic sugar for calls that are as simple as possible
#define SIMPLE(meth,...) \
void meth(PRO(SPACE, ##__VA_ARGS__)) {CALL(meth, PRO(COMMA, ##__VA_ARGS__));}
// e.g. PRO(COMMA, int, double) => int,arg2, double,arg1
#define PRO(...) PRON(__VA_ARGS__,4,3,2,1,0)
#define PRON(DELIM, a,b,c,d,e,...) PRO##e(DELIM, a,b,c,d,e)
#define PROCOMMA(a,b) a, b
#define PROSPACE(a,b) a b
#define PRO0(DELIM, ...) 
#define PRO1(DELIM, a,...) PRO##DELIM(a,arg1)
#define PRO2(DELIM, a,...) PRO##DELIM(a,arg2), PRO1(DELIM, __VA_ARGS__)
#define PRO3(DELIM, a,...) PRO##DELIM(a,arg3), PRO2(DELIM, __VA_ARGS__)
#define PRO4(DELIM, a,...) PRO##DELIM(a,arg4), PRO3(DELIM, __VA_ARGS__)

// API functions that make a signal call and are as simple as possible
SIMPLE(point, double, double);
SIMPLE(line, double, double, double, double);
SIMPLE(ellipse, double, double, double, double);
SIMPLE(filled_ellipse, double, double, double, double);
SIMPLE(setpenwidth, double);
SIMPLE(setfontsize, int);
SIMPLE(setcolor, int, int, int);
SIMPLE(settransparency, double);
SIMPLE(setxrange, double, double);
SIMPLE(setyrange, double, double);
SIMPLE(setwindowsize, int, int);
SIMPLE(showframe);
SIMPLE(clear);

// non-trivial API functions (adapters, converters, timing logic)

void circle(double x, double y, double r) {
   ellipse(x, y, r, r); 
}

void filled_circle(double x, double y, double r) {
   filled_ellipse(x, y, r, r);
}

void polygon(int n, const double x[], const double y[]) {
   QList<double> lx, ly; 
   for (int i=0; i<n; i++) {
      lx << x[i]; 
      ly << y[i];
   }
   CALL(polygon, QList<double>, lx, QList<double>, ly); 
}  

void filled_polygon(int n, const double x[], const double y[]) {
   QList<double> lx, ly;
   for (int i=0; i<n; i++) {
      lx << x[i]; 
      ly << y[i];
   }
   CALL(filled_polygon, QList<double>, lx, QList<double>, ly); 
}

void square(double x, double y, double s) {
   rectangle(x-s/2, y-s/2, x+s/2, y+s/2);
}

void rectangle(double x0, double y0, double x1, double y1) {
   double xc[4] = {x0, x0, x1, x1}; 
   double yc[4] = {y0, y1, y1, y0};
   polygon(4, xc, yc);
}

void filled_square(double x, double y, double s) {
   filled_rectangle(x-s/2, y-s/2, x+s/2, y+s/2);
}

void filled_rectangle(double x0, double y0, double x1, double y1) {
   double xc[4] = {x0, x0, x1, x1}; double yc[4] = {y0, y1, y1, y0};
   filled_polygon(4, xc, yc); 
}

void setcolor(Color color) {
   setcolor(color.red, color.green, color.blue);
}

void setrange(double min, double max) {
   setxrange(min, max); setyrange(min, max);
}

void image(const char* filename, double x, double y) {
   CALL(image,QString,filename,double,x,double,y); 
}

void text(const char* filename, double x, double y) {
   CALL(text,QString,filename,double,x,double,y);
}

void play(const char* filename) {
   CALL(play,QString,filename); 
}

// save is synchronous, in order that it can provide a return value
bool save(const char* filename) { 
   CALL(save,QString,filename); 
   while (save_result == -1) QThread::yieldCurrentThread();
   bool result = (save_result==1);
   save_result = -1; 
   return result; 
}

// variables to help frame logic
timespec next_frame_mintime; // earliest time next frame can be shown
int badframes = 0;
int numframes = 0;

// helper
bool mintime_has_passed() {
   timespec now;
   clock_gettime(CLOCK_MONOTONIC, &now);
   return now.tv_sec > next_frame_mintime.tv_sec ||
   (now.tv_sec == next_frame_mintime.tv_sec &&
   now.tv_nsec >= next_frame_mintime.tv_nsec);
}

void show(int ms) {
   if (numframes > 0 && mintime_has_passed())
      badframes++;
   numframes++;
   if ((numframes % 200 == 0) && (badframes > numframes / 2)) 
      cerr << "Warning! Can't show() that fast. Try increasing ms to show."
      << endl << badframes << " bad frames out of " << numframes << endl;
   
   while (!mintime_has_passed()) QThread::yieldCurrentThread();

   // calculate next mintime
   clock_gettime(CLOCK_MONOTONIC, &next_frame_mintime);
   next_frame_mintime.tv_sec += ms / 1000;
   next_frame_mintime.tv_nsec += (ms % 1000) * 1000000; // add nanoseconds
   next_frame_mintime.tv_sec += next_frame_mintime.tv_nsec / 1000000000;
   next_frame_mintime.tv_nsec %= 1000000000;
   showframe(); 
}

} // end of 'draw' namespace

