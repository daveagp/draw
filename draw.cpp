#define _DRAW_NO_XFORM_MAIN // don't want that when compiling draw.cpp
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

class ReceiveWidget; // forward decls

const int MAX_QUEUE_SIZE = 100;
QAtomicPointer<ReceiveWidget> receivewidget; // singleton
QAtomicInt retcode(0); // return code from main
QAtomicInt save_result(-1); // 0 ok, 1 error

// used only by user thread
timespec next_frame_mintime;
int badframes = 0;
int numframes = 0;

class ReceiveWidget : public QWidget
{
   Q_OBJECT  
   public:
      ReceiveWidget(QWidget *parent = 0);

      int pending;
      QPixmap* pm, *prepared_frame;
      double xmin, xmax, ymin, ymax;
      int width, height;
      int r, g, b, a;
      double penwidth;
      int fontsize;
      bool y_increases_up;
      bool animation_mode;
 
      void use_pen();
      void use_brush();
      double affx(double);
      double affy(double);
      double linx(double);
      double liny(double);
      QPainter& filler(QPainter&);
      QPainter& liner(QPainter&);
      
   protected:
      void paintEvent(QPaintEvent *event);
   public:
      void point(double, double);
      void line(double, double, double, double);
      void ellipse(double, double, double, double);
      void filled_ellipse(double, double, double, double);
      void polygon(QList<double>, QList<double>);
      void filled_polygon(QList<double>, QList<double>);
      void setcolor(int, int, int);
      void setfontsize(int);
      void setpenwidth(double);
      void settransparency(double);
      void setxrange(double, double);
      void setyrange(double, double);
      void image(QString, double, double);
      void text(QString, double, double);
      void setwindowsize(int, int);
      void save(QString);
      void play(QString);
      void showframe();
      void clear();
};

ReceiveWidget::ReceiveWidget(QWidget *parent) : QWidget(parent) { 
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
   
   this->setxrange(0, 1);
   this->setyrange(0, 1);

   pm = new QPixmap(width, height);
   (*pm).fill(); // clear
   
}

void ReceiveWidget::paintEvent(QPaintEvent *e) {
   Q_UNUSED(e);    
   QPainter(this).drawPixmap(0, 0, *pm);
}

double ReceiveWidget::affx(double x0) {
   return (x0-xmin)/(xmax-xmin)*width;
}

double ReceiveWidget::affy(double y0) {
   return (y0-ymin)/(ymax-ymin)*height;
}

double ReceiveWidget::linx(double dx) {
   return dx*width/(xmax-xmin);
}

double ReceiveWidget::liny(double dy) {
   return dy*height/(ymax-ymin);
}

QPainter& ReceiveWidget::filler(QPainter& result) {
   result.setRenderHint(QPainter::Antialiasing, true);
   result.setPen(Qt::NoPen);
   result.setBrush(QBrush(QColor(r, g, b, a), Qt::SolidPattern));
   return result;
}

QPainter& ReceiveWidget::liner(QPainter& result) {
   result.setRenderHint(QPainter::Antialiasing, true);
   result.setPen(QPen(QBrush(QColor(r, g, b, a)), penwidth, 
      Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
   return result;
}

// slots

void ReceiveWidget::point(double x, double y) {
   QPainter painter(pm);
   liner(painter).drawPoint(QPointF(affx(x), affy(y)));
   pending--;
   if (!animation_mode) this->repaint();
}

void ReceiveWidget::line(double x0, double y0, double x1, double y1) {
   QPainter painter(pm);
   liner(painter).drawLine(QPointF(affx(x0), affy(y0)), QPointF(affx(x1), affy(y1)));
   pending--;
   if (!animation_mode) this->repaint();
}

void ReceiveWidget::ellipse(double x, double y, double xr, double yr) {
   QPainter painter(pm);
   liner(painter).drawEllipse(QPointF(affx(x), affy(y)), linx(xr), liny(yr));
   pending--;
   if (!animation_mode) this->repaint();
}

void ReceiveWidget::filled_ellipse(double x, double y, double xr, double yr) {
   QPainter painter(pm);
   filler(painter).drawEllipse(QPointF(affx(x), affy(y)), linx(xr), liny(yr));
   pending--;
   if (!animation_mode) this->repaint();
}

void ReceiveWidget::polygon(QList<double> x, QList<double> y) {
   QPolygonF p;
   for (int i=0; i<x.size(); i++) p << QPointF(affx(x[i]), affy(y[i]));
   QPainter painter(pm);
   liner(painter).drawPolygon(p);
   pending--;
   if (!animation_mode) this->repaint();
}

void ReceiveWidget::filled_polygon(QList<double> x, QList<double> y) {
   QPolygonF p;
   for (int i=0; i<x.size(); i++) p << QPointF(affx(x[i]), affy(y[i]));
   QPainter painter(pm);
   filler(painter).drawPolygon(p);
   pending--;
   if (!animation_mode) this->repaint();
}

void ReceiveWidget::setfontsize(int w) {
   this->fontsize = w;
   pending--;
}

void ReceiveWidget::setpenwidth(double w) {
   this->penwidth = w;
   pending--;
}

void ReceiveWidget::setcolor(int rnew, int gnew, int bnew) {
   this->r = rnew;
   this->g = gnew;
   this->b = bnew;
   pending--;
}

void ReceiveWidget::settransparency(double t) {
   this->a = (int)(255*(1-t));
   pending--;
}

void ReceiveWidget::setxrange(double min, double max) {
   xmin = min;
   xmax = max;
   pending--;
}

void ReceiveWidget::setyrange(double min, double max) {
   if (y_increases_up) {ymin = max; ymax = min;}
   else {ymax = min; ymin = max;}
   pending--;
}

void ReceiveWidget::setwindowsize(int newwidth, int newheight) {
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
   pending--;
}

void ReceiveWidget::text(QString text, double x, double y) {
   QPainter painter(pm);
   QFont f = painter.font();
   f.setPointSize((int)fontsize);
   painter.setFont(f);
   //cout << text.toUtf8().constData() << endl;
   QRectF rect(affx(x)-width/2, affy(y)-height/2, width, height);
   liner(painter).drawText(rect, Qt::AlignCenter, text);
   pending--;
   if (!animation_mode) this->repaint();
}

void ReceiveWidget::image(QString filename, double x, double y) {
   QPainter painter(pm);
   QImage img(filename);
   painter.drawImage(QPointF(affx(x)-img.width()/2, affy(y)-img.height()/2), img);
   pending--;
   if (!animation_mode) this->repaint();
}

void ReceiveWidget::save(QString filename) {
   save_result = pm->save(filename) ? 1 : 0;
   pending--;
}

void ReceiveWidget::play(QString filename) {
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
   pending--;
}

void ReceiveWidget::clear() {
   this->pm->fill();
   pending--;
   if (!animation_mode) this->repaint();
}

void ReceiveWidget::showframe() {   
   if (!animation_mode) {
      animation_mode = true;
      prepared_frame = new QPixmap(*pm);
   }
   else {
      QPainter p(prepared_frame);
      p.drawPixmap(0, 0, *pm);
   }
   this->repaint();
   pending--;
}

// end of ReceiveWidget member functions


void send() {
   // a giant queue may crash your VM. prevent that!
   while ((*receivewidget).pending > MAX_QUEUE_SIZE) 
      QThread::yieldCurrentThread();
   QThread::yieldCurrentThread();
   (*receivewidget).pending++;
}

#define CALL(meth,...) \
 send(); \
 const QMetaObject* mo = (*receivewidget).metaObject(); \
 QByteArray normalizedSignature = \
 QMetaObject::normalizedSignature( SIG(meth,__VA_ARGS__) ); \
 int methodIndex = mo->indexOfMethod(normalizedSignature); \
 QMetaMethod method = mo->method(methodIndex); \
 method.invoke(receivewidget, Qt::QueuedConnection ARGS(__VA_ARGS__));

// e.g. SIG(foo, int, bar, double, x) => "foo(int, double)"
#define SIG(meth,...) #meth "(" SIGN(__VA_ARGS__,4,4,3,3,2,2,1,0,0,0) ")"
#define SIGN(a,b,c,d,e,f,g,h,i,...) SIG##i(a,b,c,d,e,f,g,h)
#define SIG0(...)
#define SIG1(a,b,...) #a 
#define SIG2(a,b,...) #a "," SIG1(__VA_ARGS__)
#define SIG3(a,b,...) #a "," SIG2(__VA_ARGS__)
#define SIG4(a,b,...) #a "," SIG3(__VA_ARGS__)

// e.g. ARGS(int, bar, double, x) => Q_ARG(int, bar), Q_ARG(double, x)
#define ARGS(...) ARGSN(__VA_ARGS__,4,4,3,3,2,2,1,0,0,0)
#define ARGSN(a,b,c,d,e,f,g,h,i,...) ARGS##i(a,b,c,d,e,f,g,h)
#define ARGS0(...) 
#define ARGS1(a,b,...) ,Q_ARG(a,b)
#define ARGS2(a,b,...) ,Q_ARG(a,b) ARGS1(__VA_ARGS__)
#define ARGS3(a,b,...) ,Q_ARG(a,b) ARGS2(__VA_ARGS__)
#define ARGS4(a,b,...) ,Q_ARG(a,b) ARGS3(__VA_ARGS__)

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

SIMPLE(point,double,double);
SIMPLE(line,double,double,double,double);
SIMPLE(ellipse,double,double,double,double);
SIMPLE(filled_ellipse,double,double,double,double);
SIMPLE(setpenwidth,double);
SIMPLE(setfontsize,int);
SIMPLE(setcolor,int,int,int);
SIMPLE(settransparency,double);
SIMPLE(setxrange,double,double);
SIMPLE(setyrange,double,double);
SIMPLE(setwindowsize,int,int);
SIMPLE(showframe);
SIMPLE(clear);

void circle(double x, double y, double r) 
{ ellipse(x, y, r, r); }
void filled_circle(double x, double y, double r) 
{ filled_ellipse(x, y, r, r); }
void polygon(int n, double x[], double y[])
{ QList<double> lx, ly; for (int i=0; i<n; i++) {lx << x[i]; ly << y[i];}
  CALL(polygon, QList<double>, lx, QList<double>, ly); }  
void filled_polygon(int n, double x[], double y[])
{ QList<double> lx, ly; for (int i=0; i<n; i++) {lx << x[i]; ly << y[i];}
  CALL(filled_polygon, QList<double>, lx, QList<double>, ly); }
void square(double x, double y, double s)
{ rectangle(x-s/2, y-s/2, x+s/2, y+s/2); }
void rectangle(double x0, double y0, double x1, double y1)
{ double xc[4] = {x0, x0, x1, x1}; double yc[4] = {y0, y1, y1, y0};
  polygon(4, xc, yc); }
void filled_square(double x, double y, double s)
{ filled_rectangle(x-s/2, y-s/2, x+s/2, y+s/2); }
void filled_rectangle(double x0, double y0, double x1, double y1)
{ double xc[4] = {x0, x0, x1, x1}; double yc[4] = {y0, y1, y1, y0};
  filled_polygon(4, xc, yc); }
void setcolor(int color[3]) 
{ setcolor(color[0], color[1], color[2]); }
void setcolor(int const color[3]) 
{ setcolor(color[0], color[1], color[2]); }
void setrange(double min, double max) 
{ setxrange(min, max); setyrange(min, max); }
void image(char* filename, double x, double y)
{ CALL(image,QString,filename,double,x,double,y); }
void image(const char* filename, double x, double y)
{ image(const_cast<char*>(filename), x, y); }
void text(char* filename, double x, double y)
{ CALL(text,QString,filename,double,x,double,y); }
void text(const char* filename, double x, double y)
{ text(const_cast<char*>(filename),x,y); }
void play(char* filename)
{ CALL(play,QString,filename); }
void play(const char* filename)
{ play(const_cast<char*>(filename)); }
bool save(char* filename) // synchronous, wait for result
{ CALL(save,QString,filename); 
  while (save_result == -1) QThread::yieldCurrentThread();
  bool result = (save_result==1); save_result = -1; return result; }
bool save(const char* filename) // synchronous, wait for result
{ return save(const_cast<char*>(filename)); }

bool now_past_next_frame_mintime() {
   timespec now;
   clock_gettime(CLOCK_MONOTONIC, &now);
   return now.tv_sec > next_frame_mintime.tv_sec ||
   (now.tv_sec == next_frame_mintime.tv_sec &&
   now.tv_nsec >= next_frame_mintime.tv_nsec);
}

void show(int ms)
{ if (numframes > 0 && now_past_next_frame_mintime())
     badframes++;
  numframes++;
  if ((numframes % 200 == 0) && (badframes > numframes / 2)) 
     cerr << "Warning! Can't show() that fast. Try increasing ms to show."
      << endl << badframes << " bad frames out of " << numframes << endl;
  while (!now_past_next_frame_mintime())
      QThread::yieldCurrentThread();
  
  clock_gettime(CLOCK_MONOTONIC, &next_frame_mintime);
  next_frame_mintime.tv_sec += ms / 1000;
  next_frame_mintime.tv_nsec += (ms % 1000) * 1000000; // add nanoseconds
  next_frame_mintime.tv_sec += next_frame_mintime.tv_nsec / 1000000000;
  next_frame_mintime.tv_nsec %= 1000000000;
  showframe(); }


}
// escape namespace to forward declare
int _main(int, char**); // user's main will be transformed to this
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

int actual_main(int argc, char** argv) {
   QApplication app(argc, argv);
   qRegisterMetaType<QList<double> >("QList<double>");
   app.setApplicationName("draw");
   receivewidget = new ReceiveWidget;
   StudentThread* st = new StudentThread(argc, argv);
   st->start(); // start student main() in its own thread
   app.exec(); // wait until user closes window
   return retcode; // pass result of user's main, if it finished
}

} // end of 'draw' namespace

