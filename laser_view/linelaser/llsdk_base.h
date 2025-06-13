#ifndef LLSDK_BASE_H
#define LLSDK_BASE_H
#include <QSerialPort>
#include <QSerialPortInfo>
#include "inc/radarscan.h"
typedef enum
{
    TYPE_LASER_EAI = 0,//EAIwanzhishang
    TYPE_LASER_RS = 1,//ruishi
    TYPE_LASER_RS_NEW = 2,//ruishi
    TYPE_LASER_RS_XVB02 = 3,
    TYPE_LASER_YX = 4,//yuanxingshikong
    TYPE_LASER_LD = 5,//ledong
    TYPE_LASER_other = 6,

    TYPE_LASER_NONE = 255,
}TYPE_LASER_;

class LLSDK_BASE
{
public:
    LLSDK_BASE() {};
    virtual ~LLSDK_BASE() = default;

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
