#ifndef LLSDK_BASE_H
#define LLSDK_BASE_H
#include <QSerialPort>
#include <QSerialPortInfo>
#include "inc/radarscan.h"
typedef enum
{
    TYPE_LASER_EAI = 0,//EAI
    TYPE_LASER_RS = 1,//瑞识
    TYPE_LASER_RS_NEW = 2,//瑞识
    TYPE_LASER_YX = 3,//远行时空
    TYPE_LASER_LD = 4,//乐动

    TYPE_LASER_NONE = 255,
}TYPE_LASER_;

class LLSDK_BASE
{
public:
    virtual bool initLaserScan(QSerialPort *serial) = 0;
    virtual bool StartLaserScan(QSerialPort *serial) = 0;
    virtual bool StopLaserScan(QSerialPort *serial) = 0;
    virtual void DataParsing(const QByteArray &data) = 0;

    virtual void setDeviceSnCallback(std::function<void(const TYPE_LASER_ type, QString sn)> callback) = 0;
    virtual void setDataCallback(std::function<void(std::vector<W_DataScan>)> callback) = 0;
    virtual QString GetOtherMessage() = 0;

    void MySleep(uint32_t ms)
    {
        QElapsedTimer timer;
        timer.start();

        while(timer.elapsed() < ms){
            // 处理其他任务
            QCoreApplication::processEvents(QEventLoop::AllEvents);
        }
    }

};

#endif // LLSDK_BASE_H
