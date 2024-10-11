#ifndef COMREADTHREAD_HPP
#define COMREADTHREAD_HPP
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QDebug>
#include <QThread>

class comReadThread : public QThread
{
public:
    comReadThread(QSerialPort *serial)
        :m_serial(serial)
    {
        thread_out = false;
    };
    comReadThread()
    {
    };
    ~comReadThread()
    {
        ThreadOut();
        quit();
        wait();
    };

    void ThreadOut()
    {
        thread_out = true;
    }

    void ReadCallback(std::function<void(const QByteArray &)> callback)
    {
        myFunc = callback;
    }

    void MergeData(const QByteArray &data)
    {
        m_data.append(data.begin(), data.size());
    }

private:
    void run() override
    {
        while (!thread_out && !isInterruptionRequested())
        {
            if (m_serial != nullptr && m_serial->isOpen() && m_serial->waitForReadyRead(200))
            { // 等待数据就绪，超时时间为1000ms
                QByteArray data = m_serial->readAll();

                //处理接收到的数据
                if (myFunc != nullptr && data.size() > 0)
                {
                    myFunc(data);
                }
            }
        }
    }
private:
    QSerialPort *m_serial = nullptr;
    std::function<void(const QByteArray &)> myFunc = nullptr;
    bool thread_out;
    QByteArray m_data;
};

#endif // COMREADTHREAD_HPP
