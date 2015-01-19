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
using std::this_thread::yield;

namespace draw {

class SendWidget; class ReceiveWidget; // forward declarations

std::thread gui_thread; // used to spawn and join gui thread
std::atomic<SendWidget*> sendwidget; // singleton
std::atomic<bool> ready(false); // is initialization complete?
std::atomic<int> pending(0);
QApplication* app;

class ReceiveWidget : public QWidget
{
   Q_OBJECT  
   public:
      ReceiveWidget(QWidget *parent = 0);
      ~ReceiveWidget();
   protected:
      void paintEvent(QPaintEvent *event);
   public slots:
      void ellipse(double, double, double, double);
      void circle(double, double, double);
      void setxscale(double, double);
      void setyscale(double, double);
      void setscale(double, double);
      void request_show(int);
      void clear();
   private:
      double affx(double);
      double affy(double);
      double linx(double);
      double liny(double);
      QPixmap* pm;
      double xmin, xmax, ymin, ymax;
      int width, height;
};

class SendWidget : public QObject
{
  Q_OBJECT
  public:
     void ellipse(double, double, double, double);
     void circle(double, double, double);
     void setxscale(double, double);
     void setyscale(double, double);
     void setscale(double, double);
     void request_show(int);
     void clear();
  signals:
     void s_ellipse(double, double, double, double);
     void s_circle(double, double, double);
     void s_setxscale(double, double);
     void s_setyscale(double, double);
     void s_setscale(double, double);
     void s_request_show(int);
     void s_clear();
}; 

ReceiveWidget::ReceiveWidget(QWidget *parent) : QWidget(parent) { 
   width = 512;
   height = 512;
   this->resize(width, height);
   this->move(100, 100);     // not crammed in corner 
   this->setWindowTitle("draw");
   this->QWidget::show();
   
   xmin = 0; xmax = 1; ymin = 0; ymax = 1;

   pm = new QPixmap(width, height);
   (*pm).fill(); // clear
   
   connect(sendwidget, SIGNAL(s_ellipse(double,double,double,double)), 
           this,   SLOT(ellipse(double,double,double,double)));
   connect(sendwidget, SIGNAL(s_circle(double,double,double)), 
           this,   SLOT(circle(double,double,double)));
   connect(sendwidget, SIGNAL(s_setxscale(double,double)), 
           this,   SLOT(setxscale(double,double)));
   connect(sendwidget, SIGNAL(s_setyscale(double,double)), 
           this,   SLOT(setyscale(double,double)));
   connect(sendwidget, SIGNAL(s_setscale(double,double)), 
           this,   SLOT(setscale(double,double)));           
   connect(sendwidget, SIGNAL(s_request_show(int)), this, SLOT(request_show(int)));
   connect(sendwidget, SIGNAL(s_clear()), this, SLOT(clear()));
   
   ready = true;
}

ReceiveWidget::~ReceiveWidget() {
   quick_exit(0); // if user closes window, terminate main thread
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

void ReceiveWidget::ellipse(double x, double y, double xr, double yr) {
   QPainter(this->pm).drawEllipse(
      QPointF(affx(x), affy(y)), linx(xr), liny(yr));
   pending--;
}

void ReceiveWidget::circle(double x, double y, double r) {
   this->ellipse(x, y, r, r);
}

void ReceiveWidget::setxscale(double min, double max) {
   xmin = min;
   xmax = max;
   pending--;
}

void ReceiveWidget::setyscale(double min, double max) {
   ymin = min;
   ymax = max;
   pending--;
}

void ReceiveWidget::setscale(double min, double max) {
   setxscale(min, max);
   setyscale(min, max);
   pending--;
}

void ReceiveWidget::clear() {
   this->pm->fill();
   pending--;
}

void ReceiveWidget::request_show(int ms) {
   this->repaint();
   pending--;
}

// infrastructure
void _start() {   
   int argc = 0;
   char* argv[0];
   app = new QApplication(argc, argv); 
   sendwidget = new SendWidget;
   ReceiveWidget window;
   app->exec();
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

// show is special since we need the sending thread to sleep, not receiving
void request_show(int); // forward declaration
void show(int ms) {
   request_show(ms);
   std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

void wait() {
   // a giant queue may crash your VM. prevent that!
   while (pending > 1000) yield();
   yield();
   pending++;
}

// adaptors since we don't want students to have to deal with objects
void ellipse(double x, double y, double xr, double yr) 
{ wait(); (*sendwidget).ellipse(x, y, xr, yr); }
void circle(double x, double y, double r) 
{ wait(); (*sendwidget).circle(x, y, r); }
void setxscale(double min, double max) 
{ wait(); (*sendwidget).setxscale(min, max); }
void setyscale(double min, double max) 
{ wait(); (*sendwidget).setyscale(min, max); }
void setscale(double min, double max) 
{ wait(); (*sendwidget).setscale(min, max); }
void request_show(int ms) 
{ wait(); (*sendwidget).request_show(ms); }
void clear() 
{ wait(); (*sendwidget).clear(); }

// adaptors since signals can't be public, only protected
void SendWidget::ellipse(double x, double y, double xr, double yr) 
{ emit s_ellipse(x, y, xr, yr); }
void SendWidget::circle(double x, double y, double r)
{ emit s_circle(x, y, r); }
void SendWidget::setxscale(double min, double max)
{ emit s_setxscale(min, max); }
void SendWidget::setyscale(double min, double max)
{ emit s_setyscale(min, max); }
void SendWidget::setscale(double min, double max)
{ emit s_setscale(min, max); }
void SendWidget::request_show(int ms)
{ emit s_request_show(ms); }
void SendWidget::clear()
{ emit s_clear(); }


} // end of 'draw' namespace

