#define _DRAW_NO_XFORM_MAIN // don't want that when compiling draw.cpp
#include "draw.h"
#include <QPainter>
#include <QApplication>
#include <QPixmap>
#include <QWidget>
#include <QThread>
#include <time.h>
#include <iostream>
#ifdef DRAW_UNMUTE
#include <phonon/phonon>
#endif

namespace draw {

using std::cout; using std::endl; using std::cerr;

class SendWidget; class ReceiveWidget; // forward decls

const int MAX_QUEUE_SIZE = 100;
QAtomicPointer<SendWidget> sendwidget; // singleton
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
   public slots:
      void r_point(double, double);
      void r_line(double, double, double, double);
      void r_ellipse(double, double, double, double);
      void r_filled_ellipse(double, double, double, double);
      void r_polygon(QList<double>, QList<double>);
      void r_filled_polygon(QList<double>, QList<double>);
      void r_setcolor(int, int, int);
      void r_setfontsize(int);
      void r_setpenwidth(double);
      void r_settransparency(double);
      void r_setxrange(double, double);
      void r_setyrange(double, double);
      void r_image(QString, double, double);
      void r_text(QString, double, double);
      void r_setwindowsize(int, int);
      void r_save(QString);
      void r_play(QString);
      void r_showframe();
      void r_clear();
};

class SendWidget : public QObject
{
   Q_OBJECT
   signals:
      void s_point(double, double);
      void s_line(double, double, double, double);
      void s_ellipse(double, double, double, double);
      void s_filled_ellipse(double, double, double, double);
      void s_polygon(QList<double>, QList<double>);
      void s_filled_polygon(QList<double>, QList<double>);
      void s_settransparency(double);
      void s_setcolor(int, int, int);
      void s_setpenwidth(double);
      void s_setfontsize(int);
      void s_setxrange(double, double);
      void s_setyrange(double, double);
      void s_image(QString, double, double);
      void s_text(QString, double, double);
      void s_setwindowsize(int, int);
      void s_save(QString);
      void s_play(QString);
      void s_showframe();
      void s_clear();
   public: // adaptors since signals can't be public, only protected
      void point(double x, double y)
      { emit s_point(x, y); }
      void line(double x0, double y0, double x1, double y1)
      { emit s_line(x0, y0, x1, y1); }
      void ellipse(double x, double y, double xr, double yr) 
      { emit s_ellipse(x, y, xr, yr); }
      void filled_ellipse(double x, double y, double xr, double yr) 
      { emit s_filled_ellipse(x, y, xr, yr); }
      void polygon(QList<double> x, QList<double> y)
      { emit s_polygon(x, y); }
      void filled_polygon(QList<double> x, QList<double> y)
      { emit s_filled_polygon(x, y); }
      void setcolor(int r, int g, int b)
      { emit s_setcolor(r, g, b); }
      void settransparency(double t)
      { emit s_settransparency(t); }
      void setfontsize(int w)
      { emit s_setfontsize(w); }
      void setpenwidth(double w)
      { emit s_setpenwidth(w); }
      void setxrange(double min, double max)
      { emit s_setxrange(min, max); }
      void setyrange(double min, double max)
      { emit s_setyrange(min, max); }
      void image(QString filename, double x, double y)
      { emit s_image(filename, x, y); }
      void text(QString filename, double x, double y)
      { emit s_text(filename, x, y); }
      void setwindowsize(int w, int h)
      { emit s_setwindowsize(w, h); }
      void play(char* filename)
      { emit s_play(filename);}
      void save(char* filename)
      { emit s_save(filename);}
      void showframe()
      { emit s_showframe(); }
      void clear()
      { emit s_clear(); }
}; 

SendWidget* send() {
   // a giant queue may crash your VM. prevent that!
   while ((*receivewidget).pending > MAX_QUEUE_SIZE) 
      QThread::yieldCurrentThread();
   QThread::yieldCurrentThread();
   (*receivewidget).pending++;
   return sendwidget;
}

// adaptors since we don't want students to have to deal with objects
void point(double x, double y)
{ send()->point(x, y); }
void line(double x0, double y0, double x1, double y1)
{ send()->line(x0, y0, x1, y1); }
void ellipse(double x, double y, double xr, double yr) 
{ send()->ellipse(x, y, xr, yr); }
void circle(double x, double y, double r) 
{ ellipse(x, y, r, r); }
void filled_ellipse(double x, double y, double xr, double yr) 
{ send()->filled_ellipse(x, y, xr, yr); }
void filled_circle(double x, double y, double r) 
{ filled_ellipse(x, y, r, r); }
void polygon(int n, double x[], double y[])
{ QList<double> lx, ly; for (int i=0; i<n; i++) {lx << x[i]; ly << y[i];}
  send()->polygon(lx, ly); }
void filled_polygon(int n, double x[], double y[])
{ QList<double> lx, ly; for (int i=0; i<n; i++) {lx << x[i]; ly << y[i];}
  send()->filled_polygon(lx, ly); }
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

void setpenwidth(double w) 
{ send()->setpenwidth(w); }
void setfontsize(int w) 
{ send()->setfontsize(w); }
void setcolor(int r, int g, int b) 
{ send()->setcolor(r, g, b); }
void setcolor(int color[3]) 
{ setcolor(color[0], color[1], color[2]); }
void setcolor(int const color[3]) 
{ setcolor(color[0], color[1], color[2]); }
void settransparency(double t) 
{ send()->settransparency(t); }
void setxrange(double min, double max) 
{ send()->setxrange(min, max); }
void setyrange(double min, double max) 
{ send()->setyrange(min, max); }
void setrange(double min, double max) 
{ setxrange(min, max); setyrange(min, max); }
void setwindowsize(int width, int height) 
{ send()->setwindowsize(width, height); }
void image(char* filename, double x, double y)
{ send()->image(QString(filename), x, y); }
void image(const char* filename, double x, double y)
{ image(const_cast<char*>(filename), x, y); }
void text(char* filename, double x, double y)
{ send()->text(QString(filename), x, y); }
void text(const char* filename, double x, double y)
{ send()->text(QString(filename), x, y); }
void play(char* filename)
{ send()->play(filename); }
void play(const char* filename)
{ play(const_cast<char*>(filename)); }
bool save(char* filename) // synchronous, wait for result
{ send()->save(filename); while (save_result == -1) QThread::yieldCurrentThread();
  bool result = (save_result==1); save_result = -1; return result; }
bool save(const char* filename) // synchronous, wait for result
{ return save(const_cast<char*>(filename)); }
void showframe() 
{ send()->showframe(); }
void clear() 
{ send()->clear(); }

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
   
   this->r_setxrange(0, 1);
   this->r_setyrange(0, 1);

   pm = new QPixmap(width, height);
   (*pm).fill(); // clear
   
   connect(sendwidget, SIGNAL(s_point(double,double)), 
           this, SLOT(r_point(double,double)));
   connect(sendwidget, SIGNAL(s_line(double,double,double,double)), 
           this, SLOT(r_line(double,double,double,double)));
   connect(sendwidget, SIGNAL(s_ellipse(double,double,double,double)), 
           this, SLOT(r_ellipse(double,double,double,double)));
   connect(sendwidget, SIGNAL(s_filled_ellipse(double,double,double,double)), 
           this, SLOT(r_filled_ellipse(double,double,double,double)));
   connect(sendwidget, SIGNAL(s_polygon(QList<double>,QList<double>)), 
           this, SLOT(r_polygon(QList<double>,QList<double>)));
   connect(sendwidget, SIGNAL(s_filled_polygon(QList<double>,QList<double>)), 
           this, SLOT(r_filled_polygon(QList<double>,QList<double>)));
   connect(sendwidget, SIGNAL(s_setfontsize(int)), 
           this, SLOT(r_setfontsize(int)));
   connect(sendwidget, SIGNAL(s_setpenwidth(double)), 
           this, SLOT(r_setpenwidth(double)));
   connect(sendwidget, SIGNAL(s_setcolor(int,int,int)), 
           this, SLOT(r_setcolor(int,int,int)));
   connect(sendwidget, SIGNAL(s_settransparency(double)), 
           this, SLOT(r_settransparency(double)));
   connect(sendwidget, SIGNAL(s_setxrange(double,double)), 
           this, SLOT(r_setxrange(double,double)));
   connect(sendwidget, SIGNAL(s_setyrange(double,double)), 
           this, SLOT(r_setyrange(double,double)));
   connect(sendwidget, SIGNAL(s_setwindowsize(int,int)), 
           this, SLOT(r_setwindowsize(int,int)));
   connect(sendwidget, SIGNAL(s_image(QString,double,double)), 
           this, SLOT(r_image(QString,double,double)));           
   connect(sendwidget, SIGNAL(s_text(QString,double,double)), 
           this, SLOT(r_text(QString,double,double)));           
   connect(sendwidget, SIGNAL(s_play(QString)), 
           this, SLOT(r_play(QString)));           
   connect(sendwidget, SIGNAL(s_save(QString)), 
           this, SLOT(r_save(QString)));           
   connect(sendwidget, SIGNAL(s_showframe()), this, SLOT(r_showframe()));
   connect(sendwidget, SIGNAL(s_clear()), this, SLOT(r_clear()));
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

void ReceiveWidget::r_point(double x, double y) {
   QPainter painter(pm);
   liner(painter).drawPoint(QPointF(affx(x), affy(y)));
   pending--;
   if (!animation_mode) this->repaint();
}

void ReceiveWidget::r_line(double x0, double y0, double x1, double y1) {
   QPainter painter(pm);
   liner(painter).drawLine(QPointF(affx(x0), affy(y0)), QPointF(affx(x1), affy(y1)));
   pending--;
   if (!animation_mode) this->repaint();
}

void ReceiveWidget::r_ellipse(double x, double y, double xr, double yr) {
   QPainter painter(pm);
   liner(painter).drawEllipse(QPointF(affx(x), affy(y)), linx(xr), liny(yr));
   pending--;
   if (!animation_mode) this->repaint();
}

void ReceiveWidget::r_filled_ellipse(double x, double y, double xr, double yr) {
   QPainter painter(pm);
   filler(painter).drawEllipse(QPointF(affx(x), affy(y)), linx(xr), liny(yr));
   pending--;
   if (!animation_mode) this->repaint();
}

void ReceiveWidget::r_polygon(QList<double> x, QList<double> y) {
   QPolygonF p;
   for (int i=0; i<x.size(); i++) p << QPointF(affx(x[i]), affy(y[i]));
   QPainter painter(pm);
   liner(painter).drawPolygon(p);
   pending--;
   if (!animation_mode) this->repaint();
}

void ReceiveWidget::r_filled_polygon(QList<double> x, QList<double> y) {
   QPolygonF p;
   for (int i=0; i<x.size(); i++) p << QPointF(affx(x[i]), affy(y[i]));
   QPainter painter(pm);
   filler(painter).drawPolygon(p);
   pending--;
   if (!animation_mode) this->repaint();
}

void ReceiveWidget::r_setfontsize(int w) {
   this->fontsize = w;
   pending--;
}

void ReceiveWidget::r_setpenwidth(double w) {
   this->penwidth = w;
   pending--;
}

void ReceiveWidget::r_setcolor(int rnew, int gnew, int bnew) {
   this->r = rnew;
   this->g = gnew;
   this->b = bnew;
   pending--;
}

void ReceiveWidget::r_settransparency(double t) {
   this->a = (int)(255*(1-t));
   pending--;
}

void ReceiveWidget::r_setxrange(double min, double max) {
   xmin = min;
   xmax = max;
   pending--;
}

void ReceiveWidget::r_setyrange(double min, double max) {
   if (y_increases_up) {ymin = max; ymax = min;}
   else {ymax = min; ymin = max;}
   pending--;
}

void ReceiveWidget::r_setwindowsize(int newwidth, int newheight) {
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

void ReceiveWidget::r_text(QString text, double x, double y) {
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

void ReceiveWidget::r_image(QString filename, double x, double y) {
   QPainter painter(pm);
   QImage img(filename);
   painter.drawImage(QPointF(affx(x)-img.width()/2, affy(y)-img.height()/2), img);
   pending--;
   if (!animation_mode) this->repaint();
}

void ReceiveWidget::r_save(QString filename) {
   save_result = pm->save(filename) ? 1 : 0;
   pending--;
}

void ReceiveWidget::r_play(QString filename) {
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

void ReceiveWidget::r_clear() {
   this->pm->fill();
   pending--;
   if (!animation_mode) this->repaint();
}

void ReceiveWidget::r_showframe() {   
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
   sendwidget = new SendWidget;
   receivewidget = new ReceiveWidget;
   StudentThread* st = new StudentThread(argc, argv);
   st->start(); // start student main() in its own thread
   app.exec(); // wait until user closes window
   return retcode; // pass result of user's main, if it finished
}

} // end of 'draw' namespace

