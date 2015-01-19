#include "draw.h"
#include "drawmoc.h"
#include <QPainter>
#include <QApplication>
#include <QPixmap>
#include <QWidget>
#include <thread>
#include <atomic>
#include <chrono>
#include <iostream>
using namespace std;

atomic<APIWidget*> aw;
atomic<bool> ready; // is initialization complete?

WindowWidget::WindowWidget(QWidget *parent) : QWidget(parent) { 
   int width = 512;
   int height = 512;
   this->resize(width, height);
   this->move(100, 100);     // not crammed in corner 
   this->setWindowTitle("std_draw");
   this->show();

   pm = new QPixmap(width, height);
   (*pm).fill(); // clear
   
   connect(aw, SIGNAL(sendRepaint()), this, SLOT(catchRepaint()));
   connect(aw, SIGNAL(sendEllipse(int,int,int,int)), this, 
               SLOT(catchEllipse(int,int,int,int)));
   connect(aw, SIGNAL(sendClear()), this, SLOT(catchClear()));
   
   ready = true;
}

WindowWidget::~WindowWidget() {
   delete pm;
   quick_exit(0); // if user closes window, terminate all including 
   // main computation thread that has nothing to do with qt
}

void WindowWidget::catchEllipse(int x, int y, int width, int height) {
   QPainter(this->pm).drawEllipse(x, y, width, height);
}

void WindowWidget::catchRepaint() {
   this->repaint();
}

void WindowWidget::catchClear() {
   this->pm->fill();
}

void WindowWidget::paintEvent(QPaintEvent *e) {
   Q_UNUSED(e);    
   QPainter(this).drawPixmap(0, 0, *pm);
}

// method that the gui thread will run
void start_std_draw() {   
   int argc = 0;
   char* argv[0];
   QApplication app(argc, argv); 
   aw = new APIWidget;
   WindowWidget window;
   app.exec();
}

// ensure gui starts, and continues after 'main' is 'done'
std::thread std_draw_thread;

int start_stuff() {
   std_draw_thread = std::thread(start_std_draw);
   while (!ready) ; // wait for initialization to finish
   return 0;
}
int garbage = start_stuff();

[[noreturn]] void done() {
   std_draw_thread.join();
   quick_exit(0);
}

void repaint();

// convenience function
void show(int ms) {
   this_thread::sleep_for(chrono::milliseconds(ms));
   repaint();
}

// adaptors since we don't want students to have to deal with objects
void ellipse(int x, int y, int width, int height) 
{ (*aw).sendEllipsePub(x, y, width, height); this_thread::yield(); }
void repaint() 
{ (*aw).sendRepaintPub(); this_thread::yield(); }
void clear() 
{ (*aw).sendClearPub(); this_thread::yield(); }
void circle(double x, double y, double r) {
   ellipse((x-r)*256+256, (y-r)*256+256, r*512, r*512);
}

// adaptors since signals can't be public, only protected
void APIWidget::sendRepaintPub()
{ emit sendRepaint(); }
void APIWidget::sendEllipsePub(int x, int y, int width, int height) 
{ emit sendEllipse(x, y, width, height); }
void APIWidget::sendClearPub() 
{ emit sendClear(); }

