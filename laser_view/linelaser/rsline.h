#ifndef RSLINE_H
#define RSLINE_H
#include <QWidget>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QThread>
#include "rsline_tool.h"
#include "llsdk_base.h"

struct LineLaserDataScan
{
    float ranges_;
    float angles_;
    float intensity_;
};



class RSLine : public LLSDK_BASE
{
    typedef struct pro_com_
    {
        const uint8_t start_bit_[2] = {0xAA, 0xAA};
        uint8_t addr_;
        uint8_t cmd_;
        uint16_t offset_;
        uint16_t length_;
    }protocol_com_;
public:
    RSLine();
    ~RSLine();

    virtual bool initLaserScan(QSerialPort *serial);
    virtual bool StartLaserScan(QSerialPort *serial);
    virtual bool StopLaserScan(QSerialPort *serial);
    virtual void DataParsing(const QByteArray &data);
    virtual void setDeviceSnCallback(std::function<void(const TYPE_LASER_ type, QString sn)> callback)
    {
        RsDevSnFun = callback;
    }
    virtual void setDataCallback(std::function<void(std::vector<W_DataScan>)> callback)
    {
        DataFun = callback;
    }
    virtual QString GetOtherMessage()
    {
        return other_message;
    }

private:
    uint8_t check(const QByteArray data, size_t len);
    Message Unpack(const std::vector<uint8_t> &unkown_data, int &processed_num);
    std::vector<uint8_t> Pack(Message &msg);
    bool CalcDistance(const Message &msg);
    bool ConfigAddress(const Message &msg);
    bool ConfigParam(const Message &msg);
    void TransformSignlePoint(const uint16_t dist, const int n, const uint8_t device_id);
    uint8_t AddrToDevId(uint8_t addr);
    uint8_t AddrToDevCount(uint8_t addr);
    constexpr size_t data_pos()
    {
        return offsetof(Protocol_vx90, data_);
    }
    bool StopWork(const Message &msg)
    {
        return true;
    }
    bool HandleElseTask(const Message &msg)
    {
        return true;
    }
    uint64_t getClockTime_ms(void)
    {
        struct timespec time_data_in;
        clock_gettime(CLOCK_MONOTONIC, &time_data_in);
        return (uint64_t)(time_data_in.tv_sec * 1000 + time_data_in.tv_nsec / 1000000);
    };

    bool LaserScanSendCMD(QSerialPort *serial, int addr, uint8_t cmd);
    bool UnpackData(const uint8_t *data, size_t len);

private:
    const int kCmdDistance{0x01};
    const int kCmdAddress{0x04};
    const int kCmdParam{0x03};
    const int kCmdStop{0x02};
    const int kCmdWave{0x0f};
    const int kCmdVersion{0x06};
    const int kCmdSN{0x0d};

    const uint8_t start_bit_[2] = {0xAA, 0xAA};
    uint16_t offset_{0};
    int device_addr_;
    bool is_addr_ready_{false};
    bool is_param_ready_{false};
    bool is_SN_ready_{false};
    bool intensity_need_{false};
    uint64_t last_time;
    QString other_message;
    bool get_Wave{false};
    bool get_Version{false};

    Protocol_vx90 format_;
    std::vector<Point3f_t> pointcloud_;
    std::vector<std::vector<float>> param_;
    std::vector<uint8_t> rev_data_;
    std::vector<LineLaserDataScan> LaserDataScanData;
    bool laserdatascan_reday{false};
    uint64_t lineSensorSn{0x0};

    std::mutex _mutex_LaserData;

    std::function<void(const TYPE_LASER_, QString)> RsDevSnFun = nullptr;
    std::function<void(std::vector<W_DataScan>)> DataFun = nullptr;
};

#endif // RSLINE_H
