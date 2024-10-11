#include "inc/widget.h"
#include <QApplication>
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Widget w;
    w.show();
    // 连续检测串口接入
    QTimer *timer = new QTimer();
    QObject::connect(timer, &QTimer::timeout, &w, [&w](){
        w.checkPorts();
    });
    timer->start(1000); // 每秒检查一次

    return a.exec();
}
