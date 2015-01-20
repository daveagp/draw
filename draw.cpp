#define _DRAW_NO_XFORM_MAIN
#include "draw.h"
#include <QPainter>
#include <QApplication>
#include <QPixmap>
#include <QWidget>
#include <thread>
#include <atomic>
#include <chrono>
#include <iostream>
#ifndef DRAW_MUTE
#include <phonon/phonon>
#endif

using std::this_thread::yield; using std::cout; using std::endl;

namespace draw {

class SendWidget; class ReceiveWidget; // forward decls

std::thread gui_thread; // used to spawn and join gui thread
std::atomic<SendWidget*> sendwidget; // singleton
std::atomic<ReceiveWidget*> receivewidget; // singleton
std::atomic<bool> ready(false); // is initialization complete?

class ReceiveWidget : public QWidget
{
   Q_OBJECT  
   public:
      ReceiveWidget(QWidget *parent = 0);

      int pending;
      QPixmap* pm;
      double xmin, xmax, ymin, ymax;
      int width, height;
      int r, g, b, a;
      
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
      void r_ellipse(double, double, double, double);
      void r_filled_ellipse(double, double, double, double);
      void r_setcolor(int, int, int);
      void r_settransparency(double);
      void r_setxscale(double, double);
      void r_setyscale(double, double);
      void r_image(QString, double, double);
      void r_play(QString);
      void r_refresh();
      void r_clear();
};

class SendWidget : public QObject
{
   Q_OBJECT
   signals:
      void s_ellipse(double, double, double, double);
      void s_filled_ellipse(double, double, double, double);
      void s_settransparency(double);
      void s_setcolor(int, int, int);
      void s_setxscale(double, double);
      void s_setyscale(double, double);
      void s_image(QString, double, double);
      void s_play(QString);
      void s_refresh();
      void s_clear();
   public: // adaptors since signals can't be public, only protected
      void ellipse(double x, double y, double xr, double yr) 
      { emit s_ellipse(x, y, xr, yr); }
      void filled_ellipse(double x, double y, double xr, double yr) 
      { emit s_filled_ellipse(x, y, xr, yr); }
      void setcolor(int r, int g, int b)
      { emit s_setcolor(r, g, b); }
      void settransparency(double t)
      { emit s_settransparency(t); }
      void setxscale(double min, double max)
      { emit s_setxscale(min, max); }
      void setyscale(double min, double max)
      { emit s_setyscale(min, max); }
      void image(QString filename, double x, double y)
      { emit s_image(filename, x, y); }
      void play(char filename[80])
      { emit s_play(filename);}
      void refresh()
      { emit s_refresh(); }
      void clear()
      { emit s_clear(); }
}; 

SendWidget* send() {
   // a giant queue may crash your VM. prevent that!
   while ((*receivewidget).pending > 1000) yield();
   yield();
   (*receivewidget).pending++;
   return sendwidget;
}

// adaptors since we don't want students to have to deal with objects
void ellipse(double x, double y, double xr, double yr) 
{ send()->ellipse(x, y, xr, yr); }
void circle(double x, double y, double r) 
{ ellipse(x, y, r, r); }
void filled_ellipse(double x, double y, double xr, double yr) 
{ send()->filled_ellipse(x, y, xr, yr); }
void filled_circle(double x, double y, double r) 
{ filled_ellipse(x, y, r, r); }
void setcolor(int r, int g, int b) 
{ send()->setcolor(r, g, b); }
void settransparency(double t) 
{ send()->settransparency(t); }
void setxscale(double min, double max) 
{ send()->setxscale(min, max); }
void setyscale(double min, double max) 
{ send()->setyscale(min, max); }
void setscale(double min, double max) 
{ setxscale(min, max); setyscale(min, max); }
void image(char* filename, double x, double y)
{ send()->image(QString(filename), x, y); }
void play(char filename[80])
{ send()->play(filename); }
void refresh() 
{ send()->refresh(); }
void clear() 
{ send()->clear(); }
void show(int ms)
{ refresh(); std::this_thread::sleep_for(std::chrono::milliseconds(ms)); }

ReceiveWidget::ReceiveWidget(QWidget *parent) : QWidget(parent) { 
   width = height = 512;
   r = g = b = 0;
   a = 255;
   this->resize(width, height);
   this->move(100, 100);     // not crammed in corner 
   this->setWindowTitle("draw");
   this->QWidget::show();
   pending = 0;
   
   xmin = 0; xmax = 1; ymin = 0; ymax = 1;

   pm = new QPixmap(width, height);
   (*pm).fill(); // clear
   
   connect(sendwidget, SIGNAL(s_ellipse(double,double,double,double)), 
           this, SLOT(r_ellipse(double,double,double,double)));
   connect(sendwidget, SIGNAL(s_filled_ellipse(double,double,double,double)), 
           this, SLOT(r_filled_ellipse(double,double,double,double)));
   connect(sendwidget, SIGNAL(s_setcolor(int,int,int)), 
           this, SLOT(r_setcolor(int,int,int)));
   connect(sendwidget, SIGNAL(s_settransparency(double)), 
           this, SLOT(r_settransparency(double)));
   connect(sendwidget, SIGNAL(s_setxscale(double,double)), 
           this, SLOT(r_setxscale(double,double)));
   connect(sendwidget, SIGNAL(s_setyscale(double,double)), 
           this, SLOT(r_setyscale(double,double)));
   connect(sendwidget, SIGNAL(s_image(QString,double,double)), 
           this, SLOT(r_image(QString,double,double)));           
   connect(sendwidget, SIGNAL(s_play(QString)), 
           this, SLOT(r_play(QString)));           
   connect(sendwidget, SIGNAL(s_refresh()), this, SLOT(r_refresh()));
   connect(sendwidget, SIGNAL(s_clear()), this, SLOT(r_clear()));
   
   ready = true;
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
   result.setPen(Qt::NoPen);
   result.setBrush(QBrush(QColor(r, g, b, a), Qt::SolidPattern));
   return result;
}

QPainter& ReceiveWidget::liner(QPainter& result) {
   result.setPen(QColor(r, g, b, a));
   return result;
}

// slots

void ReceiveWidget::r_ellipse(double x, double y, double xr, double yr) {
   QPainter painter(pm);
   liner(painter).drawEllipse(QPointF(affx(x), affy(y)), linx(xr), liny(yr));
   pending--;
}

void ReceiveWidget::r_filled_ellipse(double x, double y, double xr, double yr) {
   QPainter painter(pm);
   filler(painter).drawEllipse(QPointF(affx(x), affy(y)), linx(xr), liny(yr));
   pending--;
}

void ReceiveWidget::r_setcolor(int r, int g, int b) {
   this->r = r;
   this->g = g;
   this->b = b;
   pending--;
}

void ReceiveWidget::r_settransparency(double t) {
   this->a = 255*(1-t);
   pending--;
}

void ReceiveWidget::r_setxscale(double min, double max) {
   xmin = min;
   xmax = max;
   pending--;
}

void ReceiveWidget::r_setyscale(double min, double max) {
   ymin = min;
   ymax = max;
   pending--;
}

void ReceiveWidget::r_image(QString filename, double x, double y) {
   QPainter painter(pm);
   QImage img(filename);
   painter.drawImage(affx(x)-img.width()/2, affy(y)-img.height()/2, img);
   pending--;
}

void ReceiveWidget::r_play(QString filename) {
#ifndef DRAW_MUTE
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
}

void ReceiveWidget::r_refresh() {
   this->repaint();
   pending--;
}

// end of ReceiveWidget member functions

// infrastructure
void _start() {   
   int argc = 0;
   char* argv[0];
   QApplication app(argc, argv);
   app.setApplicationName("draw"); 
   sendwidget = new SendWidget;
   receivewidget = new ReceiveWidget;
   app.exec();
   quick_exit(0); // if user closes window, terminate main thread
}

int _spawn() {
   gui_thread = std::thread(_start);
   while (!ready) yield(); // wait for initialization to finish
   return 0;
}
int _dont_care_just_call_this_plz = _spawn();

void _done(int r) {
   gui_thread.join();
   quick_exit(r);
}

} // end of 'draw' namespace

