#ifndef _H_DRAW_MOC
#define _H_DRAW_MOC

#include <QWidget>

class WindowWidget : public QWidget
{
  Q_OBJECT  
  public:
    WindowWidget(QWidget *parent = 0);
    ~WindowWidget();
  protected:
    void paintEvent(QPaintEvent *event);
  public slots:
    void catchEllipse(int, int, int, int);
    void catchRepaint();
    void catchClear();
  private:
    QPixmap* pm;
};

class APIWidget : public QObject
{
  Q_OBJECT
  public:
    void sendEllipsePub(int, int, int, int);
    void sendRepaintPub();
    void sendClearPub();
  signals:
    void sendEllipse(int, int, int, int);
    void sendRepaint();
    void sendClear();
};

#endif
